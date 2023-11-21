#include "headers.h"
#include "lpc17xx_uart.h"

void configPins(void) {
	LPC_GPIO2->FIODIR |= (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
	LPC_GPIO2->FIOSET |= (1 << 5) | (1 << 6) | (1 << 7);
	LPC_GPIO2->FIOCLR |= (1 << 4) | (1 << 8);
}

void configInterrupts(void) {
	LPC_PINCON->PINSEL4 |= ( (1<<20) | (1<<22) | (1<<24) );		// EINT0, EINT1 y EINT2
	LPC_SC->EXTINT |=   ( (1) | (1<<1) | (1<<2) );				// Limpio flags interrupción
	LPC_SC->EXTMODE |=  ( (1) | (1<<1) | (1<<2) );				// Interrupciones por flanco
	LPC_SC->EXTPOLAR |= ( (1) | (1<<1) | (1<<2) );				// Interrumpe cuando es de subida
	NVIC_EnableIRQ(EINT0_IRQn);									// P2.10 Cambio ADC a UART
	NVIC_EnableIRQ(EINT1_IRQn);									// P2.11 Cambio de sentido
	NVIC_EnableIRQ(EINT2_IRQn);									// P2.12 Parada de emergencia
}

void configPWM1(void) {
	LPC_SC->PCONP |= (1 << 6);  // Encender el módulo PWM1
	LPC_SC->PCLKSEL0 |= (3 << 12); // Establecer la fuente de reloj para PWM1 CCLK/8
	LPC_PWM1->PR = 20;
	LPC_PINCON->PINSEL4 |= ((1 << 4) | (1 << 6));		// P2.3 y P2.4 PWM
	LPC_PWM1->MR0 = 4095;  // Establecer el valor máximo del contador del canal
	LPC_PWM1->MR3 = 40; // Establecer el valor deseado para el canal PWM1.3
	LPC_PWM1->MR4 = 0;	// Valor del PWM1.4 en 0
	LPC_PWM1->MCR |= (1 << 9); // Habilitar la interrupción al coincidir con el canal 3		//revisar
	LPC_PWM1->LER |= (1 << 3); // Cargar el nuevo valor de MR3 al registro de comparación
	LPC_PWM1->PCR |= ((1 << 11) | (1 << 12)); // Habilitar el control de PWM1.3 y PWM1.4
	LPC_PWM1->TCR = 1;  // Habilitar el temporizador y el contador
	LPC_PWM1->TCR |= (1 << 3);  // Restablecer el contador y el temporizador
}

void configTimerCap(void){
	LPC_SC->PCONP |= (1<<1);		// Habilito Timer
	LPC_SC->PCLKSEL0 |= (1<<2);		// Clock 100 MHz
	LPC_PINCON->PINSEL3 |= (3<<20);	// Configuro Pin como Capture
	LPC_TIM0->CCR |= (1<<1);  		//pag. 508
	LPC_TIM0->TCR = 3;              //pag. 505
	LPC_TIM0->TCR &= ~(1<<1);
	NVIC_EnableIRQ(TIMER0_IRQn);	// Habilito Interrupción por Timer 0
}

void configADC(void) {
	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->ADCR |= (1 << 21); //habilita el ADC
	LPC_SC->PCLKSEL0 |= (3 << 24);  //CCLK/8
	LPC_ADC->ADCR &= ~(255 << 8);  //[15:8] CLKDIV
	LPC_ADC->ADCR |= (1 << 0);   // channel
	LPC_ADC->ADCR |= (1 << 16);   // burst
	LPC_PINCON->PINMODE1 |= (1 << 15); //neither pull-up nor pull-down.
	LPC_PINCON->PINSEL1 |= (1 << 14);
	LPC_ADC->ADINTEN |= (1 << 0);
	LPC_ADC->ADINTEN &= ~(1 << 8);
	NVIC_DisableIRQ(ADC_IRQn);
	return;
}

void configUART0(void) {
	LPC_PINCON->PINSEL0 |= (1 << 4) | (1 << 6); // P0.2 como TXD0 y P0.3 como RXD0
	LPC_PINCON->PINMODE0 &= ~(3 << 4) | (3 << 6);
	UART_CFG_Type uartConfig;
	UART_ConfigStructInit(&uartConfig);
	UART_Init(LPC_UART0, &uartConfig);
	UART_IntConfig(LPC_UART0, UART_INTCFG_RBR, ENABLE);
	NVIC_DisableIRQ(UART0_IRQn);
}

void emergencyStop(void) {
	// Apago salidas motores, leds azul y verde, parpadeo rojo
	LPC_GPIO2->FIOSET |= ((1 << 7) | (1 << 8));	// Leds verde y azul apagados
	LPC_GPIO2->FIOCLR |= ((1 << 4) | (1 << 5)); 	// P2.4 y P2.5 off
	LPC_PWM1->MR3 = 0;  						// Valor de PWM1.3 en 0
	LPC_PWM1->MR4 = 0;							// Valor de PWM1.4 en 0
	LPC_PWM1->LER |= ((1 << 3) | (1 << 4));	// Actualizo ambos valores
	LPC_PWM1->PCR &= ~(1 << 11) | ~(1 << 12); // Habilitar el control de PWM1.3 y PWM1.4
	LPC_GPIO2->FIOCLR |= ((1 << 2) | (1 << 3));	// PWM1.3 y PWM1.4 off
}

// Generar movimiento

/*   // Verificar primero si no está la parada de emergencia		// Revisar, no hace falta
 * Verificar el sentido, con 1 --> P2.2 PWM1.3 y P2.4, con 0 --> P2.3 PWM1.4 y P2.5
 * Verificar previo
 * Apagar los que no van y encender los nuevos
 * Darle la velocidad al pwm
 * */

void changeRotation(void) {
	LPC_PWM1->TCR = (1 << 1);
	if (flags & 1) {// Para un lado, apago los otros y enciendo los que van --> P2.2 y P2.4
		LPC_GPIO2->FIOCLR |= (1 << 5);		// P2.5 Trans. inferior apagado
		LPC_PWM1->MR3 = 0;
		LPC_PWM1->LER |= (1 << 3);
		LPC_PWM1->PCR &= ~(1 << 11);
		LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 8));
		LPC_GPIO2->FIOCLR |= (1 << 7);			// Solo led Verde
		LPC_PWM1->PCR |= (1 << 12);
		LPC_GPIO2->FIOSET |= (1 << 4);	// P2.4 Trans. inferior encendido

	} else {				// Para el otro lado	--> P2.3 y P2.5
		LPC_GPIO2->FIOCLR |= (1 << 4);		// P2.4 Trans. inferior apagado
		LPC_PWM1->MR4 = 0;
		LPC_PWM1->LER |= (1 << 4);
		LPC_PWM1->PCR &= ~(1 << 12);
		LPC_GPIO2->FIOSET |= ((1 << 6) | (1 << 7));
		LPC_GPIO2->FIOCLR |= (1 << 8);			// Solo led Azul
		LPC_PWM1->PCR |= (1 << 11);
		LPC_GPIO2->FIOSET |= (1 << 5);	// P2.5 Trans. inferior encendido
	}
	LPC_PWM1->TCR = (1 << 0) | (1 << 3);
}

