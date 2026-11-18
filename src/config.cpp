#include "config.h"
#include "i18n.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

AppConfig::AppConfig() {
    bool sp = i18n::IsSpanish();
    notesContent = GetDefaultNotesTemplate(0, sp);
    terminalContent = GetDefaultTerminalTemplate(0, sp);
}

std::wstring AppConfig::GetConfigPath() {
    return utils::GetAppDataDir() + L"\\config.ini";
}

std::wstring AppConfig::GetDefaultNotesTemplate(int idx, bool spanish) {
    if (spanish) {
        if (idx == 1) {
            return
                L"ACTA DE REUNIÓN SEMANAL - EQUIPO DE OPERACIONES\r\n"
                L"=================================================\r\n"
                L"Fecha: Miércoles, 11:00 - 11:45\r\n"
                L"Asistentes: Dirección Técnica, Producto, QA, Operaciones\r\n\r\n"
                L"1. REVISIÓN DE HITOS DEL SPRINT ACTUAL\r\n"
                L"   - Optimización de latencia en el servicio principal: completado al 92%.\r\n"
                L"   - Migración de reportes automáticos a formato consolidado: en revisión.\r\n"
                L"   - Corrección de incidencias prioritarias reportadas por clientes: cerradas 11/12.\r\n\r\n"
                L"2. PRÓXIMOS PASOS Y RESPONSABLES\r\n"
                L"   - Validar métricas de rendimiento en entorno de preproducción antes del viernes.\r\n"
                L"   - Preparar documentación de entrega y notas de versión para la próxima semana.\r\n"
                L"   - Revisar presupuesto de infraestructura para el siguiente trimestre.\r\n\r\n"
                L"3. NOTAS ADICIONALES\r\n"
                L"   - Pendiente confirmar disponibilidad para la demostración ejecutiva del lunes.\r\n";
        }
        if (idx == 2) {
            return
                L"PLANIFICACIÓN DE PROYECTO - FASE 2\r\n"
                L"==================================\r\n\r\n"
                L"[Objetivos Clave]\r\n"
                L"• Reducir el tiempo medio de respuesta por debajo de 120 ms.\r\n"
                L"• Automatizar el flujo de verificación y pruebas de integración.\r\n"
                L"• Estandarizar los paneles de seguimiento mensual.\r\n\r\n"
                L"[Cronograma Tentativo]\r\n"
                L"Semana 1: Auditoría de arquitectura actual y cuello de botella.\r\n"
                L"Semana 2: Implementación de mejoras de caché y refactorización.\r\n"
                L"Semana 3: Pruebas de carga, validación de seguridad y ajustes.\r\n"
                L"Semana 4: Despliegue gradual y monitorización en producción.\r\n\r\n"
                L"[Riesgos Identificados]\r\n"
                L"- Dependencia de actualización de librerías externas (mitigación en curso).\r\n"
                L"- Ventana de mantenimiento programada para el último sábado del mes.\r\n";
        }
        return
            L"INFORME TRIMESTRAL DE RENDIMIENTO Y OPERACIONES (Q4)\r\n"
            L"====================================================\r\n\r\n"
            L"1. RESUMEN EJECUTIVO\r\n"
            L"Durante el último trimestre se ha consolidado la mejora operativa en todas las áreas\r\n"
            L"clave, alcanzando un cumplimiento del 96,4% respecto a los objetivos anuales.\r\n"
            L"La eficiencia en tiempos de entrega mejoró un 14,2% gracias a la automatización\r\n"
            L"de procesos internos y a la reducción de tareas manuales repetitivas.\r\n\r\n"
            L"2. INDICADORES PRINCIPALES (KPIs)\r\n"
            L"   • Disponibilidad del sistema:        99,94% (Objetivo: 99,90%)\r\n"
            L"   • Tiempo medio de resolución:        2h 18m (-19% vs trimestre anterior)\r\n"
            L"   • Tasa de satisfacción interna:      4,7 / 5,0\r\n"
            L"   • Desviación presupuestaria:         -2,1% (por debajo del límite previsto)\r\n\r\n"
            L"3. CONCLUSIONES Y RECOMENDACIONES\r\n"
            L"   - Mantener el ritmo de auditorías preventivas quincenales.\r\n"
            L"   - Priorizar la documentación técnica de los nuevos módulos desplegados.\r\n"
            L"   - Revisar la planificación de capacidad antes del inicio del próximo ejercicio.\r\n\r\n"
            L"Borrador en revisión — pendiente de validación final.\r\n";
    }

    // English templates
    if (idx == 1) {
        return
            L"WEEKLY OPERATIONS TEAM MEETING MINUTES\r\n"
            L"======================================\r\n"
            L"Date: Wednesday, 11:00 AM - 11:45 AM\r\n"
            L"Attendees: Technical Director, Product Lead, QA, Operations\r\n\r\n"
            L"1. SPRINT MILESTONE REVIEW\r\n"
            L"   - Latency optimization in primary pipeline: 92% completed.\r\n"
            L"   - Consolidated reporting daemon migration: under QA review.\r\n"
            L"   - Resolution of critical customer-reported issues: 11 of 12 closed.\r\n\r\n"
            L"2. NEXT STEPS & ACTION ITEMS\r\n"
            L"   - Benchmark staging latency metrics prior to Friday cutoff.\r\n"
            L"   - Complete release documentation and change log for next sprint.\r\n"
            L"   - Review infrastructure allocation budget for next quarter.\r\n\r\n"
            L"3. ADDITIONAL NOTES\r\n"
            L"   - Executive demo scheduled for Monday morning.\r\n";
    }
    if (idx == 2) {
        return
            L"PROJECT ROADMAP & ARCHITECTURE SPRINT PLAN (PHASE 2)\r\n"
            L"===================================================\r\n\r\n"
            L"[Key Objectives]\r\n"
            L"- Reduce P95 round-trip latency below 120 ms.\r\n"
            L"- Automate end-to-end integration verification pipeline.\r\n"
            L"- Unify telemetry dashboards across all services.\r\n\r\n"
            L"[Tentative Timeline]\r\n"
            L"Week 1: Core bottleneck profiling and baseline analysis.\r\n"
            L"Week 2: Distributed memory cache refactoring and index tuning.\r\n"
            L"Week 3: Stress testing, concurrency audit, and security validation.\r\n"
            L"Week 4: Phased production canary deployment and telemetry tracking.\r\n\r\n"
            L"[Identified Risks]\r\n"
            L"- Third-party library upgrade dependencies (mitigation in progress).\r\n"
            L"- Scheduled maintenance window set for last Saturday of the month.\r\n";
    }
    return
        L"QUARTERLY OPERATIONS & PERFORMANCE REPORT (Q4)\r\n"
        L"==============================================\r\n\r\n"
        L"1. EXECUTIVE SUMMARY\r\n"
        L"Over the past quarter, operational resilience across all core services achieved a\r\n"
        L"96.4% target fulfillment rate against our annual enterprise goals.\r\n"
        L"Turnaround time improved by 14.2% through streamlined background pipelines\r\n"
        L"and automated workload scheduling.\r\n\r\n"
        L"2. KEY PERFORMANCE INDICATORS (KPIs)\r\n"
        L"   • Service Availability SLA:          99.94% (Target: 99.90%)\r\n"
        L"   • Mean Time to Resolution:           2h 18m (-19% vs previous quarter)\r\n"
        L"   • Internal Satisfaction Score:       4.7 / 5.0\r\n"
        L"   • Infrastructure Variance:           -2.1% (within budget threshold)\r\n\r\n"
        L"3. CONCLUSIONS AND RECOMMENDATIONS\r\n"
        L"   - Maintain bi-weekly automated preventative audits.\r\n"
        L"   - Finalize technical documentation for the newly deployed modules.\r\n"
        L"   - Review capacity scaling forecast ahead of Q1.\r\n\r\n"
        L"Draft under review — awaiting final approval.\r\n";
}

