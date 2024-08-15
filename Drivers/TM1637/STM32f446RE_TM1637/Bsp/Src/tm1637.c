/*
 * tim1637.c
 *
 *  Created on: May 6, 2024
 *      Author: DM
 */

#include <tm1637.h>
#include "main.h"


/*	*********************************
 * 		Declare Private variables
 *  *********************************/

static const uint8_t DispNumber[] = { // Represent the Value to display each number
		0b00111111,	// Display 0
		0b00000110,	// 	1
		0b01011011, //	2
		0b01001111, //	3
		0b01100110, //  4
		0b01101101, // 	5
		0b01111101, //	6
		0b00000111, //	7
		0b01111111, //	8
		0b01100111, //	9
};


/*	*********************************
 * 		Declare Private Methods
 *  *********************************/
static void tim1637_send_displayctrl( TIM1637_Handle_t* tim1637, TIM1637_DisplayCtrl_e OnOff , TIM1637_PulseWidth_e Brightness );
static void tim1637_send_1byte( TIM1637_Handle_t* tim1637, uint8_t DisplayValue , TIM1637_DisplayAddress_e DisplayAddr );
static void tim1637_send_6bytes( TIM1637_Handle_t* tim1637, uint8_t ArrayBytes[] );

static void tim1637_start_condition(TIM1637_Handle_t* tim1637);
static void tim1637_stop_condition(TIM1637_Handle_t* tim1637);

static void tim1637_msp_gpio(TIM1637_Handle_t* tim1637);
static void tim1637_msp_tim(TIM1637_Handle_t* tim1637);

/**
  * @brief  Initialize the peripheral and configure the timer to generate SCLK frequency.
  * @note	Use TIM1637_Handle_t to set the GPIO, PINS and Timer to use. (Recommend use a Basic Timer)
  * @param  None
  * @retval None
  */
void tim1637_Init(TIM1637_Handle_t* tim1637){

	/* Check the parameters	*/
	assert_param(IS_GPIO_ALL_INSTANCE(tim1637->SCLK_gpio));
	assert_param(IS_GPIO_PIN(tim1637->SCLK_pin));

	assert_param(IS_GPIO_ALL_INSTANCE(tim1637->SDIO_gpio));
	assert_param(IS_GPIO_PIN(tim1637->SDIO_pin));

	assert_param(IS_TIM_INSTANCE(tim1637->Timer));

	/* Enable clock and peripheral configuration */
	tim1637_msp_gpio(tim1637);
	tim1637_msp_tim(tim1637);

	uint16_t prescaler = 0;
	uint32_t PCLK = 0;

	#ifdef STM32F446xx	// Search in which APB is located the TIM
		if( ( APB1PERIPH_BASE < (uint32_t)(tim1637->Timer.Instance) ) && ( (uint32_t)(tim1637->Timer.Instance)  < APB2PERIPH_BASE ) ){
			PCLK = HAL_RCC_GetPCLK1Freq();
		}else{
			PCLK = HAL_RCC_GetPCLK2Freq();
		}
	#elif defined(STM32F103x6)
		if( ( APB1PERIPH_BASE < (uint32_t)(tim1637->Timer.Instance) ) && ( (uint32_t)(tim1637->Timer.Instance)  < APB2PERIPH_BASE ) ){
			PCLK = HAL_RCC_GetPCLK1Freq();
		}else{
			PCLK = HAL_RCC_GetPCLK2Freq();
		}
	#elif defined(STM32H723xx)
		if( ( APB1PERIPH_BASE < (uint32_t)(tim1637->Timer.Instance) ) && ( (uint32_t)(tim1637->Timer.Instance)  < APB2PERIPH_BASE ) ){
			PCLK = HAL_RCC_GetPCLK1Freq();
		}else{
			PCLK = HAL_RCC_GetPCLK2Freq();
		}
	#endif

	/* Configure Timer to generate an Update Interrupt Event @ tim1637->SCLK_Freq / 2 */
	prescaler = ( (PCLK * 2) / ( tim1637->SCLK_Freq * 4 ) ) - 1;
	tim1637->Timer.Init.Prescaler = prescaler;
	tim1637->Timer.Init.Period = 1;


	if( HAL_TIM_Base_Init( &(tim1637->Timer) ) != HAL_OK){
		Error_Handler();
	}else{
		tim1637->State = TIM1637_STATE_READY;
	}


	tim1637_ClearAll(tim1637);
	if( tim1637->DispCtrl == TIM1637_DISPLAY_ON ){
			tim1637_TurnOn(tim1637);
	}else{
		tim1637_TurnOff(tim1637);
	}

}

