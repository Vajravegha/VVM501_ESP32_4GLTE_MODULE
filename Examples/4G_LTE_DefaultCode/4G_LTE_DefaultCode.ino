//This is the default code present in all the shipped modules
// This code enables to publish test messages to the well known public broker by hivemq.com at http://www.hivemq.com/demos/websocket-client/
// Go to this webpage and click on Connect. Click on Add New Topic Subscription and type topic name as "4GLTE/testTopic"
// If the module is connected to Internet via 4G, test messages will be published by the module and these messages will be displayed on the screen at regular intervals
//refer AT command datasheet
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

#define RXD2 27     // This is for ESP32 to 4G Module communication MODULE RXD INTERNALLY CONNECTED
#define TXD2 26 // MODULE TXD INTERNALLY CONNECTED
#define powerPin 4  // MODULE ESP32 PIN D4 CONNECTED TO POWER KEY PIN OF A7670C CHIPSET
#define SerialAT Serial1  //Serial communication port between ESP32 and 4G module
int rx = -1;
String rxString;
String topicName = "4GLTE/testTopic"; // or enter a topic name of your choice
String payload = "Hello from 4G Module";  //test message
char yourMQTTServer[50];
char yourMQTTPort[20];
int yourMQTTPortNumber = 0;
void setup() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  Serial.begin(115200);
  delay(100);
  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(10000);

  Serial.println("Modem Reset, Please Wait");
  SerialAT.println("AT+CRESET");
  delay(1000);
  SerialAT.println("AT+CRESET");  //reset the module
  delay(20000);

  SerialAT.flush();

  SerialAT.println("ATE0");   // turns off the echo mode, only module responses will be received, not user commands
  delay(1000);
  SerialAT.println("ATE0");
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("OK");  //checking for valid response from Modem
  if (rx != -1)
    Serial.println("Modem Ready");
  delay(1000);

  Serial.println("SIM card check");
  SerialAT.println("AT+CPIN?"); //Checking SIM card and network status
  rxString = SerialAT.readString();
  Serial.print("Got: ");
  Serial.println(rxString);
  rx = rxString.indexOf("+CPIN: READY");
  if (rx != -1)
    Serial.println("SIM Card Ready"); //checking for valid SIM ready response from Modem
  delay(1000);

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
    Serial.println("Message Published Succesfully");
  else
    Serial.println("Message Failed to Publish");
  delay(5000);  //publish messages every 5 seconds

}
