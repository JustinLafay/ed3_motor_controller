/*
 * Copyright 2022 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */
#include "headers.h"

int main(void) {

	configPins();
	configInterrupts();
	configPWM1();
	configADC();

	while (1) {
		configDMA();
		GPDMA_ChannelCmd(1, ENABLE);
		ADC0Value = (dma_value>>4)&0xFFF;

		if (!(flags & (1 << 1))){
			if (ADC0Value < 300) {					// Si el PWM está muy bajo, apago
				LPC_GPIO2->FIOSET |= ((1 << 7) | (1 << 8));	// Apago leds Verde y Azul
				LPC_GPIO2->FIOCLR |= (1 << 6);				// Enciendo led Rojo
				LPC_GPIO2->FIOCLR |= ((1 << 4) | (1 << 5));	// Apago parte inferior completa Puente H
				LPC_PWM1->MR3 = 0;  						// Valor de PWM1.3 en 0
				LPC_PWM1->MR4 = 0;							// Valor de PWM1.4 en 0
				LPC_PWM1->LER |= ((1 << 3) | (1 << 4));		// Actualizo ambos valores
				flags |= (1 << 3); 					// Habilito flag de motor frenado
			} else {
				flags &= ~(1 << 3);
				if (flags & 1) {
					LPC_GPIO2->FIOCLR |= (1 << 7);
					LPC_GPIO2->FIOSET |= (1 << 6);
					LPC_PWM1->MR4 = ADC0Value;
					LPC_PWM1->LER |= (1 << 4);
				} else {
					LPC_GPIO2->FIOCLR |= (1 << 8);
					LPC_GPIO2->FIOSET |= (1 << 6);
					LPC_PWM1->MR3 = ADC0Value;
					LPC_PWM1->LER |= (1 << 3);
				}
			}
		}

	}
	return 0;
}

// Handler Interrupción cambio de sentido
void EINT1_IRQHandler(void) {
	frenar();
	if (flags & (1 << 3)) {	// Verifico que el motor esté frenado para permitir el cambio de sentido
		flags ^= 1;
		changeRotation();
		if ((flags & (1)) == 1) { // Si el sentido de giro, es 1
			LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 7));	// Apago rojo verde
			LPC_GPIO2->FIOCLR |= (1 << 8); // Prendo azul
			delay(3000000);
			LPC_GPIO2->FIOSET |= (1 << 8); // Apago azul
			delay(3000000);
		} else { // Si el sentido de giro es 0
			LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 8));	// Apago rojo azul
			LPC_GPIO2->FIOCLR |= (1 << 7); // Prendo verde
			delay(3000000);
			LPC_GPIO2->FIOSET |= (1 << 7); // Apago verde
			delay(3000000);
		}
		flags &= ~(1 << 3);	// Deshabilito flag de motor frenado
	}
	delay(10000);
	LPC_SC->EXTINT |= (1 << 1);   // Limpia bandera
}

// Handler Parada de emergencia
void EINT2_IRQHandler(void) {
	LPC_GPIO2->FIOSET |= ((1 << 7) | (1 << 8));	// Apago verde y azul
	LPC_GPIO2->FIOCLR |= (1 << 6); // Prendo rojo
	delay(3000000);
	LPC_GPIO2->FIOCLR |= (1 << 6); // Apago rojo
	delay(3000000);
	if ((flags & (1 << 1)) != 0) { // Si la parada de emergencia está activada
		flags &= ~(1 << 1);	// La desactivo
		LPC_PWM1->PCR |= ((1 << 11) | (1 << 12)); // Habilitar el control de PWM1.3 y PWM1.4
	} else {
		flags |= (1 << 1); // Sino, la activo
		emergencyStop();
	}
	LPC_SC->EXTINT |= (1 << 2); // Limpia bandera
}
