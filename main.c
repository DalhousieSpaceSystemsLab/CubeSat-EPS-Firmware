#include <stdio.h>
#include <msp430.h> 

#include "LoadSwitches.h"
#include "OBCUart.h"

/*
 * LoadSwitchISR.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Liam MacDonald, Brandon Allen, Mark McCoy
 */

void load_switch_test(void);

int main(void){
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__bis_SR_register(GIE);     //enables interrupts

	init_fault_pins();
	uart_init();

	return 0;
}




