#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H
#define BUFFER_SIZE 8
#include "headers.h"

/* Flags:
 * Bit 0 : Sentido de giro de motor
 * Bit 1 : Parada de emergencia, 1 disparada
 * Bit 2 : Modo de operación, 0 ADC, 1 UART
 * Bit 3 : Flag de motor frenado
 * Bit 4 : Flag de recepción correcta de UART
 * Bit 5 : Bit de fin de acomodo UART, 1 si llegó al ";"
 * */

#define SENSO_GIRO 		0
#define EMERGENCIA 		1
#define MODO_ADC_UART	2
#define FRENANDO		3
#define UART_OK			4
#define	UART_ACC		5

extern uint8_t flags;
extern uint8_t data;
extern uint16_t velUart;
extern uint32_t ADC0Value;
extern uint32_t dma_value;
extern uint32_t value_capture;
extern volatile uint32_t buffer_capture[2];
extern volatile uint8_t rxBuffer[BUFFER_SIZE];
extern volatile uint32_t rxIndex;
extern volatile uint32_t rxComplete;

#endif
