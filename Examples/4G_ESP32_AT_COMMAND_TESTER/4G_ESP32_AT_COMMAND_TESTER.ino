// THIS EXAMPLE IS USED FOR AT COMMAND TESTING
// YOU CAN WRITE THE AT COMMANDS IN THE SERIAL PORT TO VIEW THE RESPONSES, SET BAUD RATE to 115200
// SELECT SERIAL PORT MONITOR AS "BOTH NL & CR" TO VIEW COMMAND RESPONSE CORRECTLY IF YOU ARE USING ARDUINO IDE
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

#define RXD2 27    // This is for ESP32 to 4G Module communication
#define TXD2 26
#define powerPin 4  //Power Key to enable 4G Module
#define SerialAT Serial1  //Serial communication port between ESP32 and 4G module
char a, b;
void setup() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  Serial.begin(115200);
  delay(100);
  Serial.println("Waiting for modem response");
  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(10000); // DELAY FOR MODEM TO SET
  Serial.println("Modem Reset, Please Wait");
  SerialAT.println("AT+CRESET");
  delay(1000);
  SerialAT.println("AT+CRESET");
  delay(20000);
  SerialAT.flush();
  Serial.println("Enter Standard AT commands like AT, AT+CPIN?, AT+CCLK?, etc.");
  Serial.println("SELECT SERIAL PORT MONITOR AS \"BOTH NL & CR\" TO VIEW COMMAND RESPONSE CORRECTLY IF YOU ARE USING ARDUINO IDE");
  Serial.println("Refer A7600 series datasheet for entire list of commands");
  Serial.println("Understand the AT Commands properly");
  Serial.println("Incorrect AT commands can corrupt the 4G module memory!!!");


}

void loop()
{
  if (Serial.available() > 0) // read AT commands from user Serial port and send to the Module
  {
    a = Serial.read();
    SerialAT.write(a);
  }
  if (SerialAT.available() > 0) //read Response commands from module and send to user Serial Port
  {
    b = SerialAT.read();
    Serial.write(b);
  }
}
