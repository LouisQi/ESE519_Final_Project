/*
 * ESE519FinalProject.c
 *
 * Created: 2023/11/26 14:48:55
 * Author : Xueyang Qi
 */ 
#define F_CPU 16000000UL

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//include lib
#include "lib/uart.h"
#include "lib/hx711.h"
#include "lib/i2c.h"
#include "lib/shtc3.h"

// Helper Marco
#define set(reg,bit)			(reg) |= (1<<(bit))
#define clear(reg,bit)			(reg) &= ~(1<<(bit))
#define toggle(reg,bit)			(reg) ^= (1<<(bit))

// ADC channel 
#define PHOTORESISTOR1_CHANNEL			0
#define PHOTORESISTOR2_CHANNEL			1
#define SOIL_MOISTURE_CHANNEL			2

// Moisture param
#define WATER_VALUE						300
#define AIR_VALUE						624

// Relay pin
#define RELAY_PIN						PORTD4
double pump = -1;

// Buzzer pin
#define BUZZER PORTD3

// Servo pin
#define SERVO_PIN						PORTB1
#define ROTATION_STEP					50
#define ROTATION_THRESHOLD				50


// 0 for stop, -1 for counterclockwise, 1 for clockwise
int rotationDirection = 0; 

//string container	
char printbuff[100];
char String[20];
char charArray[100];

// ADC reading initialization
volatile uint16_t photoresistor1_reading = 0;
volatile uint16_t photoresistor2_reading = 0;
volatile uint16_t photoresistor_difference = 0;
volatile uint16_t soil_mositure_reading = 0;
volatile long int soil_mositure = 0;

// HX711 param initialization
//set the gain
int8_t gain = HX711_GAINCHANNELA128;
//set the offset
int32_t offset = 8622500;//8389246;
//set the scale
double scale = 406315.82;//15797.8;
//set the calibration weight
double calibrationweight = 0.0095; /*0.082;*/ /*8540200;*/
double weight = 0;

// Initialize humanity and temperature reading 
double shtc3_temperature;
double shtc3_humanity;
SHTC3_t shtc3;

void Initialization ()
{
	//set I/O
	set (DDRD,RELAY_PIN);
	set (DDRB,SERVO_PIN);
	
	//setup timer2
	// Fast PWM with 0xFF at TOP and update OC2B at BOTTOM
	set (TCCR1A,WGM11);	
	set (TCCR1B,WGM12);
	set (TCCR1B,WGM13);		
	set (TCCR1A,COM1A1);
	
	// Set a pre-scaler of 6
	set (TCCR1B,CS11);
	
	ICR1 = 40000;	
	OCR1A = 700;
	
}

