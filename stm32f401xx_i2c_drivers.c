#include"stm32f401xx_i2c_drivers.h"
#include"stm32f401xx.h"


const uint16_t AHB_PreScaler[8]={2,4,8,16,64,128,256,512};
const uint8_t APB1_PreScaler[4]={2,4,8,16};
uint32_t RCC_GetPCLK1Value(void)
{
	uint32_t pclk1,systemclk=0;
	uint8_t clksrc,temp,ahbp,apb1p;
	//1. checking system clock source (RCC_CFGR)
	clksrc=((RCC->CFGR>>2)&0x03);
	if(clksrc==0)
	{
		systemclk=16000000;//HSI=16mhz

	}
	else if(clksrc==1)
	{
		systemclk=8000000;//Hse 8mhz
	}
	//2. ahb prescalear checking(RCC_CFGR bits[7:4] )
	temp=((RCC->CFGR>>4)&0x0f);
	if(temp<8)
	{
		ahbp=1;

	}
	else
	{
		ahbp=AHB_PreScaler[temp-8];
	}
	//3.APB! Prescaler checking(RCC_CFGR[12:10])
	temp = ((RCC->CFGR >> 10) & 0x07);
	    if(temp < 4)
	    {
	        apb1p = 1; // No division
	    }
	    else
	    {
	        apb1p = APB1_PreScaler[temp - 4];
	    }
	    //final pclk frequency
	    pclk1=(systemclk/ahbp)/apb1p;
	    return pclk1;
}
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		if(pI2Cx==I2C1)
		{
			I2C1_PCLK_EN();
		}
		else if(pI2Cx==I2C2)
		{
			I2C2_PCLK_EN();
		}
		else if(pI2Cx==I2C3)
		{
			I2C3_PCLK_EN();
		}

	}
	else if(EnorDi==DISABLE)
	{
			if(pI2Cx==I2C1)
				{
					I2C1_PCLK_DI();
				}
				else if(pI2Cx==I2C2)
				{
					I2C2_PCLK_DI();
				}
				else if(pI2Cx==I2C3)
				{
					I2C3_PCLK_DI();
				}
	}
}

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pI2Cx->CR1 |=(1<<I2C_CR1_PE);

	}
	else
	{
		pI2Cx->CR1&=~(1<<I2C_CR1_PE);
	}
}
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if(pI2Cx->SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}


void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempreg=0;
	uint16_t ccr_val=0;
	uint32_t pclk1=RCC_GetPCLK1Value();
	//0. enable peripheral clock
	I2C_PeriClockControl(pI2CHandle->pI2Cx,ENABLE);
	//1.CR1: ACK Configuration
	tempreg |= (pI2CHandle->I2C_Config.I2C_ACKControl<<I2C_CR1_ACK);
	pI2CHandle->pI2Cx->CR1=tempreg;
	//2.CR2: FREQ field setup (Assume 16 MHz HSI default)
	tempreg=0;
	tempreg|=((pclk1/1000000U)&0x3f);
	pI2CHandle->pI2Cx->CR2=tempreg;
	//3. OAR1: OWN DEVICE ADDRESS(7-bit)
	tempreg=0;
	tempreg|=(pI2CHandle->I2C_Config.I2C_DeviceAddress<<1);
	tempreg|=(1<<14);//reserved bit
	pI2CHandle->pI2Cx->OAR1=tempreg;
	//CCR: Clock Control Calculation
	tempreg=0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed<=I2C_SCLSPEED_SM)
	{
		// stand mode(100khz)
		ccr_val=(pclk1/(2*pI2CHandle->I2C_Config.I2C_SCLSpeed));
		tempreg|=(ccr_val&0x0fff);
	}
	else
	{
		//fast mode
		tempreg|=(1<<I2C_CCR_FS);
		tempreg|=(pI2CHandle->I2C_Config.I2C_FMDutyCycle<<I2C_CCR_DUTY);
		if(pI2CHandle->I2C_Config.I2C_FMDutyCycle==I2C_FMDUTY_2)
		{
			ccr_val=(pclk1/(3*pI2CHandle->I2C_Config.I2C_SCLSpeed));

		}
		else
		{
			ccr_val=(pclk1/(25*pI2CHandle->I2C_Config.I2C_SCLSpeed));
		}
		tempreg|=(ccr_val&0x0fff);

	}
	pI2CHandle->pI2Cx->CCR=tempreg;
	//5. TRISE: Maximum Rise time
	tempreg=0;
	if(pI2CHandle->I2C_Config.I2C_SCLSpeed<=I2C_SCLSPEED_SM)
	{
		//std mode 1000ns
		tempreg|=((pclk1/1000000U)*1)+1;

	}
	else
	{
		tempreg=(((pclk1/1000000U)*300)/1000)+1;
	}
	pI2CHandle->pI2Cx->TRISE=(tempreg&0x03f);


}
