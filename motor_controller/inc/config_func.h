#ifndef CONFIG_FUNC_H
#define CONFIG_FUNC_H

void configPins(void);
void configInterrupts(void);
void configPWM1(void);
void configADC(void);
void emergencyStop(void);
void changeRotation(void);
void frenar(void);
void delay(int);
void configDMA(void);

#endif
