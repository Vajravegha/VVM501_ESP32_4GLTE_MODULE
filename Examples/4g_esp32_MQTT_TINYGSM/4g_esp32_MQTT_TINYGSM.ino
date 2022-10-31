
//THIS EXAMPLE SHOWS HOW VVM501 ESP32 4G LTE MODULE CAN CONNECT TO MQTT PUBLIC BROKER HIVEMQ USING TINYGSMCLIENT AND PUBSUBCLIENT LIBRARY
//THE DEVICE CAN PUBLISH AS WELL AS SUBSCRIBE TOPICS VIA 4G MQTT
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

#define TINY_GSM_MODEM_SIM7600  // SIM7600 AT instruction is compatible with A7670
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_USE_GPRS true
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define RXD2 27    //VVM501 MODULE RXD INTERNALLY CONNECTED
#define TXD2 26    //VVM501 MODULE TXD INTERNALLY CONNECTED
#define powerPin 4 ////VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED


int LED_BUILTIN = 2;
int ledStatus = LOW;

const char *broker         = "broker.hivemq.com"; // REPLACE IF YOU ARE USING ANOTHER BROKER
const char *led_on_off     = "XXXXX/led";         //SUBSCRIBE TOPIC TO SWITCH ON LED WHENEVER THERE IS INCOMING MESSAGE FROM MQTT. REPLACE XXXXX WITH YOUR TOPIC NAME
const char *message        = "XXXXX/message";     //PUBLISH TOPIC TO SEND MESSAGE EVERY 3 SECONDS. REPLACE XXXXX WITH YOUR TOPIC NAME
const char *topicLedStatus = "XXXXX/ledStatus";   // PUBLISH TOPIC TO FIND LED STATUS (1 OR 0). REPLACE XXXXX WITH YOUR TOPIC NAME





const char apn[]      = ""; //APN automatically detects for 4G SIM, NO NEED TO ENTER, KEEP IT BLANK

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient  mqtt(client);


void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

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


  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);
}



void loop()
{
  if (!mqtt.connected()) {
    reconnect();                //Just incase we get disconnected from MQTT server
  }
  mqtt.publish(message, "HELLO FROM VVM501 ESP32 4G LTE MODULE");
  Serial.println("HELLO FROM VVM501 ESP32 4G LTE MODULE");
  delay(3000);
  if (!modem.isNetworkConnected()) {
    Serial.println("Network disconnected");
    if (!modem.waitForNetwork(180000L, true)) {
      Serial.println(" fail");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected()) {
      Serial.println("Network re-connected");
    }

#if TINY_GSM_USE_GPRS
    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      Serial.println("GPRS disconnected!");
      Serial.print(F("Connecting to "));
      Serial.print(apn);
      if (!modem.gprsConnect(apn)) {
        Serial.println(" fail");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) {
        Serial.println("GPRS reconnected");
      }
    }
#endif
  }

  mqtt.loop();
  delay(100);
}



void reconnect() {
  while (!mqtt.connected()) {       // Loop until connected to MQTT server
    Serial.print("Attempting MQTT connection...");

    boolean status = mqtt.connect("4g modem test");

    // Or, if you want to authenticate MQTT:
    // boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

    if (status == false) {
      Serial.println(" fail");
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // Will attempt connection again in 5 seconds
      //        return false;
    }

    else {

      Serial.println(" success");
      mqtt.subscribe(led_on_off);       //Subscribe to Learning Mode Topic
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {                    //The MQTT callback which listens for incoming messages on the subscribed topics

  if (String(topic) == led_on_off) {
    ledStatus = !ledStatus;
    digitalWrite(LED_BUILTIN, ledStatus);
    Serial.print("ledStatus:");
    Serial.println(ledStatus);
    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
    Serial.println("incoming message");
    // Serial.write(payload);
    Serial.println();
  } else {
    Serial.println("No");
  }
  Serial.println();
}
