#include "headers.h"

uint8_t flags = 0;
uint32_t ADC0Value = 0;
uint32_t dma_value = 0;
volatile uint8_t rxBuffer[BUFFER_SIZE];
volatile uint32_t rxIndex = 0;
volatile uint32_t rxComplete = 0;
uint16_t velUart = 0;
uint8_t data = 0;
volatile uint32_t buffer_capture[2];
uint32_t value_capture = 0;
