/*!
 * \file      definitions.h
 *
 * \brief     It contains many definitions and structs used in the code
 *
 *
 * \created on: 01/10/2021
 *
 * \author    Pol Simon
 *
 * \author    David Reiss
 */

#ifndef INC_DEFINITIONS_H_
#define INC_DEFINITIONS_H_

#include <stdbool.h>
#include <stdint.h>
//#include "stm32f4xx_hal_def.h"
//#include "stm32f4xx_hal_i2c.h"
//#include "stm32f4xx_hal_uart.h"
#include <stm32l1xx_hal.h>

/*Temperature range operation of STM32L162VE*/
#define TEMP_MIN -40
#define TEMP_MAX 105
#define TEMP_MIN_BATT 2
#define TRUE 	0x01
#define FALSE 	0x00

#define DELAY_CAMERA 2500 /*Initial operation process*/

/*Telecommands*/

/*OBC*/
#define RESET2  			01	/*The PQ might take a reset*/
#define NOMINAL				02
#define LOW					03
#define CRITICAL			04
#define EXIT_LOW_POWER		05
#define SET_TIME			06

//#define NACKDATA  			08	/*If it is received if the GS do not receive all the segments of the data.
 	 	 	 	 	 	 	 	 //*The PQ will send since the last segment received correctly.*/
/*ADCS*/
#define SET_CONSTANT_KP		10
#define TLE  				11 /*Packet from GS with the new TLE, update it inside memory
 	 	 	 	 	 	 	  the SPG4 uses it to propagate the orbit*/
#define SET_GYRO_RES		12


/*COMMS*/
#define SENDDATA  			20	/*If the acquired photo or spectogram is needed to be send to GS*/
#define SENDTELEMETRY  		21
#define STOPSENDINGDATA  	22
#define ACKDATA  			23	/*It is received when all the data is received correctly*/
#define SET_SF				24
#define SET_CRC				25
#define SEND_CALIBRATION	26

/*CAMARA*/
#define TAKEPHOTO 			30	/*Might rotate the PQ into the right position +
								wait until it is in the position where the picture is wanted to be taken.*/
#define SET_PHOTO_RESOL		31	//Photo Resolution
#define PHOTO_COMPRESSION   32

/*PAYLOAD 2: ELECTROSMOG ANTENNA*/
#define TAKERF  			40
#define F_MIN				41
#define F_MAX				42
#define DELTA_F				43
#define INTEGRATION_TIME	44


#define SEND_CONFIG			50	//Send all configuration



//#define
//#define


/*NOMINAL from 100 to 90, LOW from 90 to 85, CRITICAL from 85 to 80 (only to give us an idea*/
//enum BatteryLevel {NOMINAL=90, LOW=85, CRITICAL=80, SURVIVAL=0}; //todo talk EPS

/* USER CODE BEGIN EFP */
enum MachineState {INIT, IDLE, COMMS, PAYLOAD, CONTINGENCY, SUNSAFE, SURVIVAL};
uint8_t currentState;



/*Total of 8bytes -> 8bytes·1uit64_t/8bytes = 1 uit64_t*/
typedef union __attribute__ ((__packed__)) Temperatures {
    uint64_t raw[1];
    struct __attribute__((__packed__)) {
    	 int8_t tempbatt;
         int8_t temp1;
         int8_t temp2;
         int8_t temp3;
         int8_t temp4;
         int8_t temp5;
         int8_t temp6;
    }fields;
} Temperatures;

/*Total of 12bytes -> rounded to 16bytes -> 2 uit64_t*/
typedef union __attribute__ ((__packed__)) Voltages {
    uint64_t raw[1];
    struct __attribute__((__packed__)) {
    	uint8_t voltage1;
//    	uint8_t voltage2;
//    	uint8_t voltage3;
//    	uint8_t voltage4;
//    	uint8_t voltage5;
//    	uint8_t voltage6;
//    	uint8_t voltage7;
//    	uint8_t voltage8;
//    	uint8_t voltage9;
//    	uint8_t voltage10;
//    	uint8_t voltage11;
//    	uint8_t voltage12;
    }fields;
} Voltages;


/*Total of 5bytes -> 8 bytes -> 1 uint64_t*/
typedef union __attribute__ ((__packed__)) BatteryLevels {
    uint64_t raw[1];
    struct __attribute__((__packed__)) {
    	uint8_t totalbattery; /*Stores the total percentage of battery*/
    }fields;
} BatteryLevels;

/*Total of 7bytes -> 8bytes -> 1 uint64_t*/
typedef union __attribute__ ((__packed__)) Currents {
    uint64_t raw[1];
    struct __attribute__((__packed__)) {
    	uint8_t current1;
    	uint8_t current2;
    	uint8_t current3;
    	uint8_t current4;
    	uint8_t current5;
    	uint8_t current6;
    	uint8_t current7;
    }fields;
} Currents;

/*Total of 69bytes·2lines = 138bytes -> 144bytes -> 18 uit64_t*/
typedef union __attribute__ ((__packed__)) TLEUpdate {
    uint64_t raw[18];
    struct __attribute__((__packed__)) {
    	char tle1[69]; 						/*First line of TLE, 69 chars, 1byte each char*/
    	char tle2[69]; 						/*Second line of TLE, 69 chars, 1byte each char*/
    }fields;
} TLEUpdate;

/*Total of 20002bytes (20000+1+1) -> 20002/8 = 2500.25 rounded to 2501 uint64_t*/
typedef union __attribute__ ((__packed__)) Image {	/*const variable is stored in FLASH memory*/
    uint64_t raw[2501];
    struct __attribute__ ((__packed__)) {
    	uint8_t date;						/*When the image was acquired*/
    	uint8_t coordinates;				/*Where the image was acquired*/
    	uint8_t bufferImage[20000];			/*20000bytes worst case*/
    	uint8_t size;
    }fields;
} Image;

/*Total of 55002bytes -> 55002/8 = 6875.25 rounded to 6876 uint64_t*/
typedef union __attribute__ ((__packed__)) RadioFrequency {
    uint64_t raw[6876];
    struct __attribute__((__packed__)) {
    	uint8_t date;						/*When the image was acquired*/
    	uint8_t coordinates;				/*Where the image was acquired*/
    	uint8_t bufferRF[55000];				/*The size depends on the time acquiring, at the most about 55kB (whole orbit)
    	 	 	 	 	 	 	 	 	 	  size(bytes) = 73bits/s·(time acquiring)·1byte/8bits */
    }fields;
} RadioFrequency;

/* prova*/
#endif /* INC_DEFINITIONS_H_ */
