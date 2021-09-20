/*
 *  OBCUart.c
 *
 *  Created on: Feb 6, 2021
 *  Author: Siyan Zhang, siyan.zhang@dal.ca
 */

#include "OBCUart.h"

unsigned int stringReady = FALSE;					//string received flag
char string[UART_BUFFER_SIZE] = {"\x00\x00"};		//UART String Buffer
unsigned int RxByteCnt = 0;                         //Received String Length Counter

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



/* UART DMA Rx / Tx  */

//P3.4 = USCI_A0 TXD

void init_OBC_uart(void){
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //P3.4 = USCI_A0 TXD
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P3,
        GPIO_PIN3
        );

	USCI_A_UART_initParam param1 = {0};
    param1.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
    param1.clockPrescalar = 6;
    param1.firstModReg = 13;
    param1.secondModReg = 0;
    param1.parity = USCI_A_UART_NO_PARITY;
    param1.msborLsbFirst = USCI_A_UART_LSB_FIRST;
    param1.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
    param1.uartMode = USCI_A_UART_MODE;
    param1.overSampling = USCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if (STATUS_FAIL == USCI_A_UART_init(USCI_A0_BASE, &param1)){
        return;
    }
	
	//Enable UART module for operation
    USCI_A_UART_enable(USCI_A0_BASE);


    //Initialize and Setup DMA Channel 0
    /*
     * Configure DMA channel 0
     * Configure channel for repeated single transfers
     * DMA interrupt flag will be set after every 12 transfers
     * Use DMA Trigger Source 1 (TA0CCR0IFG)
     * Transfer Byte-to-byte
     * Trigger Upon Rising Edge of Trigger Source Signal
     */
    DMA_initParam param2 = {0};
    param2.channelSelect = DMA_CHANNEL_0;
    param2.transferModeSelect = DMA_TRANSFER_REPEATED_SINGLE;
    param2.transferSize = (sizeof String1 - 1);
    param2.triggerSourceSelect = DMA_TRIGGERSOURCE_1;
    param2.transferUnitSelect = DMA_SIZE_SRCBYTE_DSTBYTE;
    param2.triggerTypeSelect = DMA_TRIGGER_RISINGEDGE;
    DMA_init(&param2);
    /*
     * Configure DMA channel 0
     * Use String1 as source
     * Increment source address after every transfer
     */
    DMA_setSrcAddress(DMA_CHANNEL_0,
        (uint32_t)(uintptr_t)String1,
        DMA_DIRECTION_INCREMENT);
    /*
     * Base Address of the DMA Module
     * Configure DMA channel 0
     * Use UART TX Buffer as destination
     * Don't move the destination address after every transfer
     */
    DMA_setDstAddress(DMA_CHANNEL_0,
        USCI_A_UART_getTransmitBufferAddressForDMA(USCI_A0_BASE),
        DMA_DIRECTION_UNCHANGED);

    //Enable transfers on DMA channel 0
    DMA_enableTransfers(DMA_CHANNEL_0);

    //For DMA0 trigger
   	Timer_A_initCompareModeParam initCompareModeParam = {0};
    initCompareModeParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    initCompareModeParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initCompareModeParam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
    initCompareModeParam.compareValue = 1;
    Timer_A_initCompareMode(TIMER_A0_BASE, &initCompareModeParam);

    //Timer sourced by SMCLK, starts in up-mode
    Timer_A_initUpModeParam initUpModeParam = {0};
    initUpModeParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    initUpModeParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    initUpModeParam.timerPeriod = 0x8192;
    initUpModeParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    initUpModeParam.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initUpModeParam.timerClear = TIMER_A_SKIP_CLEAR;
    initUpModeParam.startTimer = true;
    Timer_A_initUpMode(TIMER_A0_BASE, &initUpModeParam);
	

    //Enter LPM3
    _BIS_SR(LPM3_bits);
}