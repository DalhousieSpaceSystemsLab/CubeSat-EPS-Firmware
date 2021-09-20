#include <stdio.h>
#include "driverlib.h"
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

	while(1){
	    //collect telemetry
	    //monitor task
	    //battery task
	    //go to sleep
	    //OBC communication task
	}


	return 0;
}