/**
  * @brief  Send 0 value to turn off all the segments in each display.
  * @note
  * @param  TIM1637_Handle_t* tim1637
  * @retval None
  */
void tim1637_ClearAll( TIM1637_Handle_t* tim1637 ){
	uint8_t DataBytes[TIM1637_NUM_DIGITS] = {0};

	while( tim1637->State != TIM1637_STATE_READY);
	tim1637_send_6bytes(tim1637, DataBytes);

}

/**
  * @brief  Send the specific value ( to set the 8 segments ) in an specific display.
  * @note
  * @param 	DisplayAddr specifies the display to set the value.
  * @param  Value represents the state of the 8 segments in the display, where LSB represents the A-Segment and MSB represents the dot-segment.
  * @retval None
  */
void tim1637_SetValue( TIM1637_Handle_t* tim1637, uint8_t DisplayAddr, uint8_t Value ){

	TIM1637_DisplayAddress_e DispAddr = 0;

	switch (DisplayAddr) {
		case 0x0:
			DispAddr = TIM1637_DISPLAYADDR_0;
			break;
		case 0x1:
			DispAddr = TIM1637_DISPLAYADDR_1;
			break;
		case 0x2:
			DispAddr = TIM1637_DISPLAYADDR_2;
			break;
		case 0x3:
			DispAddr = TIM1637_DISPLAYADDR_3;
			break;
		case 0x4:
			DispAddr = TIM1637_DISPLAYADDR_4;
			break;
		case 0x5:
			DispAddr = TIM1637_DISPLAYADDR_5;
			break;
		default:
			break;
	}

	while( tim1637->State != TIM1637_STATE_READY);
	tim1637_send_1byte(tim1637, Value, DispAddr);
}


/**
  * @brief	Use to represent an Integer Number in the displays.
  * @note
  * @param
  * @retval None
  */
void tim1637_SetIntNumber( TIM1637_Handle_t* tim1637, uint32_t Number ){

	uint8_t DisplayAddr[6] = {0} ;

	if( Number < 10 ){

		DisplayAddr[0] =  DispNumber[ Number ];

	}else if( Number < 100 ){

		DisplayAddr[0] = DispNumber[ Number % 10 ];
		DisplayAddr[1] = DispNumber[  Number / 10 ];

	}else if( Number < 1000 ){

		DisplayAddr[0] = DispNumber[ Number % 10 ];
		DisplayAddr[1] = DispNumber[  (Number % 100) / 10 ];
		DisplayAddr[2] = DispNumber[ Number/ 100 ];

	}else if( Number < 10000 ){

		DisplayAddr[0] = DispNumber[ Number % 10 ];
		DisplayAddr[1] = DispNumber[  (Number % 100) / 10 ];
		DisplayAddr[2] = DispNumber[ (Number % 1000) / 100 ];
		DisplayAddr[3] = DispNumber[ Number / 1000 ];

	}else if( Number < 100000 ){

		DisplayAddr[0] = DispNumber[ Number % 10 ];
		DisplayAddr[1] = DispNumber[  (Number % 100) / 10 ];
		DisplayAddr[2] = DispNumber[ (Number % 1000) / 100 ];
		DisplayAddr[3] = DispNumber[ (Number % 10000) / 1000 ];
		DisplayAddr[4] = DispNumber[ Number / 10000 ];

	}else if( Number < 1000000 ){

		DisplayAddr[0] = DispNumber[ Number % 10 ];
		DisplayAddr[1] = DispNumber[  (Number % 100) / 10 ];
		DisplayAddr[2] = DispNumber[ (Number % 1000) / 100 ];
		DisplayAddr[3] = DispNumber[ (Number % 10000) / 1000 ];
		DisplayAddr[4] = DispNumber[ (Number % 100000) / 10000 ];
		DisplayAddr[5] = DispNumber[ Number / 100000 ];

	}

	tim1637_send_6bytes(tim1637, DisplayAddr);
}

