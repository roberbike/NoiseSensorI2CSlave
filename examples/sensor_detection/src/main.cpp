/**
 * Ejemplo de ESP32 Maestro I2C - Detección Automática de Sensores
 * 
 * Este ejemplo muestra cómo:
 * - Escanear direcciones I2C para detectar sensores
 * - Identificar sensores de ruido usando CMD_PING
 * - Verificar el estado de los sensores (CMD_GET_READY)
 * - Leer datos de los sensores detectados
 * 
 * Este código es útil cuando el maestro maneja múltiples tipos
 * de sensores y necesita detectarlos automáticamente al arrancar.
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

// Estructura para almacenar información de sensores detectados
struct DetectedSensor {
    uint8_t address;
    SensorIdentity identity;
    bool ready;
    bool valid;
};

// Array para almacenar sensores detectados (máximo 10)
DetectedSensor detectedSensors[10];
uint8_t numDetectedSensors = 0;

/**
 * Función para detectar un sensor en una dirección I2C
 * @param address Dirección I2C a escanear
 * @return true si se detectó un sensor válido
 */
bool detectSensor(uint8_t address) {
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
        // Guardar información del sensor
        if (numDetectedSensors < 10) {
            detectedSensors[numDetectedSensors].address = address;
            detectedSensors[numDetectedSensors].identity = identity;
            detectedSensors[numDetectedSensors].valid = true;
            numDetectedSensors++;
            
            Serial.printf("✓ Sensor de ruido detectado en dirección 0x%02X\n", address);
            Serial.printf("  Versión: %d.%d\n", identity.versionMajor, identity.versionMinor);
            Serial.printf("  Estado: inicializado=%d, ADC activo=%d, datos listos=%d\n",
                         (identity.status & 0x01) ? 1 : 0,
                         (identity.status & 0x02) ? 1 : 0,
                         (identity.status & 0x04) ? 1 : 0);
            return true;
        }
    }
    
    return false;
}

/**
 * Verificar si un sensor está listo para enviar datos
 * @param address Dirección I2C del sensor
 * @return true si está listo
 */
bool checkSensorReady(uint8_t address) {
    Wire.beginTransmission(address);
    Wire.write(CMD_GET_READY);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(address, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    uint8_t ready = Wire.read();
    return (ready == 0x01);
}

/**
 * Leer datos de un sensor
 * @param address Dirección I2C del sensor
 * @param data Estructura donde se guardarán los datos
 * @return true si se leyeron los datos correctamente
 */
bool readSensorData(uint8_t address, SensorData& data) {
    Wire.beginTransmission(address);
    Wire.write(CMD_GET_DATA);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    uint8_t bytesReceived = Wire.requestFrom(address, sizeof(SensorData));
    if (bytesReceived < sizeof(SensorData)) {
        return false;
    }
    
    Wire.readBytes((uint8_t*)&data, sizeof(SensorData));
    return true;
}

/**
 * Escanear todas las direcciones I2C y detectar sensores
 */
void scanForSensors() {
    Serial.println("=== Escaneando direcciones I2C ===");
    Serial.printf("Rango: 0x%02X - 0x%02X\n", MIN_I2C_ADDRESS, MAX_I2C_ADDRESS);
    Serial.println();
    
    numDetectedSensors = 0;
    
    for (uint8_t address = MIN_I2C_ADDRESS; address <= MAX_I2C_ADDRESS; address++) {
        if (detectSensor(address)) {
            // El sensor ya fue guardado en detectSensor()
            delay(50);  // Pequeño delay entre detecciones
        }
    }
    
    Serial.println();
    Serial.printf("Total de sensores detectados: %d\n", numDetectedSensors);
    Serial.println();
}

/**
 * Verificar el estado de todos los sensores detectados
 */
void checkSensorsStatus() {
    Serial.println("=== Verificando estado de sensores ===");
    
    for (uint8_t i = 0; i < numDetectedSensors; i++) {
        uint8_t address = detectedSensors[i].address;
        bool ready = checkSensorReady(address);
        detectedSensors[i].ready = ready;
        
        Serial.printf("Sensor 0x%02X: %s\n", address, ready ? "LISTO ✓" : "NO LISTO ✗");
    }
    
    Serial.println();
}

/**
 * Leer y mostrar datos de todos los sensores listos
 */
void readAllSensors() {
    Serial.println("=== Datos de Sensores ===");
    
    for (uint8_t i = 0; i < numDetectedSensors; i++) {
        if (!detectedSensors[i].ready) {
            continue;  // Saltar sensores no listos
        }
        
        SensorData data;
        if (readSensorData(detectedSensors[i].address, data)) {
            Serial.printf("\nSensor 0x%02X:\n", detectedSensors[i].address);
            Serial.printf("  Ruido Actual:     %.2f mV\n", data.noise);
            Serial.printf("  Promedio:         %.2f mV\n", data.noiseAvg);
            Serial.printf("  Pico:             %.2f mV\n", data.noisePeak);
            Serial.printf("  Mínimo:           %.2f mV\n", data.noiseMin);
            Serial.printf("  Promedio Legal:   %.2f mV\n", data.noiseAvgLegal);
            Serial.printf("  Máximo Legal:     %.2f mV\n", data.noiseAvgLegalMax);
            Serial.printf("  Nivel Base:       %d mV\n", data.lowNoiseLevel);
            Serial.printf("  Ciclos:           %u\n", data.cycles);
        } else {
            Serial.printf("Error al leer datos del sensor 0x%02X\n", detectedSensors[i].address);
        }
    }
    
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ESP32 I2C Master - Detección Automática de Sensores ===");
    Serial.println();
    
    // Inicializar I2C como maestro
    Wire.begin();
    // Opcional: configurar velocidad I2C
    // Wire.setClock(100000);  // 100kHz (por defecto)
    // Wire.setClock(400000);  // 400kHz (Fast Mode)
    
    Serial.println("I2C maestro inicializado");
    Serial.println();
    
    // Escanear y detectar sensores
    scanForSensors();
    
    if (numDetectedSensors == 0) {
        Serial.println("⚠️  No se detectaron sensores de ruido");
        Serial.println("   Verifica las conexiones I2C y que los sensores estén encendidos");
        Serial.println();
    } else {
        // Verificar estado de los sensores
        checkSensorsStatus();
    }
}

void loop() {
    if (numDetectedSensors == 0) {
        // Si no hay sensores, intentar escanear nuevamente cada 10 segundos
        static unsigned long lastScan = 0;
        if (millis() - lastScan >= 10000) {
            lastScan = millis();
            Serial.println("Reintentando detección de sensores...");
            scanForSensors();
            if (numDetectedSensors > 0) {
                checkSensorsStatus();
            }
        }
        delay(1000);
        return;
    }
    
    // Verificar estado periódicamente (cada 5 segundos)
    static unsigned long lastStatusCheck = 0;
    if (millis() - lastStatusCheck >= 5000) {
        lastStatusCheck = millis();
        checkSensorsStatus();
    }
    
    // Leer datos de todos los sensores listos (cada 2 segundos)
    static unsigned long lastRead = 0;
    if (millis() - lastRead >= 2000) {
        lastRead = millis();
        readAllSensors();
    }
    
    delay(100);
}

