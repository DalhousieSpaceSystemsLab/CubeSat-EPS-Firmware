/*
 * LoadSwitchISR.h
 *
 *  Created on: Jan 15, 2021
 *      Author: Mark McCoy, m.mccoy@dal.ca
 */

#ifndef LOADSWITCHES_H_
#define LOADSWITCHES_H_

#include <stdio.h>
#include <msp430.h>


extern volatile unsigned int OBC_5V_Fault_Count;
extern volatile unsigned int OBC_3V3_Fault_Count;
extern volatile unsigned int ADCS_5V_Fault_Count;
extern volatile unsigned int ADCS_3V3_Fault_Count;
extern volatile unsigned int GPS_5V_Fault_Count;
extern volatile unsigned int RF_3V3_Fault_Count;


void init_fault_pins(void);

#endif /* LOADSWITCHES_H_ */