/**
  * @brief	Use to represent a double or float value in the displays.
  * @note
  * @param double Number
  * @param uint8_t NumDecimals represent the number of digits to use after of decimal point. Maximun of 3.
  * @retval None
  */
void tim1637_SetFloatNumber( TIM1637_Handle_t* tim1637, double Number, uint8_t NumDecimals ){

	uint8_t DisplayAddr[6] = {0}, idx = 0 ;
	double aux = 0;

	if( NumDecimals <= 0)		NumDecimals = 1;
	else if(NumDecimals  > 3)	NumDecimals = 3;


	// Extract the decimal number
	if( NumDecimals == 1){
		aux = ( Number * 10);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 1 ] = DispNumber[ idx ];

	}else if( NumDecimals == 2){
		aux = ( Number * 10);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 1 ] = DispNumber[ idx ];

		aux = ( Number * 100);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 2 ] = DispNumber[ idx ];

	}else if( NumDecimals == 3){
		aux = ( Number * 10);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 1 ] = DispNumber[ idx ];

		aux = ( Number * 100);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 2 ] = DispNumber[ idx ];

		aux = ( Number * 1000);
		idx = (uint32_t) aux % 10;
		DisplayAddr[ NumDecimals - 3 ] = DispNumber[ idx ];

	}

	// Extract the integer number
	if( Number < 10 ){

		DisplayAddr[NumDecimals] =   DispNumber[ (uint8_t) Number ];

	}else if( Number < 100 ){

		DisplayAddr[NumDecimals] = DispNumber[ (uint16_t) Number % 10 ];
		DisplayAddr[NumDecimals + 1] = DispNumber[ (uint16_t) Number / 10 ];

	}else if( Number < 1000 ){

		DisplayAddr[NumDecimals] = DispNumber[ (uint16_t) Number % 10 ];
		DisplayAddr[NumDecimals + 1] = DispNumber[ ( (uint16_t) Number % 100) / 10 ];
		DisplayAddr[NumDecimals + 2] = DispNumber[ (uint16_t) Number/ 100 ];

	}else if( Number < 10000 ){

		DisplayAddr[NumDecimals] = DispNumber[ (uint16_t) Number % 10 ];
		DisplayAddr[NumDecimals + 1] = DispNumber[ ( (uint16_t) Number % 100) / 10 ];
		DisplayAddr[NumDecimals + 2] = DispNumber[ ( (uint16_t) Number % 1000) / 100 ];
		DisplayAddr[NumDecimals + 3] = DispNumber[ (uint16_t) Number / 1000 ];

	}else if( Number < 100000 ){

		DisplayAddr[NumDecimals] = DispNumber[ (uint32_t) Number % 10 ];
		DisplayAddr[NumDecimals + 1] = DispNumber[ ( (uint32_t) Number % 100) / 10 ];
		DisplayAddr[NumDecimals + 2] = DispNumber[ ( (uint32_t) Number % 1000) / 100 ];
		DisplayAddr[NumDecimals + 3] = DispNumber[ ( (uint32_t) Number % 10000) / 1000 ];
		DisplayAddr[NumDecimals + 4] = DispNumber[ (uint32_t) Number / 10000 ];

	}

	// Add dot
	DisplayAddr[NumDecimals] |= TIM1637_ADD_DOT;

	tim1637_send_6bytes(tim1637, DisplayAddr);
}


/**
  * @brief
  * @note
  * @param
  * @retval None
  */
void tim1637_Demo(TIM1637_Handle_t* tim1637 ){

	uint8_t values[6] = {0};

	for( uint8_t val = 0; val < 10; val ++ ){
		for( uint8_t DispAdddr = 0; DispAdddr < 6; DispAdddr ++ ){

			values[DispAdddr] = DispNumber[val];
			tim1637_send_6bytes(tim1637, values);
			HAL_Delay(250);

		}

		values[0] = 0;
		values[1] = 0;
		values[2] = 0;
		values[3] = 0;
		values[4] = 0;
		values[5] = 0;
		tim1637_ClearAll(tim1637);
		HAL_Delay(500);
	}

}

/**
  * @brief
  * @note
  * @param
  * @retval None
  */
