/*
 * mcp4725.h
 *
 *  Created on: Aug 14, 2024
 *      Author: DM
 *
 *  The MCP4725 has a two-wire I2C compatible serial
 *  interface for standard (100 kHz), fast (400 kHz) mode.
 */

#ifndef MCP4725_MCP4725_H_
#define MCP4725_MCP4725_H_

/* Enumerators for MCP4725 configurations */

typedef enum mcp4725_powerdown_modes{
	MCP4725_NORMAL_MODE		=	0,
	MCP4725_PD_1K_TO_GND,
	MCP4725_PD_100K_TO_GND,
	MCP4725_PD_500K_TO_GND,
	MCP4725_PD_LEN
}MCP4725_PowerDown_e;

/* MCP4725 Handle Structure */

typedef struct mcp4725_handle {
	I2C_HandleTypeDef * i2c_handle;						/* Store the I2C instance */
	uint32_t 			dac_register			: 12 ;	/* Store the last value written for DAC register or read from device */
	uint32_t 			eeprom_dac_register		: 12 ;	/* Store the DAC register save in EEPROM when it is is read */
	uint32_t 			dev_addr				: 8  ;	/* Store the mcp4725 slave address, commonly address are 0x60 or 0x61, depend of the logic state of the A0 pin*/
	uint8_t				powerdown_mode			: 2  ;	/* Store the last power down mode written to the device, reference to MCP4725_PowerDown_e */
	uint8_t 			eeprom_powerdown_mode	: 2  ;	/* Store the power down mode save when EEPROM is read */
}MCP4725_Handle_t;

/* Initialization function */
HAL_StatusTypeDef mcp4725_Init(MCP4725_Handle_t* mcp4725_dev, I2C_HandleTypeDef* i2c_handle, uint8_t mcp4725_addr, uint16_t dac_data, MCP4725_PowerDown_e pd_mode);

/* Control functions */
HAL_StatusTypeDef mcp4725_Write_PowerDown_DAC_Register(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data, MCP4725_PowerDown_e pd_mode);
HAL_StatusTypeDef mcp4725_Write_DAC_Register(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data);
HAL_StatusTypeDef mcp4725_Write_PowerDown(MCP4725_Handle_t* mcp4725_dev, MCP4725_PowerDown_e pd_mode);

/* Write and Read data to/from EEPROM */
HAL_StatusTypeDef mcp4725_Write_DAC_EEPROM(MCP4725_Handle_t* mcp4725_dev, uint16_t dac_data, MCP4725_PowerDown_e pd_mode);
HAL_StatusTypeDef mcp4725_Read_DAC_EEPROM(MCP4725_Handle_t* mcp4725_dev);

/* General call*/
HAL_StatusTypeDef mcp4725_GeneralCall_Reset(MCP4725_Handle_t* mcp4725_dev);
HAL_StatusTypeDef mcp4725_GeneralCall_WakeUp(MCP4725_Handle_t* mcp4725_dev);

#endif /* MCP4725_MCP4725_H_ */
