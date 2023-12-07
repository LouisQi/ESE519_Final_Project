#define RXD2 7
#define TXD2 8

float SoilHumidity = 0.0;
float WaterTank = 0.0; 
float AirHumidity = 0.0;
float AirTemp = 0.0; 
float PumpStatus = 0.0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
}

void loop() {
  if (Serial2.available()) {
    // Read the incoming data as a string
    String data = Serial2.readStringUntil('\n');
    
    // Split the data at the comma
    int firstCommaIndex = data.indexOf(','); //find the index of the firs occurence of a comma in the string
    // Find the second comma index starting from the position after the first comma
    int secondCommaIndex = data.indexOf(',', firstCommaIndex + 1);
    // Find the third comma index starting from the position after the second comma
    int thirdCommaIndex = data.indexOf(',', secondCommaIndex + 1);
    // Find the fourth comma index starting from the position after the third comma
    int fourthCommaIndex = data.indexOf(',', thirdCommaIndex + 1);
    
    String soilHumidityStr = data.substring(0, firstCommaIndex);
    String waterTankStr = data.substring(firstCommaIndex + 1, secondCommaIndex);
    String AirHumidityStr = data.substring(secondCommaIndex + 1, thirdCommaIndex);
    String AirTempStr = data.substring(thirdCommaIndex + 1, fourthCommaIndex);
    String PumpStatusStr = data.substring(fourthCommaIndex + 1);


    // Convert the split strings to float
    SoilHumidity = soilHumidityStr.toFloat();
    WaterTank = waterTankStr.toFloat();
    AirHumidity = AirHumidityStr.toFloat();
    AirTemp = AirTempStr.toFloat();
    PumpStatus = PumpStatusStr.toFloat();
    

    // Print the values
    Serial.print("Soil Humidity: ");
    Serial.println(SoilHumidity);
    Serial.print("Water Tank: ");
    Serial.println(WaterTank);
    Serial.print("AirHumidity: ");
    Serial.println(AirHumidity);
    Serial.print("AirTemp: ");
    Serial.println(AirTemp);
    Serial.print("PumpStatus: ");
    Serial.println(PumpStatus);
  }

  //delay(200);
}
