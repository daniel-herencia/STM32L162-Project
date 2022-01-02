/*!
 * \file      sensorReadings.h
 *
 * \brief     Updates temperatures, voltages and currents values
 *
 *
 * \created on: 01/10/2021
 *
 * \author    Pol Simon
 *
 * \author    David Reiss
 */

#ifndef INC_SENSORREADINGS_H_
#define INC_SENSORREADINGS_H_

#include "definitions.h"
#include "configuration.h"

/*Overwrites into the Temperatures Union the temperature of the 8 sensors*/
void acquireTemp(I2C_HandleTypeDef *hi2c);

/*Overwrites into the Voltages Union the readings of the 12 voltages*/
void acquireVoltage(I2C_HandleTypeDef *hi2c);

/*Overwrites into the Currents Union the readings of the 7 currents*/
void acquireCurrents(I2C_HandleTypeDef *hi2c);

/*Includes the functions above*/
void sensorReadings(I2C_HandleTypeDef *hi2c);

#endif /* INC_SENSORREADINGS_H_ */
