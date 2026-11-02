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
	tempreg|=(1<<USART_CR1_RE);
	}
	else if(pUSARTHandle->USART_Config.USART_Mode==USART_MODE_ONLY_TX)
	{
		tempreg|=(1<<USART_CR1_TE);

	}
	else if(pUSARTHandle->USART_Config.USART_Mode==USART_MODE_ONLY_TXRX)
	{
		tempreg|=((1<<USART_CR1_RE)|(1<<USART_CR1_TE));
	}
	//config of wordlength
	tempreg|= (pUSARTHandle->USART_Config.USART_WordLength<<USART_CR1_M);
	//config of parity control
	if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_EN_EVEN)
	{
		//enable parity control(PCE is bit 10,PS default is 0 for even
		tempreg|=(1<<USART_CR1_PCE);
	}
	else if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_EN_ODD)
	{
		tempreg|=(1<<USART_CR1_PCE);
		tempreg|=(1<<USART_CR1_PS);

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
		tempreg|=(1<<USART_CR3_CTSE);

	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl==USART_HW_FLOW_CTRL_RTS)
	{
		tempreg|=(1<<USART_CR3_RTSE);

	}else if(pUSARTHandle->USART_Config.USART_HWFlowControl==USART_HW_FLOW_CTRL_CTS_RTS)
	{
		tempreg|=(1<<USART_CR3_CTSE);
		tempreg|=(1<<USART_CR3_RTSE);

	}
	pUSARTHandle->pUSARTx->CR3= tempreg;

	/******************************** Configuration of BRR (Baudrate) *********************************/
	  USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.USART_BaudRate);
}
void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate)
{
	uint32_t PCLKx = 16000000U; // 16 MHz default HSI
	uint32_t usartdiv;
	uint32_t M_part, F_part;
	uint32_t tempreg = 0;

	// OVER8 = 0 (16x oversampling)
	if(pUSARTx->CR1 & (1 << USART_CR1_OVER8))
	{
		// 8x oversampling
		usartdiv = ((25 * PCLKx) / (2 * BaudRate));
		M_part = usartdiv / 100;
		tempreg |= (M_part << 4);

		F_part = usartdiv - (M_part * 100);
		F_part = (((F_part * 8) + 50) / 100) & ((uint8_t)0x07);
		tempreg |= F_part;
	}
	else
	{
		// 16x oversampling (Standard default)
		usartdiv = ((25 * PCLKx) / (4 * BaudRate));
		M_part = usartdiv / 100;
		tempreg |= (M_part << 4);

		F_part = usartdiv - (M_part * 100);
		F_part = (((F_part * 16) + 50) / 100) & ((uint8_t)0x0F);
		tempreg |= F_part;
	}

	pUSARTx->BRR = tempreg;
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
		pUSARTx->CR1|=(1<<USART_CR1_UE);

	}
	else
	{
		pUSARTx->CR1&=~(1<<USART_CR1_UE);
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
/*********************************************************************
 * @fn      		  - USART_SendDataIT
 * @brief             - Initiates non-blocking transmission using interrupts
 * @param[in]         - USART Handle pointer
 * @param[in]         - Transmit buffer pointer
 * @param[in]         - Length of data to transmit
 * @return            - State of USART (USART_READY / USART_BUSY_IN_TX)
 *********************************************************************/
uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer,uint32_t Len)
{
	uint8_t tx_state=pUSARTHandle->TxBusyState;
	if(tx_state!=USART_BUSY_IN_TX)
	{
		pUSARTHandle->TxLen=Len;
		pUSARTHandle->pTxBuffer=pTxBuffer;
		pUSARTHandle->TxBusyState=USART_BUSY_IN_TX;
		//Enable TXE interrupt bit
		pUSARTHandle->pUSARTx->CR1|=(1<<USART_CR1_TXEIE);
	}
	return tx_state;
}
/*********************************************************************
 * @fn      		  - USART_ReceiveDataIT
 * @brief             - Initiates non-blocking receiption using interrupts
 * @param[in]         - USART Handle pointer
 * @param[in]         - receive  buffer pointer
 * @param[in]         - Length of data to receive
 * @return            - State of USART (USART_READY / USART_BUSY_IN_RX)
 *********************************************************************/
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle,uint8_t *pRxBuffer,uint32_t Len)
{
	uint8_t rx_state=pUSARTHandle->RxBusyState;
	if(rx_state!=USART_BUSY_IN_RX)
	{
		pUSARTHandle->RxLen=Len;
		pUSARTHandle->pRxBuffer=pRxBuffer;
		pUSARTHandle->RxBusyState=USART_BUSY_IN_RX;
		//Enable RXEIE bit
		pUSARTHandle->pUSARTx->CR1|=(1<<USART_CR1_RXNEIE);
	}
	return rx_state;
}

/*********************************************************************
 * @fn      		  - USART_IRQHandling
 * @brief             - Handles USART interrupt requests (TXE, TC, RXNE, Errors)
 * @param[in]         - USART Handle pointer
 *********************************************************************/
