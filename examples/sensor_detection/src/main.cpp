/**
 * Ejemplo de ESP32 Maestro I2C - Detección de Sensor de Ruido
 * 
 * Este ejemplo muestra cómo:
 * - Escanear direcciones I2C para detectar el sensor de ruido
 * - Identificar el sensor usando CMD_PING
 * - Verificar que está listo con CMD_GET_READY
 * - Leer datos del sensor
 */

#include <Arduino.h>
#include <Wire.h>

// Estructura de identificación del sensor (debe coincidir con el esclavo)
struct SensorIdentity {
    uint8_t sensorType;       // Tipo de sensor (0x01 = Noise Sensor)
    uint8_t versionMajor;     // Versión mayor
    uint8_t versionMinor;     // Versión menor
    uint8_t status;           // Estado: bit 0 = inicializado, bit 1 = ADC activo, bit 2 = datos listos
    uint8_t i2cAddress;       // Dirección I2C del sensor
} __attribute__((packed));

// Estructura de datos del sensor
struct SensorData {
    float noise;
    float noiseAvg;
    float noisePeak;
    float noiseMin;
    float noiseAvgLegal;
    float noiseAvgLegalMax;
    uint16_t lowNoiseLevel;
    uint32_t cycles;
};

// Comandos I2C
enum I2CCommand {
    CMD_GET_DATA = 0x01,
    CMD_GET_AVG = 0x02,
    CMD_GET_PEAK = 0x03,
    CMD_GET_MIN = 0x04,
    CMD_GET_LEGAL = 0x05,
    CMD_GET_LEGAL_MAX = 0x06,
    CMD_GET_STATUS = 0x07,
    CMD_RESET = 0x08,
    CMD_PING = 0x09,
    CMD_GET_READY = 0x0A
};

// Constantes
static constexpr uint8_t SENSOR_TYPE_NOISE = 0x01;
static constexpr uint8_t MIN_I2C_ADDRESS = 0x08;
static constexpr uint8_t MAX_I2C_ADDRESS = 0x77;

// Variables globales para el sensor detectado
uint8_t sensorAddress = 0;           // Dirección I2C del sensor (0 = no detectado)
SensorIdentity sensorIdentity;       // Información de identificación
bool sensorDetected = false;         // Flag de sensor detectado
bool sensorReady = false;            // Flag de sensor listo

/**
 * Detectar el sensor de ruido en una dirección I2C
 * @param address Dirección I2C a verificar
 * @return true si se detectó el sensor de ruido
 */
bool detectNoiseSensor(uint8_t address) {
    // Intentar comunicación con el dispositivo
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        return false;  // No hay dispositivo en esta dirección
    }
    
    // Enviar comando PING
    Wire.beginTransmission(address);
    Wire.write(CMD_PING);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    // ESP32-C3 (esclavo): más estable con STOP + pequeña espera antes del requestFrom()
    delayMicroseconds(200);
    
    // Solicitar respuesta de identificación
    uint8_t bytesReceived = Wire.requestFrom(address, sizeof(SensorIdentity));
    if (bytesReceived < sizeof(SensorIdentity)) {
        return false;
    }
    
    // Leer identificación
    SensorIdentity identity;
    Wire.readBytes((uint8_t*)&identity, sizeof(SensorIdentity));
    
    // Verificar que es un sensor de ruido
    if (identity.sensorType == SENSOR_TYPE_NOISE) {
        sensorAddress = address;
        sensorIdentity = identity;
        sensorDetected = true;
        
        Serial.printf("✓ Sensor de ruido detectado en dirección 0x%02X\n", address);
        Serial.printf("  Versión: %d.%d\n", identity.versionMajor, identity.versionMinor);
        Serial.printf("  Estado: inicializado=%d, ADC activo=%d, datos listos=%d\n",
                     (identity.status & 0x01) ? 1 : 0,
                     (identity.status & 0x02) ? 1 : 0,
                     (identity.status & 0x04) ? 1 : 0);
        return true;
    }
    
    return false;
}

/**
 * Escanear direcciones I2C para encontrar el sensor de ruido
 */
void scanForNoiseSensor() {
    Serial.println("=== Buscando sensor de ruido ===");
    Serial.printf("Escaneando direcciones I2C: 0x%02X - 0x%02X\n", MIN_I2C_ADDRESS, MAX_I2C_ADDRESS);
    Serial.println();
    
    sensorDetected = false;
    sensorAddress = 0;
    
    for (uint8_t address = MIN_I2C_ADDRESS; address <= MAX_I2C_ADDRESS; address++) {
        if (detectNoiseSensor(address)) {
            // Sensor encontrado, salir del bucle
            break;
        }
        delay(10);  // Pequeño delay entre intentos
    }
    
    if (!sensorDetected) {
        Serial.println("✗ Sensor de ruido no detectado");
        Serial.println("  Verifica las conexiones I2C y que el sensor esté encendido");
    }
    
    Serial.println();
}

