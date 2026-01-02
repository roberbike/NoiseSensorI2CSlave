#include "NoiseSensorI2CSlave.h"
#include <cstring>

// Instancia estática para los callbacks
NoiseSensorI2CSlave* NoiseSensorI2CSlave::instance = nullptr;

NoiseSensorI2CSlave::NoiseSensorI2CSlave(const Config& config) 
    : config(config), dataReady(false), initialized(false), adcActive(false), lastUpdate(0) {
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
    // Validar configuración usando el método isValid()
    if (!isValid()) {
        if (config.logLevel >= NoiseSensor::LOG_ERROR) {
            if (config.i2cAddress < MIN_I2C_ADDRESS || config.i2cAddress > MAX_I2C_ADDRESS) {
                Serial.printf("ERROR: Dirección I2C inválida (0x%02X). Debe estar entre 0x%02X y 0x%02X\n", 
                              config.i2cAddress, MIN_I2C_ADDRESS, MAX_I2C_ADDRESS);
            }
            if (config.updateInterval < MIN_UPDATE_INTERVAL) {
                Serial.printf("ERROR: Intervalo de actualización inválido (%lu ms). Debe ser >= %lu ms\n", 
                              config.updateInterval, MIN_UPDATE_INTERVAL);
            }
        }
        return;
    }
    
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
    
    // Verificar que el ADC está recibiendo señal del micrófono
    adcActive = checkADCSignal();
    
    if (!adcActive) {
        if (config.logLevel >= NoiseSensor::LOG_ERROR) {
            Serial.println("ERROR: No se detecta señal en el ADC. Verifica la conexión del micrófono.");
        }
        // No marcar como inicializado si no hay señal ADC
        return;
    }
    
    // Marcar como inicializado solo si todo fue exitoso
    initialized = true;
    
    if (config.logLevel >= NoiseSensor::LOG_INFO) {
        Serial.println("Sensor de ruido inicializado");
        Serial.println("ADC activo - Micrófono detectado");
        Serial.println("Esperando solicitudes I2C...");
        Serial.println();
    }
}

void NoiseSensorI2CSlave::update() {
    // No hacer nada si no está inicializado
    if (!initialized) {
        return;
    }
    
    // Actualizar sensor de ruido
    noiseSensor.update();
    
    // Verificar periódicamente que el ADC sigue activo (cada 5 segundos)
    static unsigned long lastADCCheck = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastADCCheck >= 5000) {
        lastADCCheck = currentMillis;
        adcActive = checkADCSignal();
        if (!adcActive && config.logLevel >= NoiseSensor::LOG_ERROR) {
            Serial.println("WARNING: Se perdió la señal del ADC");
        }
    }
    
    // Actualizar datos cada intervalo configurado
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
    // No responder si no está inicializado
    if (!initialized) {
        return;
    }
    
    // El maestro está solicitando datos
    // Enviamos los datos más recientes
    size_t bytesWritten = Wire.write((uint8_t*)&sensorData, sizeof(sensorData));
    
    // Verificar que se escribieron todos los bytes
    if (bytesWritten != sizeof(sensorData)) {
        if (config.logLevel >= NoiseSensor::LOG_ERROR) {
            Serial.printf("ERROR: Solo se escribieron %d de %d bytes en I2C\n", 
                          bytesWritten, sizeof(sensorData));
        }
    }
}