void configDMA(void) {
	NVIC_DisableIRQ(DMA_IRQn);
	GPDMA_Channel_CFG_Type channelCFG;

	GPDMA_Init();
	channelCFG.ChannelNum = 1;
	channelCFG.SrcMemAddr = 0;
	channelCFG.DstMemAddr = (uint32_t) &dma_value;
	channelCFG.TransferSize = 1;
	channelCFG.TransferType = GPDMA_TRANSFERTYPE_P2M;
	channelCFG.TransferWidth = 0;
	channelCFG.SrcConn = GPDMA_CONN_ADC;
	channelCFG.DstConn = 0;
	channelCFG.DMALLI = 0;
	GPDMA_Setup(&channelCFG);
}

void frenar(void) {
	ADC0Value = 0;
	flags |= (1 << 3);
}

void delay(int time) {
	for (int i = 0; i < time; i++) {
	}
}

int acomodar(void){		// Acomodo valores recibidos del UART, si retorna 1, es porque hubo error

	flags &= ~(1<<5);	// Limpio flag de fin de acomodo, para comenzar a acomodar

    // INDEX 1					Si no es 0 ni 1
    if( ( (rxBuffer[1]-48 != 0) & (rxBuffer[1]-48 != 1) ) ){
    	return 1;				// Error porque es distinto de 0 y 1
    }

    // INDEX 2					Si no es ","
    if(rxBuffer[2] != 44){
    	return 1;				// Error porque es un caracter distinto
    }

    // INDEX 3					Si no es un número (48 = 0, 57 = 9)
    if( (rxBuffer[3] < 48) | (rxBuffer[3] > 57) ){
    	return 1;				// Error por caracter inválido
    }

    // INDEX 4					Si no es un número (48 = 0, 57 = 9) y tampoco es ";"
    if( ( ( (rxBuffer[4] < 48) | (rxBuffer[4] > 57) ) & (rxBuffer[4] != 59) ) ){
       	return 1;				// Error por caracter inválido
    }
    else if(rxBuffer[4] == 59){	// Si es ";" valor de velocidad será de un sólo dígito
    	velUart = rxBuffer[3] - 48;
    	//flags |= (1<<5);				// Marco el fin de acomodo porque encontré el ";"
    	rxBuffer[5] = 0;
    	rxBuffer[6] = 0;
    	rxBuffer[7] = 0;				// Los últimos los hago 0
    	return 0;
    }

    // INDEX 5					Si no es un número (48 = 0, 57 = 9) y tampoco es ";"
    if( ( (rxBuffer[5] < 48) | (rxBuffer[5] > 57) ) & (rxBuffer[5] != 59) ){
       	return 1;				// Error por caracter inválido
    }
    else if(rxBuffer[5] == 59){	// Si es ";" valor de velocidad de 2 dígitos
    	velUart = (rxBuffer[3] - 48)*10 + rxBuffer[4] - 48;
    	//flags |= (1<<5);				// Marco el fin de acomodo porque encontré el ";"
    	rxBuffer[6] = 0;
    	rxBuffer[7] = 0;				// Los últimos los hago 0
    	return 0;
    }

    // INDEX 6					Si no es un número (48 = 0, 57 = 9) y tampoco es ";"
    if( ( ( (rxBuffer[6] < 48) | (rxBuffer[6] > 57) ) & (rxBuffer[6] != 59) ) ){
       	return 1;				// Error por caracter inválido
    }
    else if(rxBuffer[6] == 59){	// Si es ";" valor de velocidad de 3 dígitos
    	velUart = (rxBuffer[3] - 48)*100 + (rxBuffer[4] - 48)*10 + rxBuffer[5] - 48;
    	rxBuffer[7] = 0;				// El último lo hago 0
    	return 0;
    }

    // INDEX 7					Si no es ";"
    if(rxBuffer[7] != 59){
       	return 1;				// Error por caracter inválido
    }
    else{						// Si es ";" valor de velocidad de 4 dígitos
    	velUart = (rxBuffer[3] - 48)*1000 + (rxBuffer[4] - 48)*100 + (rxBuffer[5] - 48)*10 + rxBuffer[6] - 48;
    }

    return 0;		// Si paso todas las pruebas, devuelvo un 0
}