/**
 * Verificar si el sensor está listo para enviar datos
 * @return true si está listo
 */
bool checkSensorReady() {
    if (!sensorDetected || sensorAddress == 0) {
        return false;
    }
    
    Wire.beginTransmission(sensorAddress);
    Wire.write(CMD_GET_READY);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    delayMicroseconds(200);
    
    Wire.requestFrom(sensorAddress, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    uint8_t ready = Wire.read();
    sensorReady = (ready == 0x01);
    return sensorReady;
}

/**
 * Leer datos del sensor
 * @param data Estructura donde se guardarán los datos
 * @return true si se leyeron los datos correctamente
 */
bool readSensorData(SensorData& data) {
    if (!sensorDetected || sensorAddress == 0) {
        return false;
    }
    
    Wire.beginTransmission(sensorAddress);
    Wire.write(CMD_GET_DATA);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    delayMicroseconds(200);
    
    uint8_t bytesReceived = Wire.requestFrom(sensorAddress, sizeof(SensorData));
    if (bytesReceived < sizeof(SensorData)) {
        return false;
    }
    
    Wire.readBytes((uint8_t*)&data, sizeof(SensorData));
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 I2C Master - Sensor de Ruido ===");
    Serial.println();
    
    // Inicializar I2C como maestro
    Wire.setBufferSize(64);
    Wire.begin(4, 5);
    // Opcional: configurar velocidad I2C
    // Wire.setClock(100000);  // 100kHz (por defecto)
    // Wire.setClock(400000);  // 400kHz (Fast Mode)
    
    Serial.println("I2C maestro inicializado");
    Serial.println();
    
    // Buscar el sensor de ruido
    scanForNoiseSensor();
    
    if (sensorDetected) {
        // Verificar si está listo
        Serial.println("=== Verificando estado del sensor ===");
        if (checkSensorReady()) {
            Serial.printf("Sensor 0x%02X: LISTO ✓\n", sensorAddress);
        } else {
            Serial.printf("Sensor 0x%02X: NO LISTO ✗\n", sensorAddress);
            Serial.println("Esperando a que el sensor esté listo...");
        }
        Serial.println();
    }
}

void loop() {
    if (!sensorDetected) {
        // Si no se detectó el sensor, intentar buscarlo nuevamente cada 10 segundos
        static unsigned long lastScan = 0;
        if (millis() - lastScan >= 10000) {
            lastScan = millis();
            Serial.println("Reintentando detección del sensor...");
            scanForNoiseSensor();
            if (sensorDetected) {
                checkSensorReady();
            }
        }
        delay(1000);
        return;
    }
    
    // Verificar periódicamente si está listo (cada 5 segundos)
    static unsigned long lastStatusCheck = 0;
    if (millis() - lastStatusCheck >= 5000) {
        lastStatusCheck = millis();
        bool wasReady = sensorReady;
        checkSensorReady();
        
        if (sensorReady && !wasReady) {
            Serial.printf("Sensor 0x%02X ahora está LISTO ✓\n", sensorAddress);
        } else if (!sensorReady && wasReady) {
            Serial.printf("Sensor 0x%02X ya NO está listo ✗\n", sensorAddress);
        }
    }
    
    // Leer datos del sensor si está listo (cada 2 segundos)
    if (sensorReady) {
        static unsigned long lastRead = 0;
        if (millis() - lastRead >= 2000) {
            lastRead = millis();
            
            SensorData data;
            if (readSensorData(data)) {
                Serial.println("=== Datos del Sensor de Ruido ===");
                Serial.printf("Ruido Actual:     %.2f mV\n", data.noise);
                Serial.printf("Promedio:         %.2f mV\n", data.noiseAvg);
                Serial.printf("Pico:             %.2f mV\n", data.noisePeak);
                Serial.printf("Mínimo:           %.2f mV\n", data.noiseMin);
                Serial.printf("Promedio Legal:   %.2f mV\n", data.noiseAvgLegal);
                Serial.printf("Máximo Legal:     %.2f mV\n", data.noiseAvgLegalMax);
                Serial.printf("Nivel Base:       %d mV\n", data.lowNoiseLevel);
                Serial.printf("Ciclos:           %u\n", data.cycles);
                Serial.println();
            } else {
                Serial.println("Error al leer datos del sensor");
            }
        }
    }
    
    delay(100);
}
