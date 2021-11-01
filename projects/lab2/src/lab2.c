#include "stdlib.h"
#include "stdint.h"
#include "configuracion.h"
#include "myLib.h"
#include "led.h"
#include <string.h>

#define USB_UART LPC_USART2
#define MODEM_UART LPC_USART3

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
    pUART->LCR = UART_LCR_DLAB_EN | UART_LCR_SBS_1BIT | UART_LCR_WLEN8;

    // Configuro mi divisor principal, que será 1328 = 0x530
    pUART->DLM = 0x1;
    pUART->DLL = 0x4C;

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

void UARTEscribirByte(LPC_USART_T *pUART, uint8_t data)
{
    while(!UARTDisponible(pUART));
    pUART->THR = data;
}

void UARTEscribirString(LPC_USART_T *pUART, char* string)
{
    for(size_t i = 0; i < strlen(string); i++)
        UARTEscribirByte(pUART, (uint8_t)string[i]);
}

uint8_t EnviarComandoAT(LPC_USART_T *pUART_MODEM, char* AT)
{
    uint8_t byte0 = 0;
    uint8_t byte1 = 0;
    uint8_t byteError = 0;
    uint8_t resultado = 0;
    uint8_t aux = 0;

    // Limpiar la basura que pudiera quedar
    while(UARTLeerByte(pUART_MODEM, &byte0, &byteError));

    // ESCRIBIR COMANDO
    UARTEscribirString(pUART_MODEM, AT);

    // INTERPRETAR RESPUESTA (PISTA: BYTE 0 y 1)

    // Espero a que el RBR tenga datos para leer
    aux = UARTLeerByte(pUART_MODEM, &byte0, &byteError);
    aux = UARTLeerByte(pUART_MODEM, &byte1, &byteError);

    if(byte1 == 'K' && byte0 == 'O')
        resultado = 1;

    // Vacio el resto del buffer
    while(UARTLeerByte(pUART_MODEM, &byte0, &byteError));

    return resultado;
}

void ConfigurarMODEM(LPC_USART_T *pUART_USB, LPC_USART_T *pUART_MODEM)
{
    uint8_t aux;
    uint8_t err = 0;
    while(UARTLeerByte(pUART_MODEM, &aux, &err));

    if(1)
    {
        aux = EnviarComandoAT(pUART_MODEM, "AT\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+NAME=BTBoeri\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+PSWD=90658\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+UART=38400,0,0\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+ROLE=0\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+INIT\r\n");
        aux = EnviarComandoAT(pUART_MODEM, "AT+RESET\r\n");
    }
}

int main(void)
{
    uint8_t readData = 0;
    uint8_t readError = 0;
    uint8_t* buffer = (uint8_t*)malloc(100);    // Para almacenar caracteres
    *buffer = 0;
    uint8_t contador = 0;

    ConfigurarPuertosLaboratorio();
    ConfigurarInterrupciones();
    ConfigurarUART(USB_UART);
    ConfigurarUART(MODEM_UART);
    ConfigurarMODEM(USB_UART, MODEM_UART);

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

        if(UARTLeerByte(MODEM_UART, &readData, &readError))
        {
            if(readError)
            {
                if(LPC_TIMER0->MR[0] != 100)
                    LPC_TIMER0->MR[0] = 100;
                    
                if(readError & UART_LSR_OE)
                {
                    Led_On(RED_LED);
                    memcpy(buffer, OVERRUN, 15);
                }
                if(readError & UART_LSR_PE)
                {
                    Led_On(GREEN_LED);
                    memcpy(buffer, PARITY, 14);
                }
                if(readError & UART_LSR_FE)
                {
                    Led_On(RED_LED);
                    Led_On(GREEN_LED);
                    memcpy(buffer, FRAMING, 15);
                }
                if(readError & UART_LSR_RXFE)
                {
                    Led_On(RGB_R_LED);
                    Led_On(RGB_B_LED);
                    memcpy(buffer, RXFIFO, 18);
                }
                if(readError & UART_LSR_TXFE)
                {
                    Led_On(RGB_R_LED);
                    Led_On(RGB_B_LED);
                    Led_On(RED_LED);
                    memcpy(buffer, TXERR, 32);
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
            }

            buffer = numToString(contador);
        }
    }
}


/************    Autor: Benjamín Boeri    ************/