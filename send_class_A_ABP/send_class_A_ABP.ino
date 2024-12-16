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
const char *devAddr = "260BFEB7";
const char *nwkSKey = "58A058C1249C4D59163B1932339DBEE6";
const char *appSKey = "8DE1A3D16ED60A2AA9C4FCC81BB579E0";



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

  // gps data
  double gpsLatitude = 52.0;
  double gpsLongitude = 4.31;
  double gpsAltitude = 20.0;
  double gpsSpeed = 0.0;
  uint32_t gpsTrack = 0;
  uint32_t gpsTimeData = 0;
  
  // message
  uint32_t txCount = 0;
  uint16_t rxCount = 0;

  // measured voltages
  uint16_t battVolt = 0;
  uint16_t solarVolt = 0;
   
  uint8_t txPeriod = 10;

  uint8_t gpsUsedSats = 0;

  uint8_t chgVal = 0;

} packet;

const unsigned long interval = packet.txPeriod * 3000;    // 30 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

// ============ Battery voltage ================== //
float battery_read() {
  long sum = 0;                           // Sum of smaples taken
  float voltage = 0.0;                    // Calculated voltage

  for (int i = 0; i < 500; i++) {         // Get average value from ADC 
    sum += analogRead( A0 );
    delayMicroseconds( 100 );
  }

  // Calculating the voltage
  voltage = sum / 500.0;                  // Get the average voltage reading
  voltage = (100 * voltage) / 1023.0;     // ADC conversion to voltage value
  return voltage;
}
// =============================================== //

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

  // =========== Battery Setup ============ //
  analogReference( AR_DEFAULT );
  analogReadResolution( 10 );
  // ====================================== //

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

    // ============ Reading Battery ============= //
    packet.chgVal = (uint8_t)battery_read()
    Serial.print("Battery Level: ");
    Serial.println(packet.chgVal);
// ========================================== //

    if (gps.location.isValid()) {
      packet.gpsLatitude = gps.location.lat();
      packet.gpsLongitude = gps.location.lng();

      Serial.println("Lat=");
      Serial.println(packet.gpsLatitude, 6);
      Serial.println("Long=");
      Serial.println(packet.gpsLongitude, 6);
    }

    if (gps.altitude.isValid()) {
      packet.gpsAltitude = gps.altitude.meters();
    }

    if (gps.speed.isValid()) {
      packet.gpsSpeed = gps.speed.kmph();
    }

    if (gps.satellites.isValid()) {
      packet.gpsUsedSats = (uint8_t)gps.satellites.value();
    }

     if (gps.time.isValid()) {
      packet.gpsTimeData = gps.time.value();
    }

    

    if (gps.date.isValid()) {
      sprintf(myStr, "%d-%d-%d", gps.date.year(), gps.date.month(), gps.date.day()); 
      Serial.println(myStr);
    }

    sprintf(myStr, "Counter-%d", counter); 

    Serial.print("Sending: ");
    Serial.println(myStr);

    packet.txCount = counter;
  

    // DataPacket packet = { longitude, latitude, counter };
    
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
