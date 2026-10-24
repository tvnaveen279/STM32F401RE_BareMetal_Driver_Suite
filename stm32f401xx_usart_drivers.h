/*
 * stm32f401xx_usart_drivers.h
 *
 *  Created on: Oct 23, 2026
 *      Author: ramak
 */

#ifndef INC_STM32F401XX_USART_DRIVERS_H_
#define INC_STM32F401XX_USART_DRIVERS_H_

#include "stm32f401xx.h"

typedef struct
{
	uint8_t	USART_Mode;
	uint32_t	USART_BaudRate;
	uint8_t	USART_NoOfStopBits;
	uint8_t	USART_WordLength;
	uint8_t	USART_ParityControl;
	uint8_t	USART_HWFlowControl;

}USART_Config_t;


/*
 * Handle struct for USART peripheral
 */
typedef struct
{
	USART_RegDef_t *pUSARTx;
	USART_Config_t USART_Config;
}USART_Handle_t;


/*
 * @USART_Mode
 * possible options for USART mode
 */
#define USART_MODE_ONLY_TX		0
#define USART_MODE_ONLY_RX		1
#define USART_MODE_ONLY_TXRX	2

/*
 * @USART_BaudRate
 * possible options for USART baud
 */
#define USART_STD_BAUD_1200		1200
#define USART_STD_BAUD_2400		2400
#define USART_STD_BAUD_9600		9600
#define USART_STD_BAUD_19200	19200
#define USART_STD_BAUD_38400	38400
#define USART_STD_BAUD_57600	57600
#define USART_STD_BAUD_115200	115200
#define USART_STD_BAUD_230400	230400
#define USART_STD_BAUD_460800	460800
#define USART_STD_BAUD_921600	921600
#define USART_STD_BAUD_2M		2000000
#define USART_STD_BAUD_3M		3000000

/*
 * @USART_ParityControl
 * possible options for USART_ParityControl
 */
#define USART_PARITY_DISABLE	0
#define USART_PARITY_EN_EVEN	1
#define	USART_PARITY_EN_ODD		2

/*
 * @USART_WordLength
 * Bit 12(M) IN USART_CR1
 */
#define USART_WORDLEN_8BITS 	0
#define USART_WORDLEN_9BITS 	1

/*
 * @USART_NoOfStopBits
 * Bits 13:12 (STOP) in USART_CR2
 */
#define USART_STOPBITS_1		0
#define USART_STOPBITS_0_5		1
#define USART_STOPBITS_2		2
#define USART_STOPBITS_1_5		3

/*
 * @USART_HWFlowControl
 * Bits in USART_CR3 (RTSE: bit 8,CTSE:bit 9)
 */
#define USART_HW_FLOW_CTRL_NONE		0
#define USART_HW_FLOW_CTRL_CTS		1
#define USART_HW_FLOW_CTRL_RTS		2
#define USART_HW_FLOW_CTRL_CTS_RTS	3

/*
 * @USART Flags (Status Register USART_SR Flags)
 */
#define USART_FLAG_TXE		(1<<7)
#define USART_FLAG_TC		(1<<6)
#define USART_FLAG_RXNE		(1<<5)
#define USART_FLAG_IDLE		(1<<4)
#define USART_FLAG_CTS		(1<<9)
#define USART_FLAG_LBD		(1<<8)
#define USART_FLAG_PE		(1<<0)
#define USART_FLAG_FE		(1<<1)
#define USART_FLAG_NF		(1<<2)
#define USART_FLAG_ORE		(1<<3)


/*
 * PERIPHERAL CLOCK SETUP
 */
void USART_PeriClockControl(USART_RegDef_t *pUSARTx,uint8_t EnorDi);

/*
 * Init and De-init
 */
void USART_Init(USART_Handle_t *pUSARTHandle);
void USART_DeInit(USART_RegDef_t *pUSARTx);

/*
 * data send or receive (Blocking/polling)
 */
void USART_SendData(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer,uint32_t Len);
void USART_ReceiveData(USART_Handle_t *pUSARTHandle,uint8_t *pRxBuffer,uint32_t Len);
/*
 * IRQ Configuration and ISR handling
 */
void USART_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnorDi);
void USART_IRQPriorityConfig(uint8_t IRQNumber,uint32_t IRQPriority);
void USART_IRQHandling(USART_Handle_t *pUSARTHandle );
/*
 * other peripheral control APIs
 */
void USART_PeripheralControl(USART_RegDef_t *pUSARTx,uint8_t EnorDi);
uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx,uint32_t FlagName);
void USART_ClearFlag(USART_RegDef_t *pUSARTx,uint32_t StatusFlagName);
void USART_SetBaudRate(USART_RegDef_t *pUSARTx,uint32_t BaudRate);






#endif /* INC_STM32F401XX_USART_DRIVERS_H_ */

