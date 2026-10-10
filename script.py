#!/usr/bin/env python3
"""
Intel HEX -> PIC32 ICSP wire-protocol sender.

Matches src/uart_dma_function_manager.c / .h exactly -- see
dsl/uart_dma_function_manager.mkd for the state-machine this feeds.

Wire format, one continuous byte stream, little-endian throughout:

    per block:
        word0: bit31    = 1 "more data follows" / 0 "end of file"
               bits30-16 = upcoming_data_count, in BYTES (15 bits, <= 0x7FFF)
               bits15-8  = header CRC-8 (covers count+flag bytes + address,
                           NOT the two CRC bytes themselves)
               bits7-0   = data CRC-8 (covers the payload bytes that follow)
        word1: target address
        then upcoming_data_count bytes of payload

    terminated by one final header with the "more data" bit clear
    (address/byte_count don't matter for that header, but it still needs a
    correct header CRC).

CRC-8: poly 0x07, init 0x00, MSB-first, no reflection, no final XOR --
same bit-for-bit algorithm as Crc8Update() in uart_dma_function_manager.c.

Flow control: UART5 RX is received by DMA into two 128-byte ping-pong
buffers. The firmware can only have 2 buffers' worth of unconsumed data in
flight before a third buffer would overwrite one still being read, so the
PC gets 256 bytes of free credit up front, then must wait for one ACK byte
(0x06) per additional 128-byte chunk it puts on the wire -- headers and
payload share this same raw byte stream, the firmware doesn't care about
block boundaries when counting. The stream is padded to a multiple of 128
bytes at the end so the final DMA buffer actually completes (NDTR hits
zero) and the firmware can read the trailing EOF header out of it.
"""

import argparse
import struct
import time

BLOCK_SIZE_BYTES = 128        # chunking default; matches UART_DMA_BUFFER_BYTES
                               # for a tidy 1:1 block/flow-control mapping, but
                               # the protocol doesn't require that alignment
KSEG1_BASE       = 0xA0000000
UART_DMA_BUFFER_BYTES = 128   # must match uart_dma_function_manager.c
UART_DMA_ACK_BYTE     = 0x06
CRC8_POLY  = 0x07
CRC16_POLY = 0x1021
CRC16_INIT = 0xFFFF
MAX_BYTE_COUNT = 0x7FFF        # 15-bit field


def parse_ihex(path):
    """Parse an Intel HEX file into a flat {address: byte} map."""
    memory = {}
    upper = 0
    with open(path, "r") as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if line[0] != ":":
                raise ValueError(f"line {lineno}: doesn't start with ':'")

            byte_count = int(line[1:3], 16)
            addr16     = int(line[3:7], 16)
            rectype    = int(line[7:9], 16)
            data_field = line[9:9 + byte_count * 2]
            data       = bytes.fromhex(data_field)

            if rectype == 0x00:                        # data record
                base = upper | addr16
                for i, b in enumerate(data):
                    memory[base + i] = b
            elif rectype == 0x04:                       # ext. linear address
                upper = int.from_bytes(data, "big") << 16
            elif rectype == 0x01:                        # EOF
                break
            elif rectype in (0x02, 0x03):
                # extended segment addr / start segment addr — not used by
                # PIC32 toolchains; ignore rather than guess at meaning
                pass
            else:
                raise ValueError(f"line {lineno}: unhandled record type {rectype:#x}")
    return memory


def to_contiguous_runs(memory):
    """Turn a sparse {address: byte} map into [(start_addr, bytearray), ...]."""
    if not memory:
        return []
    addrs = sorted(memory.keys())
    runs = []
    run_start = addrs[0]
    run_bytes = bytearray()
    prev = None

    for a in addrs:
        if prev is not None and a != prev + 1:
            runs.append((run_start, run_bytes))
            run_start = a
            run_bytes = bytearray()
        run_bytes.append(memory[a])
        prev = a
    runs.append((run_start, run_bytes))
    return runs


def to_blocks(runs, block_size=BLOCK_SIZE_BYTES):
    """Split each contiguous run into block_size-byte chunks.

    Only the final chunk of a run may be short; it's padded up to the next
    multiple of 4 bytes (never further) with 0xFF, since PIC32_WordProgram
    operates word-at-a-time and the firmware requires byte_count % 4 == 0.
    """
    if block_size % 4 != 0 or block_size <= 0 or block_size > MAX_BYTE_COUNT:
        raise ValueError(f"block_size must be a positive multiple of 4, <= {MAX_BYTE_COUNT}")

    blocks = []
    for start, data in runs:
        offset = 0
        while offset < len(data):
            piece = bytes(data[offset:offset + block_size])
            if len(piece) % 4 != 0:
                piece += b"\xFF" * (4 - (len(piece) % 4))
            blocks.append((start + offset, piece))
            offset += block_size
    return blocks


def rebase_kseg1(blocks):
    return [(addr | KSEG1_BASE, data) for addr, data in blocks]


def crc8(data, crc=0):
    for byte_in in data:
        crc ^= byte_in
        for _ in range(8):
            if crc & 0x80:
                crc = ((crc << 1) ^ CRC8_POLY) & 0xFF
            else:
                crc = (crc << 1) & 0xFF
    return crc


def crc16_update(crc, byte_in):
    crc = (crc ^ (byte_in << 8)) & 0xFFFF
    for _ in range(8):
        if crc & 0x8000:
            crc = ((crc << 1) ^ CRC16_POLY) & 0xFFFF
        else:
            crc = (crc << 1) & 0xFFFF
    return crc


