/*
 * ESP32 + MAX6675 thermocouple BLE beacon
 *
 * See https://github.com/oh2mp/esp32_max6675_beacon
 *
 */
 
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"

#define LED 2                     // Onboard led on development boards
#define MISO 21
#define CS   19
#define SCLK 18

BLEAdvertising *advertising;
short temperature;                // Temperature as quarter Celsius

/* ---------------------------------------------------------------------------------- 
 * Read data from MAX6675 SPI 
 */
byte spi_read(void) { 
    byte d = 0;

    for (int i=7; i >= 0; i--) {
        digitalWrite(SCLK, LOW);
        delay(50);
        if (digitalRead(MISO)) {
            d |= (1 << i);
        }
        digitalWrite(SCLK, HIGH);
        delay(50);
    }
    return d;
}

/* ----------------------------------------------------------------------------------
 * Set up data for advertising
 */ 
void set_beacon() {
    BLEBeacon beacon = BLEBeacon();
    BLEAdvertisementData advdata = BLEAdvertisementData();
    BLEAdvertisementData scanresponse = BLEAdvertisementData();
    
    advdata.setFlags(0x06); // BR_EDR_NOT_SUPPORTED 0x04 & LE General discoverable 0x02

    std::string mfdata = "";
    mfdata += (char)0xE5; mfdata += (char)0x02;  // Espressif Incorporated Vendor ID = 0x02E5
    mfdata += (char)0x13; mfdata += (char)0x1A;  // Identifier for this sketch is 0x1A13 (6675)
    mfdata += (char)(temperature & 0xFF);        // LSB for temperature
    mfdata += (char)(temperature >> 8);          // MSB for temperature
    mfdata += (char)0xBE; mfdata += (char)0xEF;  // Beef is always good nutriment
  
    advdata.setManufacturerData(mfdata);
    advertising->setAdvertisementData(advdata);
    advertising->setScanResponseData(scanresponse);
}

/* ---------------------------------------------------------------------------------- */
void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);   // LED off
    
    pinMode(CS,   OUTPUT);
    pinMode(SCLK, OUTPUT); 
    pinMode(MISO, INPUT);

    // Enable serial for debugging if wanted
    // Serial.begin(115200);

    BLEDevice::init("ESP32+MAX6675");
    advertising = BLEDevice::getAdvertising();
}
/* ---------------------------------------------------------------------------------- */

void loop() {
    digitalWrite(CS, HIGH);
    delay(500);                // Let thermocouple to settle

    digitalWrite(CS, LOW);     // Read the thermocouple
    delay(1);
    temperature = spi_read();
    temperature <<= 8;
    temperature |= spi_read();
    digitalWrite(CS, HIGH);
    if (temperature & 0x4) {
        temperature = 32767;   // No thermocouple attached
    }
    temperature >>= 3;         // Now we have temperature stored as quarter Celsius

    // Enable serial for debugging if wanted
    // Serial.printf("%.2f°C\n", temperature*0.25);

    if (temperature != 0) {
    set_beacon();
        digitalWrite(LED, HIGH);   // LED on during the advertising
        advertising->start();
        delay(100);
        advertising->stop();
        digitalWrite(LED, LOW);   // LED off
    }
    
    delay(4500);
    
    // Reboot once in hour to be sure    
    if (millis() > 3.6E+6) {
        ESP.restart();
    }
}
/* ---------------------------------------------------------------------------------- */
