#include "myLib.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "chip.h"
#include "led.h"

uint8_t numToChar(uint8_t numero)
{
    switch(numero)
    {
        case 0: return 48;
        case 1: return 49;
        case 2: return 50;
        case 3: return 51;
        case 4: return 52;
        case 5: return 53;
        case 6: return 54;
        case 7: return 55;
        case 8: return 56;
        case 9: return 57;
        default: return 0;
    }
}

uint8_t* numToString(uint8_t numero)
{
    uint8_t* numString;
    uint8_t cantDigitos = 0, digito;

    if(numero == 0)
    {
        numString = (uint8_t*)malloc(4);
        *numString = 48;
        *(numString + 1) = 10;
        *(numString + 2) = 13;
        *(numString + 3) = 0;

        return numString;
    }

    // Consigo la cantidad total de dígitos
    for(uint8_t aux = numero; aux != 0; aux /= 10, cantDigitos++);

    numString = (uint8_t*)malloc(cantDigitos + 3);
    *numString = 0;
    
    *(numString + cantDigitos) = 10;                // Escribo el salto de línea antes del caracter nulo
    *(numString + cantDigitos + 1) = 13;
    *(numString + cantDigitos + 2) = 0;             // Escribo el caracter nulo en la posición que correspondería al final de la cadena
    for(uint8_t i = cantDigitos - 1; numero != 0; numero /= 10, i--)
    {
        digito = numero % 10;
        *(numString + i) = numToChar(digito);
    }

    return numString;
}

uint8_t arrayLength(uint8_t* array)
{
    uint8_t length = 0;
    while(*array != 0)
    {
        array++;
        length++;
    }
    
    return length;
}

uint8_t LeerLEDs()
{
    uint8_t led_encendido = 0;
    
    // Estado del LED rojo del LED RGB
    led_encendido |= Chip_GPIO_GetPinState(LPC_GPIO_PORT, 5, 0) << RGB_R_LED;

    // Estado del LED verde del LED RGB
    led_encendido |= Chip_GPIO_GetPinState(LPC_GPIO_PORT, 5, 1) << RGB_G_LED;

    // Estado del LED azul del LED RGB
    led_encendido |= Chip_GPIO_GetPinState(LPC_GPIO_PORT, 5, 2) << RGB_B_LED;

    // Estado del LED rojo
    led_encendido |= Chip_GPIO_GetPinState(LPC_GPIO_PORT, 1, 11) << RED_LED;

    // Estado del LED verde
    led_encendido |= Chip_GPIO_GetPinState(LPC_GPIO_PORT, 1, 12) << GREEN_LED;

    if(led_encendido)
        LPC_TIMER0->TCR = TIMER_ENABLE;

    return led_encendido;
}

void trim(char* s)
{
    char* aux = s;

    do
    {
        while(*aux == ' ')
            ++aux;
        *s++ = *aux++;
    } while (*s != '\n');
}

uint8_t esNumero(char* s)
{
    for(; *s != '\r'; s++)
    {
        if(isalpha(*s))
            return 0;
    }

    return 1;
}

uint8_t equalSequence(char* s, char* secuencia)
{
    for(uint8_t i = 0; i < strlen(secuencia); i++)
    {
        if(*s++ != *secuencia++)
            return 0;
    }

    return 1;
}

uint8_t controlComandoAT(char* s)
{
    trim(s);

    if(*s++ != 'A')
        return COMANDO_INVALIDO;
    if(*s++ != 'T')
        return COMANDO_INVALIDO;

    if(*s == '\r' && *(s+1) == '\n')
        return COMANDO_VALIDO;
    else
    {
        if(*s++ != '+')
            return COMANDO_INVALIDO;

        if(equalSequence(s, "RESET"))
        {
            s += strlen("RESET");

            if(*s == '\r' && *(s+1) == '\n')
                return COMANDO_VALIDO;
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "VERSION"))
        {
            s += strlen("VERSION");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "ORGL"))
        {
            s += strlen("ORGL");

            if(*s == '\r' && *(s+1) == '\n')
                return COMANDO_VALIDO;
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "ADDR"))
        {
            s += strlen("ADDR");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "NAME"))
        {
            s += strlen("NAME");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else if(*s == '=')
            {
                if(*s == '\r')
                    return COMANDO_INVALIDO;
            }
            else
                return COMANDO_INVALIDO;
        }
        // else if(equalSequence(s, "RNAME"))
        // {
        //     s += strlen(s, "RNAME");

        //     if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
        //         return COMANDO_VALIDO;
        //     else
        //         return COMANDO_INVALIDO;
        // }
        else if(equalSequence(s, "ROLE"))
        {
            s += strlen("ROLE");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else if(*s++ == '=')
            {
                if(!isalpha(*s))
                {
                    int x = atoi(s);
                    if(x != 0 && x != 1 || x != 2)
                        return COMANDO_INVALIDO;
                }
            }
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "PSWD"))
        {
            s += strlen("PSWD");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else if(*s++ == '=')
            {
                if(*s == '\r')
                    return COMANDO_INVALIDO;
            }
            else
                return COMANDO_INVALIDO;
        }
        else if(equalSequence(s, "UART"))
        {
            s += strlen("UART");

            if(*s == '?' && *(s+1) == '\r' && *(s+2) == '\n')
                return COMANDO_VALIDO;
            else if(*s++ == '=')
            {
                char* token = strtok(s, ",");
                if(esNumero(token))
                {
                    uint8_t esNumeroValido = 0;
                    int x = atoi(token);
                    int baudRatePosibles[] = {4800, 9600, 19200, 38400, 57600, 115200, 234000, 460800, 921600, 1382400};
                    for(uint8_t i = 0; i < 10; i++)
                    {
                        if(x == baudRatePosibles[i])
                            esNumeroValido = 1;
                    }
                    
                    if(!esNumeroValido)
                        return COMANDO_INVALIDO;
                }
                else
                    return COMANDO_INVALIDO;

                token = strtok(NULL, ",");
                if(esNumero(token))
                {
                    int x = atoi(token);
                    if(x != 0 && x != 1)
                        return COMANDO_INVALIDO;
                }

                token = strtok(NULL, ",");
                if(esNumero(token))
                {
                    int x = atoi(token);
                    if(x != 0 && x != 1 && x != 2)
                        return COMANDO_INVALIDO;
                }
            }
            else
                return COMANDO_INVALIDO;
        }
        else
            return COMANDO_INVALIDO;
    }

    return COMANDO_VALIDO;
}