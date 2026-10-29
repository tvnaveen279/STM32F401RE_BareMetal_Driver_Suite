#include "stm32f401xx_spi_drivers.h"
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
