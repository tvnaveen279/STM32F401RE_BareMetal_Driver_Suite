#include "stm32f401xx_spi_drivers.h"
#include "stm32f401xx.h"
// private helper function prototypes
 void SPI_TXE_INT_HANDLE(SPI_Handle_t *pSPIHandle);
 void SPI_RXNE_INT_HANDLE(SPI_Handle_t *pSPIHandle);
 void SPI_OVR_ERR_INT_HANDLE(SPI_Handle_t *pSPIHandle);





/*********************************************************************
 * @fn      		  - SPI_PeriClockControl
 * @brief             - Enables or disables peripheral clock for the given SPI port
 * @param[in]         - Base address of the SPI peripheral
 * @param[in]         - ENABLE or DISABLE macros
 * @return            - none
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		if(pSPIx==SPI1)
		{
			SPI1_PCLK_EN();
		}else if(pSPIx==SPI2)
		{
			SPI2_PCLK_EN();
		}else if(pSPIx==SPI3)
		{
			SPI3_PCLK_EN();
		}else if(pSPIx==SPI4)
		{
			SPI4_PCLK_EN();

		}
	}else
	{
		if(EnorDi==DISABLE)
			{
				if(pSPIx==SPI1)
				{
					SPI1_PCLK_DI();
				}else if(pSPIx==SPI2)
				{
					SPI2_PCLK_DI();
				}else if(pSPIx==SPI3)
				{
					SPI3_PCLK_DI();
				}else if(pSPIx==SPI4)
				{
					SPI4_PCLK_DI();

				}
			}
	}
}

uint8_t  SPI_GetFlagStatus(SPI_RegDef_t *pSPIx,uint32_t FlagName)
{
	if(pSPIx->SR&FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

	/*********************************************************************
	 * @fn      		  - SPI_Init
	 * @brief             - Initializes the SPI peripheral registers based on user config
	 * @param[in]         - Handle structure containing base address and configs
	 * @return            - none
	 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	//0. peripheral clock enable
	SPI_PeriClockControl(pSPIHandle->pSPIx, ENABLE);
	uint32_t tempreg=0;
	//1. config device mode->master/slave
	tempreg|=(pSPIHandle->SPI_Config.SPI_DeviceMode<<SPI_CR1_MSTR);
	//2. bus configuration
	if(pSPIHandle->SPI_Config.SPI_BusConfig==SPI_BUS_CONFIG_FD)
	{
		tempreg &=~(1<<SPI_CR1_BIDIMODE);
	}
	else if(pSPIHandle->SPI_Config.SPI_BusConfig==SPI_BUS_CONFIG_HD)
	{
		tempreg|=(1<<SPI_CR1_BIDIMODE);
	}
	else if (pSPIHandle->SPI_Config.SPI_BusConfig==SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		tempreg &=~(1<<SPI_CR1_BIDIMODE);
		tempreg|=(1<<SPI_CR1_RXONLY);
	}
	//3. config of serial clk speed
	tempreg|=(pSPIHandle->SPI_Config.SPI_SclkSpeed<<SPI_CR1_BR);
	//4. config of data fram format
	tempreg|=(pSPIHandle->SPI_Config.SPI_DFF<<SPI_CR1_DFF);
	//5. clk polarity
	tempreg|=(pSPIHandle->SPI_Config.SPI_CPOL<<SPI_CR1_CPOL);
	//6. clk phase
	tempreg|=(pSPIHandle->SPI_Config.SPI_CPHA<<SPI_CR1_CPHA);
	//7. config of software slave management
	tempreg|=(pSPIHandle->SPI_Config.SPI_SSM<<SPI_CR1_SSM);

	pSPIHandle->pSPIx->CR1=tempreg;


}
/*********************************************************************
 * @fn      		  - SPI_SendData
 * @brief             - transmit  data over SPI (Blocking / Polling)
 *********************************************************************/
void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *pTxBuffer,uint32_t Len)
{
	while(Len>0)
	{
		//get txe flag status check
		while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG)==FLAG_RESET);

		if(pSPIx->CR1&(1<<SPI_CR1_DFF))
		{
			//  16 bit dff:load 2 bytes into DR
			pSPIx->DR=*((uint16_t*)pTxBuffer);
			Len-=2;
			pTxBuffer+=2;
		}
		else
		{
			// 8 bit dff:load 1 bit into DR
			pSPIx->DR=*pTxBuffer;
			Len--;
			pTxBuffer++;
		}
	}
}
/*********************************************************************
 * @fn      		  - SPI_ReceiveData
 * @brief             - Receives data over SPI (Blocking / Polling)
 *********************************************************************/