long int map(long int x, long int in_min, long int in_max, long int out_min, long int out_max) 
{
	return abs((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

void ADC_Initialization ()
{
	// Clear power reduction for ADC
	clear (PRR,PRADC);
	
	// Select Vref = AVcc
	set (ADMUX, REFS0);
	clear (ADMUX, REFS1);
	
	// Set ADC clock 1/128
	set (ADCSRA,ADPS0);
	set (ADCSRA,ADPS1);
	set (ADCSRA,ADPS2);
	
	// Select Channel 0
	ADMUX = (ADMUX & 0xF8) | (PHOTORESISTOR1_CHANNEL & 0x07);
	
	// Set to free running
	set (ADCSRA, ADATE);
	clear (ADCSRB, ADTS0);
	clear (ADCSRB, ADTS1);
	clear (ADCSRB, ADTS2);
	
	// Disable digital input buffer on ADC pin
	set (DIDR0, ADC0D);
	
	// Enable ADC
	set (ADCSRA, ADEN);
	
	// Start conversion
	set (ADCSRA, ADSC);
	
}

void Watering_and_SunSeeking()
{
	
	static int ADC_channel = 0;
	    
	// Add code to read ADC value into a variable here

	// Use a switch statement for cleaner code
	switch (ADC_channel) 
	{
		case 0:
		photoresistor1_reading = ADC;
		break;
		case 1:
		photoresistor2_reading = ADC;
		break;
		case 2:
		soil_mositure_reading = ADC;
		soil_mositure = map(soil_mositure_reading, AIR_VALUE, WATER_VALUE, 0, 100);
		break;
	 }

	 ADC_channel = (ADC_channel + 1) % 3; // Simplified logic for cycling through channels

	 // Simplified ADMUX setting
	 ADMUX = (ADMUX & 0xF8) | (ADC_channel & 0x07);

	 photoresistor_difference = abs(photoresistor1_reading - photoresistor2_reading);
	 
	 if (photoresistor_difference > ROTATION_THRESHOLD)
	 {
		 if (photoresistor1_reading > photoresistor2_reading)
		 {
			 rotationDirection = -1;
			 _delay_ms(50);
		 }
		 else
		 {
			 rotationDirection = 1;
			 _delay_ms(50);
		 }
	 }
	 else
	 {
		 rotationDirection = 0;
	 }
	 
	 OCR1A += rotationDirection*ROTATION_STEP;
	 
	 if (OCR1A < 700)
	 {
		 OCR1A = 700;
	 }
	 else if (OCR1A > 5300)
	 {
		 OCR1A = 5300;
	 }
	 
	 if (soil_mositure < 40)
	 {
		//open the pump
		set(PORTD,PORTD4);
		pump = 1;
		_delay_ms(100);
		clear(PORTD,PORTD4);
		
		_delay_ms(500);
	 }else{
		 pump = -1;
	 }
	 
// 	 // Simplified UART print statements
// 	 sprintf(String, "Photoresistor reading 1: %u\n\rPhotoresistor reading 2: %u\n\r Soil moisture: %ld percent\n\r OCR1A reading is:%d\n\r",
// 	 photoresistor1_reading, photoresistor2_reading,soil_mositure, OCR1A);
// 	 UART_print(String);

}

void HX711_measure_weight()
{
	
	//get weight
	weight = hx711_getweight();
		
	if (weight < 0)
	{
		weight = -weight;
	}
		
	dtostrf(weight, 3, 2, printbuff);
	sprintf(String,"Weight: %s kg\n\r",printbuff);
	UART_print(String);
	_delay_ms(50);

}

void SHTC3_measure_humanity_temperature()
{

	if(shtc3_init(&shtc3, SHTC3_I2C_ADDRESS) != SHTC3_RET_OK) {
		sprintf(String,"Couldn't initialize SHTC3 ... aborting!\n");
		UART_print(String);
	}		
	
	if(shtc3_get_temperature(&shtc3, &shtc3_temperature, 1) == SHTC3_RET_OK) {
		dtostrf(shtc3_temperature,3,2,printbuff);
		sprintf(String,"Temp is: %s C\n\r",printbuff);
		UART_print(String);
	} else {
		sprintf(String,"ERROR\n");
		UART_print(String);
	}
			
	if(shtc3_get_humidity(&shtc3, &shtc3_humanity, 1) == SHTC3_RET_OK) {
		dtostrf(shtc3_humanity,3,2,printbuff);
		sprintf(String,"Humanity is: %s\n\r",printbuff);
		UART_print(String);
	} else {
		sprintf(String,"ERROR\n");
		UART_print(String);
	}
}

// Function to transmit a string over UART
void uart_transmit_string(const char* data) {
	while (*data) {
		UART_transmit(*data);
		data++;
	}
}

void transend(long int number1, double number2,double number3,double number4,double number5,char* charArray) {
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

void Buzzer(){
	PORTD |= (1 << BUZZER);
	_delay_ms(200);
	PORTD &= ~(1 << BUZZER);
	_delay_ms(50);
}


int main(void)
{

	Initialization ();
	
	// Initialize UART 
	UART_initilization();
	
	// Initialize ADC
	ADC_Initialization ();
	
	//Initialize HX711
	hx711_init(gain, scale, offset);
	
	//Buzzer Init
	DDRD |= (1 << BUZZER);

    while (1) 
    {		
		HX711_measure_weight();
		Watering_and_SunSeeking();
		SHTC3_measure_humanity_temperature();
		
// 		sprintf(String,"Photoresistor reading 1: %u\n\r",photoresistor1_reading);
// 		UART_print(String);
// 		sprintf(String,"Photoresistor reading 2: %u\n\r",photoresistor2_reading);
// 		UART_print(String);
// 		sprintf(String,"Soil moisture reading: %ld percent\n\r",soil_mositure);
// 		UART_print(String);
	
		transend(soil_mositure, weight ,shtc3_humanity,shtc3_temperature, pump , charArray);
		
		if (weight<0.2)
		{
			Buzzer();
		}	
    }
}
