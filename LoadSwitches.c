/*
 * LoadSwitchISR.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Mark McCoy, m.mccoy@dal.ca
 */

#include "LoadSwitches.h"

volatile unsigned int OBC_5V_Fault_Count;
volatile unsigned int OBC_3V3_Fault_Count;
volatile unsigned int ADCS_5V_Fault_Count;
volatile unsigned int ADCS_3V3_Fault_Count;
volatile unsigned int GPS_5V_Fault_Count;
volatile unsigned int RF_3V3_Fault_Count;

void init_fault_pins(void){
    P1DIR &= ~0x3F; //Pins 1.0 to 1.5 set to input
    P1REN |= 0x3F; //Enables resistors for pins 1.0 to 1.5
    //P1OUT |= 0x3F; // Makes internal resistor a pull up resistor
    P1IES |= 0x3F; //Sets interrupt triggers for pins 1.0 to 1.5 to high to low
    P1IFG &= ~0x3F; //Clears interrupt flags for pins 1.0 to 1.5
    P1IE &= ~0xFF; //Clears interrupt pins for all of port 1
    P1IE |= 0x3F; //Enables interrupts for pins 1.0 to 1.5
}

#pragma vector = PORT1_VECTOR
__interrupt void LOADswitchISR(void){
    switch(__even_in_range(P1IV, 12)){
    case 0: break;                                          //None
    case 2:                                                 //Pin 1.0: OBC-5V-Fault
        OBC_5V_Fault_Count++;
        P1IFG &= ~BIT0;
        break;
    case 4:                                                 //Pin 1.1: OBC-3V3-Fault
        OBC_3V3_Fault_Count++;
        P1IFG &= ~BIT1;
        break;
    case 6:                                                  //Pin 1.2: ADCS-5V-Fault
        ADCS_5V_Fault_Count++;
        P1IFG &= ~BIT2;
        break;
    case 8:                                                  //Pin 1.3: ADCS-3V3-Fault
        ADCS_3V3_Fault_Count++;
        P1IFG &= ~BIT3;
        break;
    case 10:                                                 //Pin 1.4: GPS-5V-Fault
        GPS_5V_Fault_Count++;
        P1IFG &= ~BIT4;
        break;
    case 12:                                                //Pin 1.5: RF-3V3-Fault
        RF_3V3_Fault_Count++;
        P1IFG &= ~BIT5;
        break;
    default: break;
    }
}
