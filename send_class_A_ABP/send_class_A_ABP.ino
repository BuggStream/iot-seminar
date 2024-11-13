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
  uint8_t tag;
  uint32_t data;
};

void setup() {
  // Setup loraid access
  Serial.begin(115200);
  Serial1.begin(9600); // GPS

  while(!Serial);

  if(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  pinMode(RFM_TCX_ON,OUTPUT);
  pinMode(RFM_SWITCH,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(A2, INPUT);

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
  while (Serial1.available()) {
    int c = Serial1.read();
    gps.encode(c);
  }

  // Check interval overflow
  if(millis() - previousMillis > interval) {
    previousMillis = millis(); 



    uint32_t analogValue = analogRead(A2);

    sprintf(myStr, "Counter-%d", counter); 

    Serial.print("Sending: ");
    Serial.println(myStr);

    DataPacket packet = { 1, 10 };

    sprintf(myStr, "packet{ tag: %d, data: %d }", packet.tag, packet.data);

    Serial.println(myStr);
    
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
