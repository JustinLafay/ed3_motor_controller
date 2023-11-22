#ifndef CONFIG_FUNC_H
#define CONFIG_FUNC_H

void configPins(void);
void configInterrupts(void);
void configPWM1(void);
void configADC(void);
void emergencyStop(void);
void frenar(void);
void delay(int);
void configDMA(void);
void configTimerCap(void);
int acomodar(void);
void configUART0(void);
void promedio(void);
void girar(uint32_t velocidad, uint8_t sentido);

#endif
