/*
 * flash.c
 *
 *  Created on: 15 nov. 2021
 *      Author: psimo
 */


#include "flash.h"
#include "stm32l1xx_hal.h"
#include "string.h"
#include "stdio.h"


/**************************************************************************************
 *                                                                                    *
 * Function:  GetPage                                                     		  *
 * --------------------                                                               *
 * defines the SECTORS according to the reference manual					          *
 * STM32L162RE has:																	  *
 *  2048 pages of 256 bytes													  *
 *  2 banks of 8 kbytes of EEPROM							*
 *                                                                                    *
 *  Address: Specific address of a read/write function                                *
 *                                                                                    *
 *  returns: page in which the address is contained		                          *
 *                                                                                    *
 **************************************************************************************/
static uint32_t GetPage(uint32_t Address)
{
  for (int indx=0; indx<128; indx++)
  {
	  if((Address < (0x08000000 + (FLASH_PAGE_SIZE *(indx+1))) ) && (Address >= (0x08000000 + FLASH_PAGE_SIZE*indx)))
	  {
		  return (0x08000000 + FLASH_PAGE_SIZE*indx);
	  }
  }

  return 0;
}


/**************************************************************************************
 *                                                                                    *
 * Function:  Flash_Write_Data                                                 		  *
 * --------------------                                                               *
 * Writes in the flash memory														  *
 *                                                                                    *
 *  StartPageAddress: first address to be written		                              *
 *	Data: information to be stored in the FLASH/EEPROM memory						  *
 *	numberofbytes: Data size in Bytes					    						  *
 *															                          *
 *  returns: Nothing or error in case it fails			                              *
 *                                                                                    *
 **************************************************************************************/
uint32_t Flash_Write_Data (uint32_t StartPageAddress, uint8_t *Data, uint16_t numberofbytes)
{

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	int sofar=0;

	  /* Unlock the Flash to enable the flash control register access *************/
	   HAL_FLASH_Unlock();

	   /* Erase the user Flash area*/

	  uint32_t StartPage = GetPage(StartPageAddress);
	  uint32_t EndPageAdress = StartPageAddress + numberofbytes;
	  uint32_t EndPage = GetPage(EndPageAdress);

	   /* Fill EraseInit structure*/
	   EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	   EraseInitStruct.PageAddress = StartPage;
	   EraseInitStruct.NbPages     = ((EndPage - StartPage)/FLASH_PAGE_SIZE) +1;

	   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	   {
	     /*Error occurred while page erase.*/
		  return HAL_FLASH_GetError ();
	   }

	   /* Program the user Flash area word by word*/

	   while (sofar<numberofbytes)
	   {
	     if (HAL_FLASH_Program(FLASH_TYPEPROGRAMDATA_BYTE, StartPageAddress, Data[sofar]) == HAL_OK)
	     {
	    	 StartPageAddress += 1;  // use StartPageAddress += 2 for half word and 8 for double word
	    	 sofar++;
	     }
	     else
	     {
	       /* Error occurred while writing data in Flash memory*/
	    	 return HAL_FLASH_GetError ();
	     }
	   }

	   /* Lock the Flash to disable the flash control register access (recommended
	      to protect the FLASH memory against possible unwanted operation) *********/
	   HAL_FLASH_Lock();

	   return 0;
}


