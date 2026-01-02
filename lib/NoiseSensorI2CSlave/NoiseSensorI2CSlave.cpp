#include "NoiseSensorI2CSlave.h"
#include <cstring>

// Instancia estática para los callbacks
NoiseSensorI2CSlave* NoiseSensorI2CSlave::instance = nullptr;

NoiseSensorI2CSlave::NoiseSensorI2CSlave(const Config& config) 
    : config(config), dataReady(false), lastUpdate(0) {
    // Configurar NoiseSensor
    NoiseSensor::Config noiseConfig;
    noiseConfig.adcPin = config.adcPin;
    noiseConfig.logLevel = config.logLevel;
    noiseSensor = NoiseSensor(noiseConfig);
    
    // Inicializar estructura de datos
    memset(&sensorData, 0, sizeof(sensorData));
    
    // Establecer instancia para callbacks estáticos
    instance = this;
}

void NoiseSensorI2CSlave::begin() {
    if (config.logLevel >= NoiseSensor::LOG_INFO) {
        Serial.println("=== Inicializando NoiseSensor I2C Slave ===");
        Serial.printf("Dirección I2C: 0x%02X\n", config.i2cAddress);
        Serial.printf("SDA Pin: %d, SCL Pin: %d\n", config.sdaPin, config.sclPin);
        Serial.printf("ADC Pin: %d\n", config.adcPin);
    }
    
    // Configurar I2C como esclavo
    Wire.begin(config.i2cAddress, config.sdaPin, config.sclPin);
    Wire.onRequest(onRequestStatic);  // Callback cuando el maestro solicita datos
    Wire.onReceive(onReceiveStatic);  // Callback cuando el maestro envía datos
    
    if (config.logLevel >= NoiseSensor::LOG_INFO) {
        Serial.println("I2C esclavo configurado");
    }
    
    // Inicializar sensor de ruido
    noiseSensor.begin();
    
    if (config.logLevel >= NoiseSensor::LOG_INFO) {
        Serial.println("Sensor de ruido inicializado");
        Serial.println("Esperando solicitudes I2C...");
        Serial.println();
    }
}

void NoiseSensorI2CSlave::update() {
    // Actualizar sensor de ruido
    noiseSensor.update();
    
    // Actualizar datos cada intervalo configurado
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate >= config.updateInterval) {
        lastUpdate = currentMillis;
        
        const auto& measurements = noiseSensor.getMeasurements();
        
        // Actualizar estructura de datos
        sensorData.noise = measurements.noise;
        sensorData.noiseAvg = measurements.noiseAvg;
        sensorData.noisePeak = measurements.noisePeak;
        sensorData.noiseMin = measurements.noiseMin;
        sensorData.noiseAvgLegal = measurements.noiseAvgLegal;
        sensorData.noiseAvgLegalMax = measurements.noiseAvgLegalMax;
        sensorData.lowNoiseLevel = measurements.lowNoiseLevel;
        sensorData.cycles = measurements.cycles;
        
        dataReady = true;
        
        // Mostrar datos por Serial (opcional, para debug)
        if (config.logLevel >= NoiseSensor::LOG_INFO) {
            Serial.println("=== Datos del Sensor ===");
            Serial.printf("Actual: %.2f mV\n", sensorData.noise);
            Serial.printf("Promedio: %.2f mV\n", sensorData.noiseAvg);
            Serial.printf("Pico: %.2f mV\n", sensorData.noisePeak);
            Serial.printf("Mínimo: %.2f mV\n", sensorData.noiseMin);
            Serial.printf("Promedio Legal: %.2f mV\n", sensorData.noiseAvgLegal);
            Serial.printf("Máximo Legal: %.2f mV\n", sensorData.noiseAvgLegalMax);
            Serial.printf("Nivel Base: %d mV\n", sensorData.lowNoiseLevel);
            Serial.printf("Ciclos: %u\n", sensorData.cycles);
            Serial.println();
        }
        
        // Si el ciclo está completo, resetear
        if (noiseSensor.isCycleComplete()) {
            if (config.logLevel >= NoiseSensor::LOG_INFO) {
                Serial.println("Ciclo completado - datos listos para enviar");
            }
            noiseSensor.resetCycle();
        }
    }
}

// Callbacks estáticos que redirigen a la instancia
void NoiseSensorI2CSlave::onRequestStatic() {
    if (instance != nullptr) {
        instance->onRequest();
    }
}

void NoiseSensorI2CSlave::onReceiveStatic(int numBytes) {
    if (instance != nullptr) {
        instance->onReceive(numBytes);
    }
}

// Implementación de los callbacks
void NoiseSensorI2CSlave::onRequest() {
    // El maestro está solicitando datos
    // Enviamos los datos más recientes
    Wire.write((uint8_t*)&sensorData, sizeof(sensorData));
}

void NoiseSensorI2CSlave::onReceive(int numBytes) {
    if (numBytes == 0) return;
    
    uint8_t command = Wire.read();
    
    switch (command) {
        case CMD_GET_DATA:
            // Los datos se enviarán en onRequest()
            break;
            
        case CMD_GET_AVG:
            Wire.write((uint8_t*)&sensorData.noiseAvg, sizeof(float));
            break;
            
        case CMD_GET_PEAK:
            Wire.write((uint8_t*)&sensorData.noisePeak, sizeof(float));
            break;
            
        case CMD_GET_MIN:
            Wire.write((uint8_t*)&sensorData.noiseMin, sizeof(float));
            break;
            
        case CMD_GET_LEGAL:
            Wire.write((uint8_t*)&sensorData.noiseAvgLegal, sizeof(float));
            break;
            
        case CMD_GET_LEGAL_MAX:
            Wire.write((uint8_t*)&sensorData.noiseAvgLegalMax, sizeof(float));
            break;
            
        case CMD_GET_STATUS:
            {
                uint8_t status = dataReady ? 0x01 : 0x00;
                Wire.write(status);
            }
            break;
            
        case CMD_RESET:
            if (noiseSensor.isCycleComplete()) {
                noiseSensor.resetCycle();
            }
            break;
            
        default:
            // Comando desconocido
            break;
    }
}

