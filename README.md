# Plant Butler: A Smart Plant Care System

## Project Description
Plant Butler is a smart plant care system designed to simplify the task of nurturing plants amidst the hustle of daily life. Conceived from a shared love for greenery and the challenge of balancing busy schedules, this system is tailored for everyone - from students to professionals. Plant Butler uses advanced technology to monitor crucial plant health indicators such as soil moisture, light, air temperature, and humidity. It is a practical and guilt-free solution for those who find it challenging to consistently tend to their plants, making plant care effortless and accessible for all.

## Hardware Requirements
- ESP32 Module
- Various sensors (soil moisture, PDV-8001,SHTC3, HX711, load cell-5KG)
- Camera Module
- Servo Motor (MG 996R)
- Water Pump
- Active Buzzer
- Transparent Box for Motor Safety
- Additional Hardware for Setup

## Software Requirements
- Arduino IDE
- Blynk App
- Atmel Studio
- Libraries: `uart.h`, `hx711.h`, `i2c.h`

## Installation
1. Clone the repository to your local directory.
   ```
   git clone https://github.com/LouisQi/ESE519_Final_Project.git
   ```
2. Install Arduino IDE and set up the ESP32 board manager.
- Arduino IDE: [Download here](https://www.arduino.cc/en/software)
- ESP32 board manager installation tutorial: [Follow these instructions](https://www.instructables.com/Installing-the-ESP32-Board-in-Arduino-IDE-Windows-/)
3. Ensure all dependent libraries are installed.

## File Descriptions
1. **`esp32_blynk1.2.ino`**: Manages data reception from Arduino Uno and controls variables like temperature, humidity, etc.
2. **`camera_index.h`**: Contains compressed HTML for the camera module's web interface.
3. **`camera_pins.h`**: Pin configurations for the camera module.
4. **`CameraWebServer.ino`**: Sets up a web server on the ESP32 for the camera module.
5. **`main.c`**: The main program for microcontroller operation, incorporating various libraries.
6. **`FreeRTOSConfig.h`**: Configuration settings for FreeRTOS.
7. **`hx711.c`**: Library for the HX711 load cell amplifier.
8. **`i2c.c`**: Library for I2C communication.
9. **`uart.c`**: Library for UART communication.

## Usage
- To use the project, follow the installation steps and ensure all hardware connections are correct.
- For the camera module, access the web interface via the provided IP after uploading `CameraWebServer.ino`.
- Monitor sensor data and control variables through the Blynk app with `esp32_blynk1.2.ino`.

## Links may be useful
1. Devpost:   https://devpost.com/software/plant-butler
2. YouTube:   https://youtu.be/8zYo-L7o6YI?si=dqHXQ3bT5Y0RU1NC

