#include "stdlib.h"
#include "stdint.h"
#include "configuracion.h"
#include "myLib.h"
#include "led.h"
#include <string.h>

#define USB_UART LPC_USART2
#define OVERRUN "OVERRUN ERROR\r\n"
#define PARITY "PARITY ERROR\r\n"
#define FRAMING "FRAMING ERROR\r\n"
#define RXFIFO "ERROR IN RX FIFO\r\n"
#define TXERR "ERROR IN TRANSMITTED CHARACTER\r\n"

static uint8_t actualizar = 0;
uint8_t led_encendido;

void SysTick_Handler(void)
{
   static int contador = 0;

   contador++;
   if (contador%500 == 0) {
      Led_Toggle(YELLOW_LED);
   }
   if(contador %1000 == 0) {
       contador = 0;
       actualizar = 1;
   }
}

void TIMER0_IRQHandler(void)
{
    LPC_TIMER0->IR = LPC_TIMER0->IR;
    LPC_TIMER0->TCR = TIMER_RESET;

    if(led_encendido & (1 << RGB_R_LED))
        Led_Off(RGB_R_LED);
    if(led_encendido & (1 << RGB_G_LED))
        Led_Off(RGB_G_LED);
    if(led_encendido & (1 << RGB_B_LED))
        Led_Off(RGB_B_LED);
    if(led_encendido & (1 << RED_LED))
        Led_Off(RED_LED);
    if(led_encendido & (1 << GREEN_LED))
        Led_Off(GREEN_LED);
}

void ConfigurarUART(LPC_USART_T *pUART)
{
    Chip_UART_Init(pUART);
    pUART->LCR = UART_LCR_DLAB_EN | UART_LCR_PARITY_EVEN | UART_LCR_PARITY_EN | UART_LCR_SBS_1BIT | UART_LCR_WLEN8;

    // Configuro mi divisor principal, que será 1328 = 0x530
    pUART->DLM = 0x05;
    pUART->DLL = 0x30;

    // Habilito la transmisión de datos
    pUART->TER2 = 0x01;

    // Deshabilito la configuración de divisores, por lo que dejo que se empiece a recibir y transmitir.
    pUART->LCR &= ~UART_LCR_DLAB_EN;
}

uint8_t UARTDisponible(LPC_USART_T *pUART)
{
    if(pUART->LSR & UART_LSR_TEMT) return 1;
    else return 0;
}

uint8_t UARTLeerByte(LPC_USART_T *pUART, uint8_t* data, uint8_t* error)
{
    uint8_t hayDatoRecibido = 0;
    if(pUART->LSR & UART_LSR_RDR)
    {
        hayDatoRecibido = 1;
        if(pUART->LSR & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_TXFE))
            *error = pUART->LSR & 0x19E;
        else
            *data = pUART->RBR & UART_RBR_MASKBIT;
    }

    return hayDatoRecibido;
}

int main(void)
{
    uint8_t readData = 0;
    uint8_t readError = 0;
    uint8_t* buffer = (uint8_t*)malloc(33);    // Para almacenar caracteres
    *buffer = 0;
    uint8_t contador = 0;

    ConfigurarPuertosLaboratorio();
    ConfigurarInterrupciones();
    ConfigurarUART(USB_UART);

    while (1)
    {
        led_encendido = LeerLEDs();

        while(arrayLength(buffer))
        {
            if(UARTDisponible(USB_UART))
            {
                uint8_t caracter = *buffer;
                buffer++;
                
                USB_UART->THR = caracter;
            }
        }
        
        if(UARTLeerByte(USB_UART, &readData, &readError))
        {
            if(readError)
            {
                if(LPC_TIMER0->MR[0] != 100)
                    LPC_TIMER0->MR[0] = 100;

                strcpy(buffer, "");

                if(readError & UART_LSR_OE)
                {
                    Led_On(RED_LED);
                    strcat(buffer, OVERRUN);
                }
                if(readError & UART_LSR_PE)
                {
                    Led_On(GREEN_LED);
                    strcat(buffer, PARITY);
                }
                if(readError & UART_LSR_FE)
                {
                    Led_On(RED_LED);
                    Led_On(GREEN_LED);
                    strcat(buffer, FRAMING);
                }
                if(readError & UART_LSR_RXFE)
                {
                    Led_On(RGB_R_LED);
                    Led_On(RGB_B_LED);
                    strcat(buffer, RXFIFO);
                }
                if(readError & UART_LSR_TXFE)
                {
                    Led_On(RGB_R_LED);
                    Led_On(RGB_B_LED);
                    Led_On(RED_LED);
                    strcat(buffer, TXERR);
                }

                readError = 0;
            }
            else
            {
                if(LPC_TIMER0->MR[0] != 25)
                        LPC_TIMER0->MR[0] = 25;
                if(readData == 'q')
                {
                    if(contador == 99)
                    {
                        contador = 0;
                        Led_On(RGB_G_LED);
                    }
                    else contador += 1;

                    Led_On(RGB_B_LED);
                }
                else if(readData == 'w')
                {
                    if(contador == 0)
                    {
                        contador = 99;
                        Led_On(RGB_G_LED);
                    }
                    else contador -= 1;
                    
                    Led_On(RGB_R_LED);
                }
                else if(readData == 'e')
                {
                    contador = 0;
                    Led_On(RGB_G_LED);
                }

                buffer = numToString(contador);
            }
        }
   }
}


/************    Autor: Benjamín Boeri    ************/