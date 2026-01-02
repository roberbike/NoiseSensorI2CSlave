/**
 * Ejemplo básico de uso de la librería NoiseSensorI2CSlave
 * 
 * Este ejemplo muestra cómo usar la librería de forma simple
 * con la configuración por defecto.
 * 
 * REQUISITOS:
 * - ESP32-C3 (o compatible)
 * - Sensor de ruido conectado al GPIO 4 (ADC)
 * - Conexiones I2C: SDA en GPIO 8, SCL en GPIO 10
 * 
 * FUNCIONALIDAD:
 * - Configura el sensor como esclavo I2C en dirección 0x08
 * - Actualiza los datos automáticamente cada segundo
 * - Responde a solicitudes I2C del maestro
 * - Muestra datos por Serial para debug
 */

#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

// Configuración básica del sensor
NoiseSensorI2CSlave::Config config;
// Personalizar valores (los valores por defecto ya están configurados en la estructura)
config.i2cAddress = 0x08;      // Dirección I2C del esclavo
config.sdaPin = 8;             // Pin SDA para I2C
config.sclPin = 10;            // Pin SCL para I2C
config.adcPin = 4;             // Pin ADC para el sensor de ruido
config.updateInterval = 1000;  // Actualizar datos cada segundo (1000ms)
config.logLevel = NoiseSensor::LOG_INFO;  // Mostrar logs informativos

// Crear instancia del sensor
NoiseSensorI2CSlave sensor(config);

void setup() {
    // Inicializar comunicación serial para debug
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== Ejemplo Básico - NoiseSensor I2C Slave ===");
    Serial.println("Inicializando sensor...");
    
    // Inicializar el sensor (configura I2C y el sensor de ruido)
    sensor.begin();
    
    Serial.println("Sensor listo. Esperando solicitudes I2C...");
    Serial.println();
}

void loop() {
    // IMPORTANTE: update() debe llamarse continuamente en loop()
    // Esto actualiza el sensor y maneja las solicitudes I2C
    sensor.update();
    
    // Opcional: acceder a los datos directamente si es necesario
    // (Los datos ya se muestran automáticamente por Serial si logLevel >= LOG_INFO)
    if (sensor.isDataReady()) {
        const auto& data = sensor.getData();
        // Aquí puedes procesar los datos si necesitas hacer algo adicional
        // Ejemplo: data.noiseAvg, data.noisePeak, etc.
    }
    
    // Pequeño delay para evitar saturar el loop
    delay(10);
}

