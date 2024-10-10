#include "xaxidma.h"

#define PROCESSING_SIZE 256*512 // 128 kB

#define BD_SIZE	4*16
#define BUFFER_CAPACITY 10 // as measured in PROCESSING_SIZE chunks

#define MAX_PKT_LEN		256 // when TLAST is generated but in bytes

#define MEM_BASE_ADDR		0x01000000
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR)
#define RX_BD_SPACE_HIGH	(RX_BD_SPACE_BASE + ((PROCESSING_SIZE*BUFFER_CAPACITY)/MAX_PKT_LEN)*BD_SIZE - 1)
#define RX_BUFFER_BASE		(RX_BD_SPACE_HIGH + 1)
#define RX_BUFFER_HIGH		(RX_BUFFER_BASE + PROCESSING_SIZE*BUFFER_CAPACITY - 1)

/*
 * Number of BDs in the transfer example
 * We show how to submit multiple BDs for one transmit.
 * The receive side get one completion interrupt per cyclic transfer.
 */
#define NUMBER_OF_BDS_PER_PKT		1

/* The interrupt coalescing threshold and delay timer threshold
 * Valid range is 1 to 255
 *
 * We set the coalescing threshold to be the total number of packets.
 * The receive side will only get one completion interrupt per cyclic transfer.
 */
#define COALESCING_COUNT		255
#define DELAY_TIMER_COUNT		0

int SetupIntrSystem();

int SetupAxiDMA();
int ConnectIntrAxiDMA();
void DisconnectIntrAxiDMA();

int ConnectIntrIatc(u8* Callback);

u32 getRxDone();
void resetRxDone();
int DMAReset();

