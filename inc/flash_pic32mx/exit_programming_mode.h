#ifndef EXIT_PROGRAMMING_MODE_H
#define EXIT_PROGRAMMING_MODE_H

/* Section 16.0 "Exiting Programming Mode", PIC32 Flash Programming
 * Specification. Programming Step 11 (Figure 3-1): the newly programmed
 * data doesn't take effect until either power is cycled or this exit
 * sequence is performed. This project only implements Section 16.2
 * (2-wire) — Section 16.1 (4-wire) is JTAG-only and doesn't apply here.
 *
 * Section 16.2's numbered steps order "Assert the MCLR pin" before "Issue a
 * clock pulse on PGECx", while Figure 16-2's waveform appears to show the
 * final PGECx pulse happening while MCLR is still at VIH, with MCLR falling
 * only afterwards. This follows the numbered steps literally (matching how
 * this project has resolved similar figure/text mismatches elsewhere,
 * e.g. check_device_status.c) rather than reordering them to match the
 * figure. */
void ExitProgrammingMode(void);

#endif /* EXIT_PROGRAMMING_MODE_H */
