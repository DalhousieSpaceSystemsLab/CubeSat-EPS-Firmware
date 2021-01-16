#include <stdio.h>
#include <msp430.h> 

#include "LoadSwitches.h"


/**
 * main.c
 */

void load_switch_test(void);

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__bis_SR_register(GIE);     //enables interrupts

	init_fault_pins();

	load_switch_test();
	//other code

	return 0;
}


//connect pin 2.0 to any of the load switch input pins
void load_switch_test(void){
        P2DIR |= 0x01;
        P2OUT = 0x01;

        unsigned int i;

        while(1){
            P2OUT ^= 0x01;
            for (i=0; i<20000; i++){}
            printf("OBC-5V-Faults: %d\n", OBC_5V_Fault_Count);
            printf("OBC-3V3-Faults: %d\n", OBC_3V3_Fault_Count);
            printf("ADCS-5V-Faults: %d\n", ADCS_5V_Fault_Count);
            printf("ADCS-3V3-Faults: %d\n", ADCS_3V3_Fault_Count);
            printf("GPS-5V-Faults: %d\n", GPS_5V_Fault_Count);
            printf("RF-3V3-Faults: %d\n", RF_3V3_Fault_Count);
        }
}
