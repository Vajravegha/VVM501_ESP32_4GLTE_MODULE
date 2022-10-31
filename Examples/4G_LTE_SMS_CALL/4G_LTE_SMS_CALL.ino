
//THIS EXAMPLE SHOWS HOW VVM501 ESP32 4G LTE MODULE CAN USE TO SEND AND RECEIVE SMS AND CALL
//FOR VVM501 PRODUCT DETAILS VISIT www.vv-mobility.com

#define RXD2 27    //VVM501 MODULE RXD INTERNALLY CONNECTED
#define TXD2 26    //VVM501 MODULE TXD INTERNALLY CONNECTED
#define powerPin 4 //VVM501 MODULE ESP32 PIN D4 CONNECTED TO POWER PIN OF A7670C CHIPSET, INTERNALLY CONNECTED
int rx = -1;
#define SerialAT Serial1
String rxString;
int _timeout;
String _buffer;
String number = "+91XXXXXXXXXX"; //REPLACE WITH YOUR NUMBER
void setup() {
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW);
  Serial.begin(115200);
  delay(100);
  SerialAT.begin(115200, SERIAL_8N1, RXD2, TXD2);
  delay(10000);

  Serial.println("Modem Reset, please wait");
  SerialAT.println("AT+CRESET");
  delay(1000);
  SerialAT.println("AT+CRESET");
  delay(20000);  // WAITING FOR SOME TIME TO CONFIGURE MODEM

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
  Serial.println("Type s to send an SMS, r to receive an SMS, and c to make a call");
}
void loop() {
  if (Serial.available() > 0)
    switch (Serial.read())
    {
      case 's':
        SendMessage();   //YOU CAN SEND MESSAGE FROM SIM TO THE MENTIONED PHONE NUMBER
        break;
      case 'r':
        RecieveMessage(); // RECEIVE MESSAGE FROM THE MENTIONED PHONE NUMBER TO SIM
        break;
      case 'c':
        callNumber();    // CALL
        break;
    }
  if (SerialAT.available() > 0)
    Serial.write(SerialAT.read());
}
void SendMessage()
{
  //Serial.println ("Sending Message");
  SerialAT.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);
  //Serial.println ("Set SMS Number");
  SerialAT.println("AT+CMGS=\"" + number + "\"\r"); //Mobile phone number to send message
  delay(1000);
  String SMS = "MESSAGE FROM VVM501 ESP32 4G LTE MODULE";
  SerialAT.println(SMS);
  delay(100);
  SerialAT.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  _buffer = _readSerial();
}
void RecieveMessage()
{
  Serial.println ("VVM501 AT7670C Read an SMS");
  delay (1000);
  SerialAT.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
  delay(1000);
  Serial.write ("Unread Message done");
}
String _readSerial() {
  _timeout = 0;
  while  (!SerialAT.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if (SerialAT.available()) {
    return SerialAT.readString();
  }
}
void callNumber() {
  SerialAT.print (F("ATD"));
  SerialAT.print (number);
  SerialAT.print (F(";\r\n"));
  _buffer = _readSerial();
  Serial.println(_buffer);
}