void SPI_ReceiveData(SPI_RegDef_t *pSPIx,uint8_t *pRxBuffer,uint32_t Len)
{
	while(Len>0)
	{
		//1. wait until RXNE(Receive buffer not empty) is set
		while(SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG)==FLAG_RESET);
		//check dff bit in CR1
		if(pSPIx->CR1&(1<<SPI_CR1_DFF))
		{
			*((uint16_t*)pRxBuffer)=(uint16_t)pSPIx->DR;
			Len-=2;
			pRxBuffer+=2;


		}
		else
		{
			*pRxBuffer=(uint8_t)pSPIx->DR;
			Len--;
			pRxBuffer++;
		}
	}
}
/*********************************************************************
 * @fn      		  - SPI_PeripheralControl
 * @brief             - Enables or Disables  SPI peripheral
 *********************************************************************/
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pSPIx->CR1|=(1<<SPI_CR1_SPE);

	}else
	{
		pSPIx->CR1&= ~(1<<SPI_CR1_SPE);
	}
}
/*********************************************************************
 * @fn      		  - SPI_SSIConfig
 * @brief             - sets or clears SSI bit in CR1
 *********************************************************************/
void SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		pSPIx->CR1|=(1<<SPI_CR1_SSI);

	}
	else
	{
		pSPIx->CR1 &=~(1<<SPI_CR1_SSI);
	}
}

/*********************************************************************
 * @fn      		  - SPI_SSOEConfig
 * @brief             -enables or disables SSOE bit
 *********************************************************************/
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pSPIx->CR2|=(1<<SPI_CR2_SSOE);
	}
	else
	{
		pSPIx->CR2 &=~(1<<SPI_CR2_SSOE);
	}
}

// SPI_IRQINTERRUPT CONFIG
void SPI_IRQInteruptConfig(uint8_t IRQNumber,uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if(IRQNumber <= 31)
		{
			//program ISER0 register
			*NVIC_ISER0 |=(1<<IRQNumber);
		}
		else if( IRQNumber >31 && IRQNumber <64)
		{
			// program for ISRE1 register
			*NVIC_ISER1 |=(1<<(IRQNumber%32));

		}else if( IRQNumber >=64 && IRQNumber <96)
		{
			// program for ISER2 register
			*NVIC_ISER2 |=(1<<(IRQNumber%64));
		}
	}else
	{
		if (EnorDi == DISABLE)
			{
				if(IRQNumber <= 31)
				{
					//program ICER0 register
					*NVIC_ICER0 |=(1<<(IRQNumber));

				}
				else if( IRQNumber >31 && IRQNumber <64)
				{
					// program for ICRE1 register
					*NVIC_ICER1 |=(1<<(IRQNumber%32));

				}else if( IRQNumber >=64 && IRQNumber <96)
				{
					// program for ICER2 register
					*NVIC_ICER2 |=(1<<(IRQNumber%64));
				}
	 }
	}

}

// SPI_IRQPRIORITY
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	uint8_t iprx=IRQNumber/4;
	uint8_t ipr_section=IRQNumber%4;
	uint8_t shiftamount=(8*ipr_section)+(8-NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASEADDR+(iprx*4)) |=(IRQPriority<<(shiftamount));
}

uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer, uint32_t Len)
{
	uint8_t state=pSPIHandle->TxState;
	if(state != SPI_BUSY_IN_TX)
	{

	 //1. save the Tx buffer address and Len info in some global variables
	pSPIHandle->pTxBuffer=pTxBuffer;
	pSPIHandle->TxLen=Len;
	//2. mark the spi state as busy in transmission so that no other code can take over until transmission complete
	pSPIHandle->TxState=SPI_BUSY_IN_TX;
	//3. Enable the TXEIE control bit to get interrupt whenever TXE flag is set in SR
	pSPIHandle->pSPIx->CR2|=(1<<SPI_CR2_TXEIE);
	}
	return state;
}

uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer, uint32_t Len)
{
	uint8_t state=pSPIHandle->RxState;
		if(state != SPI_BUSY_IN_RX)
		{

		 //1. save the rxNE buffer address and Len info in some global variables
		pSPIHandle->pRxBuffer=pRxBuffer;
		pSPIHandle->RxLen=Len;
		//2. mark the spi state as busy in transmission so that no other code can take over until transmission complete
		pSPIHandle->RxState=SPI_BUSY_IN_RX;
		//3. Enable the RXNEIE control bit to get interrupt whenever TXE flag is set in SR
		pSPIHandle->pSPIx->CR2|=(1<<SPI_CR2_RXNEIE);
		pSPIHandle->pSPIx->CR2|=(1<<SPI_CR2_ERRIE);
		}
		return state;
}

