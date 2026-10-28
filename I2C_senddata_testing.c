/*
 * pb10 as scl
 * pb9 as SDA
 * I2C2 is suing
*/
#include"stm32f401xx_i2c_drivers.h"
#include"stm32f401xx.h"
#include<string.h>
#include"stm32f401xx_gpio_drivers.h"

#define MY_ADDRR	0x61
#define SLAVE_ADDR	0x68
I2C_Handle_t I2C1handle;

//Test messege to send
char msg[]="Testing I2C1 on PB6 & PB7!";
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
	I2C1handle.I2C_Config.I2C_DeviceAddress=MY_ADDRR;
	I2C1handle.I2C_Config.I2C_FMDutyCycle=I2C_FMDUTY_2;
	I2C1handle.I2C_Config.I2C_SCLSpeed=I2C_SCLSPEED_SM;
	I2C_Init(&I2C1handle);



}
int main(void)
{
	GPIO_Button_Init();
	//i2c gpio init
	I2C1_GPIOInit();
	//i2c init
	I2C1_Init();
	// ENABLE I2C PERIPHERAL
	I2C_PeripheralControl(I2C1, ENABLE);
   while(1)
   {
	   //wiat until button is pressed
      while( GPIO_ReadfromIpin(GPIOC,GPIO_PIN_NUM_13)==1);

      // to avoid button debouncing
      delay();
      while( GPIO_ReadfromIpin(GPIOC,GPIO_PIN_NUM_13)==0);
      delay();
      I2C_SendData(&I2C1handle, (uint8_t*) msg, strlen(msg),SLAVE_ADDR, DISABLE);
      }
}