/**************************************************************************************
 *                                                                                    *
 * Function:  Write_Flash                                                		 	  *
 * --------------------                                                               *
 * It's the function that must be called when writing in the Flash memory.			  *
 * Depending on the address, it writes 1 time or 3 times (Redundancy)				  *
 *                                                                                    *
 *  StartPageAddress: first address to be written		                              *
 *	Data: information to be stored in the FLASH/EEPROM memory						  *
 *	numberofbytes: Data size in Bytes					    						  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void Write_Flash(uint32_t StartPageAddress, uint8_t *Data, uint16_t numberofbytes) {
	if (StartPageAddress >= 0x08080000 && StartPageAddress <= 0x08083FFF) {
		Flash_Write_Data(StartPageAddress, Data, numberofbytes);
		Flash_Write_Data(StartPageAddress + 0x1555, Data, numberofbytes);
		Flash_Write_Data(StartPageAddress + 0x2AAA, Data, numberofbytes);
	}
	else {
		Flash_Write_Data(StartPageAddress, Data, numberofbytes);
	}
}

/**************************************************************************************
 *                                                                                    *
 * Function:  Flash_Read_Data                                                 		  *
 * --------------------                                                               *
 * Writes in the flash memory														  *
 *                                                                                    *
 *  StartPageAddress: first address to be read		                              *
 *	RxBuf: Where the data read from memory will be stored							  *
 *	numberofbytes: Reading data size in Bytes					    				  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void Flash_Read_Data (uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes)
{
	while (1)
	{

		*RxBuf = *(__IO uint8_t *)StartPageAddress;
		StartPageAddress += 1;
		RxBuf++;
		numberofbytes--;
		if (numberofbytes == 0) break;
	}
}

/**************************************************************************************
 *                                                                                    *
 * Function:  Check_Redundancy                                                 		  *
 * --------------------                                                               *
 * Reads the data from the 3 addresses where it is stored and chooses the value		  *
 * that coincides at least in 2 of the 3 addresses (in case one gets corrupted)		  *
 * All the addresses of variables with Redundancy follow the same pattern: each		  *
 * address is separated 0x4000 positions in memory									  *
 *                                                                                    *
 *  Address: first address to be read		                              			  *
 *	RxDef: Buffer to store the lecture that coincides at least 2 times				  *
 *	numberofbytes: Data size in bytes												  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/

void Check_Redundancy(uint32_t Address, uint8_t *RxDef, uint16_t numberofbytes) {
	uint8_t lect1[numberofbytes], lect2[numberofbytes], lect3[numberofbytes];
	Flash_Read_Data(Address, lect1, numberofbytes);
	Flash_Read_Data(Address + 0x1555, lect2, numberofbytes);
	Flash_Read_Data(Address + 0x2AAA, lect3, numberofbytes);

	bool coincidence12 = true, coincidence13 = true, coincidence23 = true;
	for (int i = 0; i < numberofbytes; i++) {
		if (lect1[i] != lect2[i]) coincidence12 = false;
		if (lect1[i] != lect3[i]) coincidence13 = false;
		if (lect2[i] != lect3[i]) coincidence23 = false;
	}
	if(coincidence12 || coincidence13) {
		Flash_Read_Data(Address, RxDef, numberofbytes);
	}
	else if(coincidence23) {
		Flash_Read_Data(Address + 0x4000, RxDef, numberofbytes);
	}

	else {
		*RxDef = lect1; /*PREGUNTAR QUÈ FER QUAN NO COINCIDEIX CAP LECTURA (POC PROBABLE)*/
	}
}

/**************************************************************************************
 *                                                                                    *
 * Function:  Read_Flash	                                                 		  *
 * --------------------                                                               *
 * It's the function that must be called when reading from the Flash memory.		  *
 * Depending on the address, it reads from 1 or 3 addresses (Redundancy)			  *
 *                                                                                    *
 *  StartPageAddress: starting address to read			                              *
 *	RxBuf: Where the data read from memory will be stored							  *
 *	numberofbytes: Reading data size in Bytes					    				  *
 *															                          *
 *  returns: Nothing									                              *
 *                                                                                    *
 **************************************************************************************/
void Read_Flash(uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes) {
	if (StartPageAddress >= 0x08080000 && StartPageAddress <= 0x08083FFF) {
		Check_Redundancy(StartPageAddress, RxBuf, numberofbytes);
	}
	else {
		Flash_Read_Data(StartPageAddress, RxBuf, numberofbytes);
	}
}
