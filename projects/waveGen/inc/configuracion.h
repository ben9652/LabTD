#ifndef CPU
#error CPU shall be defined
#endif
#if (lpc4337 == CPU)
#include "chip.h"
#elif (mk60fx512vlq15 == CPU)
#else
#endif


void ConfigurarDAC(LPC_DAC_T* pDAC);

void ConfigurarPuertosLaboratorio(void);
