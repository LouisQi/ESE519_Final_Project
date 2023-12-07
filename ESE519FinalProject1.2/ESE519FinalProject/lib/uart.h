#ifndef UART_LAB3_H
#define UART_LAB3_H

void UART_initilization();
void UART_transmit(unsigned char data);
char UART_receive();
void UART_print(const char* str);

#endif