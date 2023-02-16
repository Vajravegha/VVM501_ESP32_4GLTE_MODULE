

#include "SSLClient.h"

#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>
#include "PubSubClient.h"

#include "certs.h"

#define SerialAT Serial1

#define DUMP_AT_COMMANDS false

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
TinyGsmClient gsmClient(modem);
SSLClient secureclient(&gsmClient);
PubSubClient mqtt(secureclient);
#else
TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);
SSLClient secureclient(&gsmClient);
PubSubClient mqtt(secureclient);
#endif

#define MODEM_PWKEY          4
#define MODEM_TX             26
#define MODEM_RX             27

const char MQTT_HOST[] = "aws-iot-endpoint.iot.us-east-1.amazonaws.com";
const char topic_publish[] = "your_publish_topic";
const char topic_subscribe[] = "your_subscribe_topic";


#define NTP_SERVER "pool.ntp.org"

#define AT_WAIT_TIME_MSECS 12000L
const char* ntpServer = "pool.ntp.org";


void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<len;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt.publish(topic_publish,"hello world");
      // ... and resubscribe
      mqtt.subscribe(topic_subscribe);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup(){

  Serial.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  
  modemPowerKeyToggle();

  secureclient.setCACert(cacert);
  secureclient.setCertificate(clientcert);
  secureclient.setPrivateKey(clientkey);
  
  mqtt.setServer(MQTT_HOST, 8883);
  mqtt.setCallback(mqttCallback);

  
  restartModem(&modem);
  initialiseModem(&modem);

  while(!modem.isNetworkConnected()){
      Serial.println("Network not available");
      modem.waitForNetwork();
  }
  while(!modem.isGprsConnected()){
      Serial.println("GPRS not connected");
      modem.gprsConnect("airtelgprs.com");
  }
}

 void loop(){

  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
  
  }

  bool initialiseModem(TinyGsm *modem)
  {
    Serial.println("initialising Modem");
    int ret = modem->init();
    String modemInfo = modem->getModemInfo();
    Serial.printf("modemInfo: %s\n SimStatus: %d\n", modemInfo.c_str(), modem->getSimStatus());
    return ret;
  }


  bool restartModem(TinyGsm *modem)
  {
    int ret = modem->restart();
    String modemInfo = modem->getModemInfo();
    Serial.printf("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
    return ret;
  }

void modemPowerKeyToggle() {
  pinMode(MODEM_PWKEY,OUTPUT);
  Serial.println("Power On modem");
  digitalWrite(MODEM_PWKEY, LOW);
  delay(100);
  digitalWrite(MODEM_PWKEY, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWKEY, LOW);
}


 bool updateNTPTime(TinyGsm *modem)
  {
    Serial.println("Update NTP Time");
    modem->sendAT("+CNTP=\"" NTP_SERVER "\",32");
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
    return res;
  }
