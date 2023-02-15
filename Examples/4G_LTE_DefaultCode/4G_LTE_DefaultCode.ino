//This is the default code present in all the shipped modules
// This code enables to publish test messages to the well known public broker by hivemq.com at http://www.hivemq.com/demos/websocket-client/
// Go to this webpage and click on Connect. Click on Add New Topic Subscription and type topic name as "4GLTE/testTopic"
// If the module is connected to Internet via 4G, test messages will be published by the module and these messages will be displayed on the screen at regular intervals
//refer AT command datasheet
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

#define TINY_GSM_MODEM_SIM7600  // SIM7600 AT instruction is compatible with A7670
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_USE_GPRS true
#include <TinyGsmClient.h>
#define RXD2 27    //VVM501 MODULE RXD INTERNALLY CONNECTED
#define TXD2 26    //VVM501 MODULE TXD INTERNALLY CONNECTED
#define powerPin 4 ////VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED
int rx = -1;
String rxString;
String topicName = "4GLTE/testTopic"; // or enter a topic name of your choice
String payload = "Hello from 4G Module";  //test message
char yourMQTTServer[50];
char yourMQTTPort[20];
int yourMQTTPortNumber = 0;
int LED_BUILTIN = 2;  //Default LED Blink for Message Transmit Indication

const char apn[]      = ""; //APN automatically detects for 4G SIM, NO NEED TO ENTER, KEEP IT BLANK

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
TinyGsmClient client(modem);


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  delay(100);
  digitalWrite(powerPin, HIGH);
  delay(1000);
  digitalWrite(powerPin, LOW);


  Serial.println("\nconfiguring VVM501 Module. Kindly wait");

  delay(10000);

  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);


  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    return;
  }
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.restart()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    return;
  }

  String name = modem.getModemName();
  DBG("Modem Name:", name);

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);


  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }


  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) {
    Serial.println("LTE module connected");
  }

  Serial.println("Start MQTT");
  SerialAT.println("AT+CMQTTSTART");  //Start MQTT service
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("+CMQTTSTART: 0");
  if (rx != -1)
    Serial.println("MQTT Set");
  delay(1000);

  Serial.println("Send ClientID");
  SerialAT.println("AT+CMQTTACCQ=0,\"VVM4GLTE1234\"");  // set client ID
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");
  if (rx != -1)
    Serial.println("Client Ready");
  delay(1000);

  Serial.println("Connect to Broker");
  SerialAT.println("AT+CMQTTCONNECT=0,\"tcp://broker.hivemq.com:1883\",90,1"); //connecting to hivemq MQTT broker
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");
  if (rx != -1)
    Serial.println("Connected to Broker");
  delay(1000);
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  char tn[50];  //char buffer for  topic name
  char pl[50];  //char buffer for payload
  topicName.toCharArray(tn, 50);
  payload.toCharArray(pl, 50);

  Serial.println(" Set Topic");  //0s
  SerialAT.print("AT+CMQTTTOPIC=0,"); //setting topic name length
  SerialAT.println(strlen(tn));
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf(">"); //if valid response received, then enter topic name
  if (rx != -1)
    Serial.println("Enter Topic");
  delay(500);
  SerialAT.println(topicName);  //entering topic name
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");
  if (rx != -1)
    Serial.println("Entered Topic");
  delay(1000);

  Serial.println(" Set Message");  //0s
  SerialAT.print("AT+CMQTTPAYLOAD=0,"); //setting payload/message length
  SerialAT.println(strlen(pl));
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf(">");
  if (rx != -1)
    Serial.println("Enter Message");
  delay(500);
  SerialAT.println(payload);  //entering payload
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");
  if (rx != -1)
    Serial.println("Entered Message");
  delay(1000);

  Serial.println("Publish Topic");
  SerialAT.println("AT+CMQTTPUB=0,1,60"); //attempting to publish the payload with QoS 1 and timeout of 60seconds. QoS 1 service attempts to publish the message atleast once.
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("+CMQTTPUB: 0,0");  //Modem response for successful publish
  if (rx != -1)
  {
    Serial.println("Message Published Succesfully");
    digitalWrite(LED_BUILTIN, HIGH); //Blink LED Once for Success
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    Serial.println("Message Failed to Publish");
    digitalWrite(LED_BUILTIN, HIGH); //Blink LED Twice if not published
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(3000);  //publish messages every 3 seconds

}