void SPI_IRQHandling(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp1,temp2;

	// check for RXNE
			temp1=pSPIHandle->pSPIx->SR&(1<<SPI_SR_RXNE);
			temp2=pSPIHandle->pSPIx->CR2 &(1<<SPI_CR2_RXNEIE);
			if(temp1 && temp2)
			{
				//handle RXNE
				SPI_RXNE_INT_HANDLE(pSPIHandle);

			}
	// check for TXE
	temp1=pSPIHandle->pSPIx->SR&(1<<SPI_SR_TXE);
	temp2=pSPIHandle->pSPIx->CR2 &(1<<SPI_CR2_TXEIE);
	if(temp1 && temp2)
	{
		//handle TXE
		SPI_TXE_INT_HANDLE(pSPIHandle);

	}

	// 3. check for OVR
			temp1=pSPIHandle->pSPIx->SR&(1<<SPI_SR_OVR);
			temp2=pSPIHandle->pSPIx->CR2 &(1<<SPI_CR2_ERRIE);
			if(temp1 && temp2)
			{
				//handle OVERR
				SPI_OVR_ERR_INT_HANDLE(pSPIHandle);

			}



}


void SPI_RXNE_INT_HANDLE(SPI_Handle_t *pSPIHandle)
{
	// check DFF and read data from DR to RX buffer
	if(pSPIHandle->SPI_Config.SPI_DFF==SPI_DFF_16BITS)
		{
			//16 bit data taransfer
			*((uint16_t*)pSPIHandle->pRxBuffer)=pSPIHandle->pSPIx->DR;
			pSPIHandle->RxLen-=2;
			pSPIHandle->pRxBuffer+=2;
		}
		else
		{
			//8 bit data taransfer
			*( pSPIHandle->pRxBuffer)=pSPIHandle->pSPIx->DR;
			pSPIHandle->RxLen--;
			pSPIHandle->pRxBuffer++;
		}
		// check RxLen 0
		if(! pSPIHandle->RxLen)
		{
			// disable RXNEIE bit
			pSPIHandle->pSPIx->CR2 &= ~(1<<SPI_CR2_RXNEIE);
			pSPIHandle->pSPIx->CR2&=~(1<<SPI_CR2_ERRIE);

			// reset the buffer pointers and state
			pSPIHandle->pRxBuffer=0 ;
			pSPIHandle->RxLen=0;
			pSPIHandle->RxState=SPI_READY;
			SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_RX_CMPLT);
			}
		}


void SPI_TXE_INT_HANDLE(SPI_Handle_t *pSPIHandle)
{
	// check DFF (8 bit or 16 bit)
	if(pSPIHandle->SPI_Config.SPI_DFF==SPI_DFF_16BITS)
	{
		//16 bit data taransfer
		pSPIHandle->pSPIx->DR=*((uint16_t*)pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen-=2;
		pSPIHandle->pTxBuffer+=2;
	}
	else
	{
		//8 bit data taransfer
		pSPIHandle->pSPIx->DR=*( pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}
	// check TxLen 0
	if(! pSPIHandle->TxLen)
	{
		// disable TXEIE bit
		pSPIHandle->pSPIx->CR2 &= ~(1<<SPI_CR2_TXEIE);

		// reset the buffer pointers and state
		pSPIHandle->pTxBuffer=0 ;
		pSPIHandle->TxLen=0;
		pSPIHandle->TxState=SPI_READY;
		SPI_ApplicationEventCallback(pSPIHandle,SPI_EVENT_TX_CMPLT);
	}

}

void SPI_OVR_ERR_INT_HANDLE(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp;
	//clear OVR flag only if driver is not busy
	if(pSPIHandle->TxState!=SPI_BUSY_IN_TX)
	{
		temp=pSPIHandle->pSPIx->DR;
		temp=pSPIHandle->pSPIx->SR;
		(void)temp;


	}
	//inform the application about error
	SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	uint8_t temp;
	temp=pSPIx->DR;
	temp=pSPIx->SR;
	(void)temp;
}
__attribute__((weak)) void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    // Default dummy implementation (gets overridden by main.c)
}
