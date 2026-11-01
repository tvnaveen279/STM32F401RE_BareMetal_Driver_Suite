#include "stm32f401xx.h"
#include "stm32f401xx_spi_drivers.h"
#include<string.h>
#include"stm32f401xx_gpio_drivers.h"
#include<stdio.h>

//Global flag to track transmission status
volatile uint8_t spi_tx_cmplt =0;
volatile uint8_t spi_rx_cmplt =0;
volatile uint8_t error_flag	=	0;

#define RX_LEN 10
uint8_t rx_buffer[RX_LEN];
uint8_t dummy_tx[RX_LEN];
SPI_Handle_t SPI1Handle;

//char msg[]="hello spi via interrupt !";

void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
	if(AppEv==SPI_EVENT_TX_CMPLT)
	{
		spi_tx_cmplt=1;
	}
	else if(AppEv==SPI_EVENT_RX_CMPLT)
	{
		spi_rx_cmplt=1;
	}else if(AppEv==SPI_EVENT_OVR_ERR)
	{
		error_flag=1;
		SPI_ClearOVRFlag(pSPIHandle->pSPIx);
	}
	else if(AppEv==SPI_EVENT_CRC_ERR)
	{
		error_flag=1;
	}
}
void SPI1_IRQHandler(void)
{
	SPI_IRQHandling(&SPI1Handle);
}

void delay(void)
{
	for(uint32_t i=0;i<500000;i++);

}

void GPIO_Button_Init(void)
{
	GPIOC_PCLK_EN();

	    GPIO_Handle_t btn;
	    btn.pGPIOx = GPIOC;
	    btn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NUM_13;
	    btn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	    btn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_HIGH;
	    btn.GPIO_PinConfig.GPIO_PinPupdControl = GPIO_PIN_NOPUPD; // Onboard pull-up already present

	    GPIO_Init(&btn);
}

void SPI1_GPIOInit(void)
{
	GPIOA_PCLK_EN();

	GPIO_Handle_t SPIpins;
	SPIpins.pGPIOx=GPIOA;
	SPIpins.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_ALTFN;
	SPIpins.GPIO_PinConfig.GPIO_OPType=	GPIO_OP_TYPE_PP;
	SPIpins.GPIO_PinConfig.GPIO_AltFunMode=5;
	SPIpins.GPIO_PinConfig.GPIO_PinPupdControl=GPIO_PIN_NOPUPD;
	SPIpins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_OP_SPEED_HIGH;

	//1. SCLK->PA5
	SPIpins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_5;
	GPIO_Init(&SPIpins);
	//2. MISO->PA6
	SPIpins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_6;
	GPIO_Init(&SPIpins);
	//3. MOSI->PA7
	SPIpins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_7;
	GPIO_Init(&SPIpins);
	//4. NSS->PA4
	SPIpins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_4;
	GPIO_Init(&SPIpins);
}

void SPI1_Init(void)
{

	SPI1Handle.pSPIx=SPI1;
	SPI1Handle.SPI_Config.SPI_DeviceMode=SPI_DEVICE_MODE_MASTER;
	SPI1Handle.SPI_Config.SPI_BusConfig=SPI_BUS_CONFIG_FD;
	SPI1Handle.SPI_Config.SPI_DFF=SPI_DFF_8BITS;
	SPI1Handle.SPI_Config.SPI_SclkSpeed=SPI_SCLK_SPEED_DIV64;
	SPI1Handle.SPI_Config.SPI_CPOL=SPI_CPOL_LOW;
	SPI1Handle.SPI_Config.SPI_CPHA=SPI_CPHA_LOW;
	SPI1Handle.SPI_Config.SPI_SSM=SPI_SSM_HW;
	SPI_Init(&SPI1Handle);
}



int main(void)
{
	//1. Initialize button
	GPIO_Button_Init();
	//SPI GPIO pins
	SPI1_GPIOInit();
	//intialize SPI1 peripheral regisiters
	SPI1_Init();
	//making nss internally High via SSI config
	SPI_SSOEConfig(SPI1, ENABLE);

	//config of SPI1 IRQ in NVIC
	SPI_IRQInteruptConfig(IRQ_NO_SPI1, ENABLE);
	SPI_IRQPriorityConfig(IRQ_NO_SPI1, 15);
	// enable the SPI1 peripheral
	SPI_PeripheralControl(SPI1,ENABLE);
// master transmitting dummy 0xff bytes to keep clock running
	memset(dummy_tx,0xff,RX_LEN);
	while(1)
	{
		//read input from the button is pressed
		while(GPIO_ReadfromIpin(GPIOC, GPIO_PIN_NUM_13)==1);
		delay();
		while(GPIO_ReadfromIpin(GPIOC, GPIO_PIN_NUM_13)==0);
		delay();

		// clearing rx data
		uint8_t dummy;
		while(SPI_GetFlagStatus(SPI1, SPI_RXNE_FLAG)==FLAG_SET)
		{
			dummy=SPI1->DR;
		}
		(void)dummy;
		SPI_ClearOVRFlag(SPI1);

		// clear rx buffer
		memset(rx_buffer,0,RX_LEN);
		spi_rx_cmplt=0;
		spi_tx_cmplt=0;
		while(SPI_ReceiveDataIT(&SPI1Handle, rx_buffer, RX_LEN)!=SPI_READY);
		// sent dummy bites to generate clock pulses from master
		while(SPI_SendDataIT(&SPI1Handle, dummy_tx,RX_LEN)!=SPI_READY);

		//wait until both int transmission complete
		while(spi_rx_cmplt==0);
		spi_rx_cmplt=0;

		while(spi_tx_cmplt==0);
		spi_tx_cmplt=0;


		__asm("NOP");


	}
	return 0;

}
