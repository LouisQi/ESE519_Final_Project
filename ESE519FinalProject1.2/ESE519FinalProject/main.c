/*
 * ESE519FinalProject.c
 *
 * Created: 2023/11/26 14:48:55
 * Author : Xueyang Qi
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//include UART
#include "lib/uart.h"
#include "lib/hx711.h"

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

// Servo pin
#define SERVO_PIN						PORTB1
#define ROTATION_STEP					10
#define ROTATION_THRESHOLD				30
#define BUZZER PORTD3

// 0 for stop, -1 for counterclockwise, 1 for clockwise
int rotationDirection = 0; 

//string container	
char printbuff[100];
char String[20];

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
	
	ICR1 = 39999;	
	OCR1A = 4799;
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
	set (ADCSRA,ADIE);
	
	// Start conversion
	set (ADCSRA, ADSC);
	
}

ISR (ADC_vect)
{
	static int ADC_channel = 0;
	
	if (ADC_channel == 0)
	{
		photoresistor1_reading = ADC;
	}
	else if (ADC_channel == 1)
	{
		photoresistor2_reading = ADC;
	}
	else if (ADC_channel == 2)
	{
		soil_mositure_reading = ADC;
		soil_mositure = map(soil_mositure_reading,AIR_VALUE,WATER_VALUE,0,100);
	}
	
	ADC_channel ++;
	
	if (ADC_channel > 2)
	{
		ADC_channel = 0;
	}
	
	// Set next ADC channel
	if (ADC_channel == 0) 
	{
		ADMUX = (ADMUX & 0xF8) | (PHOTORESISTOR1_CHANNEL & 0x07);
	} 
	else if (ADC_channel == 1) 
	{
		ADMUX = (ADMUX & 0xF8) | (PHOTORESISTOR2_CHANNEL & 0x07);
	} 
	else if (ADC_channel == 2) 
	{
	    ADMUX = (ADMUX & 0xF8) | (SOIL_MOISTURE_CHANNEL & 0x07);
	}
	
	photoresistor_difference = abs(photoresistor1_reading - photoresistor2_reading);
	
// 	sprintf(String,"Photoresistor reading 1: %u\n\r",photoresistor1_reading);
// 	UART_print(String);
// 	sprintf(String,"Photoresistor reading 2: %u\n\r",photoresistor2_reading);
// 	UART_print(String);
// 	sprintf(String,"Soil moisture reading: %u \n\r",soil_mositure_reading);
// 	UART_print(String);
// 	sprintf(String,"Soil moisture reading: %ld percent\n\r",soil_mositure);
// 	UART_print(String);
// 	sprintf(String,"Rotation difference is: %u\n\r",photoresistor_difference);
// 	UART_print(String);
}

void relay_open()
{
	//open the pump
	set(PORTD,PORTD4);
	_delay_ms(200);
	clear(PORTD,PORTD4);
	_delay_ms(1000);
}

void servo_rotate()
{
	cli();
	if (photoresistor_difference > ROTATION_THRESHOLD)
	{
		if (photoresistor1_reading > photoresistor2_reading)
		{
			rotationDirection = -1;
			sprintf(String,"Negative\n\r");
			UART_print(String);
		} 
		else 
		{
			rotationDirection = 1;
			sprintf(String,"Positive\n\r");
			UART_print(String);
		}
	} 
	else 
	{
		rotationDirection = 0;
	}
	
	OCR1A += rotationDirection*ROTATION_STEP;
	
	if (OCR1A < 1199)
	{
		OCR1A = 1199;
	}
	else if (OCR1A >4799)
	{
		OCR1A = 4799;
	}
	sei();
}

void HX711_measure_weight()
{
	//get weight
	weight = hx711_getweight();
		
	if (weight < 0)
	{
		weight = -weight-0.11;
	}
		
	dtostrf(weight, 3, 2, printbuff);
	sprintf(String,"Weight: %s kg\n\r",printbuff);
	UART_print(String);
// 	_delay_ms(500);	
}

void Buzzer(){
	PORTD |= (1 << BUZZER);
	_delay_ms(500);
	PORTD &= ~(1 << BUZZER);
	_delay_ms(500);
	PORTD |= (1 << BUZZER);
	_delay_ms(100);
	PORTD &= ~(1 << BUZZER);
	_delay_ms(100);
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
	DDRD |= (1 << BUZZER);
	
    while (1) 
    {
		
// 		sprintf(String,"Photoresistor reading 1: %u\n\r",photoresistor1_reading);
// 		UART_print(String);
// 		sprintf(String,"Photoresistor reading 2: %u\n\r",photoresistor2_reading);
// 		UART_print(String);
// 		sprintf(String,"Soil moisture reading: %ld percent\n\r",soil_mositure);
// 		UART_print(String);
// 		servo_rotate();

		HX711_measure_weight();
// 		if (soil_mositure < 40)
// 		{
// 			relay_open();
// 		}
		if (weight<0.2)
		{
		 	Buzzer();
		}
		
    }
}