void tim1637_TurnOn( TIM1637_Handle_t* tim1637 ){
	// Update the DispCtrl state
	tim1637->DispCtrl = TIM1637_DISPLAY_ON;
	while( tim1637->State != TIM1637_STATE_READY);
	tim1637_send_displayctrl(tim1637, TIM1637_DISPLAY_ON, tim1637->Brightness);

}

/**
  * @brief
  * @note
  * @param
  * @retval None
  */
void tim1637_TurnOff( TIM1637_Handle_t* tim1637 ){
	// Update the DispCtrl state
	tim1637->DispCtrl = TIM1637_DISPLAY_OFF;
	while( tim1637->State != TIM1637_STATE_READY);
	tim1637_send_displayctrl(tim1637, TIM1637_DISPLAY_OFF, tim1637->Brightness);
}

/**
  * @brief	Set the specified brightness
  * @note
  * @param
  * @retval None
  */
void tim1637_SetBrightness( TIM1637_Handle_t* tim1637, TIM1637_PulseWidth_e Brightness ){
	// Update the Brightness value
	tim1637->Brightness = Brightness;
	while( tim1637->State != TIM1637_STATE_READY);
	tim1637_send_displayctrl(tim1637, tim1637->DispCtrl, Brightness);
}


/**
  * @brief  Callback function for Timer Update Event to send to TIM1637, it executes when an update interrupt event rises.
  * @note	Use to Handle different states and data transfer sequences
  * @param  None
  * @retval None
  */
void tim1637_Callback(TIM1637_Handle_t* tim1637){

	static uint8_t count = 0;

	uint32_t itsource = tim1637->Timer.Instance->DIER;
	uint32_t itflag   = tim1637->Timer.Instance->SR;

	if ((itflag & (TIM_FLAG_UPDATE)) == (TIM_FLAG_UPDATE))
	  {
	    if ((itsource & (TIM_IT_UPDATE)) == (TIM_IT_UPDATE))
	    {
	    	__HAL_TIM_CLEAR_FLAG( &(tim1637->Timer), TIM_FLAG_UPDATE);


			// Generate the Start Condition before send 8 data bits
			if( count == 0 && tim1637->StartCondition == TIM1637_STARTCONDITION_ENABLED){
				tim1637_start_condition(tim1637);
			}

			if( count % 2 == 0){		// Set LOW the SCLK pin.
				HAL_GPIO_WritePin(tim1637->SCLK_gpio, tim1637->SCLK_pin, GPIO_PIN_RESET);

				if( count < 16 ){		// Change the SDIO state when SCLK is LOW.

					if( tim1637->State == TIM1637_STATE_BUSY_IN_DATA_CMD){
						HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, ( ( tim1637->Commands[ TIM1637_CMDIDX_DATA ] >> (uint8_t)(count / 2) ) & 0x1 ));

					}else if( tim1637->State == TIM1637_STATE_BUSY_IN_ADDR_CMD){
						HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, ( ( tim1637->Commands[ TIM1637_CMDIDX_ADDR ] >> (uint8_t)(count / 2) ) & 0x1 ));

					}else if( tim1637->State == TIM1637_STATE_BUSY_IN_DISPLAY_CTRL_CMD ){
						HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, ( ( tim1637->Commands[ TIM1637_CMDIDX_DISPLAY_CTR ] >> (uint8_t)(count / 2) ) & 0x1 ));

					}else if( tim1637->State == TIM1637_STATE_BUSY_IN_TX_BYTES ){
						HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, ( ( tim1637->Data[ tim1637->Data_Idx ] >> (uint8_t)(count / 2) ) & 0x1 ));
					}

				}else{

					HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, GPIO_PIN_RESET);
				}
			}else if( count % 2 == 1){

				HAL_GPIO_WritePin(tim1637->SCLK_gpio, tim1637->SCLK_pin, GPIO_PIN_SET);
			}

			count ++;

			if( count == 19 && tim1637->StopCondition == TIM1637_STOPCONDITION_ENABLED){

				if( tim1637->State == TIM1637_STATE_BUSY_IN_DATA_CMD){
					tim1637_stop_condition(tim1637);

					tim1637->State = TIM1637_STATE_BUSY_IN_ADDR_CMD;
					tim1637->StartCondition = TIM1637_STARTCONDITION_ENABLED;
					tim1637->StopCondition = TIM1637_STOPCONDITION_DISABLED;

				}else if( tim1637->State == TIM1637_STATE_BUSY_IN_DISPLAY_CTRL_CMD ){
					HAL_TIM_Base_Stop_IT( &(tim1637->Timer) );
					tim1637_stop_condition(tim1637);
					tim1637->State = TIM1637_STATE_READY;

				}else if( tim1637->State == TIM1637_STATE_BUSY_IN_TX_BYTES ){

					if( tim1637->Method == TIM1637_METHOD_1BYTE_DATA){
						HAL_TIM_Base_Stop_IT( &(tim1637->Timer) );
						tim1637_stop_condition(tim1637);
						tim1637->State = TIM1637_STATE_READY;

					}else if( tim1637->Method == TIM1637_METHOD_6BYTES_DATA && tim1637->Data_Idx == (TIM1637_NUM_DIGITS - 1 )){
						HAL_TIM_Base_Stop_IT( &(tim1637->Timer) );
						tim1637_stop_condition(tim1637);
						tim1637->State = TIM1637_STATE_READY;
					}

				}

				count = 0;

			}else if( count == 19 && tim1637->StopCondition == TIM1637_STOPCONDITION_DISABLED ){

				if( tim1637->State == TIM1637_STATE_BUSY_IN_ADDR_CMD){

					tim1637->State = TIM1637_STATE_BUSY_IN_TX_BYTES;
					tim1637->StartCondition = TIM1637_STARTCONDITION_DISABLED;

					if( tim1637->Method == TIM1637_METHOD_1BYTE_DATA){
						tim1637->StopCondition = TIM1637_STOPCONDITION_ENABLED;
					}else if( tim1637->Method == TIM1637_METHOD_6BYTES_DATA ){
						tim1637->StopCondition = TIM1637_STOPCONDITION_DISABLED;
					}

				}else if( tim1637->State == TIM1637_STATE_BUSY_IN_TX_BYTES && tim1637->Method == TIM1637_METHOD_6BYTES_DATA){

					tim1637->Data_Idx ++;
					if( tim1637->Data_Idx == (TIM1637_NUM_DIGITS - 1) ){
						tim1637->StopCondition = TIM1637_STOPCONDITION_ENABLED;
					}

				}

				count = 0;
			}
	    }
	}
}

