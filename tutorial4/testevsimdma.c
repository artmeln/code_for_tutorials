#include "testEvsimDMA.h"
#include "xil_printf.h"
#include <stdbool.h>

// 16-bit counter
const u16 MASK_TIME = 0x3FFF;
const u16 MASK_CH1 = 0x4000;
const u16 MASK_CH2 = 0x8000;
// 32-bit counter
//const u32 MASK_TIME = 0x3FFFFFFF;
//const u32 MASK_CH1 = 0x40000000;
//const u32 MASK_CH2 = 0x80000000;

u32 eventNumber = 0; // counts channel 1 and channel 2 events only, not rollovers
u32 expectedNextValue = 0;
u64 cumulativeTime = 0;

int testEvsimData(u8* buffer, size_t bufferSize, u32 maxEvsimValue, u32 maxIatcValue, u32 numReportedErrors) {

	unsigned long errorCount = 0;
	bool reportError = true;

	// 16-bit
	u16 time;
	u16 ch1;
	u16 ch2;
	u16* ptr = (u16*)buffer;

	// 32-bit
	//u32 time;
	//u32 ch1;
	//u32 ch2;
	//u32* ptr = (u32*)buffer;

	for (size_t ii = 0; ii < bufferSize / sizeof(MASK_TIME); ii++) {

		// extract time and channel information from the data
		time = *ptr & MASK_TIME;
		ch1 = *ptr & MASK_CH1;
		ch2 = *ptr & MASK_CH2;

		cumulativeTime += time;

		if (ch1 == 0 && ch2 == 0) { // rollover event
			if (time!=maxIatcValue) {
				errorCount++;
				if (reportError) {
					xil_printf("Processing error: premature counter rollover %u at %u\r\n", time, eventNumber);
				}
			}
		}
		else if (ch1 != 0 && ch2 != 0) { // dual channel event
			eventNumber++;
			errorCount++;
			if (reportError) {
				xil_printf("Processing error: dual channel event %u at %u\r\n", cumulativeTime, eventNumber);
			}
			cumulativeTime=0;
		}
		else if (ch2 != 0) { // channel 2 event
			eventNumber++;
			if (eventNumber == 1) {} // ignore event1
			else if (eventNumber == 2) { // don't check event2 but use to calculate the next value
				if (cumulativeTime < maxEvsimValue) {
					expectedNextValue = cumulativeTime + 1; // next value should be larger by 1
				}
				else {
					expectedNextValue = 2;
				}
			}
			else {
				if (cumulativeTime != expectedNextValue) {
					if (reportError) {
						xil_printf("Processing error: mismatch; expected %u but found %u\r\n", expectedNextValue, cumulativeTime);
					}
					errorCount++;
				}
				if (cumulativeTime < maxEvsimValue) {
					expectedNextValue = cumulativeTime + 1;
				}
				else {
					expectedNextValue = 2;
				}
			}
			cumulativeTime = 0;
		}
		else if (ch1 != 0) { // channel 1 event
			eventNumber++;
			errorCount++;
			if (reportError) {
				xil_printf("Processing error: detected ch1 event %u at %u\r\n", cumulativeTime, eventNumber);
			}
			cumulativeTime = 0;
		}

		if ( reportError && (errorCount==numReportedErrors) ) {
			xil_printf("Additional errors for this block will be suppressed.\r\n");
			reportError = false;
		}
		ptr++;
	}
	return errorCount;
}
