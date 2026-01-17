/**
 * Ejemplo avanzado de uso de la librería NoiseSensorI2CSlave
 * 
 * Este ejemplo muestra cómo:
 * - Configurar opciones avanzadas (actualización más frecuente, sin logs)
 * - Acceder a los datos directamente y procesarlos
 * - Personalizar la presentación de datos
 * - Usar el objeto NoiseSensor interno para configuración avanzada
 * 
 * DIFERENCIAS CON EL EJEMPLO BÁSICO:
 * - Actualización cada 500ms en lugar de 1000ms
 * - Logs desactivados (LOG_NONE)
 * - Presentación personalizada de datos cada 2 segundos
 */

#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

// Configuración avanzada personalizada
NoiseSensorI2CSlave::Config config;

// Crear instancia del sensor
NoiseSensorI2CSlave sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Personalizar valores para uso avanzado
    config.i2cAddress = 0x08;      // Dirección I2C (puede cambiarse si hay múltiples sensores)
    config.sdaPin = 8;             // Pin SDA para I2C
    config.sclPin = 10;            // Pin SCL para I2C
    config.adcPin = 4;             // Pin ADC para el sensor de ruido
    config.updateInterval = 500;   // Actualizar cada 500ms (más frecuente que el básico)
    config.logLevel = NoiseSensor::LOG_NONE;  // Sin logs automáticos (control manual)

    // Aplicar configuración antes de begin()
    if (!sensor.setConfig(config)) {
        Serial.println("ERROR: Configuración inválida, usando valores por defecto.");
    }
    
    Serial.println("=== Ejemplo Avanzado - NoiseSensor I2C Slave ===");
    Serial.println("Configuración personalizada:");
    Serial.printf("  - Actualización: cada %lu ms\n", config.updateInterval);
    Serial.printf("  - Logs: desactivados\n");
    Serial.println();
    
    // Inicializar el sensor
    sensor.begin();
    
    // Ejemplo: Acceder al objeto NoiseSensor interno para configuración avanzada
    // (si necesitas configurar parámetros específicos de NoiseSensor)
    // NoiseSensor& noiseSensor = sensor.getNoiseSensor();
    // ... aquí puedes hacer configuraciones adicionales si la librería lo permite ...
    
    Serial.println("Sensor inicializado con configuración personalizada");
    Serial.println("Mostrando datos personalizados cada 2 segundos...");
    Serial.println();
}

void loop() {
    // Actualizar el sensor (debe llamarse continuamente)
    sensor.update();
    
    // Acceder a los datos cuando estén listos
    if (sensor.isDataReady()) {
        const auto& data = sensor.getData();
        
        // Mostrar datos personalizados con nuestro propio formato
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint >= 2000) {  // Cada 2 segundos
            lastPrint = millis();
            
            Serial.println("=== Datos del Sensor (Personalizado) ===");
            Serial.printf("  Ruido Actual:     %.2f mV\n", data.noise);
            Serial.printf("  Promedio:         %.2f mV\n", data.noiseAvg);
            Serial.printf("  Pico:             %.2f mV\n", data.noisePeak);
            Serial.printf("  Mínimo:           %.2f mV\n", data.noiseMin);
            Serial.printf("  Promedio Legal:   %.2f mV\n", data.noiseAvgLegal);
            Serial.printf("  Máximo Legal:     %.2f mV\n", data.noiseAvgLegalMax);
            Serial.printf("  Nivel Base:       %d mV\n", data.lowNoiseLevel);
            Serial.printf("  Ciclos:           %u\n", data.cycles);
            Serial.println();
            
            // Ejemplo: Procesamiento adicional de datos
            if (data.noisePeak > 1000.0) {
                Serial.println("  ⚠️  ALERTA: Nivel de ruido pico muy alto!");
            }
        }
    }
    
    delay(10);
}

