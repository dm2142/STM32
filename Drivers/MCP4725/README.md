<h2 align="center">
    MCP4725
</h2>

STM32Cube IDE library for MCP4725 (12-Bit Digital-to-Analog Converter with EEPROM Memory)

#### Description

***
The library set communication with the device over I2C Bus in a Blockin Mode, to write in DAC Register, EEPROM or to read DAC Register and EEPROM.

<p align="center">
  <img src="Resources/mcp4725_demo.gif"  width="650"/>
</p>

#### Usage

***

1. Include the source file **mcp4725.c** and header file **mcp4725.h** in your project, also include the header file into the **main.c**.

```c

#include "main.h"
#include <mcp4725.h>
```

2. Use the struct **MCP4725_Handle_t** to create an instance to store the current settings of the device, as ***dac register***,  value and ***power down*** mode.

```c

typedef struct mcp4725_handle {
	I2C_HandleTypeDef * i2c_handle;		/* Store the I2C instance */
	uint32_t dac_register		: 12 ;	/* Store the last value written for DAC register or read from device */
	uint32_t eeprom_dac_register	: 12 ;	/* Store the DAC register save in EEPROM when it is is read */
	uint32_t dev_addr		: 8  ;	/* Store the mcp4725 slave address, commonly address are 0x60 or 0x61, depend of the logic state of the A0 pin*/
	uint8_t	powerdown_mode		: 2  ;	/* Store the last power down mode written to the device, reference to MCP4725_PowerDown_e */
	uint8_t eeprom_powerdown_mode	: 2  ;	/* Store the power down mode save when EEPROM is read */
}MCP4725_Handle_t;

```

3. Call the ***Init*** function to verify if the *mcp4725* is present in the I2C BUS, also configurate the initial parameters.
We need to pass a pointer for i2c handle.

```c
#define MCP4725_ADDR	0x61
MCP4725_Handle_t mcp4725_dev = {0};

int main(void){

  if( mcp4725_Init(&mcp4725_dev, &hi2c1, MCP4725_ADDR, 2047, MCP4725_NORMAL_MODE) == HAL_OK){
	  /* Return HAL_OK if mcp4725 device is present in I2C Bus */
  }

}
```

3. Write the register to change the ouptut voltage for DAC output.

```c
mcp4725_Write_DAC_Register(&mcp4725_dev, 2048);
```


#### Methods

***
```c

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

```
