#include "stdlib.h"
#include "stdint.h"
#include "configuracion.h"
#include "myLib.h"
#include "led.h"
#include "lab2.h"
#include <string.h>
#include <ctype.h>

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

void TIMER1_IRQHandler(void)
{
    LPC_TIMER1->IR = LPC_TIMER1->IR;
    LPC_TIMER1->TCR = TIMER_RESET;

    textoEnviado = 1;
    esConsulta = 0;

    buffer[index_buff++] = 'C'; buffer[index_buff++] = 'o'; buffer[index_buff++] = 'u';
    buffer[index_buff++] = 'l'; buffer[index_buff++] = 'd'; buffer[index_buff++] = 'n';
    buffer[index_buff++] = '\''; buffer[index_buff++] = 't'; buffer[index_buff++] = ' ';
    buffer[index_buff++] = 'e'; buffer[index_buff++] = 'x'; buffer[index_buff++] = 'e';
    buffer[index_buff++] = 'c'; buffer[index_buff++] = 'u'; buffer[index_buff++] = 't';
    buffer[index_buff++] = 'e'; buffer[index_buff++] = ' '; buffer[index_buff++] = 'c';
    buffer[index_buff++] = 'o'; buffer[index_buff++] = 'm'; buffer[index_buff++] = 'm';
    buffer[index_buff++] = 'a'; buffer[index_buff++] = 'n'; buffer[index_buff++] = 'd';
    buffer[index_buff++] = '\r'; buffer[index_buff++] = '\n';
}

void ConfigurarUART(LPC_USART_T *pUART, uint8_t enRBRInt)
{
    Chip_UART_Init(pUART);
    pUART->LCR = UART_LCR_DLAB_EN | UART_LCR_SBS_1BIT | UART_LCR_WLEN8;

    // Configuro mi divisor principal, que será 332 = 0x14C
    pUART->DLM = 0x1;
    pUART->DLL = 0x4C;

    // Habilito la transmisión de datos
    pUART->TER2 = 0x01;

    // Deshabilito la configuración de divisores, por lo que dejo que se empiece a recibir y transmitir.
    pUART->LCR &= ~UART_LCR_DLAB_EN;

    if(enRBRInt)
        pUART->IER = UART_IER_RBRINT;
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
    for(char* p = string; *p != 0; p++)
        UARTEscribirByte(pUART, (uint8_t)*p);
}

void EnviarComandoAT(LPC_USART_T *pUART_MODEM, char* AT)
{
    uint8_t resultado = 0;
    uint8_t dato = 0;
    uint8_t resError = 0;
    
    while(UARTLeerByte(pUART_MODEM, &dato, &resError));

    for(uint8_t i = 0; i < strlen(AT); i++)
    {
        if(AT[i] == '?')
        {
            esConsulta = 1;
            break;
        }
    }

    UARTEscribirString(pUART_MODEM, AT);
}

void UART2_IRQHandler(void)
{
    uint8_t dato = USB_UART->RBR & UART_RBR_MASKBIT;

    if(dato != 0)
        *(buffer_prueba+indexBuffPrueba++) = dato;
    else
    {
        indexBuffPrueba = 0;

        USB_UART->TER2 = 1;
        
        while(!UARTDisponible(USB_UART));
        USB_UART->THR = 'Y';
        while(!UARTDisponible(USB_UART));
        USB_UART->THR = 'E';
        while(!UARTDisponible(USB_UART));
        USB_UART->THR = 'S';
        
        USB_UART->TER2 = 0;
        memset(buffer_prueba, 0, 100);
    }

    if(dato == 0x00)
        esFn = 1;
    if(esFn && dato == 0x3C)
    {
        esF2 = 0;
        esFn = 0;
        modoAT = !modoAT;
        *buffer = 0x1B;
        *(buffer + 1) = '[';
        *(buffer + 2) = '2';
        *(buffer + 3) = 'J';
        *(buffer + 4) = 0x1B;
        *(buffer + 5) = '[';
        *(buffer + 6) = '0';
        *(buffer + 7) = ';';
        *(buffer + 8) = '0';
        *(buffer + 9) = 'f';
        *(buffer + 10) = 0;

        if(modoAT)
        {
            *(buffer + 10) = '*';
            *(buffer + 11) = ' ';
            *(buffer + 12) = 'M';
            *(buffer + 13) = 'o';
            *(buffer + 14) = 'd'; 
            *(buffer + 15) = 'o';
            *(buffer + 16) = ' ';
            *(buffer + 17) = 'c';
            *(buffer + 18) = 'o';
            *(buffer + 19) = 'n';
            *(buffer + 20) = 's';
            *(buffer + 21) = 'o';
            *(buffer + 22) = 'l';
            *(buffer + 23) = 'a';
            *(buffer + 24) = ' ';
            *(buffer + 25) = '*';
            *(buffer + 26) = '\r';
            *(buffer + 27) = '\n';
            *(buffer + 28) = 0;
        }
    }
    else if(modoAT)
    {
        if(esFn)
        {
            int x = 0;
            x++;
            x++;
        }
        else
        {
            uint8_t index_AT = 0;

            index_buff = 1;

            *buffer = dato;
            *(comandoAT + index_AT++) = dato;

            if(dato == '\b')
            {
                if(index_AT > 0)
                {
                    *(comandoAT + --index_AT) = 0;
                    *(comandoAT + --index_AT) = 0;
                }

                *(buffer + index_buff++) = 0x20;
                *(buffer + index_buff++) = 0x08;
            }

            if(dato == '\r')
            {
                *(comandoAT + index_AT++) = '\n';
                *(comandoAT + index_AT) = 0;
                index_AT = 0;
                
                *(buffer + index_buff++) = '\n';

                EnviarComandoAT(MODEM_UART, comandoAT);

                for(uint8_t i = 0; i < strlen(comandoAT); i++)
                {
                    if(isalpha(comandoAT[i]))
                        *(comandoAT + i) = toupper(*(comandoAT + i));
                }

                if(!strcmp(comandoAT, "AT+RESET\r\n"))
                {
                    modoAT = 0;
                    *(buffer + index_buff++) = 0x1B;
                    *(buffer + index_buff++) = '[';
                    *(buffer + index_buff++) = '2';
                    *(buffer + index_buff++) = 'J';
                    *(buffer + index_buff++) = 0x1B;
                    *(buffer + index_buff++) = '[';
                    *(buffer + index_buff++) = '0';
                    *(buffer + index_buff++) = ';';
                    *(buffer + index_buff++) = '0';
                    *(buffer + index_buff++) = 'f';
                    *(buffer + index_buff) = 0;
                }

                memset(comandoAT, 0, 50);

                // El timer empieza a contar
                LPC_TIMER1->TCR = TIMER_ENABLE;

                // Si el timer llega a hacer un match, significa
                // que no se llegó a obtener ningún dato.
                while(!textoEnviado && modoAT);

                textoEnviado = 0;
            }
            
            *(buffer + index_buff) = 0;
        }
    }
}

