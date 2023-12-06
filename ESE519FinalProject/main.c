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

// Servo pin
#define SERVO_PIN						PORTB1
#define ROTATION_STEP					60
#define ROTATION_THRESHOLD				50

// I2c info
#define SHTC3_I2C_ADDRESS				0x70
#define SHTC3_I2C_WAKEUP_MSB			0x35
#define SHTC3_I2C_WAKEUP_LSB			0x17
#define SHTC3_I2C_MEASURE_MSB			0x58
#define SHTC3_I2C_MEASURE_LSB			0xe0
#define SHTC3_I2C_SLEEP_MSB				0xb0
#define SHTC3_I2C_SLEEP_LSB				0x98

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

// Initialize humanity and temperature reading 
// uint16_t humanity = 0;
// uint16_t temperature = 0;
// float converted_humanity = 0;
// float converted_temperature = 0;
// uint8_t i2c_data[3]; 
// ret_code_t error_code;

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
	 
	 if (OCR1A < 1199)
	 {
		 OCR1A = 1199;
	 }
	 else if (OCR1A > 4799)
	 {
		 OCR1A = 4799;
	 }
	 
	 if (soil_mositure < 40)
	 {
		//open the pump
		set(PORTD,PORTD4);
		_delay_ms(100);
		clear(PORTD,PORTD4);
		_delay_ms(500);
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
	SHTC3_t shtc3;

	if(shtc3_init(&shtc3, SHTC3_I2C_ADDRESS) != SHTC3_RET_OK) {
		sprintf(String,"Couldn't initialize SHTC3 ... aborting!\n");
		UART_print(String);
	}
			
	float shtc3_tmp;
	if(shtc3_get_temperature(&shtc3, &shtc3_tmp, 1) == SHTC3_RET_OK) {
		dtostrf(shtc3_tmp,3,2,printbuff);
		sprintf(String,"Temp is: %s C\n\r",printbuff);
		UART_print(String);
	} else {
		sprintf(String,"ERROR\n");
		UART_print(String);
	}
			
	if(shtc3_get_humidity(&shtc3, &shtc3_tmp, 1) == SHTC3_RET_OK) {
		dtostrf(shtc3_tmp,3,2,printbuff);
		sprintf(String,"Humanity is: %s\n\r",printbuff);
		UART_print(String);
	} else {
		sprintf(String,"ERROR\n");
		UART_print(String);
	}
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
		
		
    }
}
