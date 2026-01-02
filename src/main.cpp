#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

// Configuración del sensor I2C esclavo
// Los valores por defecto están definidos en la estructura Config
// Puedes personalizarlos aquí si es necesario:
NoiseSensorI2CSlave::Config config;
// config.i2cAddress = 0x08;      // Dirección I2C del esclavo (por defecto: 0x08)
// config.sdaPin = 8;             // Pin SDA (por defecto: 8)
// config.sclPin = 10;            // Pin SCL (por defecto: 10)
// config.adcPin = 4;             // Pin ADC (por defecto: 4)
// config.updateInterval = 1000;   // Intervalo en ms (por defecto: 1000)
// config.logLevel = NoiseSensor::LOG_INFO; // Nivel de log (por defecto: LOG_INFO)

// Crear instancia del sensor I2C esclavo
NoiseSensorI2CSlave sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32-C3 Noise Sensor I2C Slave ===");
    
    // Inicializar el sensor (configura I2C y el sensor de ruido)
    sensor.begin();
}

void loop() {
    // Actualizar el sensor (maneja I2C y actualización de datos)
    sensor.update();
    
    delay(10); // Pequeño delay para evitar saturar el loop
}

