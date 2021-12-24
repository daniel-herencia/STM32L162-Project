
/*!
 * \file      comms.h
 *
 * \brief     Comms subsytem functions
 *
 *
 *
 * \code
 *
 * 				 _______    ______    ____    ____    ____    ____     ______
 * 				/ ______)  /  __  \  |    \  /    |  |    \  /    |   / _____)
 * 			   / /         | |  | |  |  \  \/  /  |  |  \  \/  /  |  ( (____
 *            ( (          | |  | |  |  |\    /|  |  |  |\    /|  |   \____ \
 *             \ \______   | |__| |  |  | \__/ |  |  |  | \__/ |  |   _____) )
 *              \_______)  \______/  |__|      |__|  |__|      |__|  (______/
 *
 *
 * \endcode
 *
 * \author    Daniel Herencia
 *
 * \author    Robert Molina
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "definitions.h"

#include <string.h>
#include <math.h>
#include "board.h"
#include "sx126x.h"
#include "radio.h"
#include "flash.h"
#include "telecomands.h"

/*This are COMMS definitions*/
#define RF_FREQUENCY 						868000000  	// 868 MHz
#define TX_OUTPUT_POWER 					22          // 22 dBm
#define LORA_SPREADING_FACTOR 				9           // [0x09 --> SF9]
#define LORA_BANDWIDTH 						0           // Radio.h changes it	Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 }			[0x04 --> 125 kHz]
#define LORA_CODINGRATE 					1           // [0x01 --> CR=4/5]
//#define LORA_LOW_DATA_RATE_OPTIMIZE 		0      		// [0x00 --> LowDataRateOptimize OFF]
#define LORA_PREAMBLE_LENGTH				8           // CSS modulations usually have 8 preamble symbols
//#define HEADER_TYPE 						0           // [0 --> Explicit header]
//#define PAYLOAD_LENGTH 					38          // [38 bytes of payload information]
//#define CRC_TYPE 							0           // [0 --> CRC OFF]	/*ARE YOU SURE??? MAYBE 4/5 0x01*/
//#define INVERT_IQ 						0           // [0 --> Standard IQ setup]
// Check CAD
// BufferBaseAddress
// LoRaSymbNumTimeout
#define TX_TIMEOUT_VALUE 					340         // Air time Tx
#define PACKET_LENGTH 						59          // Packet Size
/*End COMMS definitions*/

#define TX_WINDOW_TIMEOUT					13900

#define LORA_SYMBOL_TIMEOUT               	0       	// ??????????
#define LORA_FIX_LENGTH_PAYLOAD_ON         	false
#define LORA_FIX_LENGTH_PAYLOAD_LEN         38			//???????????
#define LORA_IQ_INVERSION_ON  				false		//?????


#define RX_TIMEOUT_VALUE                 	0			//IF NEEDED
#define BUFFER_SIZE                         30 			// Define the payload size here
#define UPLINK_BUFFER_SIZE					15
#define ACK_PAYLOAD_LENGTH					5			//ACK payload data length
#define WINDOW_SIZE							40

//CHECK THIS DEFINITIONS (I DO NOT KNOW IF THEY ARE CORRECT OR WHICH VALUE TO USE)
#define CAD_TIMER_TIMEOUT       1000        //Define de CAD timer's timeout here
#define RX_TIMER_TIMEOUT        4000        //Define de CAD timer's timeout here
#define CAD_SYMBOL_NUM          LORA_CAD_02_SYMBOL
#define CAD_DET_PEAK            22
#define CAD_DET_MIN             10
#define CAD_TIMEOUT_MS          2000
#define NB_TRY                  10


#define CONFIG_SIZE		13

void process_telecommand(uint8_t header, uint8_t info);

void configuration(void);

void tx_function(void);

void rx_function(void);

void packaging(void);

void stateMachine(void);


/*
 * FUNCTIONS OBTAINED FROM EXAMPLE MAIN.C
 */


/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/*!
 * \brief Function executed on Radio CAD Done event
 */
void OnCadDone( bool channelActivityDetected);

/*!
 * \brief Function configuring CAD parameters
 * \param [in]  cadSymbolNum   The number of symbol to use for CAD operations
 *                             [LORA_CAD_01_SYMBOL, LORA_CAD_02_SYMBOL,
 *                              LORA_CAD_04_SYMBOL, LORA_CAD_08_SYMBOL,
 *                              LORA_CAD_16_SYMBOL]
 * \param [in]  cadDetPeak     Limit for detection of SNR peak used in the CAD
 * \param [in]  cadDetMin      Set the minimum symbol recognition for CAD
 * \param [in]  cadTimeout     Defines the timeout value to abort the CAD activity
 */
void SX126xConfigureCad( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin , uint32_t cadTimeout);

/*!
 * \brief CAD timeout timer callback
 */
static void CADTimeoutTimeoutIrq( void );

/*!
 * \brief Rx timeout timer callback
 */
static void RxTimeoutTimerIrq( void );

/*!
 * \brief Average the collected RSSIs during CAD
 */
int8_t AverageCadRssi( void );

/*!
 * \brief Get the last good RSSI during CAD
 */
int8_t GetLastCadRssi( void );

/*!
 * \brief Display collected RSSIs each ms during CAD
 */
void DisplayCadRssivsTime( void );

/*!
 * \brief Function configuring CAD parameters
 * \param [in]  cadSymbolNum   The number of symbol to use for CAD operations
 *                             [LORA_CAD_01_SYMBOL, LORA_CAD_02_SYMBOL,
 *                              LORA_CAD_04_SYMBOL, LORA_CAD_08_SYMBOL,
 *                              LORA_CAD_16_SYMBOL]
 * \param [in]  cadDetPeak     Limit for detection of SNR peak used in the CAD
 * \param [in]  cadDetMin      Set the minimum symbol recognition for CAD
 * \param [in]  cadTimeout     Defines the timeout value to abort the CAD activity
 */
void SX126xConfigureCad( RadioLoRaCadSymbols_t cadSymbolNum, uint8_t cadDetPeak, uint8_t cadDetMin , uint32_t cadTimeout);





#endif /* INC_COMMS_H_ */
