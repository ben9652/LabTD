#include "stdlib.h"
#include "stdint.h"
#include "configuracion.h"
#include "myLib.h"
#include "led.h"
#include "switch.h"
#include <string.h>
#include <math.h>

#define N_MUESTRAS 256
#define PI 3.141592654

uint16_t valores_timer[10] = {127,255,511,1023,2047,4095,8191,16383,32767,65535};

uint16_t x = 0;

void DAC_IRQHandler(void)
{
    if(x < N_MUESTRAS)
        x++;
    else
        x = 0;

    if(x == 1/4.0*N_MUESTRAS)
        Led_On(YELLOW_LED);
    else
        Led_Off(YELLOW_LED);

    if(x == 2/4.0*N_MUESTRAS)
        Led_On(RED_LED);
    else
        Led_Off(RED_LED);

    if(x == 3/4.0*N_MUESTRAS)
        Led_On(GREEN_LED);
    else
        Led_Off(GREEN_LED);

    if(x != 1/4.0*N_MUESTRAS)
        Chip_DAC_UpdateValue(LPC_DAC, (sin(x*2*PI/N_MUESTRAS)+1)*512);
}

int main(void)
{
    uint8_t timer = 0;
    uint8_t tecla;
    uint8_t tecla_anterior;

    ConfigurarPuertosLaboratorio();
    ConfigurarDAC(LPC_DAC);

    // while(1)
    // {
    //     tecla = Read_Switches();

    //     if(tecla != tecla_anterior)
    //     {
    //         switch(tecla)
    //         {
    //             case TEC1:
    //                 if(timer == 9)
    //                     timer = 0;
    //                 else
    //                     timer++;
    //                 break;
    //             case TEC2:
    //                 if(timer == 0)
    //                     timer = 9;
    //                 else
    //                     timer--;
    //                 break;
    //         }
    //         Chip_DAC_SetDMATimeOut(LPC_DAC, valores_timer[timer]);
    //     }

    //     tecla_anterior = tecla;
    // }
}