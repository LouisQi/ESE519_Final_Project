// UART Library for Atmega328P 

# include <avr/io.h>

# define F_CPU 16000000UL
# define BAUD 9600
# define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

void UART_initilization()
{
	/*Set BAUD rate*/
	// High 8 bits
	UBRR0H = UBRR_VALUE >> 8;
	// Low 8 bits
	UBRR0L = UBRR_VALUE;
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	// 8-bit data
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);	
	// Set 2 stop bit
	UCSR0C |= 1<<USBS0;
}

void UART_transmit(unsigned char data)
{
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1<<UDRE0)));
	// Put data in the buffer and send it out
	UDR0 = data;
}

char UART_receive()
{
	// Wait data to be received 
	while (!(UCSR0A & (1<<RXC0)));
	// Return the received data
	return UDR0;
}

void UART_print(const char* str)
{
	while(*str)
	{
		UART_transmit(*str++);
	}
}