/**
  * @brief  Use to Control the displays, On/Off and level of brightness.
  * @note
  * @param  TIM1637_DisplayCtrl_e OnOff
  * @param  TIM1637_PulseWidth_e Brightness represent the level of brightness, value from 0 to 7.
  * @retval None
  */
static void tim1637_send_displayctrl( TIM1637_Handle_t* tim1637, TIM1637_DisplayCtrl_e OnOff , TIM1637_PulseWidth_e Brightness ){

	if( tim1637->State ==  TIM1637_STATE_READY){

		tim1637->Method = TIM1637_METHOD_DISPLAY_CTRL;
		tim1637->StartCondition = TIM1637_STARTCONDITION_ENABLED;
		tim1637->StopCondition = TIM1637_STOPCONDITION_ENABLED;

		// Set first command to send: Write SRAM data in a fixed address mode
		tim1637->Commands[ TIM1637_CMDIDX_DISPLAY_CTR ] = TIM1637_DISPLAY_CTRL |  ( (OnOff & 0x1) << 0x03 )  | ( Brightness & 0x07 );

		// Update the current state to:
		tim1637->State = TIM1637_STATE_BUSY_IN_DISPLAY_CTRL_CMD;

		// Start Update Interrupt event to send messages.
		HAL_TIM_Base_Start_IT( &(tim1637->Timer) );
	}
}

/**
  * @brief  Send 1 byte to specific display in fixed address mode.
  * @note
  * @param	uint8_t DisplayValue represent the value to decode in 8 segments display
  * @param  TIM1637_DisplayAddress_e DisplayAddr specifies the display to write the value.
  * @retval None
  */
