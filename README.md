# NoiseSensorI2CSlave - Librería para ESP32

[![PlatformIO](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://platformio.org/)
[![License](https://img.shields.io/badge/license-GPL--3.0-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-orange.svg)](https://github.com/TU_USUARIO/NoiseSensorI2CSlave/releases)

Librería para ESP32 que convierte un sensor de ruido en un dispositivo esclavo I2C. Permite leer datos del sensor de ruido, usando la librería NoiseSensor y enviarlos por I2C cuando otro ESP32 (maestro) los solicita.

## Características

- ✅ Librería reutilizable para múltiples proyectos
- ✅ Lee datos del sensor de ruido usando NoiseSensor
- ✅ Funciona como esclavo I2C configurable
- ✅ Responde a comandos del maestro
- ✅ Envía datos estructurados por I2C
- ✅ Actualización automática de datos configurable
- ✅ API simple y fácil de usar
- ✅ I2C estable en ESP32-C3 (callbacks mínimos, `IRAM_ATTR`, respuesta siempre en `onRequest()`)

## Hardware Requerido

- **ESP32-C3** (por ejemplo, Lolin C3 Mini)
- **Sensor de ruido** conectado al GPIO 4 (ADC)
- **Conexiones I2C**:
  - SDA: GPIO 8
  - SCL: GPIO 10
  - GND: Común
  - VCC: 3.3V

> Nota importante: Asegúrate de que el bus I2C tiene **pull-ups** en SDA/SCL (típicamente 4.7k–10k a 3.3V) y que ambos ESP32 comparten **GND**.

## Conexiones

```
ESP32-C3 (Esclavo)          ESP32 (Maestro)
GPIO 8 (SDA)  <---------->  SDA
GPIO 10 (SCL)  <---------->  SCL
GND           <---------->  GND
3.3V          <---------->  3.3V

Sensor de Ruido
Salida        <---------->  GPIO 4 (ADC)
GND           <---------->  GND
VCC           <---------->  3.3V
```

## Instalación

### Desde GitHub (Recomendado)

Agrega la librería directamente desde GitHub en tu `platformio.ini`:

```ini
lib_deps = 
    roberbike/NoiseSensor@^1.0.1
    https://github.com/roberbike/NoiseSensorI2CSlave.git
```


### Como Librería Local

La librería está incluida en el directorio `lib/NoiseSensorI2CSlave/`. PlatformIO la detectará automáticamente.

### Copiar Manualmente

Si prefieres copiar la librería manualmente:

1. Copia el directorio `lib/NoiseSensorI2CSlave/` a tu proyecto
2. Asegúrate de tener la dependencia en `platformio.ini`:

```ini
lib_deps = 
    roberbike/NoiseSensor@^1.0.1
```

## Uso Básico

### Ejemplo Simple

```cpp
#include <Arduino.h>
#include "NoiseSensorI2CSlave.h"

// Crear instancia con configuración por defecto
NoiseSensorI2CSlave::Config config;
NoiseSensorI2CSlave sensor(config);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // Personaliza la configuración en setup() y aplícala ANTES de begin()
    config.i2cAddress = 0x08;       // Dirección I2C
    config.sdaPin = 8;              // Pin SDA (ESP32-C3)
    config.sclPin = 10;             // Pin SCL (ESP32-C3)
    config.adcPin = 4;              // Pin ADC (ESP32-C3: GPIO0-4)
    config.updateInterval = 1000;   // Actualizar cada segundo
    config.logLevel = NoiseSensor::LOG_INFO;
    sensor.setConfig(config);

    // Inicializar el sensor
    sensor.begin();
}

void loop() {
    // Actualizar el sensor (debe llamarse continuamente)
    sensor.update();
    delay(10);
}
```

### Configuración Avanzada

```cpp
NoiseSensorI2CSlave::Config config;
NoiseSensorI2CSlave sensor(config);

void setup() {
    // Configuración personalizada (aplicar antes de begin)
    config.i2cAddress = 0x08;
    config.sdaPin = 8;
    config.sclPin = 10;
    config.adcPin = 4;
    config.updateInterval = 500;
    config.logLevel = NoiseSensor::LOG_NONE;
    sensor.setConfig(config);

    sensor.begin();
}

// Acceder a los datos
if (sensor.isDataReady()) {
    const auto& data = sensor.getData();
    Serial.printf("Promedio: %.2f mV\n", data.noiseAvg);
}
```

## Configuración

### Parámetros de Configuración

| Parámetro | Tipo | Descripción | Valor por Defecto |
|-----------|------|-------------|-------------------|
| `i2cAddress` | `uint8_t` | Dirección I2C del esclavo | `0x08` |
| `sdaPin` | `uint8_t` | Pin SDA para I2C | `8` |
| `sclPin` | `uint8_t` | Pin SCL para I2C | `10` |
| `adcPin` | `uint8_t` | Pin ADC para el sensor | `4` |
| `updateInterval` | `unsigned long` | Intervalo de actualización en ms | `1000` |
| `logLevel` | `NoiseSensor::LogLevel` | Nivel de logging | `LOG_INFO` |

## Protocolo I2C

### Comandos Disponibles

El maestro puede enviar los siguientes comandos:

| Comando | Valor | Descripción |
|---------|-------|-------------|
| `CMD_GET_DATA` | 0x01 | Obtener todos los datos del sensor |
| `CMD_GET_AVG` | 0x02 | Obtener solo el promedio |
| `CMD_GET_PEAK` | 0x03 | Obtener solo el pico |
| `CMD_GET_MIN` | 0x04 | Obtener solo el mínimo |
| `CMD_GET_LEGAL` | 0x05 | Obtener promedio legal |
| `CMD_GET_LEGAL_MAX` | 0x06 | Obtener máximo legal |
| `CMD_GET_STATUS` | 0x07 | Obtener estado (0x01 = datos listos) |
| `CMD_RESET` | 0x08 | Resetear ciclo del sensor |
| `CMD_PING` / `CMD_IDENTIFY` | 0x09 | Identificación del sensor (detecta tipo y versión) |
| `CMD_GET_READY` | 0x0A | Verificar si está listo para enviar datos (0x01 = listo, 0x00 = no listo) |

### Estructura de Datos

Cuando se solicita `CMD_GET_DATA`, se envía la siguiente estructura:

```cpp
struct SensorData {
    float noise;             // Valor actual de ruido
    float noiseAvg;          // Promedio de ruido
    float noisePeak;         // Valor pico
    float noiseMin;          // Valor mínimo
    float noiseAvgLegal;     // Promedio legal
    float noiseAvgLegalMax;  // Máximo promedio legal
    uint16_t lowNoiseLevel;  // Nivel base
    uint32_t cycles;         // Contador de ciclos
};
```

**Tamaño total**: 32 bytes

## Uso desde el Maestro (ESP32)

### Ejemplo Básico

```cpp
#include <Wire.h>

#define I2C_SLAVE_ADDRESS 0x08

struct SensorData {
    float noise;
    float noiseAvg;
    float noisePeak;
    float noiseMin;
    float noiseAvgLegal;
    float noiseAvgLegalMax;
    uint16_t lowNoiseLevel;
    uint32_t cycles;
} sensorData;

void setup() {
    Serial.begin(115200);
    Wire.setBufferSize(64);
    Wire.begin(); // Maestro I2C (o Wire.begin(SDA, SCL) si usas pines personalizados)
    delay(1000);
}

void loop() {
    // Solicitar todos los datos
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(0x01); // CMD_GET_DATA
    Wire.endTransmission();   // STOP obligatorio (más estable para ESP32-C3)
    delayMicroseconds(200);
    
    // Leer datos
    Wire.requestFrom(I2C_SLAVE_ADDRESS, sizeof(SensorData));
    if (Wire.available() >= sizeof(SensorData)) {
        Wire.readBytes((uint8_t*)&sensorData, sizeof(SensorData));
        
        Serial.println("=== Datos del Sensor ===");
        Serial.printf("Promedio: %.2f mV\n", sensorData.noiseAvg);
        Serial.printf("Pico: %.2f mV\n", sensorData.noisePeak);
        Serial.printf("Mínimo: %.2f mV\n", sensorData.noiseMin);
        Serial.printf("Promedio Legal: %.2f mV\n", sensorData.noiseAvgLegal);
        Serial.printf("Máximo Legal: %.2f mV\n", sensorData.noiseAvgLegalMax);
        Serial.println();
    }
    
    delay(5000); // Solicitar datos cada 5 segundos
}
```

### Solicitar un Valor Específico

```cpp
// Solicitar solo el promedio
Wire.beginTransmission(I2C_SLAVE_ADDRESS);
Wire.write(0x02); // CMD_GET_AVG
Wire.endTransmission();   // STOP obligatorio (más estable para ESP32-C3)
delayMicroseconds(200);

Wire.requestFrom(I2C_SLAVE_ADDRESS, sizeof(float));
if (Wire.available() >= sizeof(float)) {
    float avg;
    Wire.readBytes((uint8_t*)&avg, sizeof(float));
    Serial.printf("Promedio: %.2f mV\n", avg);
}
```

## API de la Librería

### Métodos Principales

#### `begin()`
Inicializa el esclavo I2C y el sensor de ruido. Debe llamarse una vez en `setup()`.

#### `update()`
Actualiza el sensor y los datos I2C. Debe llamarse continuamente en `loop()`.

#### `getData()`
Obtiene una referencia constante a la estructura `SensorData` con los datos actuales.

```cpp
const auto& data = sensor.getData();
float promedio = data.noiseAvg;
```

#### `isDataReady()`
Verifica si hay datos disponibles.

```cpp
if (sensor.isDataReady()) {
    // Los datos están listos
}
```

#### `getNoiseSensor()`
Obtiene una referencia al objeto `NoiseSensor` interno para configuración avanzada.

#### `isValid()`
Valida la configuración actual del sensor.

```cpp
if (sensor.isValid()) {
    // La configuración es válida
    sensor.begin();
} else {
    // La configuración tiene errores
    Serial.println("Configuración inválida");
}
```

#### `isInitialized()`
Verifica si el sensor está inicializado correctamente.

```cpp
sensor.begin();
if (sensor.isInitialized()) {
    // El sensor está listo para usar
    Serial.println("Sensor inicializado correctamente");
} else {
    // Hubo un error en la inicialización
    Serial.println("Error: Sensor no inicializado");
}
```

**Nota:** Si `begin()` falla por validación de parámetros, el sensor no se inicializará y `update()` no hará nada hasta que se corrija la configuración y se llame a `begin()` nuevamente.

## Ejemplos

El proyecto incluye varios ejemplos completos en el directorio `examples/`:

### 1. **basic_usage** - Uso Básico
Ejemplo simple que muestra cómo usar la librería con la configuración por defecto. Ideal para empezar.

**Características:**
- Configuración mínima
- Logs automáticos activados
- Actualización cada segundo
- Código comentado y fácil de entender

**Compilar:**
```bash
pio run -e basic_usage
```

### 2. **advanced_usage** - Uso Avanzado
Ejemplo que muestra configuraciones avanzadas y personalización.

**Características:**
- Actualización más frecuente (500ms)
- Logs desactivados (control manual)
- Presentación personalizada de datos
- Procesamiento adicional de datos (alertas)

**Compilar:**
```bash
pio run -e advanced_usage
```

### 3. **i2c_master_example** - Maestro I2C
Código completo para un ESP32 maestro que lee datos del sensor esclavo.

**Características:**
- Solicita todos los datos del sensor
- Solicita valores individuales
- Verifica estado del esclavo
- Manejo de errores
- Compatible con la estructura de datos del esclavo

**Compilar:**
```bash
pio run -e i2c_master_example
```

### 4. **multiple_sensors** - Múltiples Sensores
Ejemplo conceptual que muestra cómo configurar múltiples sensores con diferentes direcciones I2C.

**Características:**
- Configuración de múltiples sensores
- Diferentes direcciones I2C
- Ejemplo de uso en red de sensores

**Nota:** Requiere múltiples ESP32-C3, cada uno con su sensor.

**Compilar:**
```bash
pio run -e multiple_sensors
```

### 5. **sensor_detection** - Detección Automática de Sensores (Maestro)
Ejemplo completo para el maestro I2C que muestra cómo detectar y verificar sensores automáticamente.

**Características:**
- Escaneo automático de direcciones I2C (0x08 - 0x77)
- Detección de sensores usando `CMD_PING`
- Verificación de estado con `CMD_GET_READY`
- Lectura de datos de múltiples sensores
- Manejo de sensores no listos
- Reintentos automáticos si no se detectan sensores

**Compilar:**
```bash
pio run -e sensor_detection
```

### Ejecutar un Ejemplo Específico

Para compilar y subir un ejemplo específico:

```bash
# Compilar
pio run -e basic_usage

# Subir al dispositivo
pio run -e basic_usage -t upload

# Monitorear serial
pio device monitor
```

## Compilación y Carga

```bash
# Compilar
pio run

# Subir al ESP32-C3
pio run -t upload

# Monitorear serial
pio device monitor

# Compilar un ejemplo específico
pio run -e basic_usage
```

## Debug

La librería incluye salida Serial para debug. Puedes ver:
- Estado de inicialización
- Datos del sensor según el intervalo configurado
- Confirmación de ciclos completados

Para desactivar los logs:

```cpp
config.logLevel = NoiseSensor::LOG_NONE;
```

## Limitaciones

- **Instancia única:** Solo se puede crear una instancia de `NoiseSensorI2CSlave` por programa debido a las limitaciones de los callbacks I2C estáticos de Arduino Wire. Si necesitas múltiples sensores, cada uno debe estar en un ESP32 diferente con su propia dirección I2C.
- **Dirección I2C:** Debe estar entre 0x08 y 0x77 (rango válido para I2C)
- **Intervalo de actualización:** Debe ser >= 10 ms
- **ADC (ESP32-C3):** `adcPin` debe estar en GPIO 0–4 (ADC1)

## Notas de compatibilidad ESP32-C3 (I2C esclavo)

- El esclavo responde **siempre** desde `onRequest()` (aunque no haya datos listos devuelve 1 byte `0x00`) para evitar bloqueos del bus.
- Los callbacks I2C son **mínimos** y marcados con `IRAM_ATTR` (sin `Serial`, sin `delay`, sin cálculos pesados).
- El patrón más estable en el maestro es **comando → STOP → pequeña espera → `requestFrom()`**.
- Internamente, el esclavo se inicializa con `Wire.setBufferSize(64)` y `Wire.begin(slaveAddr, sda, scl, 100000)` (firma de Arduino-ESP32).

## ✅ Checklist de diagnóstico (si el I2C no funciona o el scanner no detecta el esclavo)

### Hardware (lo más común)

- **GND común**: maestro y esclavo deben compartir **GND**.
- **Pull-ups en SDA/SCL**: 4.7k–10k a **3.3V** (si tu placa ya trae pull-ups, no dupliques demasiadas).
- **Voltaje**: ambos a **3.3V** (evita 5V en I2C).
- **Cableado**:
  - SDA (maestro) ↔ SDA (esclavo)
  - SCL (maestro) ↔ SCL (esclavo)
- **Longitud de cables**: prueba primero con cables cortos.
- **Dirección I2C**: confirma que el esclavo está en `0x08` (o la que hayas configurado).

### Configuración de pines

- **Pines correctos en cada placa**:
  - En el **maestro**, usa `Wire.begin(SDA, SCL)` si no son los pines por defecto.
  - En el **esclavo (ESP32‑C3)**, confirma que `config.sdaPin`/`config.sclPin` son los que realmente estás cableando.
- **ESP32‑C3 ADC**: `adcPin` debe ser **GPIO0–GPIO4** (ADC1). Si no, `begin()` fallará por validación.

### Patrón de comunicación (crítico en ESP32‑C3 slave)

- **Siempre**:
  - `Wire.beginTransmission(addr);`
  - `Wire.write(COMANDO);`
  - `Wire.endTransmission();`  **(STOP obligatorio)**
  - `delayMicroseconds(200);`
  - `Wire.requestFrom(addr, N);`
- Evita “repeated start” implícitos/extraños: si notas fallos intermitentes, aumenta la espera a **500–1000 µs**.

### Buffer / tamaño de respuesta

- Asegura `Wire.setBufferSize(64)` en el **maestro** y en el **esclavo**.
- No pidas más bytes de los que esperas y verifica `bytesReceived` antes de `readBytes()`.

### Señales de que el esclavo no está respondiendo

- Si el scanner no ve nada:
  - Revisa primero **pull-ups** y **GND común**.
  - Verifica que el esclavo está realmente en modo esclavo (`Wire.begin(slaveAddr, sda, scl, freq)`).
- Si el bus “se cuelga”:
  - En ESP32‑C3, `onRequest()` debe escribir **siempre** al menos 1 byte.
  - Evita `Serial`/`delay` dentro de callbacks I2C.

### Prueba mínima recomendada

- Ejecuta `examples/sensor_detection` en el maestro: escanea, detecta por `CMD_PING` y valida `CMD_GET_READY`.

## Notas

- Los datos se actualizan automáticamente cada segundo (o según el intervalo configurado)
- El sensor mantiene sus propios ciclos internos (120 segundos por defecto)
- Los datos enviados por I2C son siempre los más recientes disponibles
- El maestro puede solicitar datos en cualquier momento
- La validación de parámetros se realiza en `begin()` y se registran errores si el nivel de log lo permite

## Contribuir

Las contribuciones son bienvenidas! Por favor:

1. Haz un fork del repositorio
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## Licencia

Este proyecto está licenciado bajo la Licencia Pública General de GNU v3.0 o posterior - ver el archivo [LICENSE](LICENSE) para más detalles.

GPL-3.0-or-later

