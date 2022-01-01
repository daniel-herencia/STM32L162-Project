/*
 * cpumodes.h
 *
 *  Created on: Dec 12, 2021
 *      Author: davidreiss
 */

#ifndef INC_CPUMODES_H_
#define INC_CPUMODES_H_

#include "stm32l1xx_hal.h"



#endif /* INC_CPUMODES_H_ */


enum MachineState {INIT, IDLE, PAYLOAD, CONTINGENCY, SUNSAFE, SURVIVAL, COMMS};
uint8_t currentState;
uint8_t previousState;

void enter_LPRun_Mode(void);

void exit_LPRun_Mode(void);

void enter_LPSleep_Mode(void);

void exit_LPSleep_Mode(void);