static void tim1637_send_1byte( TIM1637_Handle_t* tim1637, uint8_t DisplayValue , TIM1637_DisplayAddress_e DisplayAddr ){

	if( tim1637->State == TIM1637_STATE_READY ){

		tim1637->Method = TIM1637_METHOD_1BYTE_DATA;
		tim1637->StartCondition = TIM1637_STARTCONDITION_ENABLED;
		tim1637->StopCondition = TIM1637_STOPCONDITION_ENABLED;

		// Set first command to send: Write SRAM data in a fixed address mode
		tim1637->Commands[TIM1637_CMDIDX_DATA] = TIM1637_DATA_CMD_FIX_ADDR;

		// Set second command to send: The command is used to set the display register address
		tim1637->Commands[TIM1637_CMDIDX_ADDR] = TIM1637_ADDR_CMD_SETTING | DisplayAddr;

		// Load the Value to send
		tim1637->Data[0] = DisplayValue;
		tim1637->Data_Idx = 0;

		// Update the state to:
		tim1637->State = TIM1637_STATE_BUSY_IN_DATA_CMD;

		// Start Update Interrupt event to send messages.
		HAL_TIM_Base_Start_IT( &(tim1637->Timer) );
	}

}

/**
  * @brief  Use to send 6 bytes consecutive in Automatic Add address mode.
  * @note
  * @param  uint8_t ArrayBytes[] contains the 6 bytes to decode the 8 segments for each display.
  * @retval None
  */
static void tim1637_send_6bytes( TIM1637_Handle_t* tim1637, uint8_t ArrayBytes[] ){

	if( tim1637->State == TIM1637_STATE_READY ){

		tim1637->Method = TIM1637_METHOD_6BYTES_DATA;
		tim1637->StartCondition = TIM1637_STARTCONDITION_ENABLED;
		tim1637->StopCondition = TIM1637_STOPCONDITION_ENABLED;

		// Set first command to send: Write SRAM data in a fixed address mode
		tim1637->Commands[TIM1637_CMDIDX_DATA] = TIM1637_DATA_CMD_AUTO_ADDR;

		// Set second command to send: The command is used to set the display register address
		tim1637->Commands[TIM1637_CMDIDX_ADDR] = TIM1637_ADDR_CMD_SETTING;

		// Load the Values to send
		for( uint8_t digit = 0; digit < TIM1637_NUM_DIGITS; digit ++ ){

			switch (digit) {
				case 0x0:
					tim1637->Data[ TIM1637_DISPLAYADDR_0 ] = ArrayBytes[digit];
					break;
				case 0x1:
					tim1637->Data[ TIM1637_DISPLAYADDR_1 ] = ArrayBytes[digit];
					break;
				case 0x2:
					tim1637->Data[ TIM1637_DISPLAYADDR_2 ] = ArrayBytes[digit];
					break;
				case 0x3:
					tim1637->Data[ TIM1637_DISPLAYADDR_3 ] = ArrayBytes[digit];
					break;
				case 0x4:
					tim1637->Data[ TIM1637_DISPLAYADDR_4 ] = ArrayBytes[digit];
					break;
				case 0x5:
					tim1637->Data[ TIM1637_DISPLAYADDR_5 ] = ArrayBytes[digit];
					break;
				default:
					break;
			}

		}
		tim1637->Data_Idx = 0;

		// Update the state to:
		tim1637->State = TIM1637_STATE_BUSY_IN_DATA_CMD;

		// Start Update Interrupt event to send messages.
		HAL_TIM_Base_Start_IT( &(tim1637->Timer) );
	}

}



/*	*********************************
 * 		Define Private Methods
 *  *********************************/

/**
  * @brief  Generate the Start Condition for communication protocol with TIM1637
  * @note
  * @param
  * @retval None
  */
static void tim1637_start_condition(TIM1637_Handle_t* tim1637){
	HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(tim1637->SCLK_gpio, tim1637->SCLK_pin, GPIO_PIN_RESET);
}

/**
  * @brief  Generate the stop condition for communication protocol with TIM1637
  * @note
  * @param
  * @retval None
  */
static void tim1637_stop_condition(TIM1637_Handle_t* tim1637){
	HAL_GPIO_WritePin(tim1637->SCLK_gpio, tim1637->SCLK_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, GPIO_PIN_SET);
}

/**
  * @brief  Enable the GPIO peripheral clock and configure the SCLK and SDIO as outputs.
  * @note	None
  * @param  None
  * @retval None
  */
