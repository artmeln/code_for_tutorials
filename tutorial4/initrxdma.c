/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
*<pre>
* MODIFICATION HISTORY:
*
* Modified starting from xaxidma_example_sgcyclic_intr.c Ver 9.15
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "initRxDma.h"

#include "xscugic.h"

/************************** Constant Definitions *****************************/

#define DMA_DEV_ID			XPAR_AXIDMA_0_DEVICE_ID

#define RX_INTR_ID		XPAR_FABRIC_AXIDMA_0_VEC_ID
#define XPAR_FABRIC_IATC_INTROUT 62U
#define	INT_TYPE_RISING_EDGE	0x03
#define INT_TYPE_HIGHLEVEL		0x01
#define INT_TYPE_MASK			0x03
#define	INT_CFG0_OFFSET	0x00000C00

#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID

/* Timeout loop counter for reset
 */
#define RESET_TIMEOUT_COUNTER	10000

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int SetupIntrSystem();
int SetupAxiDMA();

int ConnectIntrAxiDMA();
void DisconnectIntrAxiDMA();

int ConnectIntrIatc(u8* pOverflowStatus);

static int RxSetup(XAxiDma * AxiDmaInstancePtr);
static void RxIntrHandler(void *Callback);
static void IatcIntrHandler(void *Callback);
static void IntcTypeSetup(XScuGic *InstancePtr, int intId, int intType);
static void RxCallBack(XAxiDma_BdRing * RxRingPtr);

/************************** Variable Definitions *****************************/

volatile u32 RxDone=0;
volatile int Error=0;

/* The instances to support the device drivers are global such that the
 * are initialized to zero each time the program runs.
 */
static XScuGic IntcInstance;	/* The instance of the IRQ Controller */
static XAxiDma AxiDmaInstance;

u32 getRxDone() {
	return RxDone;
}

void resetRxDone() {
	// for reasons I don't fully understand
	// the first packet after restart is compromised
	// so it is skipped
	RxDone = -1;
}

int DMAReset() {
	XAxiDma_Reset(&AxiDmaInstance);
	int TimeOut = RESET_TIMEOUT_COUNTER;
	while (TimeOut) {
		if(XAxiDma_ResetIsDone(&AxiDmaInstance)) {
			return 0;
			break;
		}
		TimeOut -= 1;
	}
	return RESET_TIMEOUT_COUNTER;
}

int SetupIntrSystem() {

	XScuGic_Config *IntcConfig;
	XScuGic* IntcInstancePtr = &IntcInstance;
	int Status;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}
	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_ExceptionInit();
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				    (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				    IntcInstancePtr);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int SetupAxiDMA() {

    XAxiDma_Config* pConfigAxiDma;
    XAxiDma * AxiDmaInstancePtr = &AxiDmaInstance;
    int Status;

    // set up AXI_DMA
    xil_printf("Setting up AXI_DMA...");
    pConfigAxiDma = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!pConfigAxiDma) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);

		return XST_FAILURE;
	}

	/* Initialize DMA engine */
	XAxiDma_CfgInitialize(AxiDmaInstancePtr, pConfigAxiDma);

	xil_printf("Number of channels: ");
	xil_printf("%u",AxiDmaInstancePtr->RxNumChannels);
	xil_printf("\r\n");

	if(!XAxiDma_HasSg(AxiDmaInstancePtr)) {
		xil_printf("Device configured as Simple mode \r\n");
		return XST_FAILURE;
	}

	Status = RxSetup(AxiDmaInstancePtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Failed RX setup\r\n");
		return XST_FAILURE;
	}
	xil_printf("Done.\r\n");

	return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	- XST_SUCCESS if the setup is successful.
*		- XST_FAILURE if fails.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	int BdCount;
	int FreeBdCount;
	UINTPTR RxBufferPtr;
	int Index;

	RxRingPtr = XAxiDma_GetRxRing(AxiDmaInstPtr);

	/* Disable all RX interrupts before RxBD space setup */
	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
				RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE,
					RX_BD_SPACE_BASE,
					XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd create failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	/*
	 * Setup a BD template for the Rx channel. Then copy it to every RX BD.
	 */
	XAxiDma_BdClear(&BdTemplate);
	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd clone failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Attach buffers to RxBD ring so we are ready to receive packets */
	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);

	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx bd alloc failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	RxBufferPtr = RX_BUFFER_BASE;

	for (Index = 0; Index < FreeBdCount; Index++) {

		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set buffer addr %x on BD %x failed %d\r\n",
			(unsigned int)RxBufferPtr,
			(UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
					RxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (UINTPTR)BdCurPtr, Status);

			return XST_FAILURE;
		}

		/* Receive BDs do not need to set anything for the control
		 * The hardware will set the SOF/EOF bits per stream status
		 */
		XAxiDma_BdSetCtrl(BdCurPtr, 0);

		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);

		RxBufferPtr += MAX_PKT_LEN;
		BdCurPtr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	/*
	 * Set the coalescing threshold
	 *
	 * If you would like to have multiple interrupts to happen, change
	 * the COALESCING_COUNT to be a smaller value
	 */
	Status = XAxiDma_BdRingSetCoalesce(RxRingPtr, COALESCING_COUNT,
			DELAY_TIMER_COUNT);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx set coalesce failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount, BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx ToHw failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Enable all RX interrupts */
	XAxiDma_BdRingIntEnable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);
	/* Enable Cyclic DMA mode */
	XAxiDma_BdRingEnableCyclicDMA(RxRingPtr);
	XAxiDma_SelectCyclicMode(AxiDmaInstPtr, XAXIDMA_DEVICE_TO_DMA, 1);

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Rx start BD ring failed with %d\r\n", Status);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

