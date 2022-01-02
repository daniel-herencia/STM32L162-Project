/*!
 * \file      configuration.h
 *
 * \brief     It contains the initializing functions and all the functions that check
 * 			  the satellite state
 *
 *
 * \created on: 01/10/2021
 *
 * \author    Pol Simon
 *
 * \author    David Reiss
 */

#ifndef INC_CONFIGURATION_H_
#define INC_CONFIGURATION_H_

#include "definitions.h"
#include "flash.h"

static const uint8_t GYRO_ADDR = 0x68 << 1; //gyroscope address, 0x68 or 0x69 depending on the SA0 pin
static const uint8_t MAG_ADDR = 0x30 << 1; //magnetometer address
static const uint8_t BATTSENSOR_ADDR = 0x34 << 1; //battery sensor address

/*Only at the beginning, includes the Antenna deployment, check batteries, configure payloads*/
/*Will be executed every time we reboot the system*/
void init(I2C_HandleTypeDef *hi2c);

/*Initialize all the sensors*/
void initsensors(I2C_HandleTypeDef *hi2c);

/*Compute the level of battery and updates batterylevel*/
void checkbatteries(I2C_HandleTypeDef *hi2c);

/*Check if the memory have enough space to store a photo/spectrum*/
bool checkmemory(void);

/*Check the temperature of the 8 sensors, returns true if the temperature is between TEMP_MAX and TEMP_MIN*/
bool checktemperature(I2C_HandleTypeDef *hi2c);

/*Activate or deactivate the battery heater*/
void heater(int state);

/*Send a signal to a resistor to burn a wire and deploy COMMS Antenna
 *Check the switch to make sure the Antenna has been deployed properly
 *And write deployment_state = true in the EEPROM memory*/
void deployment(I2C_HandleTypeDef *hi2c);

/*Send a signal to a resistor to burn a wire and deploy PL2 Antenna
 *Once confirmed the proper deployment of the antenna,  write deploymentRF_state = true in the EEPROM memory*/
void deploymentRF(I2C_HandleTypeDef *hi2c);

/*We'll have a file (SPG4) which will tell us the position
 *Check if we are in the region of contact with GS */
void check_position(void);

/*Check battery level, temperatures,etc
 *If each parameter is between a specified values returns true*/
bool system_state(I2C_HandleTypeDef *hi2c);

#endif /* INC_CONFIGURATION_H_ */
