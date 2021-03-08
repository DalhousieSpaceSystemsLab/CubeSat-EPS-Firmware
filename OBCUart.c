/*
 *  OBCUart.c
 *
 *  Created on: Feb 6, 2021
 *  Author: Siyan Zhang, siyan.zhang@dal.ca
 */

#include <PBCUart.h>

unsigned int stringReady = FALSE;					//string received flag
char string[UART_BUFFER_SIZE] = {"\x00\x00"};		//UART String Buffer
unsigned int RxByteCnt = 0;                           		//Received String Length Counter

//******************************************************************************
//********************UART Interrupt Service Routine****************************
//******************************************************************************

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;									// Vector 0 - no interrupt
  case 2:										// Vector 2 - RXIFG
    P1OUT |= BIT0;
    string[RxByteCnt++] = UCA0RXBUF;			// Store the reveived char
    if (UCA0RXBUF == '\r')						// '\r' received?
    {
      //string[i++] = '\n';						// Append new line at the end of string.
      //string[i] = '\r';
      RxByteCnt = 0;             				// Reset the length counter
      stringReady = TRUE;						// Set the string ready flag
      //UCA0IE |= UCTXIE;						// Enable USCI_A0 TX interrupt
    }
    P1OUT &= ~BIT0;								// TX -> RXed character
    break;
  case 4:										// Vector 4 - TXIFG
    P4OUT |= BIT7;
    /*UCA0TXBUF = string[RxByteCnt++];			// TX next character
    if (string[RxByteCnt] == '\x0d')			// TX over?
    {
       UCA0IE &= ~UCTXIE;						// Disable USCI_A0 TX interrupt
       RxByteCnt = 0;							// Clear the counter
    }*/
    P4OUT &= ~BIT7;
    break;
  default: break;
  }
}

void uart_init(void)
{
    P3SEL = BIT3+BIT4;                    		// P3.3,4 = USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSWRST;                   		// **Put state machine in reset**
    UCA0CTL1 |= UCSSEL_2;                  		// SMCLK
    UCA0BR0 = 6;                           		// 1MHz 9600 (see User's Guide)
    UCA0BR1 = 0;                           		// 1MHz 9600
    UCA0MCTL = UCBRS_0 + UCBRF_13 + UCOS16;		// Modln UCBRSx=0, UCBRFx=0, over sampling
    UCA0CTL1 &= ~UCSWRST;						// **Initialize USCI state machine**
    UCA0IE |= UCRXIE;							// Enable USCI_A0 RX interrupt
}

int uart_putchar(int c)
{
    while (!(UCA0IFG & UCTXIFG));				// Wait for the transmit buffer to be ready
    // Transmit data
    UCA0TXBUF = (char) c;

    return 0;
}

int uart_puts(char *str)
{
    int status = -1;
    if (str != NULL) {
        status = 0;
        while (*str != '\0')
        {
            while (!(UCA0IFG & UCTXIFG));		// Wait for the transmit buffer to be ready
            UCA0TXBUF = *str;            		// Transmit data
            if (*str == '\n')           		// If there is a line-feed, add a carriage return
            {                           		// Wait for the transmit buffer to be ready
                while (!(UCA0IFG & UCTXIFG));
                UCA0TXBUF = '\r';
            }
            str++;								// Move pointer to next char 
        }
    }
    return status;
}