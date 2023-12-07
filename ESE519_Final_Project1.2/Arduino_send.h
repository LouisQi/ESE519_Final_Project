/*
 * Arduino_send.h
 *
 * Created: 2023/12/7 3:45:57
 *  Author: wuxue
 */ 

#ifndef ARDUINO_SEND_H
#define ARDUINO_SEND_H

#include <avr/io.h>
#include <util/delay.h>

void uart_init();
void uart_transmit_char(char data);
void uart_transmit_string(const char* data);
void transend(double number1, double number2, double number3, double number4, double number5, char* charArray);

#endif

