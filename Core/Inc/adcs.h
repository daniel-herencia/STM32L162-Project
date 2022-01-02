/*!
 * \file      adcs.h
 *
 * \brief     ADCS subsystem funcitons
 *
 *
 * \created on: 14/12/2021
 *
 * \author    Pol Simon
 *
 * \author    David Reiss
 */

#ifndef INC_ADCS_H_
#define INC_ADCS_H_

#include "stm32l1xx_hal.h"

/*Detumble the satellite (ADCS subsystem)
 *Once it is stabilized, write detumble_state = true in the EEPROM memory */
void detumble(I2C_HandleTypeDef *hi2c);

void readPhotodiodes(ADC_HandleTypeDef *hadc);

void singlePhotodiode(ADC_HandleTypeDef *hadc);

#endif /* INC_ADCS_H_ */