void USART_IRQHandling(USART_Handle_t *pUSARTHandle)
{
	uint32_t temp1,temp2,temp3;
	uint16_t *pdata;
	//check for RXNE flag
	temp1=pUSARTHandle->pUSARTx->SR& USART_FLAG_RXNE;
	temp2=pUSARTHandle->pUSARTx->CR1 & (1<<USART_CR1_RXNEIE);

	if(temp1&&temp2)
	{
		//intereupt is generated because of rxne
		if(pUSARTHandle->RxBusyState==USART_BUSY_IN_RX)
		{
			if(pUSARTHandle->RxLen>0)
			{
				//CHECK WORD LENGTH
				if(pUSARTHandle->USART_Config.USART_WordLength==USART_WORDLEN_9BITS)
				{
					if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_DISABLE)
					{
						//ALL 9 BITS ARE DATA BITS
						*((uint16_t*)pUSARTHandle->pRxBuffer)=(pUSARTHandle->pUSARTx->DR&0x01FF);
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->RxLen-=2;

					}
					else
					{
						//all 8 bits are data ,9th bit is parity
						*pUSARTHandle->pRxBuffer=(pUSARTHandle->pUSARTx->DR&(uint8_t)0xFF);
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->RxLen-=1;
					}
				}
				else
				{
					// 8-bit data frame
					if(pUSARTHandle->USART_Config.USART_ParityControl == USART_PARITY_DISABLE)
					{
						// All 8 bits are data
						*pUSARTHandle->pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
					}
					else

					{

						// 7 bits data, 8th bit is parity

						*pUSARTHandle->pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint8_t)0x7F);

					}
					pUSARTHandle->pRxBuffer++;
					pUSARTHandle->RxLen -= 1;
				}
			}
			// check if all bytes are received
			if(pUSARTHandle->RxLen==0)
			{
				//1. disable RXNE interrupt
				pUSARTHandle->pUSARTx->CR1 &= ~(1<<USART_CR1_RXNEIE);
				//2. Reset communication state
				pUSARTHandle->RxBusyState=USART_READY;
				//3. NOTIFY APPLIACTION VIA CALL BACK
				USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_RX_CMPLT);
			}
		}
	}
	/// CHECK FOR TXE FLAG
	temp1=pUSARTHandle->pUSARTx->SR&USART_FLAG_TXE;
	temp2=pUSARTHandle->pUSARTx->CR1 & (1<<USART_CR1_TXEIE);

		if(temp1&&temp2)
		{
			//intereupt is generated because of TXE
			if(pUSARTHandle->TxBusyState==USART_BUSY_IN_TX)
			{
				if(pUSARTHandle->TxLen>0)
				{
					//CHECK WORD LENGTH
					if(pUSARTHandle->USART_Config.USART_WordLength==USART_WORDLEN_9BITS)
					{
						pdata=(uint16_t*)pUSARTHandle->pTxBuffer;
						pUSARTHandle->pUSARTx->DR=(*pdata & (uint16_t)0x01FF);
						if(pUSARTHandle->USART_Config.USART_ParityControl==USART_PARITY_DISABLE)
						{
							//ALL 9 BITS ARE DATA BITS
							pUSARTHandle->pTxBuffer++;
							pUSARTHandle->pTxBuffer++;
							pUSARTHandle->TxLen-=2;

						}
						else
						{
							//all 8 bits are data ,9th bit is paritY
							pUSARTHandle->pTxBuffer++;
							pUSARTHandle->TxLen-=1;
						}
					}
					else
					{
						// 8 BIT DATA
						pUSARTHandle->pUSARTx->DR=(*pUSARTHandle->pTxBuffer & (uint8_t)0XFF);
						pUSARTHandle->pTxBuffer++;
						pUSARTHandle->TxLen -= 1;
					}
				}
				// check if all bytes are Loaded
				if(pUSARTHandle->TxLen==0)
				{
					//1. disable TXEIE interrupt
					pUSARTHandle->pUSARTx->CR1 &= ~(1<<USART_CR1_TXEIE);

					//ENABLE TCIE BIT to wait until last byte is leaves from the shift register
					pUSARTHandle->pUSARTx->CR1|=(1<<USART_CR1_TCIE);
				}
			}
		}
		//CHECK FOR TC BIT
		temp1=pUSARTHandle->pUSARTx->SR & USART_FLAG_TC;
		temp2=pUSARTHandle->pUSARTx->CR1 & (1<<USART_CR1_TCIE);

		if(temp1 && temp2)
		{
			//close tarnsmission
			if(pUSARTHandle->TxBusyState==USART_BUSY_IN_TX)
			{
				if(pUSARTHandle->TxLen==0)
				{
					//1. CLEAR TCIE BIT
					pUSARTHandle->pUSARTx->CR1 &= ~(1<<USART_CR1_TCIE);
					//2. CLEAR tc FLAG
					pUSARTHandle->pUSARTx->SR &= ~(USART_FLAG_TC);
					//3. RESET COMMUNICATION STATE
					pUSARTHandle->TxBusyState=USART_READY;
					//4. RESET BUUFFER POINTERS
					pUSARTHandle->pTxBuffer= 0;
					//5. NOTIFY APPLICATION VIA CAAL BACK
					USART_ApplicationEventCallback(pUSARTHandle, USART_EVENT_TX_CMPLT);

				}
			}
		}
		//CHECK FOR OVERRUN ERROR
		temp1=pUSARTHandle->pUSARTx->SR&USART_FLAG_ORE;
		temp2=pUSARTHandle->pUSARTx->CR1&(1<<USART_CR1_RXNEIE);
		if(temp1 && temp2)
		{
			USART_ApplicationEventCallback(pUSARTHandle, USART_ERR_ORE);
		}
}

/*********************************************************************
 * @fn      		  - USART_IRQInterruptConfig
 *********************************************************************/

void USART_IRQInteruptConfig(uint8_t IRQNumber,uint8_t EnorDi)
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

/*********************************************************************
 * @fn      		  - USART_IRQPriorityConfig
 *********************************************************************/
void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	uint8_t iprx=IRQNumber/4;
	uint8_t ipr_section=IRQNumber%4;
	uint8_t shiftamount=(8*ipr_section)+(8-NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASEADDR+(iprx*4)) |=(IRQPriority<<(shiftamount));
}
