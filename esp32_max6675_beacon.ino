/*
 * ESP32 + MAX6675 thermocouple BLE beacon
 *
 * See https://github.com/oh2mp/esp32_max6675_beacon
 *
 */
 
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include "esp_system.h"

#define LED 2                     // Onboard led on development boards
#define MISO 21
#define CS   19
#define SCLK 18
#define DEEP_SLEEP_DURATION 5     // Deep sleep seconds between beaconing

BLEAdvertising *advertising;
short temperature;                // Temperature as quarter Celsius
RTC_DATA_ATTR int bootcount = 0;
hw_timer_t *timer = NULL;

/* ---------------------------------------------------------------------------------- */
void IRAM_ATTR resetModule() {
    ets_printf("Watchdog reboot\n");
    esp_restart();
}
/* ---------------------------------------------------------------------------------- 
 * Read data from MAX6675 SPI 
 */
byte spi_read(void) { 
    byte d = 0;

    for (int i=7; i >= 0; i--) {
        digitalWrite(SCLK, LOW);
        delay(1);
        if (digitalRead(MISO)) {
            d |= (1 << i);
        }
        digitalWrite(SCLK, HIGH);
        delay(1);
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
    bootcount++;

    // Enable watchdog
    timer = timerBegin(0, 80, true);  
    timerAttachInterrupt(timer, &resetModule, true);
    timerAlarmWrite(timer, 60000000, false); //set time in us
    timerAlarmEnable(timer);
      
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);   // LED off
    
    pinMode(CS,   OUTPUT);
    pinMode(SCLK, OUTPUT); 
    pinMode(MISO, INPUT);
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
    
    Serial.begin(115200);
    Serial.printf("boot %d, %.2f°C\n",bootcount, temperature*0.25);
    Serial.flush();

    BLEDevice::init("ESP32+MAX6675");
    advertising = BLEDevice::getAdvertising();
    set_beacon();
    digitalWrite(LED, HIGH);   // LED on during the advertising
    advertising->start();
    delay(100);
    advertising->stop();
    digitalWrite(LED, LOW);   // LED off

    timerWrite(timer, 0);
    esp_sleep_enable_timer_wakeup(1000000LL * DEEP_SLEEP_DURATION);
    esp_deep_sleep_start();
}
/* ---------------------------------------------------------------------------------- */
void loop() {}  // No loop but deep sleep
/* ---------------------------------------------------------------------------------- */
