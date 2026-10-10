#!/usr/bin/env bash
# Orchestrates one full PIC32 flashing cycle:
#   1. Build + flash the STM32F446RE over SWD (make flash -> openocd), which
#      resets the board into a fresh run of FlashEngine_Run() (see
#      src/flash_engine.c / dsl/flash_engine.mkd) -- Enter ICSP, Check
#      Status, Erase, Enter Serial Exec, then sit waiting on UART5 for the PE
#      image, then the firmware image.
#   2. Send the PE/RIPE hex over UART5 (script.py --pe), matching
#      FLASH_STATE_DOWNLOAD_PE.
#   3. Send the actual firmware hex (script.py, no --pe), matching
#      FLASH_STATE_DOWNLOAD_FIRMWARE -> VERIFY_DEVICE -> EXIT_PROGRAMMING_MODE.
#
# Each step only runs if the previous one succeeded (set -e), since sending
# the firmware image into a PE that never loaded, or a device that's not in
# Serial Exec mode, would just hang waiting on an ACK that never comes.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

RIPE_HEX="$SCRIPT_DIR/RIPE_11_000301.hex"
PORT="/dev/ttyUSB0"
BAUD=115200
SKIP_STM32_FLASH=0
POST_RESET_DELAY=2 # seconds -- lets SystemClock_Config/UART5/DMA init finish before the PC starts sending

usage() {
    cat <<EOF
Usage: $(basename "$0") [options] <firmware.hex>

Flashes the STM32F446RE (make flash), then streams the PE/RIPE image and
then the firmware image to the PIC32 target over UART5.

  <firmware.hex>          The actual application hex to program into the
                           PIC32's flash (required).

Options:
  --ripe <path>            PE/RIPE hex to load first (default: $RIPE_HEX)
  --port <device>          Serial port (default: $PORT)
  --baud <rate>             Baud rate (default: $BAUD)
  --skip-stm32-flash       Don't rebuild/reflash the STM32 -- use this only
                           if you just power-cycled or reset it yourself.
  -h, --help               Show this help
EOF
    exit 1
}

FIRMWARE_HEX=""
while [ $# -gt 0 ]; do
    case "$1" in
        --ripe) RIPE_HEX="$2"; shift 2 ;;
        --port) PORT="$2"; shift 2 ;;
        --baud) BAUD="$2"; shift 2 ;;
        --skip-stm32-flash) SKIP_STM32_FLASH=1; shift ;;
        -h|--help) usage ;;
        -*) echo "Unknown option: $1" >&2; usage ;;
        *)
            if [ -n "$FIRMWARE_HEX" ]; then
                echo "Unexpected extra argument: $1" >&2
                usage
            fi
            FIRMWARE_HEX="$1"
            shift
            ;;
    esac
done

if [ -z "$FIRMWARE_HEX" ]; then
    echo "Error: missing <firmware.hex>" >&2
    usage
fi
if [ ! -f "$FIRMWARE_HEX" ]; then
    echo "Error: firmware hex not found: $FIRMWARE_HEX" >&2
    exit 1
fi
if [ ! -f "$RIPE_HEX" ]; then
    echo "Error: RIPE/PE hex not found: $RIPE_HEX" >&2
    exit 1
fi
if [ ! -c "$PORT" ]; then
    echo "Error: serial port not found: $PORT" >&2
    exit 1
fi

echo "== Step 1/3: build + flash STM32F446RE =="
if [ "$SKIP_STM32_FLASH" -eq 1 ]; then
    echo "  (skipped by --skip-stm32-flash -- assuming the board was just reset)"
else
    make flash
    echo "  waiting ${POST_RESET_DELAY}s for the board to finish boot-time init..."
    sleep "$POST_RESET_DELAY"
fi

echo
echo "== Step 2/3: download PE ($RIPE_HEX, KSEG1-rebased) =="
python3 "$SCRIPT_DIR/script.py" "$RIPE_HEX" --pe --port "$PORT" --baud "$BAUD"

echo
echo "  waiting 1s for \"PE Loaded\" / the DOWNLOAD_FIRMWARE transition to settle..."
sleep 1

echo
echo "== Step 3/3: download firmware ($FIRMWARE_HEX) =="
python3 "$SCRIPT_DIR/script.py" "$FIRMWARE_HEX" --port "$PORT" --baud "$BAUD"

echo
echo "Done. Watch the STM32's UART5 output (or a terminal on $PORT) for"
echo "\"Verify Passed\" / \"Programming Complete\" to confirm Section 15/16 succeeded."
