/**
 * Ejemplo de ESP32 Maestro I2C
 * 
 * Este código muestra cómo solicitar datos del sensor de ruido
 * desde un ESP32 que actúa como maestro I2C.
 * 
 * El esclavo (ESP32-C3) debe estar configurado con dirección 0x08
 */

#include <Arduino.h>
#include <Wire.h>

// Dirección I2C del esclavo (ESP32-C3 con sensor de ruido)
#define I2C_SLAVE_ADDRESS 0x08

// Estructura de datos del sensor (debe coincidir con el esclavo)
// IMPORTANTE: Esta estructura debe coincidir exactamente con la del esclavo
struct SensorData {
    float noise;             // Valor actual de ruido
    float noiseAvg;          // Promedio de ruido
    float noisePeak;         // Valor pico
    float noiseMin;          // Valor mínimo
    float noiseAvgLegal;     // Promedio legal
    float noiseAvgLegalMax;  // Máximo promedio legal
    uint16_t lowNoiseLevel;  // Nivel base
    uint32_t cycles;         // Contador de ciclos
} sensorData;

// Comandos I2C (deben coincidir con el esclavo)
enum I2CCommand {
    CMD_GET_DATA = 0x01,
    CMD_GET_AVG = 0x02,
    CMD_GET_PEAK = 0x03,
    CMD_GET_MIN = 0x04,
    CMD_GET_LEGAL = 0x05,
    CMD_GET_LEGAL_MAX = 0x06,
    CMD_GET_STATUS = 0x07,
    CMD_RESET = 0x08
};

// Función para solicitar todos los datos del sensor
bool requestAllData() {
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(CMD_GET_DATA);
    if (Wire.endTransmission() != 0) {
        Serial.println("Error: No se pudo comunicar con el esclavo");
        return false;
    }
    
    // Solicitar los datos
    uint8_t bytesReceived = Wire.requestFrom(I2C_SLAVE_ADDRESS, sizeof(SensorData));
    
    if (bytesReceived < sizeof(SensorData)) {
        Serial.printf("Error: Datos incompletos. Recibidos: %d, Esperados: %d\n", 
                      bytesReceived, sizeof(SensorData));
        return false;
    }
    
    // Leer los datos
    Wire.readBytes((uint8_t*)&sensorData, sizeof(SensorData));
    return true;
}

// Función para solicitar un valor específico
float requestFloatValue(I2CCommand cmd) {
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(cmd);
    if (Wire.endTransmission() != 0) {
        Serial.println("Error: No se pudo comunicar con el esclavo");
        return 0.0;
    }
    
    Wire.requestFrom(I2C_SLAVE_ADDRESS, sizeof(float));
    if (Wire.available() < sizeof(float)) {
        return 0.0;
    }
    
    float value;
    Wire.readBytes((uint8_t*)&value, sizeof(float));
    return value;
}

// Función para verificar el estado del esclavo
bool checkSlaveStatus() {
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(CMD_GET_STATUS);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(I2C_SLAVE_ADDRESS, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    uint8_t status = Wire.read();
    return (status == 0x01);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 I2C Master - Noise Sensor Reader ===");
    
    // Inicializar I2C como maestro
    Wire.begin();
    // Opcional: configurar velocidad I2C (por defecto 100kHz)
    // Wire.setClock(100000);  // 100kHz
    // Wire.setClock(400000);  // 400kHz (Fast Mode)
    
    Serial.println("I2C maestro inicializado");
    Serial.println("Buscando esclavo en dirección 0x08...");
    Serial.println();
}

void loop() {
    // Verificar si el esclavo está disponible
    if (!checkSlaveStatus()) {
        Serial.println("Esclavo no disponible o sin datos");
        delay(2000);
        return;
    }
    
    Serial.println("=== Solicitud de Datos ===");
    
    // Opción 1: Solicitar todos los datos de una vez
    if (requestAllData()) {
        Serial.println("Datos completos recibidos:");
        Serial.printf("  Ruido Actual: %.2f mV\n", sensorData.noise);
        Serial.printf("  Promedio: %.2f mV\n", sensorData.noiseAvg);
        Serial.printf("  Pico: %.2f mV\n", sensorData.noisePeak);
        Serial.printf("  Mínimo: %.2f mV\n", sensorData.noiseMin);
        Serial.printf("  Promedio Legal: %.2f mV\n", sensorData.noiseAvgLegal);
        Serial.printf("  Máximo Legal: %.2f mV\n", sensorData.noiseAvgLegalMax);
        Serial.printf("  Nivel Base: %d mV\n", sensorData.lowNoiseLevel);
        Serial.printf("  Ciclos: %u\n", sensorData.cycles);
    } else {
        Serial.println("Error al recibir datos completos");
    }
    
    Serial.println();
    
    // Opción 2: Solicitar valores individuales (ejemplo)
    Serial.println("Valores individuales:");
    float avg = requestFloatValue(CMD_GET_AVG);
    Serial.printf("  Promedio (individual): %.2f mV\n", avg);
    
    float peak = requestFloatValue(CMD_GET_PEAK);
    Serial.printf("  Pico (individual): %.2f mV\n", peak);
    
    Serial.println();
    Serial.println("---");
    Serial.println();
    
    // Esperar antes de la próxima solicitud
    delay(5000); // Solicitar datos cada 5 segundos
}