void UART3_IRQHandler(void)
{
    uint8_t dato = MODEM_UART->RBR & UART_RBR_MASKBIT;
    static uint8_t saltosLinea = 0;

    LPC_TIMER1->TCR = TIMER_RESET;
    
    if(modoAT)
    {
        if(dato == '\n')
            saltosLinea++;

        if(esConsulta)
        {
            if(saltosLinea == 2)
            {
                saltosLinea = 0;
                textoEnviado = 1;
                esConsulta = 0;
            }
        }
        else
        {
            if(saltosLinea == 1)
            {
                saltosLinea = 0;
                textoEnviado = 1;
            }
        }
    }
    else
    {
        if(MODEM_UART->LSR & (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_RXFE | UART_LSR_TXFE))
        {
            uint8_t readError = MODEM_UART->LSR & 0x19E;
            
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
        }
        else
        {
            if(LPC_TIMER0->MR[0] != 25)
                LPC_TIMER0->MR[0] = 25;
            if(dato == 'q')
            {
                if(contador == 99)
                {
                    contador = 0;
                    Led_On(RGB_G_LED);
                }
                else contador += 1;

                Led_On(RGB_B_LED);
            }
            else if(dato == 'w')
            {
                if(contador == 0)
                {
                    contador = 99;
                    Led_On(RGB_G_LED);
                }
                else contador -= 1;
                
                Led_On(RGB_R_LED);
            }
            else if(dato == 'e')
            {
                contador = 0;
                Led_On(RGB_G_LED);
            }

            buffer = numToString(contador);
        }
    }

    if(modoAT)
        *(buffer + index_buff++) = dato;

    uint8_t idInterrupcion = MODEM_UART->IIR;
}

int main(void)
{
    buffer = (uint8_t*)malloc(100);
    *buffer = 0;

    comandoAT = (uint8_t*)malloc(50);
    *comandoAT = 0;

    buffer_prueba = (unsigned char*) malloc(100);
    memset(buffer_prueba, 0, 100);
    
    aux = buffer;

    USB_UART->TER2 = 0;
    
    uint8_t readData = 0;
    uint8_t readError = 0;

    uint8_t resultado = 0;
    uint8_t dato = 255;
    uint8_t resError = 0;

    contador = 0;

    ConfigurarPuertosLaboratorio();
    ConfigurarInterrupciones();
    ConfigurarUART(USB_UART, 1);
    ConfigurarUART(MODEM_UART, 1);

    while (1)
    {
        led_encendido = LeerLEDs();

        uint8_t entersLoop = 0;
        while(arrayLength(buffer))
        {
            if(!entersLoop)
                USB_UART->TER2 = 1;
            entersLoop = 1;
            if(UARTDisponible(USB_UART))
            {
                uint8_t caracter = *buffer++;
                
                USB_UART->THR = caracter;
            }
        }
        
        if(entersLoop)
        {
            USB_UART->TER2 = 0;
            entersLoop = 0;
            buffer = aux;
            memset(buffer, 0, 100);
        }
    }
}


/************    Autor: Benjamín Boeri    ************/