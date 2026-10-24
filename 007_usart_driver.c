#include <string.h>
#include "stm32f401xx.h"
#include "stm32f401xx_gpio_drivers.h"
#include "stm32f401xx_usart_drivers.h"

//USART Handle structure
USART_Handle_t usart2_handle;
//text msg send over serial
char msg[]="Type a key to echo: \r\n";

void delay(void)
{
	for(uint32_t i=0;i<500000;i++);
}

/*
 * pin config for usart2
 * PA2 ->Tx(alt fun AF7)
 * PA3->RX(alt fun AF7)
 */
void USART2_GPIOInit(void)
{
	GPIO_Handle_t usart_pins;
	usart_pins.pGPIOx=GPIOA;
	usart_pins.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_ALTFN;
	usart_pins.GPIO_PinConfig.GPIO_OPType=GPIO_OP_TYPE_PP;
	usart_pins.GPIO_PinConfig.GPIO_PinPupdControl=GPIO_PIN_UP;
	usart_pins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_OP_SPEED_HIGH;
	usart_pins.GPIO_PinConfig.GPIO_AltFunMode=7;
	//Enable GPIOA peripheral clock
	GPIO_PeriClkControl(GPIOA, ENABLE);
	//PA2->USART TX
	usart_pins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_2;
	GPIO_Init(&usart_pins);
	//PA3->USART RX
	usart_pins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_3;
	GPIO_Init(&usart_pins);

}

void USART2_Init(void)
{
	usart2_handle.pUSARTx=USART2;
	usart2_handle.USART_Config.USART_Mode=USART_MODE_ONLY_TXRX;
	usart2_handle.USART_Config.USART_BaudRate=USART_STD_BAUD_115200;
	usart2_handle.USART_Config.USART_NoOfStopBits=USART_STOPBITS_1;
	usart2_handle.USART_Config.USART_WordLength=USART_WORDLEN_8BITS;
	usart2_handle.USART_Config.USART_ParityControl=USART_PARITY_DISABLE;
	usart2_handle.USART_Config.USART_HWFlowControl=USART_HW_FLOW_CTRL_NONE;
	USART_Init(&usart2_handle);
}

int main()
{
	uint8_t rx_byte;


	//1. init gpio pins(PA2->TX,PA3->RX)
	USART2_GPIOInit();
	//2. initialize usart peripheral parameters
	USART2_Init();
	USART_PeripheralControl(USART2, ENABLE);
	USART_SendData(&usart2_handle, (uint8_t*)msg,strlen(msg));
	while(1)
	{
		//1. wait and read 1 byte from terminal(blocking until key pressed)
		USART_ReceiveData(&usart2_handle,&rx_byte,1);
		USART_SendData(&usart2_handle,&rx_byte,1);
//delay();
	}
	return 0;
}






