std::wstring AppConfig::GetDefaultTerminalTemplate(int idx, bool spanish) {
    if (spanish) {
        if (idx == 1) {
            return
                L"Windows PowerShell\r\n"
                L"Copyright (C) Microsoft Corporation. Todos los derechos reservados.\r\n\r\n"
                L"PS C:\\Proyectos\\infra-servicios> docker compose -f compose.prod.yml build --pull\r\n"
                L"[+] Building 42.6s (14/14) FINISHED\r\n"
                L" => [api internal] load build definition from Dockerfile                0.1s\r\n"
                L" => [api 1/6] FROM docker.io/library/node:20-alpine                     1.4s\r\n"
                L" => [api 2/6] WORKDIR /app                                             0.0s\r\n"
                L" => [api 3/6] COPY package*.json ./                                    0.1s\r\n"
                L" => [api 4/6] RUN npm ci --omit=dev                                   18.3s\r\n"
                L" => [api 5/6] COPY dist/ ./dist/                                       0.4s\r\n"
                L" => [api] exporting to image                                           2.1s\r\n"
                L" => => naming to registry.internal/core-api:2.14.3                     0.0s\r\n"
                L"PS C:\\Proyectos\\infra-servicios> kubectl rollout status deployment/core-api -n prod\r\n"
                L"Waiting for deployment \"core-api\" rollout to finish: 2 of 4 updated replicas are available...\r\n";
        }
        if (idx == 2) {
            return
                L"Windows PowerShell\r\n"
                L"Copyright (C) Microsoft Corporation. Todos los derechos reservados.\r\n\r\n"
                L"PS C:\\Administracion\\Diagnostico> .\\audit-sistema.ps1 -DeepScan -VerifyChecksums\r\n"
                L"[INFO]  Iniciando verificación de integridad de servicios locales...\r\n"
                L"[OK]    Servicio de red y DNS operativo (latencia media: 4.2 ms)\r\n"
                L"[OK]    Volumen C:\\ comprobado - 0 sectores reasignados\r\n"
                L"[INFO]  Analizando registros de eventos de aplicación (últimas 24h)...\r\n"
                L"[OK]    Eventos críticos: 0 | Advertencias: 2 (no bloqueantes)\r\n"
                L"[INFO]  Verificando firmas SHA-256 del repositorio de despliegue...\r\n";
        }
        return
            L"Windows PowerShell\r\n"
            L"Copyright (C) Microsoft Corporation. Todos los derechos reservados.\r\n\r\n"
            L"PS C:\\Proyectos\\core-engine> cmake --build build/release --config Release -j 8\r\n"
            L"[ 12%] Building CXX object src/CMakeFiles/core.dir/scheduler.cpp.obj\r\n"
            L"[ 24%] Building CXX object src/CMakeFiles/core.dir/memory_pool.cpp.obj\r\n"
            L"[ 38%] Building CXX object src/CMakeFiles/core.dir/storage_index.cpp.obj\r\n"
            L"[ 51%] Building CXX object src/CMakeFiles/core.dir/net_transport.cpp.obj\r\n"
            L"[ 64%] Building CXX object src/CMakeFiles/core.dir/metrics_collector.cpp.obj\r\n"
            L"[ 78%] Linking CXX static library libcore.a\r\n"
            L"[ 85%] Built target core\r\n"
            L"[ 92%] Building CXX object tests/CMakeFiles/unit_tests.dir/test_pipeline.cpp.obj\r\n"
            L"[100%] Linking CXX executable unit_tests.exe\r\n"
            L"PS C:\\Proyectos\\core-engine> ctest --test-dir build/release --output-on-failure\r\n"
            L"Test project C:/Proyectos/core-engine/build/release\r\n"
            L"    Start 1: core_scheduler_latency\r\n"
            L"1/6 Test #1: core_scheduler_latency ...........   Passed    0.18 sec\r\n"
            L"    Start 2: storage_index_consistency\r\n"
            L"2/6 Test #2: storage_index_consistency ........   Passed    0.42 sec\r\n";
    }

    // English terminal templates
    if (idx == 1) {
        return
            L"Windows PowerShell\r\n"
            L"Copyright (C) Microsoft Corporation. All rights reserved.\r\n\r\n"
            L"PS C:\\Work\\infra-services> docker compose -f compose.prod.yml build --pull\r\n"
            L"[+] Building 42.6s (14/14) FINISHED\r\n"
            L" => [api internal] load build definition from Dockerfile                0.1s\r\n"
            L" => [api 1/6] FROM docker.io/library/node:20-alpine                     1.4s\r\n"
            L" => [api 2/6] WORKDIR /app                                             0.0s\r\n"
            L" => [api 3/6] COPY package*.json ./                                    0.1s\r\n"
            L" => [api 4/6] RUN npm ci --omit=dev                                   18.3s\r\n"
            L" => [api 5/6] COPY dist/ ./dist/                                       0.4s\r\n"
            L" => [api] exporting to image                                           2.1s\r\n"
            L" => => naming to registry.internal/core-api:2.14.3                     0.0s\r\n"
            L"PS C:\\Work\\infra-services> kubectl rollout status deployment/core-api -n prod\r\n"
            L"Waiting for deployment \"core-api\" rollout to finish: 2 of 4 updated replicas are available...\r\n";
    }
    if (idx == 2) {
        return
            L"Windows PowerShell\r\n"
            L"Copyright (C) Microsoft Corporation. All rights reserved.\r\n\r\n"
            L"PS C:\\Diagnostics\\SystemAudit> .\\audit-system.ps1 -DeepScan -VerifyChecksums\r\n"
            L"[INFO]  Initiating local service integrity audit...\r\n"
            L"[OK]    Network DNS and socket transports operational (avg latency: 4.2 ms)\r\n"
            L"[OK]    Volume C:\\ checked - 0 reallocated sectors\r\n"
            L"[INFO]  Inspecting application event log records (last 24h)...\r\n"
            L"[OK]    Critical events: 0 | Warnings: 2 (non-blocking)\r\n"
            L"[INFO]  Verifying SHA-256 package manifest signatures...\r\n";
    }
    return
        L"Windows PowerShell\r\n"
        L"Copyright (C) Microsoft Corporation. All rights reserved.\r\n\r\n"
        L"PS C:\\Projects\\core-engine> cmake --build build/release --config Release -j 8\r\n"
        L"[ 12%] Building CXX object src/CMakeFiles/core.dir/scheduler.cpp.obj\r\n"
        L"[ 24%] Building CXX object src/CMakeFiles/core.dir/memory_pool.cpp.obj\r\n"
        L"[ 38%] Building CXX object src/CMakeFiles/core.dir/storage_index.cpp.obj\r\n"
        L"[ 51%] Building CXX object src/CMakeFiles/core.dir/net_transport.cpp.obj\r\n"
        L"[ 64%] Building CXX object src/CMakeFiles/core.dir/metrics_collector.cpp.obj\r\n"
        L"[ 78%] Linking CXX static library libcore.a\r\n"
        L"[ 85%] Built target core\r\n"
        L"[ 92%] Building CXX object tests/CMakeFiles/unit_tests.dir/test_pipeline.cpp.obj\r\n"
        L"[100%] Linking CXX executable unit_tests.exe\r\n"
        L"PS C:\\Projects\\core-engine> ctest --test-dir build/release --output-on-failure\r\n"
        L"Test project C:/Projects/core-engine/build/release\r\n"
        L"    Start 1: core_scheduler_latency\r\n"
        L"1/6 Test #1: core_scheduler_latency ...........   Passed    0.18 sec\r\n"
        L"    Start 2: storage_index_consistency\r\n"
        L"2/6 Test #2: storage_index_consistency ........   Passed    0.42 sec\r\n";
}

