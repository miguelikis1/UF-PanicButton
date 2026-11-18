# Botón del Pánico para Windows 11 (C++20 / Win32 Puro)

[English](README.md) | **Español**

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#compilación)
[![Plataforma](https://img.shields.io/badge/plataforma-Windows%2011%20%7C%2010-0078D6.svg?logo=windows)](https://microsoft.com)
[![Lenguaje](https://img.shields.io/badge/lenguaje-C%2B%2B20-00599C.svg?logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![Licencia](https://img.shields.io/badge/licencia-MIT-blue.svg)](LICENSE)
[![Uso de CPU](https://img.shields.io/badge/CPU%20en%20reposo-0.00%25-success.svg)](#arquitectura-y-rendimiento)
[![Dependencias](https://img.shields.io/badge/dependencias-ninguna%20(Win32%20puro)-orange.svg)](#arquitectura-y-rendimiento)

**Botón del Pánico** es una herramienta de ocultación de escritorio ultrarrápida y de cero sobrecarga para Windows 11, programada en **C++20** moderno sobre la **API Win32 pura**.

Con una sola tecla de acceso rápido o botón lateral del ratón:
1. **Oculta al instante (< 10 ms)** todas las ventanas abiertas de usuario en todos los monitores, la barra de tareas, `Alt+Tab` y la Vista de Tareas, sin finalizar procesos ni perder trabajo en curso.
2. **Pausa multimedia y silencia volumen inteligentemente**: detecta el pico real de audio mediante WASAPI (`IAudioMeterInformation`) y conmuta `VK_MEDIA_PLAY_PAUSE` solo si había sonido activo.
3. **Muestra de inmediato una estratagema de camuflaje** (informe de trabajo, terminal simulado, hoja de cálculo, imagen, app externa, web o ventana capturada).
4. **Restaura todo con fidelidad absoluta** al pulsar el mismo atajo, reconstruyendo el orden Z exacto, estados maximizados/minimizados y foco de la ventana activa original.

---

## Demostración

<!-- Reemplazar con GIF animado o capturas de pantalla -->
```
+---------------------------------------------------------------------------------+
|  [ Escritorio Habitual: Código, Vídeo, Chat ]  ===>  ATAJO / DISPARADOR RATÓN   |
|                                                                                 |
|  ==> 1. Todas las ventanas de usuario ocultadas de forma asíncrona en < 10 ms   |
|  ==> 2. Audio silenciado y reproducción multimedia pausada (WASAPI)             |
|  ==> 3. Ventana de camuflaje (ej. Informe de Trabajo) mostrada en 0 ms          |
|                                                                                 |
|  ===> PULSAR DE NUEVO ===> Reconstrucción exacta de orden Z y foco              |
+---------------------------------------------------------------------------------+
```

---

## Características Técnicas del Motor

### 1. Filtrado Estricto en Windows 11
- **Detección de Cloaked y Escritorios Virtuales**: Consulta `DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, ...)` para ignorar ventanas ocultas por el sistema o situadas en otros escritorios virtuales.
- **Exclusión de Ventanas del Sistema**: Descarta automáticamente `Shell_TrayWnd`, `Progman`, `WorkerW`, `Windows.UI.Core.CoreWindow`, y gestiona de forma segura los contenedores UWP (`ApplicationFrameWindow`).
- **Ocultación Asíncrona Paralela**: Ejecuta `ShowWindowAsync(hwnd, SW_HIDE)` para que ninguna aplicación bloqueada o lenta retrase la ocultación de las demás.

### 2. Reconstrucción Exacta del Orden Z
- Las ventanas se registran de arriba hacia abajo durante la enumeración.
- Al desactivar el modo pánico, se reconstruye la pila de ventanas atómicamente mediante `BeginDeferWindowPos`, `DeferWindowPos` y `EndDeferWindowPos` con `SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE` antes de devolver el foco a la ventana activa original.

### 3. Detección Inteligente Multimedia (WASAPI + Media Key)
- Comprueba el valor de pico de audio del hardware en tiempo real con `IAudioMeterInformation::GetPeakValue()`.
- Si había sonido reproduciéndose (`peak > 0.001f`), emite una pulsación virtual de `VK_MEDIA_PLAY_PAUSE` mediante `SendInput` y silencia el volumen maestro.
- Al restaurar, reanuda la música/vídeo **únicamente si fue pausada por la aplicación**.

### 4. Disparador por Ratón sin Hooks (Raw Input)
- Permite activar el pánico con los botones laterales del ratón (`XBUTTON1` o `XBUTTON2`).
- Registra `RegisterRawInputDevices` con el flag `RIDEV_INPUTSINK` sobre una ventana solo de mensajes (`HWND_MESSAGE`). No instala ganchos globales invasivos (`WH_MOUSE_LL`), garantizando **0,00% de CPU en reposo**.

### 5. Recuperación ante Fallos y Almacenamiento Limpio
- Guarda la configuración en `%LOCALAPPDATA%\PanicButton\config.ini`.
- Mantiene un archivo de recuperación atómico (`panic_state.dat`) mientras el pánico está activo. Si el proceso sufriera un cierre forzado o el equipo se apaga, ejecutar `PanicButton.exe --restore` recupera de inmediato todas las ventanas.

### 6. Soporte HiDPI y Estética Nativa Windows 11
- Manifiesto incrustado declarando compatibilidad `PerMonitorV2` y Controles Comunes v6.
- Escalado dinámico de tipografías y controles según `GetDpiForWindow`.
- Barra de título oscura inmersiva nativa mediante `DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, ...)`.

### 7. Internacionalización en Memoria (ES / EN)
- Catálogo de textos en memoria sin archivos externos.
- Detección automática del idioma del sistema con `GetUserDefaultUILanguage()` (español -> ES, resto -> EN).
- Selector manual de idioma en la pestaña de configuración.

---

## 8 Modos de Camuflaje (Estratagema)

| # | Estratagema | Descripción |
|---|---|---|
| **1** | **Informe / Notas de Trabajo** | Editor nativo de Windows 11 con plantillas (Informe Trimestral, Acta de Reunión, Plan de Proyecto). |
| **2** | **Terminal / Consola** | Consola oscura estilo PowerShell con simulación de actividad (C++/CMake, Docker, Auditoría). |
| **3** | **Hoja de Cálculo / Métricas** | Tabla de datos nativa con celdas seleccionables y barra de fórmulas (KPIs, Presupuesto). |
| **4** | **Captura de Pantalla / Imagen** | Muestra cualquier archivo `.png`, `.jpg` o `.bmp` a pantalla completa en `< 1 ms`. |
| **5** | **Aplicación o Archivo Externo** | Abre ejecutables (`.exe`) o documentos (`.docx`, `.xlsx`, `.pdf`) con opción de precalentamiento oculto. |
| **6** | **Página Web / URL** | Abre cualquier sitio web en modo aplicación (`--app`) en Edge/Chrome. |
| **7** | **Capturar Ventana Abierta** | Selecciona una ventana en ejecución (ej. Visual Studio, Excel, Outlook) para usarla como tapadera. |
| **8** | **Solo Escritorio Limpio** | Oculta absolutamente todas las aplicaciones dejando el escritorio despejado. |

---

## Opciones por Línea de Comandos

| Parámetro | Descripción |
|---|---|
| *(ninguno)* | Inicia en segundo plano con icono en la bandeja (o abre configuración si es primer uso). |
| `--minimized` | Inicia directamente minimizado en la bandeja del sistema (para inicio automático). |
| `--settings` | Abre directamente la ventana de configuración y opciones. |
| `--restore` | Comando de emergencia: restaura de inmediato todas las ventanas ocultas y finaliza. |

---

## Compilación

### Requisitos
- **MinGW-w64** con soporte para `C++20` (GCC 13+) o **MSVC 2022+**
- Cabeceras de la API de Windows 10/11

### Compilación Rápida (MinGW-w64)
```bat
build.bat
```

### Compilación con CMake
```cmd
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

---

## Licencia

Distribuido bajo los términos de la [Licencia MIT](LICENSE).
