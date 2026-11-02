#include <string.h>
#include <stdio.h>
#include "stm32f401xx.h"
#include "stm32f401xx_gpio_drivers.h"
#include "stm32f401xx_usart_drivers.h"

// USART Handle structure
USART_Handle_t usart2_handle;

// Text message to send over serial
char msg[] = "STM32 USART Interrupt online  \r\nEnter 5 charecters\n";
uint8_t rx_buffer[10];

volatile uint8_t rx_cmplt_flag = 0;
volatile uint8_t tx_cmplt_flag = 0;

void delay(void)
{
	for(uint32_t i = 0; i < 500000; i++);
}

/*
 * Pin configuration for USART2:
 * PA2 -> USART2_TX (Alternate Function AF7)
 * PA3 -> USART2_RX (Alternate Function AF7)
 */
void USART2_GPIOInit(void)
{
	GPIO_Handle_t usart_pins;

	usart_pins.pGPIOx = GPIOA;
	usart_pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	usart_pins.GPIO_PinConfig.GPIO_OPType = GPIO_OP_TYPE_PP;
	usart_pins.GPIO_PinConfig.GPIO_PinPupdControl = GPIO_PIN_UP;
	usart_pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_HIGH;
	usart_pins.GPIO_PinConfig.GPIO_AltFunMode = 7;

	// Enable GPIOA peripheral clock
	GPIO_PeriClkControl(GPIOA, ENABLE);

	// PA2 -> USART2_TX
	usart_pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NUM_2;
	GPIO_Init(&usart_pins);

	// PA3 -> USART2_RX
	usart_pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NUM_3;
	GPIO_Init(&usart_pins);
}

void USART2_Init(void)
{
	usart2_handle.pUSARTx = USART2;
	usart2_handle.USART_Config.USART_Mode = USART_MODE_ONLY_TXRX;
	usart2_handle.USART_Config.USART_BaudRate = USART_STD_BAUD_115200;
	usart2_handle.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
	usart2_handle.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
	usart2_handle.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
	usart2_handle.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

	USART_Init(&usart2_handle);
}

int main(void)
{
	// 1. Init GPIO pins (PA2 -> TX, PA3 -> RX)
	USART2_GPIOInit();

	// 2. Initialize USART2 peripheral parameters
	USART2_Init();

	// 3. NVIC config for USART2
	USART_IRQPriorityConfig(IRQ_NO_USART2, 15);
	USART_IRQInteruptConfig(IRQ_NO_USART2, ENABLE);

	// 4. Enable USART2 peripheral
	USART_PeripheralControl(USART2, ENABLE);
	delay();

	// 5. Send welcome message via Interrupt
	USART_SendDataIT(&usart2_handle, (uint8_t*)msg, strlen(msg));

	// Wait until TX completes
	while(!tx_cmplt_flag);
	tx_cmplt_flag = 0;

	// 6. Arm reception for 5 bytes via Interrupt
	USART_ReceiveDataIT(&usart2_handle, rx_buffer, 5);

	while(1)
	{
		// Wait until 5 bytes are fully received
		if(rx_cmplt_flag == 1)
		{
			rx_cmplt_flag = 0;

			// Echo back the received 5 bytes
			USART_SendDataIT(&usart2_handle, rx_buffer, 5);
			while(!tx_cmplt_flag);
			tx_cmplt_flag = 0;

			// Arm next 5 bytes reception
			USART_ReceiveDataIT(&usart2_handle, rx_buffer, 5);
		}
	}

	return 0;
}

// USART2 Interrupt Handler
void USART2_IRQHandler(void)
{
	USART_IRQHandling(&usart2_handle);
}

// Callback implementation
void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle, uint8_t AppEv)
{
	if(AppEv == USART_EVENT_RX_CMPLT)
	{
		rx_cmplt_flag = 1;
	}
	else if(AppEv == USART_EVENT_TX_CMPLT)
	{
		tx_cmplt_flag = 1;
	}
	else if(AppEv == USART_ERR_ORE)
	{
		volatile uint32_t dummy = pUSARTHandle->pUSARTx->SR;
		dummy = pUSARTHandle->pUSARTx->DR;
		(void)dummy;
	}
}
