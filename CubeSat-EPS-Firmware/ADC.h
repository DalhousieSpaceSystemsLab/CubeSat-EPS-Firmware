/*
 * ADC.h
 *
 *  Created on: Jan. 31, 2021
 *      Author: Mark McCoy
 */


#ifndef ADC_H_
#define ADC_H_

#include <msp430.h>
#include <stdio.h>

extern void init_ADC(void);
extern void input_channel(int Ax, int conv_mode);
extern int mode_pins(int mode, int * p_channels);
extern void print_results(void);
extern void single_channel_once(int Ax, int ADC_mode);
extern void single_channel_repeat(int Ax);
extern void sequence_of_channels_once(int mode);
extern void sequence_of_channels_repeat(int mode);

#endif /* ADC_H_ */
