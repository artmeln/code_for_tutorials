#include "customIPs.h"
#include "eventSimSmart.h"
#include "iatcollector2chSmart.h"

// disable the counter
void disableIatc() {
	IATCOLLECTOR2CHSMART_mWriteReg(XPAR_IATCOLLECTOR2CHSMART_0_BASEADDR,
								   IATCOLLECTOR2CHSMART_S00_AXI_SLV_REG0_OFFSET,
								   DISABLE_IATC);
}

// enable the counter
void enableIatc() {
	IATCOLLECTOR2CHSMART_mWriteReg(XPAR_IATCOLLECTOR2CHSMART_0_BASEADDR,
								   IATCOLLECTOR2CHSMART_S00_AXI_SLV_REG0_OFFSET,
								   ENABLE_IATC);
}

// write the max iatc count
void setMaxcountIatc(int maxIatc) {
	IATCOLLECTOR2CHSMART_mWriteReg(XPAR_IATCOLLECTOR2CHSMART_0_BASEADDR,
								   IATCOLLECTOR2CHSMART_S00_AXI_SLV_REG1_OFFSET,
								   maxIatc);
}

// disable the simulator
void disableEvsim() {
	EVENTSIMSMART_mWriteReg(XPAR_EVENTSIMSMART_0_BASEADDR,
							EVENTSIMSMART_S00_AXI_SLV_REG0_OFFSET,
							DISABLE_EVSIM);
}

// enable the simulator
void enableEvsim() {
	EVENTSIMSMART_mWriteReg(XPAR_EVENTSIMSMART_0_BASEADDR,
							EVENTSIMSMART_S00_AXI_SLV_REG0_OFFSET,
							ENABLE_EVSIM);
}

// write the max simulator count
void setMaxcountEvsim(int maxEvsim) {
	EVENTSIMSMART_mWriteReg(XPAR_EVENTSIMSMART_0_BASEADDR,
							EVENTSIMSMART_S00_AXI_SLV_REG1_OFFSET,
							maxEvsim);
}
