# Simple ESP32 + MAX6675 thermocouple BLE beacon

An ESP32 based BLE beacon that sends temperature info from a thermocouple connected to a
MAX6675 module. You can buy a module and thermocouple eg. from 
[Banggood](https://www.banggood.com/MAX6675-Sensor-Module-Thermocouple-Cable-1024-Celsius-High-Temperature-Available-p-1086406.html?p=6H24052869562201510Z)

Connect the module's GND to ESP32's GND, module's VCC to +3.3V, MISO to D21, CS to D10 and SCLK to D18. See code.

The program sends 8 bytes of data in BLE ManufacturerData every 5 seconds. The data format is:

- 2 bytes of Manufacturer ID 0x02E5 (Espressif Inc)
- 2 bytes for identifying this sketch: 0x1A13 (6675 dec)
- 2 bytes of temperature
- 2 extra bytes as 0xBEEF because it's delicious

All words are big endian.

Temperature is a signed short as quarter degrees Celsius. Theoretically the temperature can be -8191 to 8191
degrees. Thermocouples can be used even in 1000°C.

Example: You get bytes 0xA2 and 0x01. 0x01A2 = 418 dec, and that divided by 4 is 104.5°C