static void tim1637_msp_gpio(TIM1637_Handle_t* tim1637){

	#ifdef STM32F446xx
		if( tim1637->SCLK_gpio == GPIOA || tim1637->SDIO_gpio == GPIOA)		__HAL_RCC_GPIOA_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOB || tim1637->SDIO_gpio == GPIOB)		__HAL_RCC_GPIOB_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOC || tim1637->SDIO_gpio == GPIOC)		__HAL_RCC_GPIOC_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOD || tim1637->SDIO_gpio == GPIOD)		__HAL_RCC_GPIOD_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOE || tim1637->SDIO_gpio == GPIOE)		__HAL_RCC_GPIOE_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOF || tim1637->SDIO_gpio == GPIOF)		__HAL_RCC_GPIOF_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOG || tim1637->SDIO_gpio == GPIOG)		__HAL_RCC_GPIOG_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOH || tim1637->SDIO_gpio == GPIOH)		__HAL_RCC_GPIOH_CLK_ENABLE();
	#elif defined(STM32F103x6)
		if( tim1637->SCLK_gpio == GPIOA || tim1637->SDIO_gpio == GPIOA)		__HAL_RCC_GPIOA_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOB || tim1637->SDIO_gpio == GPIOB)		__HAL_RCC_GPIOB_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOC || tim1637->SDIO_gpio == GPIOC)		__HAL_RCC_GPIOC_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOD || tim1637->SDIO_gpio == GPIOD)		__HAL_RCC_GPIOD_CLK_ENABLE();
	#elif defined(STM32H723xx)
		if( tim1637->SCLK_gpio == GPIOA || tim1637->SDIO_gpio == GPIOA)		__HAL_RCC_GPIOA_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOB || tim1637->SDIO_gpio == GPIOB)		__HAL_RCC_GPIOB_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOC || tim1637->SDIO_gpio == GPIOC)		__HAL_RCC_GPIOC_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOD || tim1637->SDIO_gpio == GPIOD)		__HAL_RCC_GPIOD_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOE || tim1637->SDIO_gpio == GPIOE)		__HAL_RCC_GPIOE_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOF || tim1637->SDIO_gpio == GPIOF)		__HAL_RCC_GPIOF_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOG || tim1637->SDIO_gpio == GPIOG)		__HAL_RCC_GPIOG_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOH || tim1637->SDIO_gpio == GPIOH)		__HAL_RCC_GPIOH_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOJ || tim1637->SDIO_gpio == GPIOJ)		__HAL_RCC_GPIOJ_CLK_ENABLE();
		if( tim1637->SCLK_gpio == GPIOK || tim1637->SDIO_gpio == GPIOK)		__HAL_RCC_GPIOK_CLK_ENABLE();
	#endif

	GPIO_InitTypeDef sclk_sdio_pins = {0};
	sclk_sdio_pins.Mode = GPIO_MODE_OUTPUT_PP;
	sclk_sdio_pins.Pull = GPIO_NOPULL;
	sclk_sdio_pins.Speed = GPIO_SPEED_MEDIUM;

	sclk_sdio_pins.Pin = tim1637->SCLK_pin;
	HAL_GPIO_Init(tim1637->SCLK_gpio, &sclk_sdio_pins);
	HAL_GPIO_WritePin(tim1637->SCLK_gpio, tim1637->SCLK_pin, GPIO_PIN_SET);

	sclk_sdio_pins.Pin = tim1637->SDIO_pin;
	HAL_GPIO_Init(tim1637->SDIO_gpio, &sclk_sdio_pins);
	HAL_GPIO_WritePin(tim1637->SDIO_gpio, tim1637->SDIO_pin, GPIO_PIN_SET);

}

/**
  * @brief  Enable the selected Timer, Enable the IRQ and set the IRQ priority as lowest.
  * @note	None
  * @param  None
  * @retval None
  */
