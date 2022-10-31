// THIS EXAMPLE SHOWS HOW VVM501 ESP32 4G LTE MODULE CAN CONNECT TO THINGSPEAK VIA HTPP COMMANDS.
//WE ARE DISPLAYING DHT22 TEMPERATURE AND HUMIDITY VALUES IN REAL TIME TO THINGSPEAK VIA 4G NETWORK
// CONNECTION DIAGRAM BETWEEN DHT22 AND ESP32 4G MODULE
// DHT22    <------------->     ESP32 4G LTE MODULE
// + PIN    <------------->        3V3
// OUT PIN  <------------->        D15
// - PIN    <------------->        GND
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com


#include <stdio.h>
#include <string.h>
#include <DHT.h>


String Apikey = "XXXXXXXXXXXXXXXX"; // PASTE YOUR THINGSPEAK API KEY HERE

#define DEBUG true
#define RXD2 27    //VVM501 MODULE RXD INTERNALLY CONNECTED
#define TXD2 26    //VVM501 MODULE TXD INTERNALLY CONNECTED
#define powerPin 4 //VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED
#define Sensor_PIN 15  //D15-DHT22 CONNECT EXTERNALLY

DHT  dht(Sensor_PIN, DHT22);

int rx = -1;
#define SerialAT Serial1
String rxString;
int _timeout;
String _buffer;
void setup()
{
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  Serial.begin(115200);
  delay(100);
  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(10000);  //WAITING FOR MODEM RESET
  dht.begin();
  delay(2000);
  Serial.println("Modem Reset, Please Wait");
  SerialAT.println("AT+CRESET");
  delay(1000);
  SerialAT.println("AT+CRESET");
  delay(20000);

  SerialAT.flush();

  Serial.println("Echo Off");
  SerialAT.println("ATE0");   //120s
  delay(1000);
  SerialAT.println("ATE0");   //120s
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");
  if (rx != -1)
    Serial.println("Modem Ready");
  delay(1000);

  Serial.println("SIM card check");
  SerialAT.println("AT+CPIN?"); //9s
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("+CPIN: READY");
  if (rx != -1)
    Serial.println("SIM Card Ready");
  delay(1000);
  Serial.println("4G HTTP Test Begin!");

  dht.begin();
  delay(1000);
}

void loop()
{
  //--------Get temperature and humidity-------------
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println("*C");
  SerialAT.print("Humidity: ");
  SerialAT.print(h);
  SerialAT.println("%");
  SerialAT.print("Temperature: ");
  SerialAT.print(t);
  SerialAT.println("*C");
  delay(1000);

  //-----------HTTP---------------------
  String http_str = "AT+HTTPPARA=\"URL\",\"https://api.thingspeak.com/update?api_key=" + Apikey + "&field1=" + (String)t + "&field2=" + (String)h + "\"\r\n";
  Serial.println(http_str);
  SerialAT.println("AT+HTTPINIT");
  delay(200);
  SerialAT.println(http_str);
  delay(200);
  SerialAT.println("AT+HTTPACTION=1");
  delay(200);
  SerialAT.println("AT+HTTPTERM");

  delay(5000);
}
