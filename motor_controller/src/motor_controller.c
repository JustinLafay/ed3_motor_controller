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

	configPins();			// Inicio todas las configuraciones
	configInterrupts();
	configPWM1();
	configADC();
	configUART0();
	configTimerCap();

	while (1) {									// Loop principal

		if (flags & (1 << MODO_ADC_UART)) {				// Si está en modo UART
			velocidad_motor = velUart;			// La velocidad es la del UART
		}

		else {									// Si está en modo ADC
			configDMA();						// Configuro DMA
			GPDMA_ChannelCmd(1, ENABLE);		// Habilito canal 1
			promedio();	// Actualizo la velocidad con lo recibido por DMA
		}
		if (!(flags & (1 << EMERGENCIA))) {	// Si la parada de emergecia no está activada
			girar(velocidad_motor, flags & (1) );
			delay(3000000);
		} else {
			LPC_GPIO2->FIOSET |= (1 << 6); // Prendo rojo
			delay(3000000);
			LPC_GPIO2->FIOCLR |= (1 << 6); // Apago rojo
			delay(3000000);
		}
	}
	return 0;
}
// Handler Interrupción cambio de modo
void EINT0_IRQHandler() {
	if (flags & (1 << MODO_ADC_UART)) {		// Modo UART activo, paso a ADC
		LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 7) | (1 << 8));	// Apago leds
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOCLR |= (1 << 6);				// Prendo rojo
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 6);				// Apago rojo
		LPC_GPIO2->FIOCLR |= (1 << 7);				// Prendo verde
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 7);				// Apago verde
		LPC_GPIO2->FIOCLR |= (1 << 8);				// Prendo azul
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 8);				// Apago azul
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		flags &= ~(1 << MODO_ADC_UART);					// Flag en 0
		NVIC_DisableIRQ(UART0_IRQn);		// Deshabilito interrupción UART
	} else {				// Modo ADC activo, paso a UART
		LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 7) | (1 << 8));	// Apago leds
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOCLR |= (1 << 8);				// Prendo azul
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 8);				// Apago azul
		LPC_GPIO2->FIOCLR |= (1 << 7);				// Prendo verde
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 7);				// Apago verde
		LPC_GPIO2->FIOCLR |= (1 << 6);				// Prendo rojo
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		LPC_GPIO2->FIOSET |= (1 << 6);				// Apago rojo
		for (int i = 0; i < 3000000; i++) {
		}				// Delay
		flags |= (1 << MODO_ADC_UART);					// Flag en 1
		NVIC_EnableIRQ(UART0_IRQn);			// Habilito interrupción UART
	}
	LPC_SC->EXTINT |= (1);   // Limpia bandera
}

// Handler Interrupción cambio de sentido
void EINT1_IRQHandler(void) {
	frenar();				// Freno el motor
	if (flags & (1 << FRENANDO)) {// Verifico que el motor esté frenado para permitir el cambio de sentido
		flags ^= 1;			// Cambio el sentido de giro
		delay(3000000);
		flags &= ~(1 << FRENANDO);	// Deshabilito flag de motor frenado
	}
	delay(10000);
	LPC_SC->EXTINT |= (1 << 1);   // Limpia bandera
}

// Handler Parada de emergencia
void EINT2_IRQHandler(void) {
	LPC_GPIO2->FIOSET |= ((1 << 7) | (1 << 8));	// Apago verde y azul
	LPC_GPIO2->FIOCLR |= (1 << 6); 				// Prendo rojo
	delay(3000000);
	LPC_GPIO2->FIOSET |= (1 << 6); 				// Apago rojo
	delay(3000000);
	if ((flags & (1 << EMERGENCIA)) != 0) { // Si la parada de emergencia está activada
		flags &= ~(1 << 1);							// La desactivo
		LPC_PWM1->PCR |= ((1 << 11) | (1 << 12)); // Habilitar el control de PWM1.3 y PWM1.4
	}

	else {
		flags |= (1 << EMERGENCIA); 						// Sino, la activo
		emergencyStop();
	}
	LPC_SC->EXTINT |= (1 << 2); // Limpia bandera
}

// Handler UART
void UART0_IRQHandler(void) {
	if ((flags & (1 << MODO_ADC_UART)) != 0) {// Verifico que esté en modo UART

		// Leer el byte recibido desde el registro de recepción (RBR)
		data_uart = UART_ReceiveByte(LPC_UART0);

		rxBuffer[rxIndex] = data_uart;// Almaceno el Byte data en la posición correspondiente de buffer

		if (rxBuffer[0] != 46) {		// Si el primero no es "."
			rxIndex = 0;
		}

		if (rxBuffer[rxIndex] == 59) {		// Si es ";" fin de transmisión
			if (rxIndex != BUFFER_SIZE - 1) {
				for (int i = rxIndex + 1; i < BUFFER_SIZE; i++) {// Relleno con 0 los restantes
					rxBuffer[i] = 0;
				}
			}
			rxIndex = BUFFER_SIZE;			// Buffer completo
		}

		rxIndex++;				// Si no había llegado al ";" paso al siguiente

		// Verificar si se ha completado la recepción del buffer
		if (rxIndex >= BUFFER_SIZE) {
			if (!(acomodar())) {		// Si no obtengo errores con los datos
				if (velUart > 4095) {		// Si se pasó del límite, acomodo
					velUart = 4095;
				}
				// Si el sentido es 0, el UART pide sentido 1, y la velocidad está en 0:
				if ((!(flags & (1))) & (rxBuffer[1] - 48)) {
					flags |= 1;
				}
				// Si el sentido es 1, el UART pide sentido 0, y la velocidad está en 0
				else if ((flags & (1)) & (!(rxBuffer[1] - 48))) {
					flags &= ~(1);
				}
				flags |= (1 << UART_OK);		// Recepción correcta
			}

			else {		// Si hubo error, limpio el buffer
				for (int j = 0; j < BUFFER_SIZE; j++) {
					rxBuffer[j] = 0;
				}
				flags &= ~(1 << UART_OK);		// Recepción incorrecta
			}
			rxIndex = 0;// En caso de error, reinicio, y sino, reinicio igual
		}
	}

}

// Handler Capture
void TIMER0_IRQHandler() {
	buffer_capture[1] = buffer_capture[0];
	buffer_capture[0] = LPC_TIM0->CR0; //Variable auxiliar para observar el valor del registro de captura

	value_capture = buffer_capture[0] - buffer_capture[1];

	LPC_TIM0->IR |= 1; //Limpia bandera de interrupci�n
}
