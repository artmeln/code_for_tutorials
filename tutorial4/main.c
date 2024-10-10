//#include "xtime_l.h"
#include <stdbool.h>

#include "initRxDma.h"
#include "customIPs.h"
#include "testEvsimDMA.h"

// where processed data is stored
#define STORE_BUFFER_BASE (RX_BUFFER_HIGH+1)
#define STORE_BUFFER_SIZE (PROCESSING_SIZE*20)


u8* buffData = (u8*)RX_BUFFER_BASE;
u32 RxProcessed=0; // number of packets processed by this application so far
u32 availableDataSize = 0; // size of data available for processing

u8 iatcOverflow=0;
u8 processingOverflow=0;

u8* storeBuffer = (u8*)STORE_BUFFER_BASE;
u32 processedDataSize = 0; // size of data processed so far
u32 toProcessDataSize = 0;

u32 max_iatc = 1500; // do not exceed 2^14-1 = 16383
u32 max_evsim = 16383;  // do not exceed 2^14-1 = 16383


int main(void)
{
	int Status;
	bool isProcessing = true; // a flag indicating that data should be processed

	// initial state of the counter and the event simulator
	disableIatc();
	setMaxcountIatc(max_iatc);
	disableEvsim();
	setMaxcountEvsim(max_evsim);

	// enable the event simulator
	enableEvsim();

	Status = SetupIntrSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to setup the interrupt system, return value = %i\r\n", Status);
		return Status;
	}

	Status = SetupAxiDMA();
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to setup AxiDMA, return value = %i\r\n", Status);
		return Status;
	}

	Status = ConnectIntrAxiDMA();
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to connect AxiDMA interrupts, return value = %i\r\n", Status);
		return Status;
	}

	Status = ConnectIntrIatc(&iatcOverflow);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed to connect iatc interrupts, return value = %i\r\n", Status);
		return Status;
	}

	// enable the counter
	enableIatc();

	while (1) {

		// figure out how much data is available for processing
		availableDataSize = (getRxDone()-RxProcessed)*MAX_PKT_LEN;

		// check for processing buffer overflow
		if ( availableDataSize >= PROCESSING_SIZE*BUFFER_CAPACITY/2 ) {
			processingOverflow = 1;
		}

		if (isProcessing && availableDataSize>=PROCESSING_SIZE) { // is enough data available?
   			// figure out how much contiguous data is available for processing
			if ( (u8*)RX_BUFFER_HIGH-buffData+1 < PROCESSING_SIZE) {
   				toProcessDataSize = (u8*)RX_BUFFER_HIGH-buffData+1;
   			} else {
   				toProcessDataSize = PROCESSING_SIZE;
   			}
			// check that all that data can be processed
   			if ( toProcessDataSize > (STORE_BUFFER_SIZE-((u32)storeBuffer-STORE_BUFFER_BASE)) ) {
   				toProcessDataSize = STORE_BUFFER_SIZE-((u32)storeBuffer-STORE_BUFFER_BASE);
   			}

   			// process
   			xil_printf("Copying %i bytes... ", toProcessDataSize);
   			memcpy(storeBuffer,buffData,toProcessDataSize);
   			xil_printf("Done.\r\n");

   			// flush the processed data
   			Xil_DCacheFlushRange((INTPTR)buffData, toProcessDataSize);

   			// advance pointers etc
   			storeBuffer += toProcessDataSize;
   			processedDataSize += toProcessDataSize;
   			RxProcessed += toProcessDataSize / MAX_PKT_LEN;
			buffData += toProcessDataSize;
			if (buffData>(u8*)RX_BUFFER_HIGH) {
				buffData = (u8*)RX_BUFFER_BASE;
			}

			// terminate if there is no more space left for storage
   			if ( (u8*)STORE_BUFFER_SIZE <= (storeBuffer-STORE_BUFFER_BASE) ) {
   	    		xil_printf("Terminating on reaching storage capacity.\r\n", Status);
   	    		isProcessing = false;
   	    		// comment out the next line to observe processing buffer overflow
   				break;
   			}

   		}

   		// check for overflows
    	if (iatcOverflow!=0) {
    		xil_printf("Error: iatc fifo overflow. Stopping the execution.\r\n", Status);
    		break;
    	}
    	if (processingOverflow!=0) {
    		xil_printf("Error: processing buffer overflow. Stopping the execution.\r\n", Status);
    		break;
    	}
	}

	disableIatc();
	disableEvsim();

	// check data for accuracy
	xil_printf("Starting testing...\r\n");
	u32 totalEventCount = STORE_BUFFER_SIZE/2;
	u32 totalErrorCount;
	totalErrorCount = testEvsimData((u8*)STORE_BUFFER_BASE, STORE_BUFFER_SIZE, max_evsim, max_iatc, 10);
	xil_printf("Finished testing!\r\n");
	xil_printf("Total event count = %u\r\n", totalEventCount);
	xil_printf("Total error count = %u\r\n", totalErrorCount);

	xil_printf("Done.\r\n");

	return XST_SUCCESS;
}

