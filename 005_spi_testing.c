#include "stm32f401xx.h"
#include "stm32f401xx_spi_drivers.h"
#include<string.h>
#include"stm32f401xx_gpio_drivers.h"

#define CMD_LED_ON 		0x50
#define CMD_LED_OFF		0x51
#define CMD_READ_DATA	0x52

#define DUMMY_DATA		0xFF

uint8_t rx_ack=0;
uint8_t sensor_data=0;

//char msg[]="hello spi !";

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
	SPI_Handle_t SPI1Handle;
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

	while(1)
	{
		//read input from the button is pressed
		while(GPIO_ReadfromIpin(GPIOC, GPIO_PIN_NUM_13)==1);
		delay();
		while(GPIO_ReadfromIpin(GPIOC, GPIO_PIN_NUM_13)==0);
		delay();
		// enable the SPI1 peripheral
		SPI_PeripheralControl(SPI1,ENABLE);

		// send command to read data
		uint8_t cmd=CMD_READ_DATA;
		uint8_t dummy_rx;
		//send data
		SPI_SendData(SPI1, &cmd, 1);
		// read dummy data byte out of RX buffer to clear RXNE flag
		SPI_ReceiveData(SPI1, &dummy_rx, 1);
		//short pulse to allow arduino ISR to load data into SPDR
		for(uint32_t i=0;i<1000;i++);
		//send dummy byte to read actual response
		uint8_t dummy_tx=DUMMY_DATA;
		SPI_SendData(SPI1,&dummy_tx, 1);
		//receive actual sensor data
		SPI_ReceiveData(SPI1, &sensor_data, 1);
		//wait until data transmission complete
		while(SPI_GetFlagStatus(SPI1,SPI_TXE_FLAG)==FLAG_RESET);
		while(SPI_GetFlagStatus(SPI1,SPI_BSY_FLAG)==FLAG_SET);

		//disable spi1 peripheral
		SPI_PeripheralControl(SPI1, DISABLE);

	}

}
