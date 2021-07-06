/*
 * ADC.c
 *
 *  Created on: Jan. 31, 2021
 *      Authors: Mark McCoy, m.mccoy@dal.ca
 *               Liam MacDonald
 */

#include "ADC.h"

#define ONCE_MODE 0
#define REPEATING_MODE 1

volatile static double results[6];
volatile static int i;

void init_ADC(void)
{
    REFCTL0 &= ~ REFON;
    P6SEL = 0xF0;                             // Enable A/D channel inputs on pins 6.4 to 6.7
    P7SEL = 0x03;                             // Enable A/D channel inputs on pins 7.0 and 7.1
    //Voltage reference set to AVcc by default

    ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_2 + ADC12REFON; // Turn on ADC12, set sampling time
    ADC12CTL1 = ADC12SHP;                     // Use sampling timer
    ADC12CTL2 = ADC12RES_2;                   // Sets ADC resolution to 12 bits
    ADC12IE = 0xFFFF;                         //Enables interrupts on all ADC12 bits
}

void input_channel(int Ax, int conv_mode)
{
    switch(conv_mode)
    {
    case 0:                                 //Single Channel Once
        ADC12MCTL0 = Ax;
        break;
    case 1:                                 //Single Channel Repeating
        ADC12MCTL1 = Ax;
        break;
    default:
        break;
    }
}

int mode_pins(int mode, int * p_channels)
{
    int num_channels = 0;
    int mode1[] = {4,5,7};
    int mode2[] = {4,5,6,7,12,13};
    int mode3[] = {4,5};
    int x;

    switch(mode) {
    case 1:                                 //Check channels A4, A5, and A7
        num_channels = 3;
        for (x=0; x<num_channels; x++)
            p_channels[x] = mode1[x];
        break;

    case 2:                                 //Check all channels
        num_channels = 6;
        for (x=0; x<num_channels; x++)
            p_channels[x] = mode2[x];
        break;

    case 3:                                 //Check channels A4 and A5
        num_channels = 2;
        for (x=0; x<num_channels; x++)
            p_channels[x] = mode3[x];
        break;
    }

    return num_channels;
}

void print_results(void){
    double result;

    //result = 3.333*(results[i]/4095);
   //printf("%.3f V\n", result);
}

void single_channel_once(int Ax, int ADC_mode){
    ADC12CTL1 |= ADC12CONSEQ_0;                 // Select single-channel, single-conversion
    ADC12CTL1 |= ADC12CSTARTADD_0;              // The memory register will be ADC12MEM0
    input_channel(Ax, ONCE_MODE);               // Selects the input channel

    if (ADC_mode==ONCE_MODE)
        i=0;


    ADC12CTL0 |= ADC12ENC;                      // Enable conversions
    ADC12CTL0 |= ADC12SC;                       // Starts the conversion on channel Ax
    __delay_cycles(1000);
}

void single_channel_repeat(int Ax){
    ADC12CTL0 |= ADC12MSC;
    ADC12CTL1 |= ADC12CONSEQ_2;                 // Select single-channel, repeating conversion
    ADC12CTL1 |= ADC12CSTARTADD_1;              // The first memory register will be ADC12MEM0
    input_channel(Ax, REPEATING_MODE);          // Selects the input channel
    i = 0;

    ADC12CTL0 |= ADC12ENC;                      // Enable conversions
    ADC12CTL0 |= ADC12SC;                       // Starts the conversions on channel Ax
    __delay_cycles(1000);
}

void sequence_of_channels_once(int mode)
{
    int channels[6];
    int x, n;
    ADC12CTL1 |= ADC12CONSEQ_0;
    ADC12CTL1 |= ADC12CSTARTADD_0;
    n = mode_pins(mode, channels);
    i = 0;

    for (x=0; x<n; x++)
    {
        single_channel_once(channels[x], REPEATING_MODE);
    }
}

void sequence_of_channels_repeat(int mode)
{
    int channels[6];
    int x, n;
    ADC12CTL1 |= ADC12CONSEQ_0;
    ADC12CTL1 |= ADC12CSTARTADD_0;
    n = mode_pins(mode, channels);
    i = 0;

    while(1){
        for (x=0; x<n; x++)
        {
            if (i==6)
            {
                i = 0;
            }
            single_channel_once(channels[x], REPEATING_MODE);
        }
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADC12IV,34))
      {
      case  0: break;                           // Vector  0:  No interrupt
      case  2: break;                           // Vector  2:  ADC overflow
      case  4: break;                           // Vector  4:  ADC timing overflow
      case  6:                                  // Vector  6:  ADC12IFG0
          results[i] = ADC12MEM0;
          ADC12CTL0 &= ~ADC12SC;
          ADC12CTL0 &= ~ADC12ENC;
          print_results();
          i++;
          break;
      case  8:                                  // Vector  8:  ADC12IFG1
          results[i] = ADC12MEM1;               // Move results, IFG is cleared
          print_results();
          i++;
          if (i==6)
          {
              i = 0;
          }
          break;
      case 10: break;                           // Vector 10:  ADC12IFG2
      case 12: break;                           // Vector 12:  ADC12IFG3
      case 14: break;                           // Vector 14:  ADC12IFG4
      case 16: break;                           // Vector 16:  ADC12IFG5
      case 18: break;                           // Vector 18:  ADC12IFG6
      case 20: break;                           // Vector 20:  ADC12IFG7
      case 22: break;                           // Vector 22:  ADC12IFG8
      case 24: break;                           // Vector 24:  ADC12IFG9
      case 26: break;                           // Vector 26:  ADC12IFG10
      case 28: break;                           // Vector 28:  ADC12IFG11
      case 30: break;                           // Vector 30:  ADC12IFG12
      case 32: break;                           // Vector 32:  ADC12IFG13
      case 34: break;                           // Vector 34:  ADC12IFG14
      default: break;
      }
}