static void tim1637_msp_tim(TIM1637_Handle_t* tim1637){

	uint8_t PreemptPriority, SubPriority;
	uint32_t PriorityGrouping = HAL_NVIC_GetPriorityGrouping();
	if(PriorityGrouping == NVIC_PRIORITYGROUP_4){
		PreemptPriority = 15;
		SubPriority = 0;
	}else if(PriorityGrouping == NVIC_PRIORITYGROUP_3){
		PreemptPriority = 7;
		SubPriority = 1;
	}else if(PriorityGrouping == NVIC_PRIORITYGROUP_2){
		PreemptPriority = 3;
		SubPriority = 3;
	}else if(PriorityGrouping == NVIC_PRIORITYGROUP_1){
		PreemptPriority = 1;
		SubPriority = 7;
	}else if(PriorityGrouping == NVIC_PRIORITYGROUP_0){
		PreemptPriority = 0;
		SubPriority = 15;
	}

	#ifdef STM32F446xx
		if( tim1637->Timer.Instance == TIM1){
			__HAL_RCC_TIM1_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
			HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM2){
			__HAL_RCC_TIM2_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM2_IRQn);
			HAL_NVIC_SetPriority(TIM2_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM3){
			__HAL_RCC_TIM3_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM3_IRQn);
			HAL_NVIC_SetPriority(TIM3_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM4){
			__HAL_RCC_TIM4_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM4_IRQn);
			HAL_NVIC_SetPriority(TIM4_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM5){
			__HAL_RCC_TIM5_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM5_IRQn);
			HAL_NVIC_SetPriority(TIM5_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM6){
			__HAL_RCC_TIM6_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
			HAL_NVIC_SetPriority(TIM6_DAC_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM7){
			__HAL_RCC_TIM7_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM7_IRQn);
			HAL_NVIC_SetPriority(TIM7_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM8){
			__HAL_RCC_TIM8_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
			HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM9){
			__HAL_RCC_TIM9_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
			HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM10){
			__HAL_RCC_TIM10_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
			HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM11){
			__HAL_RCC_TIM11_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
			HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM12){
			__HAL_RCC_TIM12_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
			HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM13){
			__HAL_RCC_TIM13_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
			HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM14){
			__HAL_RCC_TIM14_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
			HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, PreemptPriority, SubPriority);
		}
	#elif defined(STM32F103x6)
		if( tim1637->Timer.Instance == TIM1){
			__HAL_RCC_TIM1_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
			HAL_NVIC_SetPriority(TIM1_UP_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM2){
			__HAL_RCC_TIM2_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM2_IRQn);
			HAL_NVIC_SetPriority(TIM2_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM3){
			__HAL_RCC_TIM3_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM3_IRQn);
			HAL_NVIC_SetPriority(TIM3_IRQn, PreemptPriority, SubPriority);
		}
	#elif defined(STM32H723xx)
		if( tim1637->Timer.Instance == TIM1){
			__HAL_RCC_TIM1_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
			HAL_NVIC_SetPriority(TIM1_UP_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM2){
			__HAL_RCC_TIM2_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM2_IRQn);
			HAL_NVIC_SetPriority(TIM2_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM3){
			__HAL_RCC_TIM3_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM3_IRQn);
			HAL_NVIC_SetPriority(TIM3_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM4){
			__HAL_RCC_TIM4_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM4_IRQn);
			HAL_NVIC_SetPriority(TIM4_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM5){
			__HAL_RCC_TIM5_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM5_IRQn);
			HAL_NVIC_SetPriority(TIM5_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM6){
			__HAL_RCC_TIM6_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
			HAL_NVIC_SetPriority(TIM6_DAC_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM7){
			__HAL_RCC_TIM7_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM7_IRQn);
			HAL_NVIC_SetPriority(TIM7_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM8){
			__HAL_RCC_TIM8_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
			HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM12){
			__HAL_RCC_TIM12_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
			HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM13){
			__HAL_RCC_TIM13_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
			HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, PreemptPriority, SubPriority);
		}
		else if( tim1637->Timer.Instance == TIM14){
			__HAL_RCC_TIM14_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
			HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM15){
			__HAL_RCC_TIM12_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM15_IRQn);
			HAL_NVIC_SetPriority(TIM15_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM16){
			__HAL_RCC_TIM13_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM16_IRQn);
			HAL_NVIC_SetPriority(TIM16_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM17){
			__HAL_RCC_TIM14_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM17_IRQn);
			HAL_NVIC_SetPriority(TIM17_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM23){
			__HAL_RCC_TIM13_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM23_IRQn);
			HAL_NVIC_SetPriority(TIM23_IRQn, PreemptPriority, SubPriority);
		}else if( tim1637->Timer.Instance == TIM24){
			__HAL_RCC_TIM14_CLK_ENABLE();
			HAL_NVIC_EnableIRQ(TIM24_IRQn);
			HAL_NVIC_SetPriority(TIM24_IRQn, PreemptPriority, SubPriority);
		}
	#endif

}
