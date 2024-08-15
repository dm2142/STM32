/*
 * tim1637.h
 *
 *  Created on: May 6, 2024
 *      Author: DM
 */

#ifndef INC_TM1637_H_
#define INC_TM1637_H_

#ifdef STM32F446xx
	#include "stm32f4xx_hal.h"
#elif defined(STM32F103x6)
	#include "stm32f1xx_hal.h"
#elif defined(STM32H723xx)
	#include "stm32h7xx_hal.h"
#endif

//#define TIM1637_SCLK_PIN		GPIO_PIN_8
//#define TIM1637_SCLK_PORT		GPIOC
//#define TIM1637_SDIO_PIN		GPIO_PIN_6
//#define TIM1637_SDIO_PORT		GPIOC
//
//#define TIM1637_TIMER			TIM6		// Basic Timer
//#define TIM1637_CLK_FREQ		150000UL	// Hz

#define TIM1637_DISPLAY_CTRL		0b10000000		//	Command: Display and control command setting
#define	TIM1637_DATA_CMD_FIX_ADDR	0b01000100		//	Command: Data command setting with Fix address, Write data to display register
#define	TIM1637_DATA_CMD_AUTO_ADDR	0b01000000		//	Command: Data command setting with Automatic address adding Write data to display register
#define	TIM1637_ADDR_CMD_SETTING	0b11000000		//	Command: Display and control command setting

#define TIM1637_ADD_DOT				0b10000000		// 	Add the 8-bit to represent the dot in the display.
#define TIM1637_NUM_DIGITS			6				// 	Specifies the number of digits to control.

typedef enum{
	PulseWidth_1_16	= 0,
	PulseWidth_2_16,
	PulseWidth_4_16,
	PulseWidth_10_16,
	PulseWidth_11_16,
	PulseWidth_12_16,
	PulseWidth_13_16,
	PulseWidth_14_16,
}TIM1637_PulseWidth_e;

typedef enum{
	TIM1637_STATE_READY,
	TIM1637_STATE_BUSY_IN_DATA_CMD,
	TIM1637_STATE_BUSY_IN_ADDR_CMD,
	TIM1637_STATE_BUSY_IN_DISPLAY_CTRL_CMD,
	TIM1637_STATE_BUSY_IN_TX_BYTES,
}TIM1637_State_e;

typedef enum{
	TIM1637_METHOD_DISPLAY_CTRL,
	TIM1637_METHOD_6BYTES_DATA,
	TIM1637_METHOD_1BYTE_DATA,
}TIM1637_Methods_e;

typedef enum{
	TIM1637_DISPLAYADDR_0 = 0x3,
	TIM1637_DISPLAYADDR_1 = 0x4,
	TIM1637_DISPLAYADDR_2 = 0x5,
	TIM1637_DISPLAYADDR_3 = 0x0,
	TIM1637_DISPLAYADDR_4 = 0x1,
	TIM1637_DISPLAYADDR_5 = 0x2,
}TIM1637_DisplayAddress_e;

typedef enum{
	TIM1637_DISPLAY_OFF,
	TIM1637_DISPLAY_ON,
}TIM1637_DisplayCtrl_e;

typedef enum{
	TIM1637_CMDIDX_DATA = 0,
	TIM1637_CMDIDX_ADDR = 1,
	TIM1637_CMDIDX_DISPLAY_CTR = 2,
}TIM1637_CmdIdx_e;

typedef enum{
	TIM1637_STARTCONDITION_DISABLED = 0,
	TIM1637_STARTCONDITION_ENABLED,
}TIM1637_StartCondition_e;

typedef enum{
	TIM1637_STOPCONDITION_DISABLED = 0,
	TIM1637_STOPCONDITION_ENABLED,
}TIM1637_StopCondition_e;


/*	**************************************
 * 		Handle structure for TIM1637
 *  **************************************/
typedef struct tim1637_handle{
	uint16_t					SCLK_pin;		 	/*!< Specifies the GPIO pins to be configured as SCLK.
									 	 	 	 	 This parameter can be any value of @ref GPIO_pins_define */

	GPIO_TypeDef *				SCLK_gpio;			/*!< Specifies the base address of GPIO where SCLK pin is located @ref (GPIO_TypeDef *) */

	uint16_t					SDIO_pin;			/*!< Specifies the GPIO pins to be configured as SDIO.
	                           	 	 	 	 	 	 This parameter can be any value of @ref GPIO_pins_define */

	GPIO_TypeDef *				SDIO_gpio;			/*!< Specifies the base address of GPIO where SDIO pin is located @ref (GPIO_TypeDef *) */

	TIM_HandleTypeDef			Timer;				/*!< Specifies the TIMER handle and save the (TIM_TypeDef *) to generate the Clock Signal base on UPDATE interrupt event*/

	uint32_t					SCLK_Freq;			/*!< Specifies the Clock frequency */

	TIM1637_DisplayCtrl_e		DispCtrl;			/*!< Use to set the Initial state of the display ON/OFF @ref TIM1637_DisplayCtrl_e */
	TIM1637_PulseWidth_e		Brightness;			/*!< Use to save the Brightness value of the display @ref TIM1637_PulseWidth_e */

	TIM1637_State_e				State;				/*!< Use for flow control in Data sending */
	TIM1637_Methods_e			Method;				/*!< Use for flow control in Data sending */
	TIM1637_StartCondition_e	StartCondition;	/*!< Set/Reset the start condition in data transfer */
	TIM1637_StopCondition_e		StopCondition;		/*!< Set/Reset the stop condition in data transfer  */
	uint8_t						Commands[3];		/*!< Use to save Commands to send base on the required sequence */
	uint8_t						Data[6];			/*!< Use to save the value of each display-digit */
	uint8_t						Data_Idx;			/*!< Index to set the byte to send */
}TIM1637_Handle_t;


/*	*************************************
 * 					METHODS
 *  ************************************/
void tim1637_Init(TIM1637_Handle_t* tim1637);

void tim1637_ClearAll( TIM1637_Handle_t* tim1637 );
void tim1637_SetValue( TIM1637_Handle_t* tim1637, uint8_t DisplayAddr, uint8_t Value );
void tim1637_SetIntNumber( TIM1637_Handle_t* tim1637, uint32_t Number );
void tim1637_SetFloatNumber( TIM1637_Handle_t* tim1637, double Number, uint8_t NumDecimals );
void tim1637_Demo(TIM1637_Handle_t* tim1637 );

/*
 *		Control Methods
 */
void tim1637_TurnOn( TIM1637_Handle_t* tim1637 );
void tim1637_TurnOff( TIM1637_Handle_t* tim1637 );
void tim1637_SetBrightness( TIM1637_Handle_t* tim1637, TIM1637_PulseWidth_e Brightness );

/*
 *	Use in the Timer IRQ
 */
void tim1637_Callback(TIM1637_Handle_t* tim1637);

#endif /* INC_TM1637_H_ */
