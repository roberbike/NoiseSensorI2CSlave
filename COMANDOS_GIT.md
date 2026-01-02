# Comandos Git para Publicar en GitHub

## Inicializar el Repositorio

```bash
# 1. Inicializar git (si no está inicializado)
git init

# 2. Agregar todos los archivos
git add .

# 3. Hacer el primer commit
git commit -m "Initial commit: NoiseSensorI2CSlave library v1.0.0"
```

## Conectar con GitHub

```bash
# 4. Agregar el repositorio remoto
# Reemplaza TU_USUARIO con tu usuario de GitHub
git remote add origin https://github.com/TU_USUARIO/NoiseSensorI2CSlave.git

# 5. Cambiar a la rama main (si estás en master)
git branch -M main

# 6. Subir el código
git push -u origin main
```

## Comandos Útiles

```bash
# Ver el estado
git status

# Ver los cambios
git diff

# Agregar archivos específicos
git add archivo.cpp

# Hacer commit
git commit -m "Mensaje descriptivo"

# Ver el historial
git log --oneline

# Actualizar desde GitHub
git pull origin main

# Subir cambios
git push origin main

# Ver remotos configurados
git remote -v
```

## Crear un Tag para Release

```bash
# Crear un tag
git tag -a v1.0.0 -m "Release version 1.0.0"

# Subir el tag
git push origin v1.0.0
```

