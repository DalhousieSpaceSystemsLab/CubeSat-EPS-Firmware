#include <stdio.h>
#include "driverlib.h"
#include <msp430.h> 

#include "common.h"
#include "clock.h"

#include "LoadSwitches.h"
#include "OBCUart.h"

/*
 * main.c
 *
 *  Created on: Jan 15, 2021
 *      Author: Liam MacDonald, Brandon Allen, Mark McCoy
 */





int main(void){
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__bis_SR_register(GIE);     //enable interrupts


	clockInit();
	systemTickInit();
	init_fault_pins();
	uart_init();

	while(1){
	    //collect telemetry
	    //monitor task
	    //battery task
	    //go to sleep
	    //OBC communication task

	    //wait 1 seconds

	}


	return 0;
}




