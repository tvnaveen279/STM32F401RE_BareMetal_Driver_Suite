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
		pI2Cx->CR1|=(1<<I2C_CR1_ACK);

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
		// by default the ccr is in std mode so no need to set it
		ccr_val=(pclk1/(2*pI2CHandle->I2C_Config.I2C_SCLSpeed));
		tempreg|=(ccr_val&0x0fff);// masking the remaining bits other than ccr 12 bits and storing into ccr register

	}
	else
	{
		//fast mode
		tempreg|=(1<<I2C_CCR_FS);// setting the 15th bit of ccr in fast mode
		tempreg|=(pI2CHandle->I2C_Config.I2C_FMDutyCycle<<I2C_CCR_DUTY);// setting the duty cycle of ccr
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

void I2C_SendData(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer,uint32_t Len,uint8_t Slaveaddr,uint8_t Sr)
{
	pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_START);// start condition
	while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_SB));// checking start condition by using SB flag


		Slaveaddr=Slaveaddr<<1;
		Slaveaddr&=~(1);//bit 0 is 0 for write
		pI2CHandle->pI2Cx->DR=Slaveaddr;
	while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));
	 uint32_t DummyRead=pI2CHandle->pI2Cx->SR1;
	 DummyRead=pI2CHandle->pI2Cx->SR2;
	 (void)DummyRead;
	 while(Len>0)
	 {
		 while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));

		 pI2CHandle->pI2Cx->DR=*pTxBuffer;
		 pTxBuffer++;
		 Len--;

	 }
	 // checking both txe flag and bty flag to ensure that all data is tarnsfered from data register and shift register;
	 while (! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));
	 while (! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF));
	 // generate stop condition
	 if(Sr==DISABLE)
	 {
		 pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_STOP);
	 }



}

void I2C_ReceiveData(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer,uint32_t Len,uint8_t SlaveAddr,uint8_t Sr)
{
	pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_START);
	while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

	SlaveAddr=SlaveAddr<<1;
	SlaveAddr|=1;//bit 0 is 1 for read
	pI2CHandle->pI2Cx->DR=SlaveAddr;
	while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));

	if(Len==1)
	{
		pI2CHandle->pI2Cx->CR1 &=~(1<<10);//disabling ack before byte receive


		// reading sr1 and sr2 to clear addr flag
		uint32_t dummyRead=pI2CHandle->pI2Cx->SR1;
		dummyRead=pI2CHandle->pI2Cx->SR2;
		(void)dummyRead;


		//setting stop condition

		if(Sr==DISABLE)
		{
			pI2CHandle->pI2Cx->CR1 |=(1<<9);
		}
		//waiting for RXNE flag
		while(! I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_RXNE));

		//reading byte from DR
		*pRxBuffer=(uint8_t)pI2CHandle->pI2Cx->DR;
	}
	if(Len >1)
	{
		//clear addr flag
		uint32_t dummyRead=pI2CHandle->pI2Cx->SR1;
				dummyRead=pI2CHandle->pI2Cx->SR2;
				(void)dummyRead;
		for(uint32_t i=Len;i>0;i--)
		{
			//wait until RXNE=1
			while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));
			if(i==2)
			{
				//disabling acking
				pI2CHandle->pI2Cx->CR1 &= ~(1<<10);
				//generate stop condition
				if(Sr==DISABLE)
				{
					pI2CHandle->pI2Cx->CR1 |=(1<<9);
				}
			}
			//Read byte from data register
			*pRxBuffer=(uint8_t)pI2CHandle->pI2Cx->DR;
			pRxBuffer++;
		}

	}
	//REenabling ack for next transaction
	if(pI2CHandle->I2C_Config.I2C_ACKControl==I2C_ACK_ENABLE)
	{
		pI2CHandle->pI2Cx->CR1|=(1<<10);
	}
}



uint8_t	I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer,uint32_t	Len,uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate=pI2CHandle->TxRxState;
	if((busystate != I2C_BUSY_IN_TX)&&(busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer=pTxBuffer;
		pI2CHandle->TxLen=Len;
		pI2CHandle->TxRxState=I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr=SlaveAddr;
		pI2CHandle->Sr=Sr;
		//enable ITBUFEN
		pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITBUFEN);
		//eneable ITEVTEN
		pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITEVTEN);
		//enable ITERREN
		pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITERREN);
		//generate start condition
		pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_START);
	}
	return busystate;
}

uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer,uint32_t	Len,uint8_t SlaveAddr,uint8_t Sr)
{
	uint8_t busystate=pI2CHandle->TxRxState;
		if((busystate != I2C_BUSY_IN_TX)&&(busystate != I2C_BUSY_IN_RX))
		{
			pI2CHandle->pRxBuffer=pRxBuffer;
			pI2CHandle->RxLen=Len;
			pI2CHandle->RxSize=Len;
			pI2CHandle->TxRxState=I2C_BUSY_IN_RX;
			pI2CHandle->DevAddr=SlaveAddr;
			pI2CHandle->Sr=Sr;

			//enable ITBUFEN
			pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITBUFEN);
			//eneable ITEVTEN
			pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITEVTEN);
			//enable ITERREN
			pI2CHandle->pI2Cx->CR2|=(1<<I2C_CR2_ITERREN);
			//generate start condition
			pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_START);
		}
		return busystate;
}

void I2C_EV_IRQHandler(I2C_Handle_t *pI2CHandle)
{
	uint32_t temp1,temp2,temp3;
	//check for interrupt enable bits in CR2
	temp1 = (pI2CHandle->pI2Cx->CR2 & (1<<I2C_CR2_ITEVTEN));
	temp2 = (pI2CHandle->pI2Cx->CR2 & (1<<I2C_CR2_ITBUFEN));

	//1. start bit (SB) event
	temp3 = (pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_SB) );

	if(temp1 && temp3)
	{
		volatile uint32_t dummy =pI2CHandle->pI2Cx->SR1;
		(void)dummy;
		//sb flag is set :send slave addr with write bit 0
		if(pI2CHandle->TxRxState==I2C_BUSY_IN_TX)
		{
			uint8_t addr=(pI2CHandle->DevAddr<<1);
			addr &= ~(1);//clear 0th bit for write
			pI2CHandle->pI2Cx->DR=addr;

		}
		else if(pI2CHandle->TxRxState==I2C_BUSY_IN_RX)
		{
			uint8_t addr=(pI2CHandle->DevAddr<<1);
			addr |=(1);
			pI2CHandle->pI2Cx->DR=addr;
		}
	}
	//2. addr event
	temp3=(pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_ADDR));
	if(temp1 && temp3)
	{
		// addr flag is set:clear it by reading SR1 following SR2
		volatile uint32_t dummy=pI2CHandle->pI2Cx->SR1;
		dummy=pI2CHandle->pI2Cx->SR2;
		(void)dummy;
	}
	//3.TXE event
	temp3 =(pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_TXE));
	if (temp1 && temp2 && temp3)
	{
		// check if device is in master TX mode
		if(pI2CHandle->pI2Cx->SR2 &(1<<I2C_SR2_MSL))
		{
			if(pI2CHandle->TxRxState==I2C_BUSY_IN_TX)
			{
				if(pI2CHandle->TxLen>0)
				{
					//write byte to DR
					pI2CHandle->pI2Cx->DR=*(pI2CHandle->pTxBuffer);
					pI2CHandle->TxLen--;
					pI2CHandle->pTxBuffer++;
				}else
				{
					pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITBUFEN);
				}
			}
		}
	}

	// 4. RXNE EVENT
	temp3 = (pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_RXNE));
	if(temp1 && temp2 && temp3)
	{
		//check if device in rx mode
		if(pI2CHandle->pI2Cx->SR2 &(1<<I2C_SR2_MSL))
		{
			if(pI2CHandle->TxRxState==I2C_BUSY_IN_RX)
			{
				//total receiption size is 1  byte
				if(pI2CHandle->RxSize==1)
				{
					*pI2CHandle->pRxBuffer=(uint8_t)pI2CHandle->pI2Cx->DR;
					pI2CHandle->RxLen--;

				}
				//case 2: multi byte reception
				else if(pI2CHandle->RxSize>1)
				{
					if(pI2CHandle->RxLen==2)
					{
						//disable ack before receiving the last byte
						pI2CHandle->pI2Cx->CR1 &= ~(1<<I2C_CR1_ACK);
						//generate stop condition if Sr is not requested
						if(pI2CHandle->Sr==DISABLE)
						{
							pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_STOP);
						}
					}
					//ready byte from dr
					*pI2CHandle->pRxBuffer=(uint8_t)pI2CHandle->pI2Cx->DR;
					pI2CHandle->pRxBuffer++;
					pI2CHandle->RxLen--;
				}
				//check if reception is completed
				if(pI2CHandle->RxLen==0)
				{
					//for 1 byte case
					if(pI2CHandle->RxSize==1)
					{
						if(pI2CHandle->Sr==DISABLE)
						{
							pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_STOP);
						}
					}
					//disable all interrupts
					pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITBUFEN);
					pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITEVTEN);
					//3. reset state
					pI2CHandle->TxRxState=I2C_READY;
					pI2CHandle->pRxBuffer= 0;
					pI2CHandle->RxLen=0;
					pI2CHandle->RxSize=0;
					//re enable ack for future transactions
					if(pI2CHandle->I2C_Config.I2C_ACKControl==I2C_ACK_ENABLE)
					{
						pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_ACK);
					}
					//notify application
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
				}
			}
		}
	}
	//5. BTF event
	temp3=(pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_BTF));
	if(temp1 && temp3)
	{
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			if((pI2CHandle->TxLen==0))
			{
				//generate stop condition
				if(pI2CHandle->Sr==DISABLE)
				{
					pI2CHandle->pI2Cx->CR1 |=(1<<I2C_CR1_STOP);

				}
				//disable interrupts
				pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITBUFEN);
				pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITEVTEN);
				//3. reset state
				pI2CHandle->TxRxState=I2C_READY;
				pI2CHandle->pTxBuffer= 0;
				pI2CHandle->TxLen=0;
				//4. notify application
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);

			}
		}
	}
}

