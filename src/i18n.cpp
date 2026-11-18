#include "i18n.h"
#include <vector>

namespace i18n {

namespace {

Language g_configuredLanguage = Language::Auto;

Language DetectSystemLanguage() {
    LANGID langId = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(langId);
    if (primary == LANG_SPANISH) {
        return Language::Spanish;
    }
    return Language::English;
}

struct TranslationEntry {
    const wchar_t* en;
    const wchar_t* es;
};

// Index matches StrId enum sequentially
const TranslationEntry kStrings[] = {
    // AppName, AppTagline, StatusReady, StatusPanicActive, StatusRamLabel
    { L"Panic Button", L"Botón del Pánico" },
    { L"Windows 11  •  Native", L"Windows 11  •  Nativo" },
    { L"Idle", L"En espera" },
    { L"PANIC MODE ACTIVE", L"MODO PÁNICO ACTIVO" },
    { L"0.0% CPU at idle", L"0,0% CPU en reposo" },

    // NavGeneral, NavCamouflage, NavEngine
    { L"General & Shortcuts", L"General y Atajos" },
    { L"Camouflage", L"Camuflaje" },
    { L"Exclusions & Engine", L"Exclusiones y Motor" },

    // SectionHotkeys, SectionHotkeysDesc
    { L"Global Shortcuts", L"Atajos Globales" },
    { L"Configure keyboard hotkeys and mouse triggers for instant activation.", L"Configura tus teclas rápidas y disparadores de ratón para activación instantánea." },

    // PanicHotkeyLabel, PanicHotkeyDesc
    { L"Panic Shortcut", L"Atajo de Pánico" },
    { L"Hides everything in milliseconds and restores when pressed again", L"Oculta todo al instante y restaura al pulsar de nuevo" },

    // SettingsHotkeyLabel, SettingsHotkeyDesc
    { L"Settings Shortcut", L"Atajo de Configuración" },
    { L"Opens this window even with the tray icon hidden (Stealth Mode)", L"Abre esta ventana incluso con el icono oculto (Modo Sigilo)" },

    // HotkeyClickToAssign, HotkeyPressCombo, PresetQuickLabel
    { L"Click to assign shortcut", L"Haz clic para asignar atajo" },
    { L"Press your key combo... (Esc cancels)", L"Pulsa tu combinación... (Esc cancela)" },
    { L"Presets:", L"Rápidos:" },

    // SectionMouseTrigger, MouseTriggerLabel, MouseTriggerDesc
    { L"Mouse Trigger", L"Disparador por Ratón" },
    { L"Mouse Side Buttons (Raw Input):", L"Botones laterales del ratón (Raw Input):" },
    { L"Captures side buttons globally without hooks (0% CPU at idle).", L"Captura los botones laterales globalmente sin hooks (0% CPU en reposo)." },
    { L"Disabled", L"Desactivado" },
    { L"Side Button Back (XButton 1)", L"Botón lateral atrás (XButton 1)" },
    { L"Side Button Forward (XButton 2)", L"Botón lateral adelante (XButton 2)" },
    { L"Any Side Button (XButton 1 or 2)", L"Cualquier botón lateral (XButton 1 o 2)" },

    // SectionBehavior
    { L"Behavior & Silence", L"Comportamiento y Silencio" },

    // TogMuteAudioTitle, TogMuteAudioDesc
    { L"Mute system master volume immediately", L"Silenciar audio del sistema al instante" },
    { L"Cuts audio via WASAPI on panic and restores volume on return", L"Corta el sonido vía WASAPI al activar y lo restaura al volver" },

    // TogPauseMediaTitle, TogPauseMediaDesc
    { L"Smart multimedia pause (WASAPI Peak + Media Key)", L"Pausa inteligente de multimedia (WASAPI + Media Key)" },
    { L"Checks if audio is playing; sends Pause key, then resumes on return", L"Comprueba si hay audio sonando; envía pausa y reanuda al volver" },

    // TogStealthTitle, TogStealthDesc
    { L"Stealth Mode (hide system tray icon)", L"Modo Sigilo (ocultar icono de la bandeja)" },
    { L"Hides the tray icon; reopen with shortcut or running .exe again", L"Oculta el icono del System Tray; reabre con el atajo o el .exe" },

    // TogStartWinTitle, TogStartWinDesc
    { L"Start automatically with Windows", L"Iniciar automáticamente con Windows" },
    { L"Launches silently in background when turning on PC", L"Arranca silenciosamente en segundo plano al encender el PC" },

    // TogStartTipTitle, TogStartTipDesc
    { L"Show brief startup notification", L"Mostrar aviso breve al arrancar" },
    { L"Shows the active shortcut in a balloon tip on boot (if not stealth)", L"Indica el atajo activo al iniciar si no usas Modo Sigilo" },

    // SectionPrivileges, AdminStatusActive, AdminStatusInactive, BtnRelaunchAdmin, BtnRelaunchAdminDone
    { L"Elevation & Permissions", L"Permisos y Privilegios" },
    { L"Running with Administrator privileges (elevated)", L"Privilegios elevados activos (Administrador)" },
    { L"Running as standard user", L"Ejecutando como usuario estándar" },
    { L"Restart as Administrator", L"Reiniciar como Admin" },
    { L"Administrator ✓", L"Administrador ✓" },

    // SectionLanguage, LanguageLabel, LanguageDesc, LangAuto, LangEnglish, LangSpanish
    { L"Language / Idioma", L"Idioma / Language" },
    { L"Interface Language:", L"Idioma de la interfaz:" },
    { L"Switch language without restarting the application.", L"Cambia el idioma sin necesidad de reiniciar la aplicación." },
    { L"Auto (System Language)", L"Automático (Windows)" },
    { L"English", L"English" },
    { L"Español", L"Español" },

    // ThemeToggleLight, ThemeToggleDark, BtnTestPanic, BtnRestoreAll, BtnHide, BtnExit, BtnSaveApply
    { L"☀  Switch to Light Theme", L"☀  Cambiar a Modo Claro" },
    { L"☾  Switch to Dark Theme", L"☾  Cambiar a Modo Oscuro" },
    { L"⚡  Test Panic", L"⚡  Probar Pánico" },
    { L"↩  Restore Everything", L"↩  Restaurar Todo" },
    { L"Hide", L"Ocultar" },
    { L"Exit", L"Salir" },
    { L"Save & Apply", L"Guardar y Aplicar" },

    // Toasts
    { L"Saved and applied  •  Shortcut: ", L"Guardado y aplicado  •  Atajo: " },
    { L"Captured shortcut: ", L"Atajo capturado: " },
    { L"Error applying shortcut", L"No se pudo aplicar el atajo" },
    { L"No windows currently hidden", L"No hay ventanas ocultas en este momento" },
    { L"Window list refreshed", L"Lista de ventanas actualizada" },
    { L"Window captured as camouflage ✓", L"Ventana capturada como camuflaje ✓" },
    { L"Window released", L"Ventana liberada" },
    { L"Metrics refreshed", L"Métricas actualizadas" },
    { L"Pre-warming camouflage in background RAM...", L"Preparando camuflaje oculto en memoria..." },
    { L"Template loaded", L"Plantilla cargada" },

    // Decoy selector titles & tags (8)
    { L"Notes", L"0 ms · Editable" },
    { L"Notas", L"0 ms · Editable" },
    { L"Terminal", L"0 ms · Live" },
    { L"Terminal", L"0 ms · En vivo" },
    { L"Spreadsheet", L"0 ms · Data" },
    { L"Hoja Cálculo", L"0 ms · Datos" },
    { L"Image", L"< 1 ms · RAM" },
    { L"Imagen", L"< 1 ms · RAM" },
    { L"App / File", L"Pre-warmable" },
    { L"App / Archivo", L"Pre-cargable" },
    { L"Web Page", L"New window" },
    { L"Página Web", L"Ventana aparte" },
    { L"Window", L"Capture open" },
    { L"Ventana", L"Capturar actual" },
    { L"Clean Desktop", L"Clear only" },
    { L"Escritorio", L"Solo limpiar" },

    // Decoy detail controls
    { L"Camouflage status: ", L"Estado del camuflaje: " },
    { L"Camouflage ready", L"Camuflaje listo" },
    { L"Window Title:", L"Título de la ventana:" },
    { L"Template:", L"Plantilla:" },
    { L"Load template", L"Cargar plantilla" },
    { L"Theme: Auto", L"Tema: Auto" },
    { L"Theme: Light", L"Tema: Claro" },
    { L"Theme: Dark", L"Tema: Oscuro" },
    { L"Document content (editable live during panic):", L"Contenido del documento (editable en directo durante el pánico):" },
    { L"Console Window Title:", L"Título de la consola:" },
    { L"Scenario:", L"Escenario:" },
    { L"Load scenario", L"Cargar escenario" },
    { L"Simulate live activity", L"Actividad en vivo" },
    { L"Initial console output:", L"Salida inicial de la consola:" },
    { L"Spreadsheet Window Title:", L"Título de la hoja de cálculo:" },
    { L"Data Template:", L"Plantilla de datos:" },
    { L"Native high-performance spreadsheet (< 2 ms)", L"Hoja de cálculo nativa ultrarrápida (< 2 ms)" },
    { L"Includes Excel-style green ribbon, interactive formula bar, column headers, and real cell selection on click.",
      L"Incluye barra verde estilo Excel, barra de fórmulas interactiva, encabezados de columna y selección real de celdas al hacer clic." },
    { L"Image or screenshot path (.png, .jpg, .bmp):", L"Ruta de imagen o captura (.png, .jpg, .bmp):" },
    { L"Browse...", L"Examinar..." },
    { L"Borderless fullscreen", L"Pantalla completa sin bordes" },
    { L"Covers the entire monitor including taskbar for maximum realism", L"Cubre toda la pantalla incluyendo la barra de tareas para máximo realismo" },
    { L"Pro Tip: Take a screenshot of your Excel, IDE, or email and pick it here.", L"Consejo: Haz una captura de tu Excel, correo o editor de código y selecciónala aquí." },
    { L"It decodes once into RAM to pop up in less than 1 millisecond.", L"Se decodifica una sola vez en RAM para aparecer en menos de 1 milisegundo." },
    { L"Application or document path (.exe, .docx, .xlsx, .pdf...):", L"Ruta del programa o documento (.exe, .docx, .xlsx, .pdf...):" },
    { L"Browse...", L"Examinar..." },
    { L"Optional arguments:", L"Argumentos opcionales:" },
    { L"Show maximized", L"Mostrar maximizada" },
    { L"Pre-warm hidden (< 10 ms)", L"Pre-cargar oculta (< 10 ms)" },
    { L"When leaving panic mode:", L"Al desactivar el pánico:" },
    { L"Keep hidden in RAM", L"Ocultar y mantener en RAM" },
    { L"Close upon return", L"Cerrar al volver" },
    { L"Leave open", L"Dejar abierta" },
    { L"Pre-warm now", L"Preparar ahora" },
    { L"Web Address (URL):", L"Dirección web (URL):" },
    { L"Browser launch mode:", L"Modo de apertura del navegador:" },
    { L"Isolated window (Edge/Chrome --new-window)", L"Ventana independiente (Edge/Chrome --new-window)" },
    { L"Web App Mode without tabs (--app)", L"Modo Aplicación web sin barra de pestañas (--app)" },
    { L"Default Windows Browser", L"Navegador predeterminado de Windows" },
    { L"Pre-warm web page hidden in background (< 10 ms)", L"Pre-cargar página oculta en segundo plano (< 10 ms)" },
    { L"Pre-warm page", L"Preparar página" },
    { L"Select an already open window on your PC:", L"Selecciona una ventana ya abierta en tu PC:" },
    { L"Refresh", L"Actualizar" },
    { L"Keep visible normally (only hide other windows on panic)", L"Dejarla visible normalmente (solo ocultar las demás al pulsar el atajo)" },
    { L"Capture selected window", L"Capturar ventana seleccionada" },
    { L"Release / Show", L"Liberar / Mostrar" },
    { L"Current captured window:", L"Ventana capturada actual:" },
    { L"(None)", L"(Ninguna)" },
    { L"Clean Desktop Mode", L"Modo Escritorio Limpio" },
    { L"All windows, tabs, and taskbar entries will vanish in milliseconds, leaving a clean desktop.\nPressing the shortcut again restores everything to its exact state.",
      L"Todas las ventanas, pestañas y accesos en la barra de tareas desaparecerán en milisegundos, dejando tu escritorio completamente despejado.\nAl pulsar el mismo atajo de nuevo, todo volverá a su estado exacto." },

    // Engine & Exclusions
    { L"Excluded processes (optional)", L"Procesos excluidos (opcional)" },
    { L"These programs will never be hidden (separated by commas or lines, e.g. spotify.exe, obs64.exe):",
      L"Estos programas nunca se ocultarán (separados por comas o líneas, ej: spotify.exe, obs64.exe):" },
    { L"Real-time native engine telemetry", L"Métricas del motor nativo en tiempo real" },
    { L"RAM Working Set", L"Memoria RAM" },
    { L"0.0% CPU at idle", L"0,0% CPU en reposo" },
    { L"Last Hide Latency", L"Última ocultación" },
    { L" windows hidden", L" ventanas ocultas" },
    { L"Last Restore Latency", L"Última restauración" },
    { L" windows restored", L" ventanas devueltas" },
    { L"Architecture:", L"Arquitectura:" },
    { L"C++20 / Native Win32 API (Zero Electron, Zero .NET)", L"C++20 / Win32 API Nativo (sin Electron ni .NET)" },
    { L"Camouflage:", L"Camuflaje:" },
    { L"Configuration:", L"Configuración:" },
    { L"State File:", L"Recuperación:" },
    { L"Refresh Metrics", L"Actualizar métricas" },
    { L"Restore Hidden Windows", L"Restaurar ventanas ocultas" },

    // Tray Menu & Tooltips
    { L"Activate Panic", L"Activar Pánico" },
    { L"Restore Everything", L"Restaurar todo" },
    { L"Pre-warm Camouflage in RAM", L"Preparar camuflaje en memoria" },
    { L"Settings...", L"Configuración..." },
    { L"Exit", L"Salir" },
    { L"Panic Button — Shortcut: ", L"Botón del Pánico — Atajo: " },
    { L"Panic Mode Active — Press: ", L"En espera (" },
    { L"Panic Button", L"Botón del Pánico" },
    { L"Active in background. Shortcut: ", L"Activo en segundo plano. Atajo: " },
    { L"Restored orphaned windows from previous session: ", L"Se restauraron ventanas de una sesión anterior: " }
};

} // namespace

void SetConfiguredLanguage(Language lang) {
    g_configuredLanguage = lang;
}

Language GetConfiguredLanguage() {
    return g_configuredLanguage;
}

Language GetEffectiveLanguage() {
    if (g_configuredLanguage == Language::Spanish) return Language::Spanish;
    if (g_configuredLanguage == Language::English) return Language::English;
    return DetectSystemLanguage();
}

bool IsSpanish() {
    return GetEffectiveLanguage() == Language::Spanish;
}

const wchar_t* Get(StrId id) {
    size_t idx = static_cast<size_t>(id);
    if (idx >= sizeof(kStrings) / sizeof(kStrings[0])) {
        return L"";
    }
    return IsSpanish() ? kStrings[idx].es : kStrings[idx].en;
}

} // namespace i18n