void NoiseSensorI2CSlave::onReceive(int numBytes) {
    // No procesar si no está inicializado
    if (!initialized) {
        return;
    }
    
    if (numBytes == 0) {
        if (config.logLevel >= NoiseSensor::LOG_INFO) {
            Serial.println("WARNING: Recibido comando I2C sin bytes");
        }
        return;
    }
    
    // Verificar que hay datos disponibles
    if (!Wire.available()) {
        if (config.logLevel >= NoiseSensor::LOG_ERROR) {
            Serial.println("ERROR: No hay datos disponibles en I2C");
        }
        return;
    }
    
    uint8_t command = Wire.read();
    
    // Verificar que se leyó el comando correctamente
    if (numBytes < 1) {
        if (config.logLevel >= NoiseSensor::LOG_ERROR) {
            Serial.println("ERROR: No se pudo leer el comando I2C");
        }
        return;
    }
    
    switch (command) {
        case CMD_GET_DATA:
            // Los datos se enviarán en onRequest()
            break;
            
        case CMD_GET_AVG: {
            size_t bytesWritten = Wire.write((uint8_t*)&sensorData.noiseAvg, sizeof(float));
            if (bytesWritten != sizeof(float) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir promedio en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(float));
            }
            break;
        }
            
        case CMD_GET_PEAK: {
            size_t bytesWritten = Wire.write((uint8_t*)&sensorData.noisePeak, sizeof(float));
            if (bytesWritten != sizeof(float) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir pico en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(float));
            }
            break;
        }
            
        case CMD_GET_MIN: {
            size_t bytesWritten = Wire.write((uint8_t*)&sensorData.noiseMin, sizeof(float));
            if (bytesWritten != sizeof(float) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir mínimo en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(float));
            }
            break;
        }
            
        case CMD_GET_LEGAL: {
            size_t bytesWritten = Wire.write((uint8_t*)&sensorData.noiseAvgLegal, sizeof(float));
            if (bytesWritten != sizeof(float) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir promedio legal en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(float));
            }
            break;
        }
            
        case CMD_GET_LEGAL_MAX: {
            size_t bytesWritten = Wire.write((uint8_t*)&sensorData.noiseAvgLegalMax, sizeof(float));
            if (bytesWritten != sizeof(float) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir máximo legal en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(float));
            }
            break;
        }
            
        case CMD_GET_STATUS: {
            uint8_t status = dataReady ? 0x01 : 0x00;
            size_t bytesWritten = Wire.write(status);
            if (bytesWritten != 1 && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.println("ERROR: Error al escribir estado en I2C");
            }
            break;
        }
            
        case CMD_RESET:
            if (noiseSensor.isCycleComplete()) {
                noiseSensor.resetCycle();
                if (config.logLevel >= NoiseSensor::LOG_INFO) {
                    Serial.println("Ciclo reseteado por comando I2C");
                }
            } else {
                if (config.logLevel >= NoiseSensor::LOG_INFO) {
                    Serial.println("WARNING: Intento de resetear ciclo que no está completo");
                }
            }
            break;
            
        case CMD_PING:  // CMD_PING y CMD_IDENTIFY tienen el mismo valor (0x09), usar solo uno
        {
            // Enviar información de identificación del sensor
            SensorIdentity identity;
            identity.sensorType = SENSOR_TYPE_NOISE;
            identity.versionMajor = VERSION_MAJOR;
            identity.versionMinor = VERSION_MINOR;
            identity.status = 0;
            if (initialized) identity.status |= 0x01;  // Bit 0: inicializado
            if (adcActive) identity.status |= 0x02;   // Bit 1: ADC activo
            if (dataReady) identity.status |= 0x04;   // Bit 2: datos listos
            identity.i2cAddress = config.i2cAddress;
            
            size_t bytesWritten = Wire.write((uint8_t*)&identity, sizeof(identity));
            if (bytesWritten != sizeof(identity) && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.printf("ERROR: Error al escribir identificación en I2C (%d/%d bytes)\n", 
                             bytesWritten, sizeof(identity));
            }
            break;
        }
            
        case CMD_GET_READY: {
            // Enviar estado de "ready" (1 byte: 0x01 = listo, 0x00 = no listo)
            uint8_t ready = isReady() ? 0x01 : 0x00;
            size_t bytesWritten = Wire.write(ready);
            if (bytesWritten != 1 && config.logLevel >= NoiseSensor::LOG_ERROR) {
                Serial.println("ERROR: Error al escribir estado ready en I2C");
            }
            break;
        }
            
        default:
            // Comando desconocido
            if (config.logLevel >= NoiseSensor::LOG_INFO) {
                Serial.printf("WARNING: Comando I2C desconocido: 0x%02X\n", command);
            }
            break;
    }
}

// Implementación de métodos públicos
bool NoiseSensorI2CSlave::isReady() const {
    // Está listo si está inicializado Y el ADC está activo
    return initialized && adcActive;
}

// Implementación de método privado para verificar señal ADC
bool NoiseSensorI2CSlave::checkADCSignal() {
    // Leer varias muestras del ADC para verificar que hay señal
    const int numSamples = 10;
    int activeSamples = 0;
    
    for (int i = 0; i < numSamples; i++) {
        // Leer directamente del ADC
        int adcValue = analogRead(config.adcPin);
        
        // Considerar que hay señal si el valor está en un rango razonable
        // (no en 0 o máximo, lo que indicaría problema de conexión)
        if (adcValue > 50 && adcValue < 4000) {  // Rango razonable para ESP32-C3 ADC (0-4095)
            activeSamples++;
        }
        delay(10);  // Pequeño delay entre muestras
    }
    
    // Si al menos el 70% de las muestras muestran señal activa, consideramos que el ADC está activo
    return (activeSamples * 100 / numSamples) >= 70;
}

