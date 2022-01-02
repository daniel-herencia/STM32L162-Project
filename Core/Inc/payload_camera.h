/*
 * payload_camera.h
 *
 *  Created on: Nov 25, 2021
 *      Author: Jaume
 */

#ifndef INC_PAYLOAD_CAMERA_H_
#define INC_PAYLOAD_CAMERA_H_

#include "stm32l1xx_hal.h"
#include <string.h> // Usado para la funcion memcmp
#include <stdio.h>
#include <stdbool.h> //PARA EL BOOL
#include <stdlib.h> //PARA GUARDAR DATOS
//todo hacer criba de las librerias, estraer solo las funciones utiles.



uint8_t readResponse(UART_HandleTypeDef *huart, uint8_t expLength, uint8_t attempts);
bool runCommand(UART_HandleTypeDef *huart, uint8_t command, uint8_t *hexData, uint8_t dataArrayLength, uint8_t expLength, bool doFlush);

/**
  * #2
  * Takes photo and saves it in the camera memory
  */
bool captureImage(UART_HandleTypeDef *huart);

/**
  * #3
  * Actualize the value of the variable frameLength with the new length of the image
  */
void getFrameLength(UART_HandleTypeDef *huart);

/**
  * #4
  * Saves the image to the flash memory of the STM32
  */
void retrieveImage(UART_HandleTypeDef *huart);

/**
  * #5
  *
  */
void stopCapture(UART_HandleTypeDef *huart);

/**
  * #6
  *
  */
void setCopressibility(UART_HandleTypeDef *huart, uint8_t compressibility);

/**
  * #7
  *
  */
void setResolution(UART_HandleTypeDef *huart, uint8_t resolution);

/**
  * #8
  *
  */
bool takePhoto(UART_HandleTypeDef *huart);


int min(int bSize, int frameLength);


#endif /* INC_PAYLOAD_CAMERA_H_ */
