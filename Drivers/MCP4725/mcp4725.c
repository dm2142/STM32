/*
 * mcp4725.c
 *
 *  Created on: Aug 14, 2024
 *      Author: DM
 */


/* Include HAL libraries according the device */

#if defined(STM32F446xx) || defined(STM32F401xC)
	#include "stm32f4xx_hal.h"
#elif defined(STM32F103x6)
	#include "stm32f1xx_hal.h"
#elif defined(STM32H723xx)
	#include "stm32h7xx_hal.h"
#endif

#include "mcp4725.h"

#define MAX_TRIALS	3
#define TIMEOUT		5

#define FM_POWERDOWN_POS		4	/* Power Down Position bits to write DAC register in Fast Mode */

#define WRITE_EEPROM_CMD		0x03
#define WRITE_EEPROM_CMD_POS	5
#define WRITE_EEPROM_PD_POS		1

#define GENERAL_CALL_RESET		0x06
#define GENERAL_CALL_WAKEUP		0x09

/**
  * @brief  Initialize the mcp4725 instance with corresponding values. Write the DAC register and the power down mode selected.
  * Also, read the EEPROM to know the power on reset parameters (DAC Register and Power Down modes).
  * @param  mcp4725_dev Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  *@param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for the specified I2C.
  *@param mcp4725_addr Specified the mcp4725 slave address (0x60 or 0x61).
  *		  According with the Datasheet; MPC4725 has the device MSB 1100 A2 A1 A0.
  *		  Normally A2 and A1 are 0. A0 use the logic level state of A0 pin.
  *@param dac_data  12-bit value for DAC output.
  *@param pd_mode	Set Power Down Mode for device. Reference to MCP4725_PowerDown_e
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Init(MCP4725_Handle_t* mcp4725_dev, I2C_HandleTypeDef* i2c_handle, uint8_t mcp4725_addr, uint16_t dac_data, MCP4725_PowerDown_e pd_mode){

	/* Verify MCP4725 device with mcp4725_addr address is present in I2C Bus */
	if( HAL_I2C_IsDeviceReady(i2c_handle, mcp4725_addr << 1, MAX_TRIALS, TIMEOUT) == HAL_OK ){

		mcp4725_dev->i2c_handle = i2c_handle;
		mcp4725_dev->dev_addr = mcp4725_addr;

		uint8_t status1, status2;
		/* Write the configuration for mcp4725 device in Fast Mode Command */
		status1 = mcp4725_Write_PowerDown_DAC_Register(mcp4725_dev, dac_data, pd_mode);

		/* Update the EEPROM values in the instance reading the mcp4725 device EEPROM */
		status2 = mcp4725_Read_DAC_EEPROM(mcp4725_dev);

		if( status1 == HAL_OK && status2 == HAL_OK ){
			/* If Initial configuration is successful */
			return HAL_OK;
		}else{
			return HAL_ERROR;
		}

	}else{
		return HAL_ERROR;	/* The specified address device is not present in the I2C Bus */
	}

}

