/**
 * Ejemplo de uso de múltiples sensores I2C esclavos
 * 
 * Este ejemplo muestra cómo usar la librería con múltiples sensores
 * en diferentes direcciones I2C.
 * 
 * NOTA: Este ejemplo es conceptual. En la práctica, necesitarías:
 * - Múltiples ESP32-C3, cada uno con su sensor
 * - Cada uno configurado con una dirección I2C diferente
 * - O usar un ESP32 con múltiples buses I2C
 * 
 * Este ejemplo muestra cómo configurar dos sensores teóricos.
 */

#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

// Configuración del Sensor 1
NoiseSensorI2CSlave::Config config1;

// Configuración del Sensor 2 (conceptual)
NoiseSensorI2CSlave::Config config2;

// Crear instancia del sensor
// IMPORTANTE: La librería solo permite UNA instancia por programa (limitación de Wire callbacks).
// Para múltiples sensores, usa múltiples ESP32 (cada uno con su dirección I2C).
NoiseSensorI2CSlave sensor1(config1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Configurar Sensor 1
    config1.i2cAddress = 0x08;      // Primera dirección I2C
    config1.sdaPin = 8;
    config1.sclPin = 10;
    config1.adcPin = 4;
    config1.updateInterval = 1000;
    config1.logLevel = NoiseSensor::LOG_NONE;  // Sin logs para evitar confusión

    // Aplicar configuración al sensor antes de begin()
    sensor1.setConfig(config1);
    
    // Configurar Sensor 2
    config2.i2cAddress = 0x09;      // Segunda dirección I2C (diferente)
    config2.sdaPin = 8;            // Mismo bus I2C
    config2.sclPin = 10;
    config2.adcPin = 4;            // ADC válido en ESP32-C3 (GPIO0-4). Ejemplo conceptual.
    config2.updateInterval = 1000;
    config2.logLevel = NoiseSensor::LOG_NONE;
    
    Serial.println("=== Ejemplo: Múltiples Sensores I2C ===");
    Serial.println("NOTA: Este ejemplo requiere múltiples ESP32-C3");
    Serial.println("      cada uno con su sensor y dirección I2C única");
    Serial.println();
    
    // Inicializar el primer sensor
    Serial.println("Inicializando Sensor 1 (dirección 0x08)...");
    sensor1.begin();
    
    // Si tuvieras un segundo ESP32 con la misma librería, inicializarías ese OTRO dispositivo
    // con config2 (dirección 0x09). No es posible crear sensor2 en el mismo firmware.
    
    Serial.println("Sensores listos");
    Serial.println();
}

void loop() {
    // Actualizar todos los sensores
    sensor1.update();
    // sensor2.update();  // Si tienes múltiples sensores
    
    // Mostrar datos de todos los sensores
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 3000) {  // Cada 3 segundos
        lastPrint = millis();
        
        Serial.println("=== Datos de Sensores ===");
        
        // Sensor 1
        if (sensor1.isDataReady()) {
            const auto& data1 = sensor1.getData();
            Serial.println("Sensor 1 (0x08):");
            Serial.printf("  Promedio: %.2f mV, Pico: %.2f mV\n", 
                         data1.noiseAvg, data1.noisePeak);
        }
        
        // Sensor 2 (ejemplo)
        // if (sensor2.isDataReady()) {
        //     const auto& data2 = sensor2.getData();
        //     Serial.println("Sensor 2 (0x09):");
        //     Serial.printf("  Promedio: %.2f mV, Pico: %.2f mV\n", 
        //                  data2.noiseAvg, data2.noisePeak);
        // }
        
        Serial.println();
    }
    
    delay(10);
}

