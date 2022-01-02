/*!
 * \file      sensorReadings.c
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

#include "sensorReadings.h"

/**************************************************************************************
 *                                                                                    *
 * Function:  acquireTemp	                                             	  		  *
 * --------------------                                                               *
 * Reads temperatures from all the sensors, and stores the struct in memory			  *
 *																					  *
 *  hi2c: I2C to read from the temperature sensors				    				  *
 *															                          *
 *  returns: Nothing									                              *
 *  		 																		  *
 **************************************************************************************/
void acquireTemp(I2C_HandleTypeDef *hi2c){
	Temperatures temperatures_local;
	int i;
	HAL_StatusTypeDef ret;
	uint8_t buf[6];
	int16_t val;
	uint8_t REG_TEMP = 0x00;
	float temp_c; //defineixo float però no sé si es pot guardar al tipus Temperatures.raw
	uint8_t ADDR[6] = {/*Adreça 1, Adreça 2, etc*/};//adreces deks diferents sensors de temperatura
	for(i=0; i < 6; i++){
		// Tell TMP102 that we want to read from the temperature register
		buf[0] = REG_TEMP;
		ret = HAL_I2C_Master_Transmit(&hi2c, ADDR[i], buf, 1, 1000);
		if ( ret != HAL_OK ) {
			//strcpy((char*)buf, "Error Tx\r\n");
		} else {
			  // Read 2 bytes from the temperature register
			  ret = HAL_I2C_Master_Receive(&hi2c, ADDR[i], buf, 2, 1000);
			  if ( ret != HAL_OK ) {
				  //strcpy((char*)buf, "Error Rx\r\n");
			  } else {
				  //Combine the bytes
				  val = ((int16_t)buf[0] << 4) | (buf[1] >> 4);

				  // Convert to 2's complement, since temperature can be negative
				  if ( val > 0x7FF ) {
					  val |= 0xF000;
				  }
				  // Convert to float temperature value (Celsius)
				  temp_c = val * 0.0625;
				  switch(i) {
				  case 0:
					  temperatures_local.fields.temp1 = temp_c;
					  break;
				  case 1:
					  temperatures_local.fields.temp2 = temp_c;
					  break;
				  case 2:
					  temperatures_local.fields.temp3 = temp_c;
					  break;
				  case 3:
					  temperatures_local.fields.temp4 = temp_c;
					  break;
				  case 4:
					  temperatures_local.fields.temp5 = temp_c;
					  break;
				  case 5:
					  temperatures_local.fields.temp6 = temp_c;
					  break;
				  default:
					  break;
				  }
			  }
		}
	}
	ret = HAL_I2C_Master_Transmit(hi2c, BATTSENSOR_ADDR, (uint8_t*)0x0A, 1, 500); //we want to read from the register 0x0A
	if (ret != HAL_OK) {

	} else {
		HAL_I2C_Master_Receive(hi2c, BATTSENSOR_ADDR, &buf, 1, 500);
		val = ((int16_t)buf[0] << 4);
		  // Convert to 2's complement, since temperature can be negative
		if ( val > 0x7FF ) {
			val |= 0xF000;
		}
		// Convert to float temperature value (Celsius)
		temp_c = (val/32)*(125/100);
		temperatures_local.fields.tempbatt = temp_c;
	}
	Write_Flash(TEMP_ADDR, &temperatures_local.raw, sizeof(temperatures_local));
}

/**************************************************************************************
 *                                                                                    *
 * Function:  acquireVoltage                                             	  		  *
 * --------------------                                                               *
 * Reads Voltages from all the sensors, and stores the struct in memory				  *
 *																					  *
 *  hi2c: I2C to read from the sensors							    				  *
 *															                          *
 *  returns: Nothing									                              *
 *  		 																		  *
 **************************************************************************************/
void acquireVoltage(I2C_HandleTypeDef *hi2c){
	uint8_t buf, value_to_store;
	HAL_StatusTypeDef ret;
	float volt_mV;
	ret = HAL_I2C_Master_Transmit(hi2c, BATTSENSOR_ADDR, (uint8_t*)0x0C, 1, 500); //we want to read from the register 0x0C
	if (ret != HAL_OK) {

	} else {
		HAL_I2C_Master_Receive(hi2c, BATTSENSOR_ADDR, &buf, 1, 500);
	}
	//To obtain the value in mV
	volt_mV = (buf/32)*4.88;
	//We want 1 decimal
	value_to_store = volt_mV/100;
	Write_Flash(VOLTAGE_ADDR, &value_to_store, 1);
}


/**************************************************************************************
 *                                                                                    *
 * Function:  acquireCurrents                                            	  		  *
 * --------------------                                                               *
 * Reads currents from all the sensors, and stores the struct in memory				  *
 *																					  *
 *  hi2c: I2C to read from the sensors							    				  *
 *															                          *
 *  returns: Nothing									                              *
 *  		 																		  *
 **************************************************************************************/
void acquireCurrents(I2C_HandleTypeDef *hi2c){
	uint8_t buf[1], value_to_store;
	HAL_StatusTypeDef ret;
	float current;
	ret = HAL_I2C_Master_Transmit(hi2c, BATTSENSOR_ADDR, (uint8_t*)0x08, 1, 500); //we want to read from the register 0x08
	if (ret != HAL_OK) {

	} else {
		HAL_I2C_Master_Receive(hi2c, BATTSENSOR_ADDR, &buf, 1, 500);
	}
	//To obtain the value in mV
	current = buf[0]*1.0416*pow(10,-4);
	//We want 1 decimal
	value_to_store = current;
	Write_Flash(CURRENT_ADDR, &value_to_store, 1);
}

/**************************************************************************************
 *                                                                                    *
 * Function:  SensorReadings                                             	  		  *
 * --------------------                                                               *
 * Updates all the readings (temp, voltages, currents)								  *
 *																					  *
 *  hi2c: I2C to read from the sensors							    				  *
 *															                          *
 *  returns: Nothing									                              *
 *  		 																		  *
 **************************************************************************************/
void sensorReadings(I2C_HandleTypeDef *hi2c){
	acquireTemp(hi2c);
	acquireVoltage(hi2c);
	acquireCurrents(hi2c);
}
