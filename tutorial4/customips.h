#include "xparameters.h"

#define DISABLE_IATC 0x00000000
#define ENABLE_IATC 0x00000001
#define DISABLE_EVSIM 0x00000000
#define ENABLE_EVSIM 0x00000001

void disableIatc();
void enableIatc();
void setMaxcountIatc(int maxIatc);
void disableEvsim();
void enableEvsim();
void setMaxcountEvsim(int maxEvsim);