int ConnectIntrAxiDMA()
{
    XAxiDma * AxiDmaInstancePtr = &AxiDmaInstance;
	XScuGic* IntcInstancePtr = &IntcInstance;

    XAxiDma_BdRing *RxRingPtr = XAxiDma_GetRxRing(AxiDmaInstancePtr);
	int Status;

	// set low priority to DMA and overflow interrupts
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, RX_INTR_ID, 0xA0, 0x3);

	// connect DMA interrupt handler
	Status = XScuGic_Connect(IntcInstancePtr, RX_INTR_ID,
				(Xil_InterruptHandler)RxIntrHandler,
				RxRingPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	// enable all interrupts
	XScuGic_Enable(IntcInstancePtr, RX_INTR_ID);

	return XST_SUCCESS;

}

void DisconnectIntrAxiDMA()
{
	XScuGic* IntcInstancePtr = &IntcInstance;
	/* Disconnect and disable the interrupt for DMA */
	XScuGic_Disconnect(IntcInstancePtr, RX_INTR_ID);
}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* presents, then it calls the callback function.
*
* @param	Callback is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxIntrHandler(void *Callback)
{
	XAxiDma_BdRing *RxRingPtr = (XAxiDma_BdRing *) Callback;
	u32 IrqStatus;
	int TimeOut;

	/* Read pending interrupts */
	IrqStatus = XAxiDma_BdRingGetIrq(RxRingPtr);

	/* Acknowledge pending interrupts */
	XAxiDma_BdRingAckIrq(RxRingPtr, IrqStatus);

	/*
	 * If no interrupt is asserted, we do not do anything
	 */
	if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
		return;
	}

	/*
	 * If error interrupt is asserted, raise error flag, reset the
	 * hardware to recover from the error, and return with no further
	 * processing.
	 */
	if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {

		XAxiDma_BdRingDumpRegs(RxRingPtr);

		Error = 1;


		/* Reset could fail and hang
		 * NEED a way to handle this or do not call it??
		 */
		XAxiDma_Reset(&AxiDmaInstance);

		TimeOut = RESET_TIMEOUT_COUNTER;

		while (TimeOut) {
			if(XAxiDma_ResetIsDone(&AxiDmaInstance)) {
				break;
			}

			TimeOut -= 1;
		}

		return;
	}

	/*
	 * If completion interrupt is asserted, call RX call back function
	 * to handle the processed BDs and then raise the according flag.
	 */
	if ((IrqStatus & (XAXIDMA_IRQ_DELAY_MASK | XAXIDMA_IRQ_IOC_MASK))) {
		RxCallBack(RxRingPtr);
	}
}

/*****************************************************************************/
/*
*
* This is the DMA RX callback function called by the RX interrupt handler.
* This function handles finished BDs by hardware, attaches new buffers to those
* BDs, and give them back to hardware to receive more incoming packets
*
* @param	RxRingPtr is a pointer to RX channel of the DMA engine.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void RxCallBack(XAxiDma_BdRing * RxRingPtr)
{
	int BdCount=0;
	XAxiDma_Bd* BdPtr;

	// Get finished BDs from hardware
	BdCount = XAxiDma_BdRingFromHw(RxRingPtr, XAXIDMA_ALL_BDS, &BdPtr);

	RxDone += BdCount;
}


int ConnectIntrIatc(u8* pOverflowStatus)
{
	int Status;
	XScuGic* IntcInstancePtr = &IntcInstance;

	// set low priority to overflow interrupts
	XScuGic_SetPriorityTriggerType(IntcInstancePtr, XPAR_FABRIC_IATC_INTROUT, 0xA0, 0x3);

	// connect iatc interrupt handler
	Status = XScuGic_Connect(IntcInstancePtr, XPAR_FABRIC_IATC_INTROUT,
				(Xil_InterruptHandler)IatcIntrHandler, pOverflowStatus);
	if (Status != XST_SUCCESS) {
		return Status;
	}
	// iatc interrupt happens on rising edge
	IntcTypeSetup(IntcInstancePtr, XPAR_FABRIC_IATC_INTROUT, INT_TYPE_RISING_EDGE);

	// enable all interrupts
	XScuGic_Enable(IntcInstancePtr, XPAR_FABRIC_IATC_INTROUT);

	return XST_SUCCESS;
}

static void IatcIntrHandler(void *Callback) {
	*(u8*)Callback = 1;
}

// is this function necessary?
static void IntcTypeSetup(XScuGic *InstancePtr, int intId, int intType)
{
	int mask;

	intType &= INT_TYPE_MASK;
	mask = XScuGic_DistReadReg(InstancePtr, INT_CFG0_OFFSET + (intId/16)*4);
	mask &= ~(INT_TYPE_MASK << (intId%16)*2);
	mask |= intType << ((intId%16)*2);
	XScuGic_DistWriteReg(InstancePtr, INT_CFG0_OFFSET + (intId/16)*4, mask);
}
