/*!
 * \file      adcs.c
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

#include "adcs.h"

/**************************************************************************************
 *                                                                                    *
 * Function:  detumble                                                 		  		  *
 * --------------------                                                               *
 * Checks the gyroscope measurements and stabilizes the satellite. 					  *
 * It is called when the satellite is ejected from the deployer						  *
 *                                                                                    *
 *  hi2c: I2C to read outputs from gyroscope					    				  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void detumble(I2C_HandleTypeDef *hi2c) {

}

/**************************************************************************************
 *                                                                                    *
 * Function:  readPhotodiodes                                                 		  *
 * --------------------                                                               *
 * Obtains the output values from all the photodiodes, varying the selectors		  *
 * of the multiplexor (GPIOs PA11 and PA12) 										  *
 *                                                                                    *
 *  hadc: ADC to read outputs from the photodiodes				    				  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void readPhotodiodes(ADC_HandleTypeDef *hadc) { // I think the four ADC should be passed as parameters
	/*3 photodiodes are directly connected to 3 of the 4 ADC pins
	 * the other 3 photodiodes are connected through the inputs 1,2 and 3 of a multiplexor*/
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	singlePhotodiode(hadc);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	singlePhotodiode(hadc);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	singlePhotodiode(hadc);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

}

/**************************************************************************************
 *                                                                                    *
 * Function:  singlePhotodiode                                         		  		  *
 * --------------------                                                               *
 * Obtains the output value from a single photodiode.								  *
 *                                                                                    *
 *  hadc: ADC to read outputs from the photodiodes				    				  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void singlePhotodiode(ADC_HandleTypeDef *hadc) {

}
