# Changelog - Rama devel

## Mejoras Implementadas

### 1. ✅ Documentación de Limitación de Instancia Única (ALTA)
- Agregada documentación en el header de la clase sobre la limitación
- Agregada sección "Limitaciones" en el README.md
- Explicación clara de por qué solo se puede crear una instancia

### 2. ✅ Validación de Parámetros en begin() (MEDIA)
- Implementada validación de dirección I2C (0x08 - 0x77)
- Implementada validación de intervalo de actualización (>= 10ms)
- Mensajes de error informativos según el nivel de log
- Método `isValid()` agregado para validar configuración

### 3. ✅ Mejora de Manejo de Errores en I2C (MEDIA)
- Verificación de disponibilidad de datos antes de leer
- Verificación de bytes escritos en todas las operaciones I2C
- Logging de errores y advertencias según el nivel de log configurado
- Manejo mejorado de comandos desconocidos
- Validación de reset de ciclo antes de ejecutarlo

### 4. ✅ Constantes para Valores Mágicos (BONUS)
- Agregadas constantes en el header:
  - `DEFAULT_I2C_ADDRESS = 0x08`
  - `MIN_I2C_ADDRESS = 0x08`
  - `MAX_I2C_ADDRESS = 0x77`
  - `MIN_UPDATE_INTERVAL = 10` (ms)
  - `DEFAULT_UPDATE_INTERVAL = 1000` (ms)
- Uso de constantes en lugar de valores mágicos en el código
- Mejora la mantenibilidad y claridad del código

## Archivos Modificados

1. `lib/NoiseSensorI2CSlave/NoiseSensorI2CSlave.h`
   - Agregadas constantes para valores de configuración
   - Agregado método `isValid()`
   - Mejorada documentación del constructor

2. `lib/NoiseSensorI2CSlave/NoiseSensorI2CSlave.cpp`
   - Mejorada validación en `begin()` usando `isValid()`
   - Mejorado manejo de errores en `onRequest()`
   - Mejorado manejo de errores en `onReceive()`
   - Agregado logging de errores y advertencias

3. `README.md`
   - Agregada sección "Limitaciones"
   - Documentado método `isValid()`
   - Mejorada documentación de validación

## Correcciones Aplicadas

### Fix: LOG_WARNING no existe en NoiseSensor
- **Problema:** El código usaba `NoiseSensor::LOG_WARNING` que no existe
- **Solución:** Reemplazado por `LOG_INFO` para advertencias no críticas
- **Commit:** `88a7b96 - fix: Reemplazar LOG_WARNING por LOG_INFO`
- **Estado:** ✅ Compila correctamente

## Pruebas Recomendadas

1. **Validación de parámetros:**
   - Probar con dirección I2C inválida (< 0x08 o > 0x77)
   - Probar con intervalo de actualización = 0
   - Verificar que se muestran mensajes de error apropiados

2. **Manejo de errores I2C:**
   - Probar con maestro I2C desconectado
   - Probar con comandos inválidos
   - Verificar logging de errores

3. **Funcionalidad normal:**
   - Verificar que todo funciona correctamente con configuración válida
   - Probar todos los comandos I2C
   - Verificar que los datos se envían correctamente

## Estado de Compilación

✅ **Compilación exitosa** - Todos los cambios compilan correctamente

## Próximos Pasos

1. Probar todas las mejoras en hardware real
2. Verificar que el CI/CD sigue pasando
3. Si todo está bien, hacer merge a `main`