void I2C_ER_IRQHandler(I2C_Handle_t *pI2CHandle)
{
	uint32_t temp1,temp2;
	//check ITERREN bit in cr2
	temp1=(pI2CHandle->pI2Cx->CR2 & (1<<I2C_CR2_ITERREN));
	//check AF(ack failure) flag in SR1
	temp2=(pI2CHandle->pI2Cx->SR1 & (1<<I2C_SR1_AF));
	if(temp1 && temp2)
	{
		//1.clear AF flag( write 0 to bit 10 in SR1)
		pI2CHandle->pI2Cx->SR1 &= ~(1<<I2C_SR1_AF);
		//2.generate stop condition
		pI2CHandle->pI2Cx->CR1|=(1<<I2C_CR1_STOP);
		//disable interrupts
		pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITBUFEN);
		pI2CHandle->pI2Cx->CR2 &= ~(1<<I2C_CR2_ITEVTEN);
		//reset state
		pI2CHandle->TxRxState=I2C_READY;
		//5. notify app error
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);


	}
}


/*********************************************************************
 * @fn      		  - I2C_IRQInterruptConfig
 * @brief             - Enables or disables the given IRQ in NVIC
 * @param[in]         - IRQNumber
 * @param[in]         - EnorDi (ENABLE or DISABLE)
 *********************************************************************/
void I2C_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
    if(EnorDi == ENABLE)
    {
        if(IRQNumber <= 31)
        {
            // ISER0 register
            *NVIC_ISER0 |= (1 << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64)
        {
            // ISER1 register
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96)
        {
            // ISER2 register
            *NVIC_ISER2 |= (1 << (IRQNumber % 64));
        }
    }
    else
    {
        if(IRQNumber <= 31)
        {
            // ICER0 register
            *NVIC_ICER0 |= (1 << IRQNumber);
        }
        else if(IRQNumber > 31 && IRQNumber < 64)
        {
            // ICER1 register
            *NVIC_ICER1 |= (1 << (IRQNumber % 32));
        }
        else if(IRQNumber >= 64 && IRQNumber < 96)
        {
            // ICER2 register
            *NVIC_ICER2 |= (1 << (IRQNumber % 64));
        }
    }
}

/*********************************************************************
 * @fn      		  - I2C_IRQPriorityConfig
 * @brief             - Configures the priority of the given IRQ in NVIC
 * @param[in]         - IRQNumber
 * @param[in]         - IRQPriority
 *********************************************************************/
void I2C_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	uint8_t iprx=IRQNumber/4;
		uint8_t ipr_section=IRQNumber%4;
		uint8_t shiftamount=(8*ipr_section)+(8-NO_PR_BITS_IMPLEMENTED);
		*(NVIC_PR_BASEADDR+(iprx)) |=(IRQPriority<<(shiftamount));
}
