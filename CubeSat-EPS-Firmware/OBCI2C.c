//   Nima Eskandari
//   Texas Instruments Inc.
//   April 2017
//   Built with CCS V7.0
//
//	 Modified by Siyan Zhang 
//******************************************************************************
#include <OBCI2C.h>
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * RXByteCtr:
 * ReceiveIndex:
 * TransmitBuffer:
 * TXByteCtr:
 * TransmitIndex:
 * */
uint8_t RXByteCtr = 0;  // Number of bytes left to receive
uint8_t TXByteCtr = 0;  // Number of bytes left to transfer
uint8_t ReceiveIndex = 0;   // The index of the next byte to be received in ReceiveBuffer
uint8_t TransmitIndex = 0;  // The index of the next byte to be transmitted in TransmitBuffer
uint8_t ReceiveBuffer[MAX_BUFFER_SIZE] = {0};   // Buffer used to receive data in the ISR
uint8_t TransmitBuffer[MAX_BUFFER_SIZE] = {0};  // Buffer used to transmit data in the ISR

uint8_t TransmitRegAddr = 0;

/* Used to track the state of the software state machine*/
I2C_Mode MasterMode = IDLE_MODE;

/* initialize GPIO for I2C
   I2C Pins, SDA = P4.1, SCL = P4.2
   P4SEL |= BIT1 + BIT2;                     // P4.1,2 option select
 */
void i2c_init()
{
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB1CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB1BR0 = 10;                            // fSCL = SMCLK/160 = ~100kHz
    UCB1BR1 = 0;
    //UCB1I2CSA = SLAVE_ADDR;                   // Slave Address is 048h
    UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB1IE |= UCNACKIE;
}

/* For slave device with dev_addr, read the data specified in slaves reg_addr.
 * The received data is available in ReceiveBuffer
 *
 * dev_addr: The slave device address.
 *           Example: SLAVE_ADDR
 * reg_addr: The register or command to send to the slave.
 *           Example: CMD_TYPE_0_SLAVE
 * count: The length of data to read
 *           Example: TYPE_0_LENGTH
 *  */
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t count)
{
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    RXByteCtr = count;
    TXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    /* Initialize slave address and interrupts */
    UCB1I2CSA = dev_addr;
    UCB1IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
    UCB1IE &= ~UCRXIE;                       // Disable RX interrupt
    UCB1IE |= UCTXIE;                        // Enable TX interrupt

    UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register( GIE);              // Enter LPM0 w/ interrupts

    return MasterMode;
}

/* For slave device with dev_addr, writes the data specified in *reg_data
 *
 * dev_addr: The slave device address.
 *           Example: SLAVE_ADDR
 * reg_addr: The register or command to send to the slave.
 *           Example: CMD_TYPE_0_MASTER
 * *reg_data: The buffer to write
 *           Example: MasterType0
 * count: The length of *reg_data
 *           Example: TYPE_0_LENGTH
 *  */
I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    /* Initialize state machine */
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;

    //Copy register data to TransmitBuffer
    CopyArray(reg_data, TransmitBuffer, count);

    TXByteCtr = count;
    RXByteCtr = 0;
    ReceiveIndex = 0;
    TransmitIndex = 0;

    /* Initialize slave address and interrupts */
    UCB1I2CSA = dev_addr;
    UCB1IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
    UCB1IE &= ~UCRXIE;                       // Disable RX interrupt
    UCB1IE |= UCTXIE;                        // Enable TX interrupt

    UCB1CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(LPM0_bits + GIE);              // Enter LPM0 w/ interrupts

    return MasterMode;
}

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for (copyIndex = 0; copyIndex < count; copyIndex++)
    {
        dest[copyIndex] = source[copyIndex];
    }
}

//******************************************************************************
//*************************I2C Interrupt Service Routine************************
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  //Must read from UCB1RXBUF
  uint8_t rx_val = 0;

  switch(__even_in_range(UCB1IV,0xC))
  {
    case USCI_NONE:break;                               // Vector 0 - no interrupt
    case USCI_I2C_UCALIFG:break;                        // Interrupt Vector: I2C Mode: UCALIFG
    case USCI_I2C_UCNACKIFG:break;                      // Interrupt Vector: I2C Mode: UCNACKIFG
    case USCI_I2C_UCSTTIFG:break;                       // Interrupt Vector: I2C Mode: UCSTTIFG
    case USCI_I2C_UCSTPIFG:break;                       // Interrupt Vector: I2C Mode: UCSTPIFG
    case USCI_I2C_UCRXIFG:                              // Interrupt Vector: I2C Mode: UCRXIFG
        rx_val = UCB1RXBUF;
        if (RXByteCtr)
        {
          ReceiveBuffer[ReceiveIndex++] = rx_val;
          RXByteCtr--;
        }

        if (RXByteCtr == 1)
        {
          UCB1CTL1 |= UCTXSTP;
        }
        else if (RXByteCtr == 0)
        {
          UCB1IE &= ~UCRXIE;
          MasterMode = IDLE_MODE;
          __bic_SR_register_on_exit(CPUOFF);            // Exit LPM0
        }
        break;
    case USCI_I2C_UCTXIFG:                              // Interrupt Vector: I2C Mode: UCTXIFG
        switch (MasterMode)
        {
          case TX_REG_ADDRESS_MODE:
              UCB1TXBUF = TransmitRegAddr;
              if (RXByteCtr)
                  MasterMode = SWITCH_TO_RX_MODE;       // Need to start receiving now
              else
                  MasterMode = TX_DATA_MODE;            // Continue to transmission with the data in Transmit Buffer
              break;

          case SWITCH_TO_RX_MODE:
              UCB1IE |= UCRXIE;                         // Enable RX interrupt
              UCB1IE &= ~UCTXIE;                        // Disable TX interrupt
              UCB1CTL1 &= ~UCTR;                        // Switch to receiver
              MasterMode = RX_DATA_MODE;                // State state is to receive data
              UCB1CTL1 |= UCTXSTT;                      // Send repeated start
              if (RXByteCtr == 1)
              {
                  //Must send stop since this is the N-1 byte
                  while((UCB1CTL1 & UCTXSTT));
                  UCB1CTL1 |= UCTXSTP;                  // Send stop condition
              }
              break;

          case TX_DATA_MODE:
              if (TXByteCtr)
              {
                  UCB1TXBUF = TransmitBuffer[TransmitIndex++];
                  TXByteCtr--;
              }
              else
              {
                  //Done with transmission
                  UCB1CTL1 |= UCTXSTP;     // Send stop condition
                  MasterMode = IDLE_MODE;
                  UCB1IE &= ~UCTXIE;                       // disable TX interrupt
                  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0
              }
              break;

          default:
              __no_operation();
              break;
        }
        break;
    default: break;
  }
}



