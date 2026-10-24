#include "stm32f401xx_usart_drivers.h"

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi)
{
	if(EnorDi == ENABLE)
	{
		if(pUSARTx == USART1)
		{
			USART1_PCLK_EN();
		}
		else if(pUSARTx == USART2)
		{
			USART2_PCLK_EN();
		}
		else if(pUSARTx == USART6)
		{
			USART6_PCLK_EN();
		}
	}
	else
	{
		if(pUSARTx == USART1)
		{
			USART1_PCLK_DI();
		}
		else if(pUSARTx == USART2)
		{
			USART2_PCLK_DI();
		}
		else if(pUSARTx == USART6)
		{
			USART6_PCLK_DI();
		}
	}
}

void USART_Init(USART_Handle_t *pUSARTHandle)
{
	uint32_t tempreg=0;

	USART_PeriClockControl(pUSARTHandle->pUSARTx,ENABLE);
	/******************************** Configuration of CR1 *********************************/
	//config of mode
	if(pUSARTHandle->USART_Config.USART_Mode==USART_MODE_ONLY_RX)
	{
	tempreg|=(1<<2);
	}
	else if(pUSARTHandle->USART_Config.USART_Mode==USART_MODE_ONLY_TX)
	{
		tempreg|=(1<<3);

	}
	else if(pUSARTHandle->USART_Config.USART_Mode==USART_MODE_ONLY_TXRX)
	{
		tempreg|=((1<<2)|(1<<3));
	}
	//config of wordlength
	tempreg|= (pUSARTHandle->USART_Config.USART_WordLength<<12);
	//config of parity control
	if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_EN_EVEN)
	{
		//enable parity control(PCE is bit 10,PS default is 0 for even
		tempreg|=(1<<10);
	}
	else if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_EN_ODD)
	{
		tempreg|=(1<<10);
		tempreg|=(1<<9);

	}
	pUSARTHandle->pUSARTx->CR1=tempreg;
	/******************************** Configuration of CR2 *********************************/
	tempreg=0;

	tempreg|=(pUSARTHandle->USART_Config.USART_NoOfStopBits<<12);
	pUSARTHandle->pUSARTx->CR2=tempreg;
	/******************************** Configuration of CR3 *********************************/

	tempreg=0;
	if(pUSARTHandle->USART_Config.USART_HWFlowControl==USART_HW_FLOW_CTRL_CTS)
	{
		tempreg|=(1<<9);

	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl==USART_HW_FLOW_CTRL_RTS)
	{
		tempreg|=(1<<8);

	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl==USART_HW_FLOW_CTRL_CTS_RTS)
	{
		tempreg|=(1<<9);
		tempreg|=(1<<8);

	}
	pUSARTHandle->pUSARTx->CR3= tempreg;

	/******************************** Configuration of BRR (Baudrate) *********************************/
	    USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.USART_BaudRate);
}
void USART_SetBaudRate(USART_RegDef_t *pUSARTx,uint32_t BaudRate)
{
	uint32_t PCLKx,usartdiv,M_part,F_part,tempreg=0;
	//1. get the peripheral clock frequency
	PCLKx=16000000U;
	//2.check the over8config(assuming standard OVER8=0->16x oversampling)
	if(pUSARTx->CR1&(1<<15))
	{
		//OVER=1(8x oversampling)
		usartdiv=((25*PCLKx)/(2*BaudRate));
	}
	else
	{
		usartdiv=((25*PCLKx)/(4*BaudRate));
	}
	//3. calculate Mantissa
	M_part=usartdiv/100;
	//place Matissa into tempreg(15:4)
	tempreg|=(M_part<<4);
	//calculate fraction
	F_part=(usartdiv-(M_part*100));
	if(pUSARTx->CR1&(1<<15))
	{
		F_part=(((F_part*8)+50)/100)&((uint8_t)0x07);

	}
	else
	{
		F_part=(((F_part*16)+50)/100)&((uint8_t)0x0F);
	}
	tempreg|=F_part;
	pUSARTx->BRR=tempreg;
}


/*********************************************************************
 * @fn      		  - USART_PeripheralControl
 *
 * @brief             - Enables or disables the USART peripheral (UE bit in CR1)
 *
 * @param[in]         - Base address of the USART peripheral
 * @param[in]         - ENABLE or DISABLE
 *
 * @return            - None
 */
void USART_PeripheralControl(USART_RegDef_t *pUSARTx,uint8_t EnorDi)
{
	if(EnorDi==ENABLE)
	{
		pUSARTx->CR1|=(1<<13);

	}
	else
	{
		pUSARTx->CR1&=~(1<<13);
	}
}

/*********************************************************************
 * @fn      		  - USART_GetFlagStatus
 *
 * @brief             - Reads flag status from USART_SR register
 *
 * @param[in]         - Base address of the USART peripheral
 * @param[in]         - FlagName (e.g., USART_FLAG_TXE, USART_FLAG_TC, etc.)
 *
 * @return            - FLAG_SET or FLAG_RESET
 */
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx,uint32_t FlagName)
{
	if(pUSARTx->SR&FlagName)
	{
		return FLAG_SET;

	}
	return FLAG_RESET;
}

/*********************************************************************
 * @fn      		  - USART_SendData
 *
 * @brief             - Transmits data over USART in blocking (polling) mode
 *
 * @param[in]         - Handle pointer
 * @param[in]         - Pointer to Tx buffer
 * @param[in]         - Length of data to transmit
 *
 * @return            - None
 */
void USART_SendData(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer,uint32_t Len)
{
	uint16_t *pdata;
	// loop over
	for(uint32_t i=0;i<Len;i++)
	{
		while(! USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TXE));

		if(pUSARTHandle->USART_Config.USART_WordLength==USART_WORDLEN_9BITS)
		{
			pdata=(uint16_t*)pTxBuffer;
			pUSARTHandle->pUSARTx->DR=(*pdata&(uint16_t)0x01FF);

			if (pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_DISABLE)
			{
				pTxBuffer++;
				pTxBuffer++;
			}
			else
			{
				pTxBuffer++;
			}
		}
		else
		{
			pUSARTHandle->pUSARTx->DR=(*pTxBuffer&(uint8_t)0xff);

			pTxBuffer++;


		}
	}
	while(! USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_FLAG_TC));
}

void USART_ReceiveData(USART_Handle_t *pUSARTHandle,uint8_t *pRxBuffer,uint32_t Len)
{
	for(uint32_t i=0;i<Len;i++)
	{
		while(! USART_GetFlagStatus(pUSARTHandle->pUSARTx,USART_FLAG_RXNE));
		if(pUSARTHandle->USART_Config.USART_WordLength==USART_WORDLEN_9BITS)
		{
			if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_DISABLE)
			{
				*((uint16_t*)pRxBuffer)=(pUSARTHandle->pUSARTx->DR&(uint16_t)0x01ff);
				pRxBuffer++;
				pRxBuffer++;
			}
			else
			{
				*pRxBuffer=(pUSARTHandle->pUSARTx->DR&(uint8_t)0xff);
				pRxBuffer++;

			}
		}
		else
		{
			if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_DISABLE)
			{
				*pRxBuffer=(uint8_t)(pUSARTHandle->pUSARTx->DR&(uint8_t)0xff);

			}
			else
			{
				*pRxBuffer=(uint8_t)(pUSARTHandle->pUSARTx->DR&(uint8_t)0x7f);
			}
			pRxBuffer++;
		}
	}
}
