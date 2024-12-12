/**
 * Example of ABP device
 * Authors: 
 *        Ivan Moreno
 *        Eduardo Contreras
 *  June 2019
 * 
 * This code is beerware; if you see me (or any other collaborator 
 * member) at the local, and you've found our code helpful, 
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include <lorawan.h>
#include <TinyGPS++.h>


//ABP Credentials 
const char *devAddr = "260BFBD6";
const char *nwkSKey = "7641969B78B77A8376640D8D8AC8930D";
const char *appSKey = "CA504E06F8960F443D80DAA4C61255B9";

const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

TinyGPSPlus gps;


char myStr[50];
char payload[50];
char outStr[255];
byte recvStatus = 0;

const sRFM_pins RFM_pins = {
  .CS = SS,
  .RST = RFM_RST,
  .DIO0 = RFM_DIO0,
  .DIO1 = RFM_DIO1,
  .DIO2 = RFM_DIO2,
  .DIO5 = RFM_DIO5,
};

struct __attribute__ ((packed)) DataPacket {
  double latitude;
  double longitude;
};

void setup() {
  // Setup loraid access
  Serial.begin(115200);

  while(!Serial);

  if(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  Serial1.begin(9600); // GPS
  while(!Serial1);

  pinMode(RFM_TCX_ON,OUTPUT);
  pinMode(RFM_SWITCH,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_A);

  // Set Data Rate
  lora.setDataRate(SF12BW125);

  lora.setTxPower(14, PA_BOOST_PIN);

  // set channel to random
  lora.setChannel(CHRX2);
  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
}

void loop() {
  while (Serial1.available() > 0) {
    char c = Serial1.read();
    // Serial.print(c);
    gps.encode(c);
  }

  // Check interval overflow
  if(millis() - previousMillis > interval) {
    previousMillis = millis(); 

    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print("Chars: ");
    Serial.println(gps.charsProcessed());

    if (gps.location.isValid()) {
      Serial.println("Lat=");
      Serial.println(gps.location.lat(), 6);
      Serial.println("Long=");
      Serial.println(gps.location.lng(), 6);
    }

    if (gps.date.isValid()) {
      sprintf(myStr, "%s-%s-%s", gps.date.year(), gps.date.month(), gps.date.day()); 
      Serial.println(myStr);
    }

    sprintf(myStr, "Counter-%d", counter); 

    Serial.print("Sending: ");
    Serial.println(myStr);

    DataPacket packet = { 1.0, 10.0 };
    
    lora.sendUplink((char*) &packet, sizeof(packet), 0, 1);
    counter++;
  }

  recvStatus = lora.readData(outStr);
  if(recvStatus) {
    Serial.println(outStr);
  }
  
  // Check Lora RX
  lora.update();
}
