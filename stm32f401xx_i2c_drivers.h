
#ifndef INC_STM32F401XX_I2C_DRIVER_H_
#define INC_STM32F401XX_I2C_DRIVER_H_
#include"stm32f401xx.h"
/*
 * i2c config structure
 */
typedef struct
{
	uint32_t	I2C_SCLSpeed;
	uint8_t		I2C_DeviceAddress;
	uint8_t 	I2C_ACKControl;
	uint16_t	I2C_FMDutyCycle;

}I2C_Config_t;

/*
 * I2C Handle structure
 *
 */
typedef struct
{
	I2C_RegDef_t	*pI2Cx;
	I2C_Config_t	I2C_Config;
}I2C_Handle_t;

/*
 * @I2C_SCLSpeed
 */
#define I2C_SCLSPEED_SM		100000	//100KHZ(STANDARD SPEED)
#define I2C_SCLSPEED_FM4K	400000	//400KHZ(FAST MODE)
#define I2C_SCLSPEED_FM2K	200000	//200KHZ

/*
 * @I2C_ACKControl
 */
#define I2C_ACK_ENABLE	1
#define	I2C_ACK_DISABLE	0

/*
 * @I2C_FMDutyCycle
 */
#define I2C_FMDUTY_2	0	//Tlow/Thigh=2
#define	I2C_FMDUTY_16_9	1	//Tlow/tHIGH=16/9

/*
 * SR1&SR2 status flags definition
 */
#define I2C_FLAG_TXE 	(1<<I2C_SR1_TXE)
#define I2C_FLAG_RXNE	(1<<I2C_SR1_RXNE)
#define I2C_FLAG_SB		(1<<I2C_SR1_SB)	//(START BIT GENERATED)
#define I2C_FLAG_ADDR	(1<<I2C_SR1_ADDR)//(ADDRESS SENT OR MATCHED)
#define I2C_FLAG_BTF	(1<<I2C_SR1_BTF)//(BITE TRANSFER FINISHED)
#define I2C_FLAG_STOPF	(1<<I2C_SR1_STOPF)//(STOP DETECTED)
#define I2C_FLAG_BERR	(1<<I2C_SR1_BERR)//(BUS ERROR)
#define I2C_FLAG_ARLO	(1<<I2C_SR1_ARLO)//ARBITION LOST)
#define I2C_FLAG_AF		(1<<I2C_SR1_AF)//(ACKNOWLEDGE FAILURE)
#define I2C_FLAG_OVR	(1<<I2C_SR1_OVR)//(OVERRUN)
#define I2C_FLAG_TIMEOUT (1<<I2C_SR1_TIMEOUT)



/*
 * CLOCK AND PERIPHERAL ENABLE API'S
 */
void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx,uint8_t	EnorDi);
void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx,uint8_t	EnorDi);
void I2C_Init(I2C_Handle_t *pI2CHandle);
void I2C_DeInit(I2C_RegDef_t *pI2Cx);


/*
 * data send and receiving API's

 */
void I2C_SendData(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer,uint32_t Len,uint8_t SlaveAddr,uint8_t Sr);
void I2C_ReceiveData(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer,uint32_t Len,uint8_t SlaveAddr,uint8_t Sr);

/*
 * flag status checking API's
 */
uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx,uint32_t FlagName);
/*
 * application callback API's
 */
void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle,uint8_t AppEv);

#endif
