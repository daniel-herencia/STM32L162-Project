/*
 * cpumodes.c
 *
 *  Created on: Dec 12, 2021
 *      Author: davidreiss
 */



#include "cpumodes.h"


/******************************************************************
*  																  *
*  Function enter_LPRuun_Mode: makes the system to enter into the *
*  which is a low power mode that makes the system to reduce its  *
*  consumption by slowing down the frequency of the system clock  *
*  ---------------           									  *
*  No inputs are needed                                           *
*                                                                 *
*  returns : nothing                                              *
*																  *
*******************************************************************/

void enter_LPRun_Mode(){

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;//Each IP digital clock must be enabled or disabled by using the RCC_APBxENR and RCC_AHBENR registers


	RCC->ICSCR |= RCC_ICSCR_MSIRANGE_1; //We are setting here the SYSCLK freq. not to exceed the fMSI range 1 by re-initializing the SYSCLK

	//The regulator must be forced to low power mode (by setting LPSDSR and LPRUN bits)

	PWR->CR &= ~PWR_CR_LPRUN; // Be sure LPRUN is cleared

	PWR->CR |= PWR_CR_LPSDSR; // must be set before LPRUN
	PWR->CR |= PWR_CR_LPRUN; //both LPSDSR and LPRUN bits are now set



}

/****************************************************************************************
*																						*
* Function enter_LPSleep_Mode: the idea behind this state is the same as in the previous*
* one but, here the SCR register bits are set on a different way as you can see, 		*
* basically following the instructions the datasheet give us for entering this mode     *
* 																						*
* --------------																		*
*																						*
* no inputs are needed																	*
*																						*
* returns : nothing																		*
*																						*
*****************************************************************************************/

void enter_LPSleep_Mode(){

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;//Each IP digital clock must be enabled or disabled by using the RCC_APBxENR and RCC_AHBENR registers
	RCC->ICSCR |= RCC_ICSCR_MSIRANGE_1;//frequency decreased

	PWR->CR |= PWR_CR_LPSDSR;//voltage regulator in low power mode

	SCB->SCR &= ~( SCB_SCR_SLEEPDEEP_Msk ); // low-power mode = sleep mode
	SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // reenter low-power mode after ISR
	__WFI(); // enter low-power mode



}

/****************************************************************************************
*																		            	*
* Function exit_LPRun_Mode: resets the changed values on the enter_LPRun_Mode to its	*
* original ones																			*
*																						*
* ---------------------																	*
*																						*
* no inputs are needed and returns nothing												*
*																						*
*****************************************************************************************/


void exit_LPRun_Mode(){

	/* Enable/disable Clocks again */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	/* Force the regulator into main mode */
	// Reset LPRUN bit
	PWR->CR &= ~( PWR_CR_LPRUN );
	// LPSDSR can be reset only when LPRUN bit = 0;
	PWR->CR &= ~( PWR_CR_LPSDSR );


	RCC->ICSCR |= RCC_ICSCR_MSIRANGE_5;//frequency increased again to the default MSI range



}
