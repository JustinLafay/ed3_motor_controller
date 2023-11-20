#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#include "headers.h"

/* Flags:
 * Bit 0 : Sentido de giro de motor
 * Bit 1 : Parada de emergencia, 1 disparada
 * Bit 2 : Modo de operación, 0 ADC, 1 UART
 * Bit 3 : Flag de motor frenado
 * Bit 4 : Flag de recepción correcta de UART
 * Bit 5 : Bit de fin de acomodo UART, 1 si llegó al ";"
 * */

extern uint8_t flags;
extern uint32_t ADC0Value;

#endif
