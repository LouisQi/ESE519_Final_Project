#include <avr/io.h>
#include <util/delay.h>

// Function to initialize UART
void uart_init() {
  // Set the baud rate
  const uint16_t BAUD_PRESCALE = F_CPU / 16 / 9600 - 1;

  UBRR0H = (BAUD_PRESCALE >> 8);
  UBRR0L = BAUD_PRESCALE;
  UCSR0B = (1 << TXEN0) | (1 << RXEN0); // Enable transmitter and receiver
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Set frame format: 8 data bits, 1 stop bit
}

// Function to transmit a character over UART
void uart_transmit_char(char data) {
  while (!(UCSR0A & (1 << UDRE0))); // Wait for the buffer to be empty
  UDR0 = data; // Transmit the data
}

// Function to transmit a string over UART
void uart_transmit_string(const char* data) {
  while (*data) {
    uart_transmit_char(*data);
    data++;
  }
}

void transend(double number1, double number2,double number3,double number4,double number5,char* charArray) {
    char temp1[20];
    char temp2[20];
    char temp3[20];
    char temp4[20];
    char temp5[20];
    dtostrf(number1, 3, 2, temp1);
    dtostrf(number2, 3, 2, temp2);
    dtostrf(number3, 3, 2, temp3);
    dtostrf(number4, 3, 2, temp4);
    dtostrf(number5, 3, 2, temp5);
    sprintf(charArray, "%s,%s,%s,%s,%s\n\r", temp1, temp2,temp3,temp4,temp5); // Format with both numbers separated by a comma
    uart_transmit_string(charArray); // Send the message over UART
}

int main(void) {
  uart_init(); // Initialize UART

  double number1 = 34.5; // soil humidity
  double number2 = 0.86; // water tank weight
  double number3 = 50.5; //air humidity
  double number4 = 28.6; // air temperature
  double number5 = 1; // pumpstatus
  char charArray[100]; // Adjust the array size as needed
  
  while (1) {
    transend(number1, number2,number3,number4,number5, charArray); // Send both numbers over UART
    //_delay_ms(100); // Wait for 1.5 seconds before sending the next message
  }

  return 0;
}
