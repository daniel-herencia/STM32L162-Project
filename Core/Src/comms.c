
/*!
 * \file      comms.c
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

#include <comms.h>

/*------TO DO----------*/
/*
 * STATE CONTINGENCY ONLY RX => from main setContingency(true)
 * state sunsafe or survival => end comms thread (in main)
 * CHECK SLEEP MODE OF SX1262 WHILE NOT TX OR RX
 * MULTY THREAD
 * MATRIX OF READ-SALOMON => FADING IN SEVERAL PACKETS
 * Airtime timer
 */

static RadioEvents_t RadioEvents;	//To handle Radio library functions

uint32_t air_time;					//LoRa air time value
uint8_t Buffer[BUFFER_SIZE];		//Buffer to store received packets or the next packet to transit

uint8_t calib_packets = 0;			//Counter of the calibration packets received
uint8_t tle_packets = 0;			//Counter of the tle packets received
uint8_t telemetry_packets = 0;		//Counter of telemetry packets sent


/*
 * To avoid the variables being erased if a reset occurs, we have to store them in the Flash memory
 * Therefore, they have to be declared as a single-element array
 */
uint8_t count_packet[] = {0};		//To count how many packets have been sent (maximum WINDOW_SIZE)
uint8_t count_window[] = {0};		//To count the window number
uint8_t count_rtx[] = {0};			//To count the number of retransmitted packets

uint8_t i = 0;						//variable for loops
uint8_t j=0;						//variable for loops
uint8_t k=0;						//variable for loops

uint64_t ack;						//Information rx in the ACK (0 => ack, 1 => nack)
uint8_t nack_number;				//Number of the current packet to retransmit
bool nack;							//True when retransmission necessary
bool full_window;					//Stop & wait => to know when we reach the limit packet of the window
bool statemach = true;				//If true, comms workflow follows the state machine. This value should be controlled by OBC
									//Put true before activating the statemachine thread. Put false before ending comms thread


bool send_data = false;				//If true, the state machine send packets every airtime
bool send_telemetry = false;		//If true, we have to send telemetry packets instead of payload data
uint8_t num_telemetry = 0;			//Total of telemetry packets that have to be sent (computed when telecomand send telemetry received)
bool contingency = false;			//True if we are in contingency state => only receive

/*
 * STATE MACHINE VARIABLES AND DEFINITIONS
 */

//Possible states
typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
    START_CAD,
}States_t;

//Possible CAD events
typedef enum
{
    CAD_FAIL,
    CAD_SUCCESS,
    PENDING,
}CadRx_t;

States_t State = LOWPOWER;				//Variable to store the current state

int8_t RssiValue = 0;					//Rssi computed value
int8_t SnrValue = 0;					//SNR computed value

CadRx_t CadRx = CAD_FAIL;				//Current CAD state
bool PacketReceived = false;			//To know if a packet have been received or not
bool RxTimeoutTimerIrqFlag = false;		//Flag to know if there has been any interruption
int16_t RssiMoy = 0;					//Rssi stored value
int8_t SnrMoy = 0;						//SNR stored value
uint16_t RxCorrectCnt = 0;				//Counter of correct received packets
uint16_t BufferSize = BUFFER_SIZE;		//Buffer size

//TIMERS
TimerEvent_t CADTimeoutTimer;			//CAD timer
TimerEvent_t RxAppTimeoutTimer;			//Reception Timer


