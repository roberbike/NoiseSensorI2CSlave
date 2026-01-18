#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

#ifndef I2C_ADDRESS
#define I2C_ADDRESS 0x08
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 8
#endif

#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN 10
#endif

#ifndef NOISE_ADC_PIN
#define NOISE_ADC_PIN 4
#endif

NoiseSensorI2CSlave::Config config;
NoiseSensorI2CSlave sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("=== NoiseSensor I2C Slave (ESP32) ===");
    Serial.printf("I2C addr: 0x%02X | SDA=%d | SCL=%d | ADC=%d\n",
                  static_cast<unsigned>(I2C_ADDRESS),
                  static_cast<int>(I2C_SDA_PIN),
                  static_cast<int>(I2C_SCL_PIN),
                  static_cast<int>(NOISE_ADC_PIN));

    config.i2cAddress = static_cast<uint8_t>(I2C_ADDRESS);
    config.sdaPin = static_cast<uint8_t>(I2C_SDA_PIN);
    config.sclPin = static_cast<uint8_t>(I2C_SCL_PIN);
    config.adcPin = static_cast<uint8_t>(NOISE_ADC_PIN);
    config.updateInterval = 1000;
    config.logLevel = NoiseSensor::LOG_INFO;

    if (!sensor.setConfig(config)) {
        Serial.println("ERROR: Configuración inválida (setConfig falló). Revisa pines/dirección.");
    }

    sensor.begin();
}

void loop() {
    sensor.update();
    delay(10);
}

