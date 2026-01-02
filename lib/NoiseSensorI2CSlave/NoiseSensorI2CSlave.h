#ifndef NOISE_SENSOR_I2C_SLAVE_H
#define NOISE_SENSOR_I2C_SLAVE_H

#include <Arduino.h>
#include <Wire.h>
#include "NoiseSensor.h"

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
    CMD_GET_DATA = 0x01,      // Solicitar todos los datos
    CMD_GET_AVG = 0x02,       // Solicitar promedio
    CMD_GET_PEAK = 0x03,      // Solicitar pico
    CMD_GET_MIN = 0x04,       // Solicitar mínimo
    CMD_GET_LEGAL = 0x05,     // Solicitar promedio legal
    CMD_GET_LEGAL_MAX = 0x06, // Solicitar máximo legal
    CMD_GET_STATUS = 0x07,    // Solicitar estado
    CMD_RESET = 0x08          // Resetear ciclo
};

/**
 * Clase para manejar un sensor de ruido como esclavo I2C
 */
class NoiseSensorI2CSlave {
public:
    /**
     * Configuración del esclavo I2C
     */
    struct Config {
        uint8_t i2cAddress = 0x08;      // Dirección I2C del esclavo
        uint8_t sdaPin = 8;              // Pin SDA
        uint8_t sclPin = 10;             // Pin SCL
        uint8_t adcPin = 4;              // Pin ADC para el sensor
        unsigned long updateInterval = 1000; // Intervalo de actualización en ms
        NoiseSensor::LogLevel logLevel = NoiseSensor::LOG_INFO;
    };

    /**
     * Constructor
     * @param config Configuración del esclavo I2C
     */
    NoiseSensorI2CSlave(const Config& config);

    /**
     * Inicializar el esclavo I2C y el sensor
     */
    void begin();

    /**
     * Actualizar el sensor y los datos I2C (debe llamarse en loop())
     */
    void update();

    /**
     * Obtener los datos actuales del sensor
     * @return Referencia a la estructura SensorData
     */
    const SensorData& getData() const { return sensorData; }

    /**
     * Verificar si hay datos listos
     * @return true si hay datos disponibles
     */
    bool isDataReady() const { return dataReady; }

    /**
     * Obtener el objeto NoiseSensor para configuración avanzada
     * @return Referencia al objeto NoiseSensor
     */
    NoiseSensor& getNoiseSensor() { return noiseSensor; }

private:
    Config config;
    NoiseSensor noiseSensor;
    SensorData sensorData;
    bool dataReady;
    unsigned long lastUpdate;

    // Callbacks I2C (deben ser estáticos o usar punteros)
    static NoiseSensorI2CSlave* instance;
    static void onRequestStatic();
    static void onReceiveStatic(int numBytes);
    
    void onRequest();
    void onReceive(int numBytes);
};

#endif // NOISE_SENSOR_I2C_SLAVE_H

