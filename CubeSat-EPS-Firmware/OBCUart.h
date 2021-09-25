/*
 *  OBCUart.h
 *
 *  Created on: Feb 6, 2021
 *  Author: Siyan Zhang, siyan.zhang@dal.ca
 */
#ifndef OBCUART_H_
#define OBCUART_H_

#include <stdio.h>
#include <msp430.h>

#define UART_BUFFER_SIZE 64
#define TRUE 1
#define FALSE 0

void uart_init(void);		//init UART A0 @ 9600 baudrate
int uart_putchar(int c);	//send a sigle char
int uart_puts(char *str);	//send a string

void OBC_task(void);

#endif
