/*
 * pb6 as scl
 * pb7 as SDA
 * I2C2 is suing
*/
#include"stm32f401xx_i2c_drivers.h"
#include"stm32f401xx.h"
#include<string.h>
#include<stdio.h>
#include"stm32f401xx_gpio_drivers.h"
#define	MY_ADDR		0X61
#define SLAVE_ADDR	0x68
I2C_Handle_t I2C1handle;

#define RX_LEN	23
uint8_t	rx_buffer[32];
volatile uint8_t rx_cmplt_flag=0;
//Test messege to send
char msg[]="Testing I2C1 INT on PB6 & PB7!";
volatile uint8_t tx_cmplt_flag=0;

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
    btn.GPIO_PinConfig.GPIO_PinPupdControl = GPIO_PIN_NOPUPD; // External pull-up already present on board
    GPIO_Init(&btn);
}
void GPIO_Led_Init(void)
{
    GPIOA_PCLK_EN();
    GPIO_Handle_t led;
    led.pGPIOx = GPIOA;
    led.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NUM_5;
    led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_OP_SPEED_HIGH;
    led.GPIO_PinConfig.GPIO_OPType = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPupdControl = GPIO_PIN_NOPUPD;
    GPIO_Init(&led);
}
/*
 * I2C GPIO_Init
 */
void I2C1_GPIOInit(void)
{
	GPIOB_PCLK_EN();
	GPIO_Handle_t I2C_pins;
	I2C_pins.pGPIOx=GPIOB;
	I2C_pins.GPIO_PinConfig.GPIO_PinMode=GPIO_MODE_ALTFN;
	I2C_pins.GPIO_PinConfig.GPIO_OPType=GPIO_OP_TYPE_OD;
	I2C_pins.GPIO_PinConfig.GPIO_PinPupdControl=GPIO_PIN_UP;
	I2C_pins.GPIO_PinConfig.GPIO_PinSpeed=GPIO_OP_SPEED_HIGH;
	I2C_pins.GPIO_PinConfig.GPIO_AltFunMode=4;
	//1. Config Pb6 as scl ->AF4
	I2C_pins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_6;

	GPIO_Init(&I2C_pins);
	//2. config pb7 as sda ->Af4
	I2C_pins.GPIO_PinConfig.GPIO_PinNumber=GPIO_PIN_NUM_7;
	GPIO_Init(&I2C_pins);

}
/*
 * I2C Init
 */
void I2C1_Init(void)
{
	I2C1handle.pI2Cx=I2C1;
	I2C1handle.I2C_Config.I2C_ACKControl=I2C_ACK_ENABLE;
	I2C1handle.I2C_Config.I2C_DeviceAddress=MY_ADDR;
	I2C1handle.I2C_Config.I2C_FMDutyCycle=I2C_FMDUTY_2;
	I2C1handle.I2C_Config.I2C_SCLSpeed=I2C_SCLSPEED_SM;
	I2C_Init(&I2C1handle);



}
int main(void)
{
	GPIO_Led_Init();
	GPIO_Button_Init();
	//i2c gpio init
	I2C1_GPIOInit();
	//i2c init
	I2C1_Init();

	//NVIC interrupt config
	I2C_IRQPriorityConfig(IRQ_NO_I2C1_EV, 15);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_EV, ENABLE);
	I2C_IRQPriorityConfig(IRQ_NO_I2C1_ER, 15);
	I2C_IRQInterruptConfig(IRQ_NO_I2C1_ER, ENABLE);

	// ENABLE I2C PERIPHERAL
		I2C_PeripheralControl(I2C1, ENABLE);
	delay();

	while(1)
	{
		 //wiat until button is pressed
		while( GPIO_ReadfromIpin(GPIOC,GPIO_PIN_NUM_13)==1);

		// to avoid button debouncing
		delay();
		while( GPIO_ReadfromIpin(GPIOC,GPIO_PIN_NUM_13)==0);

		GPIO_writetoOpin(GPIOA, GPIO_PIN_NUM_5	, GPIO_PIN_SET);
		delay();
		tx_cmplt_flag=0;
		I2C_MasterSendDataIT(&I2C1handle, (uint8_t*)msg	, strlen(msg), SLAVE_ADDR,DISABLE);
		while(!tx_cmplt_flag);
		tx_cmplt_flag=0;
		// delay between transsmission
		for(volatile int i =0;i<5000;i++);
		memset(rx_buffer,0,sizeof(rx_buffer));
		rx_cmplt_flag=0;
		I2C_MasterReceiveDataIT(&I2C1handle, rx_buffer, RX_LEN, SLAVE_ADDR, DISABLE);

		//wait until RX is complete
		while(!rx_cmplt_flag);
		rx_cmplt_flag=0;

		rx_buffer[RX_LEN]='\0';
		GPIO_writetoOpin(GPIOA, GPIO_PIN_NUM_5, GPIO_PIN_RESET);

		//INTER TRANSACTION DELAY
		for(int k=0;k<3;k++)
		{
			delay();
		}

	}
	return 0;
}

	//i2c1 event int handler
	void I2C1_EV_IRQHandler(void)
	{
		I2C_EV_IRQHandler(&I2C1handle);
	}
	void I2C1_ER_IRQHandler(void)
	{
		I2C_ER_IRQHandler(&I2C1handle);
	}
	void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv )
	{
		if(AppEv== I2C_EV_TX_CMPLT)
		{
			tx_cmplt_flag=1;
		}
		else if(AppEv==I2C_EV_RX_CMPLT)
		{
			rx_cmplt_flag=1;
		}
		else if(AppEv==I2C_ERROR_AF)
		{
			tx_cmplt_flag=1;
			rx_cmplt_flag=1;
		}
	}