def build_header(address, byte_count, data_crc, more_data):
    if byte_count > MAX_BYTE_COUNT:
        raise ValueError(f"byte_count {byte_count} exceeds 15-bit field (max {MAX_BYTE_COUNT})")

    count_low = byte_count & 0xFF
    count_high_flag = ((byte_count >> 8) & 0x7F) | (0x80 if more_data else 0x00)
    addr_bytes = struct.pack("<I", address)

    header_crc = crc8(bytes([count_low, count_high_flag]) + addr_bytes)

    return bytes([data_crc & 0xFF, header_crc, count_low, count_high_flag]) + addr_bytes


def build_stream(blocks):
    """Serialize all blocks + the terminating EOF header into one byte
    stream, padded to a multiple of UART_DMA_BUFFER_BYTES so the firmware's
    DMA double-buffer always completes its final fill."""
    stream = bytearray()
    running_crc16 = CRC16_INIT
    first_address = blocks[0][0] if blocks else 0
    total_bytes = 0

    for address, data in blocks:
        data_crc = crc8(data)
        stream += build_header(address, len(data), data_crc, more_data=True)
        stream += data
        total_bytes += len(data)
        for b in data:
            running_crc16 = crc16_update(running_crc16, b)

    stream += build_header(0, 0, 0, more_data=False)  # EOF header

    pad = (-len(stream)) % UART_DMA_BUFFER_BYTES
    stream += bytes(pad)

    summary = {"start_address": first_address, "total_bytes": total_bytes, "crc16": running_crc16}
    return bytes(stream), summary


def _drain_pending_text(ser, quiet_s=0.3, max_wait_s=5.0):
    """Read and print whatever's already sitting in the RX buffer before the
    binary ACK handshake starts.

    flash_engine.c's UART5_SendString() progress messages ("Entered Enhanced
    ICSP Mode", "Device is ready", "Erasing Completed", ...) share this same
    UART5 line ahead of UART_DMA_RunTransfer()'s block protocol -- if left
    unread, send_stream()'s first ser.read(1) waiting for a real 0x06 ACK
    would read a stray text byte instead and fail immediately. Waits for a
    quiet gap (no byte for quiet_s) rather than a fixed delay so it isn't
    tuned to any one state's timing, capped at max_wait_s so a firmware
    that's unexpectedly chatty (or stuck) can't hang this forever.
    """
    pending = bytearray()
    ser.timeout = quiet_s
    deadline = time.time() + max_wait_s
    while time.time() < deadline:
        byte = ser.read(1)
        if not byte:
            break
        pending += byte
    if pending:
        text = pending.decode("ascii", errors="replace").strip()
        if text:
            print(f"[STM32] {text}")


def send_stream(ser, stream, timeout_s=5.0):
    """Send stream honoring the credit-of-2 UART5 RX DMA flow control."""
    _drain_pending_text(ser)
    ser.timeout = timeout_s
    credit = 2  # buffers of free credit before the first ACK is required
    pos = 0
    total = len(stream)

    while pos < total:
        chunk = stream[pos:pos + UART_DMA_BUFFER_BYTES]
        if credit == 0:
            ack = ser.read(1)
            if ack != bytes([UART_DMA_ACK_BYTE]):
                raise IOError(
                    f"expected ACK (0x{UART_DMA_ACK_BYTE:02X}) at stream offset {pos}, "
                    f"got {ack!r} (timeout or protocol mismatch)"
                )
            credit += 1
        ser.write(chunk)
        credit -= 1
        pos += len(chunk)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("hexfile")
    ap.add_argument("--port", help="Serial port, e.g. /dev/ttyACM0 or COM5")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--pe", action="store_true",
                     help="Rebase addresses into KSEG1 (0xA0000000) — PE image only")
    ap.add_argument("--block-size", type=int, default=BLOCK_SIZE_BYTES,
                     help=f"bytes per block header (default {BLOCK_SIZE_BYTES}, must be a "
                          f"multiple of 4, <= {MAX_BYTE_COUNT})")
    ap.add_argument("--timeout", type=float, default=5.0,
                     help="seconds to wait for each flow-control ACK (default 5.0)")
    ap.add_argument("--dry-run", action="store_true",
                     help="Parse and print the chunk plan, don't open a serial port")
    args = ap.parse_args()

    memory = parse_ihex(args.hexfile)
    runs = to_contiguous_runs(memory)
    blocks = to_blocks(runs, args.block_size)
    if args.pe:
        blocks = rebase_kseg1(blocks)

    stream, summary = build_stream(blocks)

    print(f"{len(memory)} bytes parsed -> {len(runs)} contiguous run(s) -> "
          f"{len(blocks)} block(s) ({summary['total_bytes']} payload bytes, "
          f"{len(stream)} bytes on the wire incl. headers+padding, "
          f"{'KSEG1-rebased' if args.pe else 'unmodified'} addresses)")

    for start, data in runs:
        print(f"  run: start=0x{start:08X} length={len(data)} bytes "
              f"(ends at 0x{start+len(data)-1:08X})")

    print(f"  expected summary (Section 15 Verify Device): start=0x{summary['start_address']:08X} "
          f"total_bytes={summary['total_bytes']} crc16=0x{summary['crc16']:04X}")

    if args.dry_run or not args.port:
        print()
        for addr, data in blocks:
            print(f"  addr=0x{addr:08X}  byte_count={len(data)}")
        if not args.port:
            print("\n(no --port given, so nothing was sent)")
        return

    import serial
    with serial.Serial(args.port, args.baud) as ser:
        send_stream(ser, stream, timeout_s=args.timeout)
    print("done.")


if __name__ == "__main__":
    main()
