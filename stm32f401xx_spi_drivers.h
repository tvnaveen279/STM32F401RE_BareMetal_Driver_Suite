/*
 * stm32f401xx_spi_drivers.h
 *
 *  Created on: Oct 6, 2026
 *      Author: ramak
 */
#include"stm32f401xx.h"
#ifndef INC_STM32F401XX_SPI_DRIVERS_H_
#define INC_STM32F401XX_SPI_DRIVERS_H_




/*
 * configuration structure for SPIx Peripherals
 */
typedef struct
{
	uint8_t	SPI_DeviceMode;
	uint8_t	SPI_BusConfig;
	uint8_t	SPI_SclkSpeed;
	uint8_t	SPI_DFF;
	uint8_t	SPI_CPOL;
	uint8_t	SPI_CPHA;
	uint8_t	SPI_SSM;

}SPI_Config_t;

/*
 * handle structure for SPI peripherals
 */
typedef struct
{
	SPI_RegDef_t *pSPIx;
	SPI_Config_t SPI_Config;
}SPI_Handle_t;


//SPI possible modes
#define SPI_DEVICE_MODE_SLAVE 0
#define SPI_DEVICE_MODE_MASTER 1
//SPI possible configurations
#define SPI_BUS_CONFIG_FD          1  //full duplex:4 WIRES(MISO,MOSI,SCLK,NSS)
#define SPI_BUS_CONFIG_HD          2  //HALF DUPLEX:3 WIRES(SCK,NSS,1 BI DIRECTIONAL DATA LINE
#define SPI_BUS_CONFIG_SIMPLEX_RXONLY 3  //SIMPLEX RX ONLY : MOSI IS DISSCONNECTED TO RECEIVE ONLY
//SPI possible clock speeds
#define SPI_SCLK_SPEED_DIV2      0    //(FPCLK/2)
#define SPI_SCLK_SPEED_DIV4      1    //(FPCLK/4)
#define SPI_SCLK_SPEED_DIV8      2    //(FPCLK/8)
#define SPI_SCLK_SPEED_DIV16     3    //(FPCLK/16)
#define SPI_SCLK_SPEED_DIV32     4 	  //(FPCLK/32)
#define SPI_SCLK_SPEED_DIV64     5		//(FPCLK/64)
#define SPI_SCLK_SPEED_DIV128    6		//(FPCLK/128)
#define SPI_SCLK_SPEED_DIV256    7		//(FPCLK/256)

//DFF POSSIBLE FORMATS
#define SPI_DFF_8BITS  0
#define SPI_DFF_16BITS 1
// SPI CLOCK POLARITY
#define SPI_CPOL_LOW	0
#define SPI_CPOL_HIGH	1
//SPI CLOCK PHASE
#define SPI_CPHA_LOW	0		//1ST CLOCK TRANSITION
#define SPI_CPHA_HIGH	1		// 2ND CLOCK TRANSITION
//SPI SOFTWARE SLAVE MANAGEMENT
#define SPI_SSM_HW		0		//HARDWARE NSS
#define SPI_SSM_SW		1		//SOFTWARE NSS


/*
 * SPI related status flags
 */
#define SPI_TXE_FLAG	(1<<SPI_SR_TXE)
#define SPI_RXNE_FLAG	(1<<SPI_SR_RXNE)
#define SPI_CHSIDE_FLAG	(1<<SPI_SR_CHSIDE)
#define SPI_UDR_FLAG	(1<<SPI_SR_UDR)
#define SPI_CRCEER_FLAG	(1<<SPI_SR_CRCERR)
#define SPI_MODF_FLAG	(1<<SPI_SR_MODF)
#define SPI_OVR_FLAG	(1<<SPI_SR_OVR)
#define SPI_BSY_FLAG	(1<<SPI_SR_BSY)
#define SPI_FRE_FLAG	(1<<SPI_SR_FRE)



/******************************************************************************************
 *                              APIs supported by this driver
 *             For more information about the APIs check the function definitions
 ******************************************************************************************/
/*
 * PERIPHERAL CLOCK SETUP
 */

void SPI_PeriClockControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi);

/*
 * Init and De-init
 */
void SPI_Init(SPI_Handle_t *pSPIHandle);
void SPI_DeInit(SPI_RegDef_t *pSPIx);
/*
 * data send and receive (Blocking/Polling based)
 *
 */
void SPI_SendData(SPI_RegDef_t *pSPIx,uint8_t *pTxBuffer, uint32_t Len);
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len);
/*
 * IRQ Configuration and IRQ handling
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber,uint8_t EnorDi);
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority);
void SPI_IRQHandling(SPI_Handle_t *pHandle);
/*
 * other peripheral control APIs
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
void SPI_SSIConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx,uint8_t EnorDi);
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx,uint32_t FlagName);
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx);
void SPI_CloseTransmission(SPI_Handle_t *pSPIHandle);
void SPI_CloseReception(SPI_Handle_t *pSPIHandle);
/*
 * application call back
 */
void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv);










#endif /* INC_STM32F401XX_SPI_DRIVERS_H_ */
