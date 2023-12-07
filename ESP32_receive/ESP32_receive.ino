#define RXD2 7
#define TXD2 8

float SoilHumidity = 0.0;
float WaterTank = 0.0; 

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  if (Serial2.available()) {
    // Read the incoming data as a string
    String data = Serial2.readStringUntil('\n');
    
    // Split the data at the comma
    int commaIndex = data.indexOf(',');
    String soilHumidityStr = data.substring(0, commaIndex);
    String waterTankStr = data.substring(commaIndex + 1);

    // Convert the split strings to float
    SoilHumidity = soilHumidityStr.toFloat();
    WaterTank = waterTankStr.toFloat();

    // Print the values
    Serial.print("Soil Humidity: ");
    Serial.println(SoilHumidity);
    Serial.print("Water Tank: ");
    Serial.println(WaterTank);
  }

  //delay(200);
}
