#include "stdint.h"

enum {COMANDO_INVALIDO, COMANDO_VALIDO};

void trim(char* s);

uint8_t esNumero(char* s);

uint8_t equalSequence(char* s, char* secuencia);

uint8_t controlComandoAT(char* s);

uint8_t numToChar(uint8_t numero);

uint8_t* numToString(uint8_t numero);

uint8_t arrayLength(uint8_t* array);

uint8_t LeerLEDs();