/**
  * @brief  Write the specified DAC data and Power Mode.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Write_PowerDown_DAC_Register(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data, MCP4725_PowerDown_e pd_mode){

	uint8_t data[2] = {0};
	data[0] = ( ( (uint8_t) pd_mode) << FM_POWERDOWN_POS ) | ( ( dac_data & 0xF00 ) >> 8 );
	data[1] =  (uint8_t) ( dac_data & 0xFF);

	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, mcp4725_dev->dev_addr << 1, &data[0], 2, TIMEOUT) == HAL_OK ){

		mcp4725_dev->powerdown_mode = pd_mode;
		mcp4725_dev->dac_register = dac_data;
		return HAL_OK;

	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  The fast write command is used to update the DAC register. The data in the EEPROM of the device is not
  * affected by this command. This command updates Power-Down mode selection bits (PD1 and PD0) and 12 bits of the DAC input code in the DAC register.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Write_DAC_Register(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data){

	uint8_t data[2] = {0};
	data[0] = ( ( (uint8_t) mcp4725_dev->powerdown_mode) << FM_POWERDOWN_POS ) | ( ( dac_data & 0xF00 ) >> 8 );
	data[1] =  (uint8_t) ( dac_data & 0xFF);

	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, mcp4725_dev->dev_addr << 1, &data[0], 2, TIMEOUT) == HAL_OK ){

		mcp4725_dev->dac_register = dac_data;
		return HAL_OK;

	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  he fast write command is used to update the DAC register. The data in the EEPROM of the device is not
  * affected by this command. This command updates Power-Down mode selection bits (PD1 and PD0) and 12 bits of the DAC input code in the DAC register..
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Write_PowerDown(MCP4725_Handle_t* mcp4725_dev, MCP4725_PowerDown_e pd_mode){

	uint8_t data[2] = {0};
	data[0] = ( ( (uint8_t) pd_mode) << FM_POWERDOWN_POS ) | ( ( mcp4725_dev->dac_register & 0xF00 ) >> 8 );
	data[1] =  (uint8_t) ( mcp4725_dev->dac_register & 0xFF);

	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, mcp4725_dev->dev_addr << 1, &data[0], 2, TIMEOUT) == HAL_OK ){

		mcp4725_dev->powerdown_mode = pd_mode;
		return HAL_OK;

	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  When the device receives this command, it (a) loads the configuration and data bits to the DAC register,
  * and (b) also writes the EEPROM.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Write_DAC_EEPROM(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data, MCP4725_PowerDown_e pd_mode){

	uint8_t data[3] = {0};

	data[0] = ( WRITE_EEPROM_CMD << WRITE_EEPROM_CMD_POS )  | ( pd_mode << WRITE_EEPROM_PD_POS);
	data[1] = ( (dac_data & 0xFF0) >> 4);
	data[2] = ( (dac_data & 0x0F) << 4);

	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, mcp4725_dev->dev_addr << 1, data, 3, TIMEOUT) == HAL_OK ){
		return HAL_OK;
	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  Read the current settings and EEPROM settings from device and updated in the MCP4725_Handle_t instance.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_Read_DAC_EEPROM(MCP4725_Handle_t* mcp4725_dev){

	uint8_t data[5] = {0};
	if( HAL_I2C_Master_Receive(mcp4725_dev->i2c_handle, mcp4725_dev->dev_addr << 1, data, 5, TIMEOUT) == HAL_OK ){

		mcp4725_dev->powerdown_mode = (data[0] & 0x06) >> 1;
		mcp4725_dev->dac_register = (data[1] << 4) | ( ( data[2] & 0xF0) >> 4 );
		mcp4725_dev->eeprom_powerdown_mode = (data[3] & 0x60) >> 5;
		mcp4725_dev->eeprom_dac_register = ( (data[3] & 0x0F) << 8 ) | data[4];

		return HAL_OK;
	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  Make a general call reset, the device will abort current conversion and perform an
  * 		internal reset similar to a power-on-reset (POR). Immediately after this reset event, the device uploads the
  * 		contents of the EEPROM into the DAC register.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_GeneralCall_Reset(MCP4725_Handle_t* mcp4725_dev){

	const uint8_t data = GENERAL_CALL_RESET;
	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, 0x00, (uint8_t *) &data, 1, TIMEOUT) == HAL_OK){

		mcp4725_dev->dac_register = mcp4725_dev->eeprom_dac_register;
		mcp4725_dev->powerdown_mode = mcp4725_dev->eeprom_powerdown_mode;
		return HAL_OK;
	}else{
		return HAL_ERROR;
	}

}

/**
  * @brief  Make a general call wakeup, after receiving this command, the power-down bits of the DAC register are set
  * to a normal operation (PD1, PD2 = 0,0). The powerdown bit settings in EEPROM are not affected.
  * @param  hi2c Pointer to a MCP4725_Handle_t structure that contains
  *         the configuration information for the specified mcp4527 device.
  * @retval HAL status
  */
HAL_StatusTypeDef mcp4725_GeneralCall_WakeUp(MCP4725_Handle_t* mcp4725_dev){

	const uint8_t data = GENERAL_CALL_WAKEUP;
	if( HAL_I2C_Master_Transmit(mcp4725_dev->i2c_handle, 0x00, (uint8_t *) &data, 1, TIMEOUT) == HAL_OK){

		mcp4725_dev->dac_register = mcp4725_dev->eeprom_dac_register;
		mcp4725_dev->powerdown_mode = mcp4725_dev->eeprom_powerdown_mode;
		return HAL_OK;
	}else{
		return HAL_ERROR;
	}

}
