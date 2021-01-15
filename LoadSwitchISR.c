#include <stdio.h>
#include <msp430.h> 


/**
 * main.c
 */
volatile static int OBC_5V_Fault_Count=0, OBC_3V3_Fault_Count=0, ADCS_5V_Fault_Count=0, ADCS_3V3_Fault_Count=0, GPS_5V_Fault_Count=0, RF_3V3_Fault_Count=0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	__bis_SR_register(GIE);

	int i;


	P1DIR &= ~0x3F; //Pins 1.0 to 1.5 set to input
	P1REN |= 0x3F; //Enables resistors for pins 1.0 to 1.5
	P1IES |= 0x3F; //Sets interrupt triggers for pins 1.0 to 1.5 to high to low
	P1IFG &= ~0x3F; //Clears interrupt flags for pins 1.0 to 1.5
	P1IE &= ~0xFF; //Clears interrupt pins for all of port 1
	P1IE |= 0x3F; //Enables interrupts for pins 1.0 to 1.5

	P2DIR |= 0x01;
	P2OUT = 0x01;

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

	return 0;
}


#pragma vector = PORT1_VECTOR
__interrupt void LOADswitchISR(void)
{

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

