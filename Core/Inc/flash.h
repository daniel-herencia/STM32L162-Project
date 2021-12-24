/*
 * flash.h
 *
 *  Created on: 15 nov. 2021
 *      Author: psimo
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

#define PHOTO_ADDR 					0x08020000
#define PAYLOAD_STATE_ADDR 			0x08008000
#define COMMS_STATE_ADDR 			0x08008001
#define DEPLOYMENT_STATE_ADDR 		0x08008002
#define DEPLOYMENTRF_STATE_ADDR 	0x08008003
#define DETUMBLE_STATE_ADDR 		0x08008004
#define COUNT_PACKET_ADDR 			0x08008005
#define COUNT_WINDOW_ADDR 			0x08008006
#define COUNT_RTX_ADDR 				0x08008007
#define NOMINAL_ADDR 				0x08008008
#define LOW_ADDR 					0x08008009
#define CRITICAL_ADDR 				0x0800800A
#define PL_TIME_ADDR 				0x0800800B

//CONFIGURATION ADDRESSES
#define CONFIG_ADDR 				0x08008010
#define KP_ADDR 					0x08008010
#define GYRO_RES_ADDR 				0x08008011
#define SF_ADDR 					0x08008012
#define CRC_ADDR 					0x08008013
#define PHOTO_RESOL_ADDR 			0x08008014
#define PHOTO_COMPRESSION_ADDR 		0x08008015
#define F_MIN_ADDR 					0x08008016
#define F_MAX_ADDR 					0x08008018
#define DELTA_F_ADDR 				0x0800801A
#define INTEGRATION_TIME_ADDR 		0x0800801C

#define TLE_ADDR 					0x08008020
#define EXIT_LOW_POWER_FLAG_ADDR 	0x080080AA

//CALIBRATION ADDRESSES
#define CALIBRATION_ADDR			0x080080AB
#define MAGNETO_MATRIX_ADDR			0x080080AB
#define MAGNETO_OFFSET_ADDR			0x080080CF
#define GYRO_POLYN_ADDR 			0x080080DB
#define PHOTODIODES_OFFSET_ADDR 	0x080080F3

//TELEMETRY ADDRESSES
#define TELEMETRY_ADDR				0x08008100
#define TEMP_ADDR 					0x08008100
#define VOLTAGE_ADDR 				0x08008108
#define CURRENT_ADDR 				0x08008109
#define BATT_LEVEL_ADDR 			0x0800810A

uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint8_t *Data, uint16_t numberofbytes);

void Write_Flash(uint32_t StartPageAddress, uint8_t *Data, uint16_t numberofbytes);

void Flash_Read_Data (uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes);

void Check_Redundancy(uint32_t Address, uint8_t *RxDef, uint16_t numberofbytes);

void Read_Flash(uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes);

/********************  FLASH_Error_Codes   ***********************//*
HAL_FLASH_ERROR_NONE      0x00U  // No error
HAL_FLASH_ERROR_PROG      0x01U  // Programming error
HAL_FLASH_ERROR_WRP       0x02U  // Write protection error
HAL_FLASH_ERROR_OPTV      0x04U  // Option validity error
*/


#endif /* INC_FLASH_H_ */