/**************************************************************************************
 *                                                                                    *
 * 	Function:  configuration		                                                  *
 * 	--------------------                                                              *
 * 	function to configure the transceiver and the transmission protocol parameters    *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void configuration(void){

	Radio.Init( &RadioEvents );

	Radio.SetChannel( RF_FREQUENCY );

	uint8_t sf;
	Read_Flash(SF_ADDR, &sf, 1);
	uint8_t coding_rate;
	Read_Flash(CRC_ADDR, &coding_rate, 1);

	Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH, sf, coding_rate,
								   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
								   true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE );	//In the original example it was 3000


	//SHALL WE CARE ABOUT THE RX TIMEOUT VALUE??? IF YES, CHANGE IT IN SetRx FUNCTION
	Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, sf, coding_rate, 0, LORA_PREAMBLE_LENGTH,
								   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
								   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );



	//Air time calculus
	air_time = Radio.TimeOnAir( MODEM_LORA , PACKET_LENGTH );

	Flash_Read_Data( COUNT_PACKET_ADDR , &count_packet , sizeof(count_packet) );	//Read from Flash count_packet
	Flash_Read_Data( COUNT_WINDOW_ADDR , &count_window , sizeof(count_window) ); 	//Read from Flash count_window
	Flash_Read_Data( COUNT_RTX_ADDR , &count_rtx , sizeof(count_rtx) ); 			//Read from Flash count_rtx
	ack = 0xFFFFFFFFFFFFFFFF;														//Initially confifured 111..111
	nack = false;
	State = RX;

};


/**************************************************************************************
 *                                                                                    *
 * 	Function:  tx_function			                                                  *
 * 	--------------------                                                              *
 * 	function to transmit the next packet											  *
 * 																					  *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void tx_function(void){
	//configuration();
	if (!full_window)
	{
		packaging(); //Start the TX by packaging all the data that will be transmitted
		Radio.Send( Buffer, BUFFER_SIZE );
		Flash_Write_Data( COUNT_PACKET_ADDR , &count_packet , sizeof(count_packet) );	//Read from Flash count_packet
		Flash_Write_Data( COUNT_WINDOW_ADDR , &count_window , sizeof(count_window) ); 	//Read from Flash count_window
		Flash_Write_Data( COUNT_RTX_ADDR , &count_rtx , sizeof(count_rtx) ); 			//Read from Flash count_rtx
	}
};


/**************************************************************************************
 *                                                                                    *
 * 	Function:  rx_function              		                                      *
 *  --------------------                                                              *
 * 	to receive a packet detected												      *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void rx_function(void){
	Radio.Rx( RX_TIMEOUT_VALUE );

};

/**************************************************************************************
 *                                                                                    *
 * 	Function:  packaging                                                   			  *
 * --------------------                                                               *
 * 	to store in Buffer the next packet to send (taking into account that it could be  *
 * 	data coming from the payload, telemetry or a retransmission of a packet from the  *
 * 	last windows																	  *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void packaging(void){
	if (nack)	//NACK packets are sent at the beginning of the next window
	{
		while(i<sizeof(ack))
		{
			//function to obtain the packets to retx
			if(!((ack >> i) & 1)) //When position of the ack & 1 != 1 --> its a 0 --> NACK
			{
				nack_number = i;	//Current packet to rtx
				//Packet from last window => count_window - 1
				Flash_Read_Data( PHOTO_ADDR + (count_window[0]-1)*WINDOW_SIZE*BUFFER_SIZE + (nack_number)*BUFFER_SIZE , &Buffer , sizeof(Buffer) );	//Direction in HEX
				count_rtx[0]++;
				i++;
				break;	//Exits the while when the we find a packet to rtx
			}
			i++;
		}
		if (i==sizeof(ack)){
			i=0;
			ack = 0xFFFFFFFFFFFFFFFF;
			nack = false;
		}
	}
	else if (send_telemetry){
		Flash_Read_Data( TELEMETRY_ADDR + telemetry_packets*(UPLINK_BUFFER_SIZE-1) , &Buffer , sizeof(Buffer) );
		Radio.Send( Buffer, BUFFER_SIZE );
		telemetry_packets++;
		if (telemetry_packets == num_telemetry){
			send_telemetry = false;
		}
	}
	else //no NACKS nor telemetry
	{
		Flash_Read_Data( PHOTO_ADDR + count_window[0]*WINDOW_SIZE*BUFFER_SIZE + (count_packet[0]-count_rtx[0])*BUFFER_SIZE , &Buffer , sizeof(Buffer) );	//Direction in HEX
		if (count_packet[0] < WINDOW_SIZE - 1)
		{
			count_packet[0]++;
		}
		else
		{
			count_packet[0] = 0;
			count_window[0]++;
			full_window = true;
		}
	}
};

/**************************************************************************************
 *                                                                                    *
 * 	Function:  resetCommsParams                                                       *
 * 	--------------------                                                              *
 * 	This function is called when a new photo is stored in the last photo position,	  *
 * 	and we have to send the new photo (the old one is not necessary, otherwise we 	  *
 * 	would have not received a send photo telecommand)								  *																					      *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void resetCommsParams(void){
	count_packet[0] = 0;
	count_window[0] = 0;
	count_rtx[0] 	= 0;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  stateMachine			                                                  *
 * 	--------------------                                                              *
 * 	communication process state machine												  *
 * 	States:																			  *
 * 	- RX_TIMEOUT: when the reception ends 											  *
 * 	- RX_ERROR: when an error in the reception process occurs						  *
 * 	- RX: when a packet has been received											  *
 * 	- TX: to transmit a packet														  *
 * 	- TX_TIMEOUT: when the transmission ends										  *
 * 	- START_CAD: to detect channel activity (necessary to receive packets correctly)  *
 * 	- LOWPOWER: when the transceiver is not transmitting nor receiving				  *
 * 																				      *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void stateMachine(void){
    uint16_t PacketCnt = 0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.CadDone = OnCadDone;

    //Timer used to restart the CAD
    TimerInit( &CADTimeoutTimer, CADTimeoutTimeoutIrq );
    TimerSetValue( &CADTimeoutTimer, CAD_TIMER_TIMEOUT );

    //App timmer used to check the RX's end
    TimerInit( &RxAppTimeoutTimer, RxTimeoutTimerIrq );
    TimerSetValue( &RxAppTimeoutTimer, RX_TIMER_TIMEOUT );

    configuration();

    SX126xConfigureCad( CAD_SYMBOL_NUM, CAD_DET_PEAK,CAD_DET_MIN, CAD_TIMEOUT_MS);      // Configure the CAD
    Radio.StartCad( );                                                                  // do the config and lunch first CAD

    State = RX;
    statemach = true;

    while(statemach){
		switch( State )
		{
			case RX_TIMEOUT:
			{
				#if(FULL_DBG)
					printf( "RX Timeout\r\n");
				#endif
				//RxTimeoutCnt++;
				State = START_CAD;
				break;
			}
			case RX_ERROR:
			{
				#if(FULL_DBG)
					printf( "RX Error\r\n");
				#endif
				//RxErrorCnt++;
				PacketReceived = false;
				State = START_CAD;
			break;
			}
			case RX:
			{
				if( PacketReceived == true )
				{
					PacketReceived = false;     // Reset flag
					RxCorrectCnt++;         	// Update RX counter
					process_telecommand(Buffer[0], Buffer[1]);	//We send the buffer to be used only in the cases of info = 1 byte
					#if(FULL_DBG)
						printf( "Rx Packet n %d\r\n", PacketCnt );
					#endif
					State = START_CAD;
				}
				else
				{
					if (CadRx == CAD_SUCCESS)
					{
						//channelActivityDetectedCnt++;   // Update counter
						#if(FULL_DBG)
							printf( "Rxing\r\n");
						#endif
						RxTimeoutTimerIrqFlag = false;
						TimerReset(&RxAppTimeoutTimer);	// Start the Rx's's Timer
						Radio.Rx( RX_TIMEOUT_VALUE );
					}
					else
					{
						TimerStart(&CADTimeoutTimer);   // Start the CAD's Timer
					}
					State = LOWPOWER;
				}
				break;
			}
			case TX:
			{
				printf("Send Packet n %d \r\n",PacketCnt);
				if( PacketCnt == 0xFFFF)
				{
					PacketCnt = 0;
				}
				else
				{
					PacketCnt ++;
				}
				//Send Frame
				DelayMs( 1 );
				if (send_data){
					tx_function();
				}
				State = LOWPOWER;
				break;
			}
			case TX_TIMEOUT:
			{
				State = LOWPOWER;
				break;
			}
			case START_CAD:
			{
				//i++;    // Update NbTryCnt
				TimerStop(&RxAppTimeoutTimer);  // Stop the Rx's Timer
				// Trace for debug
				if(CadRx == CAD_FAIL)
				{
					#if(FULL_DBG)
						printf("No CAD detected\r\n");
					#endif
				}
				CadRx = CAD_FAIL;           // Reset CAD flag
				//DelayMs(randr(10,500));     //Add a random delay for the PER test => CHECK THIS WARNING
				DelayMs(100);     //Add a random delay for the PER test => CHECK THIS WARNING
				#if(FULL_DBG)
					printf("CAD %d\r\n",i);
				#endif
				Radio.StartCad( );          //StartCad Again
				State = LOWPOWER;
			break;
			}
			case LOWPOWER:
			default:
				// Set low power
				break;
		}
    }
}


/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnTxDone   				                                              *
 * 	--------------------                                                              *
 * 	when the transmission finish correctly										      *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnTxDone( void )
{
    Radio.Standby( );
    State = TX;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnRxDone			                                                      *
 * 	--------------------                                                              *
 * 	processes the information when the reception has been done correctly			  *
 * 	calculates the rssi and snr													      *
 *                                                                                    *
 *  payload: information received			                                          *
 *  size: size of the payload								  						  *
 *  rssi: rssi value																  *
 *  snr: snr value					         										  *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Standby( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    PacketReceived = true;
    RssiMoy = (((RssiMoy * RxCorrectCnt) + RssiValue) / (RxCorrectCnt + 1));
    SnrMoy = (((SnrMoy * RxCorrectCnt) + SnrValue) / (RxCorrectCnt + 1));
    State = RX;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnTxTimeout             			                                      *
 * 	--------------------                                                              *
 * 	to process transmission timeout												      *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnTxTimeout( void )
{
    Radio.Standby( );
    State = TX_TIMEOUT;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnRxTimeout                                     			              *
 * 	--------------------                                                              *
 * 	to process reception timeout													  *
 * 																					  *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnRxTimeout( void )
{
    Radio.Standby( );
    if( RxTimeoutTimerIrqFlag )
    {
        State = RX_TIMEOUT;
    }
    else
    {
		#if(FULL_DBG)
        	printf(".");
		#endif
        Radio.Rx( RX_TIMEOUT_VALUE );   //  Restart Rx
        //SymbTimeoutCnt++;               //  if we pass here because of Symbol Timeout
        State = LOWPOWER;
    }
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnRxError                                   				              *
 * 	--------------------                                                              *
 * 	function called when a reception error occurs									  *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnRxError( void )
{
    Radio.Standby( );
    State = RX_ERROR;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  OnCadDone                                                  			  *
 * 	--------------------                                                              *
 * 	Function to check if the CAD has been done correctly or not						  *
 *                                                                                    *
 *  channelActivityDetected: boolean that contains the CAD flat                       *
 *																					  *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void OnCadDone( bool channelActivityDetected)
{
    Radio.Standby( );

    if( channelActivityDetected == true )
    {
        CadRx = CAD_SUCCESS;
    }
    else
    {
        CadRx = CAD_FAIL;
    }
    State = RX;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  CADTimeoutTimeoutIrq                                                   *
 * 	--------------------                                                              *
 * 	Function called automatically when a Channel Activity Detected (CAD) Irq occurs   *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
static void CADTimeoutTimeoutIrq( void )
{
    Radio.Standby( );
    State = START_CAD;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  RxTimeoutTimerIrq                                                      *
 * 	--------------------                                                              *
 * 	Function called automatically when a Timeout interruption (Irq) occurs 			  *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
static void RxTimeoutTimerIrq( void )
{
    RxTimeoutTimerIrqFlag = true;
}

/**************************************************************************************
 *                                                                                    *
 * 	Function:  setContingency                                     		              *
 * 	--------------------                                                              *
 *  Sets the value of contingency true or false										  *
 *																					  *
 *  cont: value to set contingency (true or false)                                    *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void setContingency(bool cont){
	contingency = cont;
}


/**************************************************************************************
 *                                                                                    *
 * 	Function:  process_telecommand                                                    *
 * --------------------                                                               *
 * 	processes the information contained in the packet depending on the telecommand    *
 * 	received																	      *
 *                                                                                    *
 *  header: number of telecommand			                                          *
 *  info: information contained in the received packet								  *
 *                                                                                    *
 *  returns: nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void process_telecommand(uint8_t header, uint8_t info) {
	switch(header) {
	case RESET2:
		HAL_NVIC_SystemReset();
		break;
	case NOMINAL:
		Write_Flash(NOMINAL_ADDR, &info, 1);
		break;
	case LOW:
		Write_Flash(LOW_ADDR, &info, 1);
		break;
	case CRITICAL:
		Write_Flash(CRITICAL_ADDR, &info, 1);
		break;
	case EXIT_LOW_POWER:{
		Write_Flash(EXIT_LOW_POWER_FLAG_ADDR, &info, 1);
		Write_Flash(EXIT_LOW_ADDR, TRUE, 1);
		break;
	}
	case SET_TIME:{
		uint8_t time[4];
		for (k=0; k<4; k++){
			time[k]=Buffer[k+1];
		}
		Write_Flash(TEMP_ADDR, &time, sizeof(time));
		break;
	}
	case SET_CONSTANT_KP:
		Write_Flash(KP_ADDR, &info, 1);
		break;
	case TLE:{
		uint8_t tle[UPLINK_BUFFER_SIZE-1];
		for (k=1; k<UPLINK_BUFFER_SIZE; k++){
			tle[k-1]=Buffer[k];
		}
		Write_Flash(TLE_ADDR + tle_packets*UPLINK_BUFFER_SIZE, &tle, sizeof(tle));
		tle_packets++;
		uint8_t integer_part = (uint8_t) 138/UPLINK_BUFFER_SIZE;
		if (tle_packets == integer_part+1){
			tle_packets = 0;
		}
		break;
	}
	case SET_GYRO_RES:
		/*4 possibles estats, rebrem 00/01/10/11*/
		Write_Flash(GYRO_RES_ADDR, &info, 1);
		break;
	case SENDDATA:{
		if (!contingency){
			State = TX;
			send_data = true;
		}
		break;
	}
	case SENDTELEMETRY:{
		if (!contingency){
			send_telemetry = true;
			num_telemetry = (uint8_t) 34/BUFFER_SIZE + 1; //cast to integer to erase the decimal part
			State = TX;
		}
		break;
	}
	case STOPSENDINGDATA:{
		send_data = false;
		count_packet[0] = 0;
		break;
	}
	case ACKDATA:{
		//check it
	 	 ack = ack & Buffer[1];
		 for(j=2; j<ACK_PAYLOAD_LENGTH; j++){
			 ack = (ack << 8*j) & Buffer[j];
		 }
		 count_window[0] = 0;
		 full_window = false;
		 if (ack != 0xFFFFFFFFFFFFFFFF){
			 nack = true;
		 }
		 State = TX;
		break;
	}
	case SET_SF: {
		uint8_t SF;
		if (info == 0) SF = 7;
		else if (info == 1) SF = 8;
		else if (info == 2) SF = 9;
		else if (info == 3) SF = 10;
		else if (info == 4) SF = 11;
		else if (info == 5) SF = 12;
		Write_Flash(SF_ADDR, &SF, 1);
		break;
	}
	case SET_CRC:
		/*4 cases (4/5, 4/6, 4/7,1/2), so we will receive and store 0, 1, 2 or 3*/
		Write_Flash(CRC_ADDR, &info, 1);
		break;
	case SEND_CALIBRATION:{	//Rx calibration
		uint8_t calib[UPLINK_BUFFER_SIZE-1];
		for (k=1; k<UPLINK_BUFFER_SIZE; k++){
			calib[k-1]=Buffer[k];
		}
		Write_Flash(CALIBRATION_ADDR, &calib, sizeof(calib));
		calib_packets = calib_packets + 1;
		uint8_t integer_part = (uint8_t) 138/UPLINK_BUFFER_SIZE;
		if(calib_packets == integer_part+1){
			calib_packets = 0;
		}
		break;
	}
	case TAKEPHOTO:{
		/*GUARDAR TEMPS FOTO?*/
		Write_Flash(PAYLOAD_STATE_ADDR, TRUE, 1);
		Write_Flash(PL_TIME_ADDR, &info, 4);
		break;
	}
	case SET_PHOTO_RESOL:
		Write_Flash(PHOTO_RESOL_ADDR, &info, 1);
		break;
	case PHOTO_COMPRESSION:
		Write_Flash(PHOTO_COMPRESSION_ADDR, &info, 1);
		break;
	case TAKERF:{
		Write_Flash(PAYLOAD_STATE_ADDR, TRUE, 1);
		Write_Flash(PL_TIME_ADDR, &info, 4);
		break;
	}
	case F_MIN:
		Write_Flash(F_MIN_ADDR, &info, 2);
		break;
	case F_MAX:
		Write_Flash(F_MAX_ADDR, &info, 2);
		break;
	case DELTA_F:
		Write_Flash(DELTA_F_ADDR, &info, 2);
		break;
	case INTEGRATION_TIME:
		Write_Flash(INTEGRATION_TIME_ADDR, &info, 1);
		break;
	case SEND_CONFIG:{
		uint8_t config[CONFIG_SIZE];
		Read_Flash(CONFIG_ADDR, &config, CONFIG_SIZE);
		break;
	}
	}
}