bool AppConfig::Load(const std::wstring& path) {
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        startWithWindows = GetStartWithWindows();
        return false;
    }

    DWORD size = GetFileSize(hFile, nullptr);
    std::string raw;
    if (size > 0 && size < 4 * 1024 * 1024) {
        raw.resize(size);
        DWORD readBytes = 0;
        ReadFile(hFile, raw.data(), size, &readBytes, nullptr);
        raw.resize(readBytes);
    }
    CloseHandle(hFile);

    std::wstring content = utils::FromUtf8(raw);
    std::unordered_map<std::wstring, std::wstring> kv;

    std::wistringstream iss(content);
    std::wstring line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty() || line[0] == L'#' || line[0] == L';' || line[0] == L'[') continue;
        auto eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring key = utils::Trim(line.substr(0, eq));
        std::wstring val = utils::UnescapeIniValue(line.substr(eq + 1));
        kv[key] = val;
    }

    auto getInt = [&](const wchar_t* k, int def) -> int {
        auto it = kv.find(k);
        if (it == kv.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getUInt = [&](const wchar_t* k, UINT def) -> UINT {
        auto it = kv.find(k);
        if (it == kv.end()) return def;
        try { return (UINT)std::stoul(it->second); } catch (...) { return def; }
    };
    auto getBool = [&](const wchar_t* k, bool def) -> bool {
        return getInt(k, def ? 1 : 0) != 0;
    };
    auto getStr = [&](const wchar_t* k, const std::wstring& def) -> std::wstring {
        auto it = kv.find(k);
        return (it == kv.end()) ? def : it->second;
    };

    panicModifiers = getUInt(L"panicModifiers", panicModifiers);
    panicVk = getUInt(L"panicVk", panicVk);
    settingsModifiers = getUInt(L"settingsModifiers", settingsModifiers);
    settingsVk = getUInt(L"settingsVk", settingsVk);

    int mt = getInt(L"mouseTrigger", (int)mouseTrigger);
    if (mt >= 0 && mt <= 3) mouseTrigger = (MouseTrigger)mt;

    muteAudioOnPanic = getBool(L"muteAudioOnPanic", muteAudioOnPanic);
    pauseMediaOnPanic = getBool(L"pauseMediaOnPanic", pauseMediaOnPanic);
    stealthTray = getBool(L"stealthTray", stealthTray);
    startWithWindows = GetStartWithWindows();
    showStartupNotification = getBool(L"showStartupNotification", showStartupNotification);
    uiTheme = getInt(L"uiTheme", uiTheme);
    language = getInt(L"language", language);

    int dt = getInt(L"decoyType", (int)decoyType);
    if (dt >= 0 && dt <= (int)DecoyType::CleanDesktop) decoyType = (DecoyType)dt;

    externalPath = getStr(L"externalPath", externalPath);
    externalArgs = getStr(L"externalArgs", externalArgs);
    externalMaximized = getBool(L"externalMaximized", externalMaximized);
    prewarmExternal = getBool(L"prewarmExternal", prewarmExternal);

    int ra = getInt(L"decoyRestoreAction", (int)decoyRestoreAction);
    if (ra >= 0 && ra <= 2) decoyRestoreAction = (DecoyRestoreAction)ra;

    urlAddress = getStr(L"urlAddress", urlAddress);
    int um = getInt(L"urlBrowserMode", (int)urlBrowserMode);
    if (um >= 0 && um <= 2) urlBrowserMode = (UrlBrowserMode)um;
    prewarmUrl = getBool(L"prewarmUrl", prewarmUrl);

    capturedKeepVisibleUntilPanic = getBool(L"capturedKeepVisibleUntilPanic", capturedKeepVisibleUntilPanic);

    notesTitle = getStr(L"notesTitle", notesTitle);
    notesContent = getStr(L"notesContent", notesContent);
    notesTheme = getInt(L"notesTheme", notesTheme);

    terminalTitle = getStr(L"terminalTitle", terminalTitle);
    terminalContent = getStr(L"terminalContent", terminalContent);
    terminalSimulateActivity = getBool(L"terminalSimulateActivity", terminalSimulateActivity);

    sheetTitle = getStr(L"sheetTitle", sheetTitle);
    sheetTemplate = getInt(L"sheetTemplate", sheetTemplate);

    imagePath = getStr(L"imagePath", imagePath);
    imageFullscreen = getBool(L"imageFullscreen", imageFullscreen);

    excludedProcesses = getStr(L"excludedProcesses", excludedProcesses);
    return true;
}

bool AppConfig::Save(const std::wstring& path) const {
    std::wostringstream oss;
    oss << L"# Panic Button Configuration\r\n";
    auto putStr = [&](const wchar_t* k, const std::wstring& v) {
        oss << k << L"=" << utils::EscapeIniValue(v) << L"\r\n";
    };
    auto putInt = [&](const wchar_t* k, long long v) {
        oss << k << L"=" << v << L"\r\n";
    };

    putInt(L"panicModifiers", panicModifiers);
    putInt(L"panicVk", panicVk);
    putInt(L"settingsModifiers", settingsModifiers);
    putInt(L"settingsVk", settingsVk);
    putInt(L"mouseTrigger", (int)mouseTrigger);

    putInt(L"muteAudioOnPanic", muteAudioOnPanic ? 1 : 0);
    putInt(L"pauseMediaOnPanic", pauseMediaOnPanic ? 1 : 0);
    putInt(L"stealthTray", stealthTray ? 1 : 0);
    putInt(L"showStartupNotification", showStartupNotification ? 1 : 0);
    putInt(L"uiTheme", uiTheme);
    putInt(L"language", language);

    putInt(L"decoyType", (int)decoyType);

    putStr(L"externalPath", externalPath);
    putStr(L"externalArgs", externalArgs);
    putInt(L"externalMaximized", externalMaximized ? 1 : 0);
    putInt(L"prewarmExternal", prewarmExternal ? 1 : 0);
    putInt(L"decoyRestoreAction", (int)decoyRestoreAction);

    putStr(L"urlAddress", urlAddress);
    putInt(L"urlBrowserMode", (int)urlBrowserMode);
    putInt(L"prewarmUrl", prewarmUrl ? 1 : 0);

    putInt(L"capturedKeepVisibleUntilPanic", capturedKeepVisibleUntilPanic ? 1 : 0);

    putStr(L"notesTitle", notesTitle);
    putStr(L"notesContent", notesContent);
    putInt(L"notesTheme", notesTheme);

    putStr(L"terminalTitle", terminalTitle);
    putStr(L"terminalContent", terminalContent);
    putInt(L"terminalSimulateActivity", terminalSimulateActivity ? 1 : 0);

    putStr(L"sheetTitle", sheetTitle);
    putInt(L"sheetTemplate", sheetTemplate);

    putStr(L"imagePath", imagePath);
    putInt(L"imageFullscreen", imageFullscreen ? 1 : 0);

    putStr(L"excludedProcesses", excludedProcesses);

    std::string utf8 = utils::ToUtf8(oss.str());
    std::wstring tmpPath = path + L".tmp";

    HANDLE hFile = CreateFileW(tmpPath.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    BOOL ok = WriteFile(hFile, utf8.data(), (DWORD)utf8.size(), &written, nullptr);
    CloseHandle(hFile);
    if (!ok || written != utf8.size()) {
        DeleteFileW(tmpPath.c_str());
        return false;
    }

    return MoveFileExW(tmpPath.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
}

std::vector<std::wstring> AppConfig::GetExcludedProcessList() const {
    auto list = utils::SplitLinesOrCommas(excludedProcesses);
    for (auto& item : list) item = utils::ToLower(item);
    return list;
}

static constexpr const wchar_t* kRunRegKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static constexpr const wchar_t* kRunValueName = L"PanicButton";

bool SetStartWithWindows(bool enable) {
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunRegKey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    LSTATUS st;
    if (enable) {
        std::wstring cmd = L"\"" + utils::GetExePath() + L"\" --minimized";
        st = RegSetValueExW(hKey, kRunValueName, 0, REG_SZ,
                            reinterpret_cast<const BYTE*>(cmd.c_str()),
                            (DWORD)((cmd.size() + 1) * sizeof(wchar_t)));
    } else {
        st = RegDeleteValueW(hKey, kRunValueName);
        if (st == ERROR_FILE_NOT_FOUND) st = ERROR_SUCCESS;
    }
    RegCloseKey(hKey);
    return st == ERROR_SUCCESS;
}

bool GetStartWithWindows() {
    DWORD type = 0;
    DWORD cb = 0;
    LSTATUS st = RegGetValueW(HKEY_CURRENT_USER, kRunRegKey, kRunValueName,
                              RRF_RT_REG_SZ, &type, nullptr, &cb);
    return st == ERROR_SUCCESS && cb > 0;
}
