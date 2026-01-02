# Guía para Publicar la Librería en GitHub

Esta guía te ayudará a publicar la librería NoiseSensorI2CSlave en GitHub.

## Pasos para Publicar

### 1. Crear el Repositorio en GitHub

1. Ve a [GitHub](https://github.com) e inicia sesión
2. Haz clic en el botón "+" en la esquina superior derecha
3. Selecciona "New repository"
4. Configura el repositorio:
   - **Name**: `NoiseSensorI2CSlave` (o el nombre que prefieras)
   - **Description**: "Librería para ESP32 que convierte un sensor de ruido en un dispositivo esclavo I2C"
   - **Visibility**: Público (recomendado para librerías) o Privado
   - **NO** marques "Initialize this repository with a README" (ya tenemos uno)
   - **NO** agregues .gitignore ni LICENSE (ya los tenemos)
5. Haz clic en "Create repository"

### 2. Inicializar Git en tu Proyecto Local

Abre una terminal en la carpeta del proyecto y ejecuta:

```bash
# Inicializar git (si no está inicializado)
git init

# Agregar todos los archivos
git add .

# Hacer el primer commit
git commit -m "Initial commit: NoiseSensorI2CSlave library v1.0.0"
```

### 3. Conectar con GitHub

```bash
# Agregar el repositorio remoto (reemplaza TU_USUARIO con tu usuario de GitHub)
git remote add origin https://github.com/TU_USUARIO/NoiseSensorI2CSlave.git

# Cambiar a la rama main (si estás en master)
git branch -M main

# Subir el código
git push -u origin main
```

### 4. Actualizar library.json

Antes de publicar, actualiza el archivo `lib/NoiseSensorI2CSlave/library.json` con tu información:

1. Cambia `"Tu Nombre"` por tu nombre real
2. Cambia `"tu@email.com"` por tu email
3. Actualiza la URL del repositorio con la URL real de tu repositorio

### 5. Crear un Release (Opcional pero Recomendado)

1. Ve a tu repositorio en GitHub
2. Haz clic en "Releases" → "Create a new release"
3. Configura:
   - **Tag version**: `v1.0.0`
   - **Release title**: `v1.0.0 - Initial Release`
   - **Description**: Copia las características principales del README
4. Publica el release

## Publicar en PlatformIO Library Registry (Opcional)

Si quieres que otros puedan instalar tu librería fácilmente con PlatformIO:

### Opción 1: Usar como Librería Local (Recomendado para desarrollo)

Los usuarios pueden agregar tu librería directamente desde GitHub:

```ini
lib_deps = 
    https://github.com/TU_USUARIO/NoiseSensorI2CSlave.git
```

### Opción 2: Publicar en PlatformIO Library Registry

1. Ve a [PlatformIO Library Registry](https://platformio.org/lib)
2. Crea una cuenta o inicia sesión
3. Sigue las instrucciones para publicar tu librería

**Requisitos:**
- El repositorio debe ser público
- Debe tener un `library.json` válido
- Debe tener al menos un release/tag
- El código debe compilar correctamente

## Estructura del Repositorio

Tu repositorio debería tener esta estructura:

```
NoiseSensorI2CSlave/
├── .github/
│   └── workflows/
│       └── ci.yml          # CI/CD para compilar ejemplos
├── examples/
│   ├── basic_usage/
│   ├── advanced_usage/
│   ├── i2c_master_example/
│   └── multiple_sensors/
├── lib/
│   └── NoiseSensorI2CSlave/
│       ├── library.json
│       ├── NoiseSensorI2CSlave.h
│       └── NoiseSensorI2CSlave.cpp
├── src/
│   └── main.cpp            # Ejemplo principal
├── .gitignore
├── LICENSE
├── platformio.ini
├── README.md
└── PUBLICAR_GITHUB.md      # Este archivo
```

## Verificación Final

Antes de publicar, verifica:

- [ ] El README está completo y actualizado
- [ ] El `library.json` tiene tu información correcta
- [ ] El LICENSE está presente
- [ ] El `.gitignore` está configurado
- [ ] Todos los ejemplos compilan correctamente
- [ ] El código está comentado y documentado

## Comandos Útiles

```bash
# Ver el estado de git
git status

# Ver los cambios
git diff

# Agregar archivos específicos
git add archivo.cpp

# Hacer commit
git commit -m "Mensaje descriptivo"

# Ver el historial
git log

# Actualizar desde GitHub
git pull origin main

# Subir cambios
git push origin main
```

## Siguientes Pasos

Después de publicar:

1. **Agrega badges al README** (opcional):
   ```markdown
   ![PlatformIO](https://img.shields.io/badge/platform-ESP32-blue)
   ![License](https://img.shields.io/badge/license-GPL--3.0-green)
   ```

2. **Crea issues templates** para que otros reporten bugs o soliciten features

3. **Agrega más ejemplos** si encuentras casos de uso interesantes

4. **Mantén el repositorio actualizado** con mejoras y correcciones

¡Buena suerte con tu librería! 🚀

