#include "builtin_decoys.h"

#include <objidl.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <algorithm>

namespace {

constexpr const wchar_t* kDecoyClassName = L"Win11DocWorkspaceFrame";
constexpr UINT_PTR kTerminalTimerId = 4101;
constexpr int kEditCtrlId = 5001;

struct SheetData {
    std::vector<std::wstring> headers;
    std::vector<std::vector<std::wstring>> rows;
    std::wstring sampleFormula;
};

SheetData GetSheetTemplateData(int tpl) {
    SheetData d;
    if (tpl == 1) {
        d.headers = {L"ID", L"Módulo / Tarea", L"Responsable", L"Estado", L"Progreso", L"Prioridad", L"Entrega"};
        d.sampleFormula = L"=PROMEDIO(E2:E13)";
        d.rows = {
            {L"PRJ-101", L"Auditoría de arquitectura base", L"A. Morales", L"Completado", L"100%", L"Alta", L"04/11/2026"},
            {L"PRJ-102", L"Optimización de consultas SQL", L"M. Serrano", L"Completado", L"100%", L"Alta", L"08/11/2026"},
            {L"PRJ-103", L"Caché distribuida de sesiones", L"L. Navarro", L"En curso", L"85%", L"Alta", L"20/11/2026"},
            {L"PRJ-104", L"Integración SSO corporativo", L"C. Herrera", L"En revisión", L"95%", L"Media", L"22/11/2026"},
            {L"PRJ-105", L"Panel de métricas operativas", L"D. Ortega", L"En curso", L"70%", L"Media", L"25/11/2026"},
            {L"PRJ-106", L"Pruebas de carga automatizadas", L"R. Gil", L"En curso", L"60%", L"Alta", L"27/11/2026"},
            {L"PRJ-107", L"Documentación técnica API v3", L"S. Castro", L"Planificado", L"35%", L"Baja", L"02/12/2026"},
            {L"PRJ-108", L"Migración certificados TLS", L"A. Morales", L"Completado", L"100%", L"Crítica", L"10/11/2026"},
            {L"PRJ-109", L"Exportador PDF consolidado", L"E. Ruiz", L"En revisión", L"90%", L"Media", L"21/11/2026"},
            {L"PRJ-110", L"Revisión de accesibilidad UI", L"P. Vega", L"En curso", L"55%", L"Media", L"29/11/2026"},
            {L"PRJ-111", L"Backup incremental cifrado", L"M. Serrano", L"Completado", L"100%", L"Alta", L"12/11/2026"},
            {L"PRJ-112", L"Plan de contingencia Q1", L"L. Navarro", L"Planificado", L"20%", L"Media", L"05/12/2026"}
        };
    } else if (tpl == 2) {
        d.headers = {L"Partida", L"Departamento", L"Presupuesto", L"Ejecutado", L"Desviación", L"% Uso", L"Observaciones"};
        d.sampleFormula = L"=SUMA(C2:C11)-SUMA(D2:D11)";
        d.rows = {
            {L"Infraestructura Cloud", L"Sistemas", L"48.500 €", L"46.120 €", L"-2.380 €", L"95,1%", L"Dentro de objetivo"},
            {L"Licencias Software", L"TI", L"22.000 €", L"21.450 €", L"-550 €", L"97,5%", L"Renovación anual cerrada"},
            {L"Seguridad y Auditoría", L"Cumplimiento", L"18.000 €", L"16.800 €", L"-1.200 €", L"93,3%", L"Informe Q4 pendiente"},
            {L"Formación Técnica", L"RRHH", L"12.500 €", L"10.900 €", L"-1.600 €", L"87,2%", L"Certificaciones en curso"},
            {L"Soporte Externo", L"Operaciones", L"15.000 €", L"14.320 €", L"-680 €", L"95,5%", L"SLA cumplido 99,9%"},
            {L"Equipamiento", L"General", L"19.200 €", L"18.950 €", L"-250 €", L"98,7%", L"Reposición trimestral"},
            {L"Servicios Red / VPN", L"Sistemas", L"9.600 €", L"9.480 €", L"-120 €", L"98,8%", L"Tarifa consolidada"},
            {L"Consultoría Datos", L"Analítica", L"24.000 €", L"22.100 €", L"-1.900 €", L"92,1%", L"Fase 2 aprobada"},
            {L"Contingencia", L"Dirección", L"10.000 €", L"3.400 €", L"-6.600 €", L"34,0%", L"Reserva disponible"},
            {L"TOTAL GENERAL", L"Consolidado", L"178.800 €", L"163.520 €", L"-15.280 €", L"91,5%", L"Balance favorable"}
        };
    } else {
        d.headers = {L"Indicador (KPI)", L"Unidad", L"Q1 2026", L"Q2 2026", L"Q3 2026", L"Q4 (Est.)", L"Variación"};
        d.sampleFormula = L"=((F2-C2)/C2)*100";
        d.rows = {
            {L"Disponibilidad SLA", L"%", L"99,88%", L"99,91%", L"99,93%", L"99,95%", L"+0,07%"},
            {L"Latencia media P95", L"ms", L"184", L"162", L"141", L"118", L"-35,9%"},
            {L"Incidencias resueltas", L"uds", L"312", L"298", L"274", L"251", L"-19,6%"},
            {L"Tiempo medio respuesta", L"horas", L"3,4", L"2,9", L"2,5", L"2,1", L"-38,2%"},
            {L"Eficiencia operativa", L"%", L"84,2%", L"87,6%", L"91,0%", L"94,3%", L"+10,1%"},
            {L"Cobertura pruebas auto", L"%", L"72,5%", L"78,0%", L"84,4%", L"89,2%", L"+16,7%"},
            {L"Despliegues sin fallos", L"%", L"94,0%", L"95,8%", L"97,2%", L"98,6%", L"+4,6%"},
            {L"Coste por transacción", L"EUR", L"0,048", L"0,044", L"0,039", L"0,035", L"-27,1%"},
            {L"Satisfacción interna", L"/ 5.0", L"4,2", L"4,4", L"4,6", L"4,7", L"+11,9%"},
            {L"Cumplimiento plazos", L"%", L"88,5%", L"91,2%", L"94,1%", L"96,4%", L"+7,9%"}
        };
    }
    return d;
}

const std::vector<std::wstring>& GetSimulatedTerminalLines() {
    static const std::vector<std::wstring> kLines = {
        L"    Start 3: net_transport_throughput\r\n",
        L"3/6 Test #3: net_transport_throughput .........   Passed    0.61 sec\r\n",
        L"    Start 4: memory_pool_fragmentation\r\n",
        L"4/6 Test #4: memory_pool_fragmentation ........   Passed    0.34 sec\r\n",
        L"    Start 5: metrics_collector_snapshot\r\n",
        L"5/6 Test #5: metrics_collector_snapshot .......   Passed    0.27 sec\r\n",
        L"    Start 6: end_to_end_pipeline_stress\r\n",
        L"6/6 Test #6: end_to_end_pipeline_stress .......   Passed    1.12 sec\r\n",
        L"100% tests passed, 0 tests failed out of 6\r\n",
        L"Total Test time (real) = 2.94 sec\r\n",
        L"PS C:\\Proyectos\\core-engine> git status --short\r\n",
        L" M src/scheduler.cpp\r\n M src/storage_index.cpp\r\n",
        L"PS C:\\Proyectos\\core-engine> .\\scripts\\verify_artifacts.ps1 -CheckHash\r\n",
        L"[OK] SHA-256 verificado para build/release/core.lib\r\n",
        L"[INFO] Monitorizando métricas de rendimiento en espera...\r\n"
    };
    return kLines;
}

} // namespace

BuiltinDecoyWindow::~BuiltinDecoyWindow() {
    Destroy();
}

void BuiltinDecoyWindow::ClearIcons() {
    if (iconBig_) { DestroyIcon(iconBig_); iconBig_ = nullptr; }
    if (iconSmall_) { DestroyIcon(iconSmall_); iconSmall_ = nullptr; }
}

void BuiltinDecoyWindow::Destroy() {
    if (hwnd_) {
        KillTimer(hwnd_, kTerminalTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        editCtrl_ = nullptr;
        statusBar_ = nullptr;
    }
    if (editFont_) { DeleteObject(editFont_); editFont_ = nullptr; }
    if (uiFont_) { DeleteObject(uiFont_); uiFont_ = nullptr; }
    if (uiBoldFont_) { DeleteObject(uiBoldFont_); uiBoldFont_ = nullptr; }
    if (editBgBrush_) { DeleteObject(editBgBrush_); editBgBrush_ = nullptr; }
    if (imageBmp_) { DeleteObject(imageBmp_); imageBmp_ = nullptr; }
    loadedImagePath_.clear();
    ClearIcons();
}

void BuiltinDecoyWindow::LoadImageBitmap(const std::wstring& path) {
    if (path == loadedImagePath_ && imageBmp_) return;
    if (imageBmp_) { DeleteObject(imageBmp_); imageBmp_ = nullptr; }
    imageWidth_ = 0;
    imageHeight_ = 0;
    loadedImagePath_ = path;

    if (path.empty()) return;

    Gdiplus::Bitmap src(path.c_str());
    if (src.GetLastStatus() != Gdiplus::Ok) return;

    imageWidth_ = (int)src.GetWidth();
    imageHeight_ = (int)src.GetHeight();
    src.GetHBITMAP(Gdiplus::Color(255, 24, 24, 24), &imageBmp_);
}

void BuiltinDecoyWindow::EnsureWindowCreated(HINSTANCE hInst, const AppConfig& cfg) {
    hInst_ = hInst;
    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = BuiltinDecoyWindow::WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = kDecoyClassName;
        RegisterClassExW(&wc);
        registered = true;
    }

    if (!hwnd_) {
        hwnd_ = CreateWindowExW(
            0,
            kDecoyClassName,
            L"Documento",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1180, 760,
            nullptr, nullptr, hInst, this);

        UINT dpi = utils::GetDpiForWindowSafe(hwnd_);
        uiFont_ = utils::CreateUIFont(dpi, 9, false, L"Segoe UI");
        uiBoldFont_ = utils::CreateUIFont(dpi, 9, true, L"Segoe UI");

        editCtrl_ = CreateWindowExW(
            0, L"EDIT", L"",
            WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            0, 0, 100, 100,
            hwnd_, (HMENU)(INT_PTR)kEditCtrlId, hInst, nullptr);

        // Increase edit text limit to 2 MB
        SendMessageW(editCtrl_, EM_SETLIMITTEXT, 2 * 1024 * 1024, 0);

        statusBar_ = CreateWindowExW(
            0, L"STATIC", L"",
            WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            0, 0, 100, 24,
            hwnd_, nullptr, hInst, nullptr);
        SendMessageW(statusBar_, WM_SETFONT, (WPARAM)uiFont_, TRUE);

        ShowWindow(hwnd_, SW_HIDE);
        SetWindowPos(hwnd_, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
    }

    ApplyModeAndContent(cfg);
}

void BuiltinDecoyWindow::Prepare(HINSTANCE hInst, const AppConfig& cfg) {
    if (cfg.decoyType == DecoyType::BuiltinNotes ||
        cfg.decoyType == DecoyType::BuiltinTerminal ||
        cfg.decoyType == DecoyType::BuiltinSpreadsheet ||
        cfg.decoyType == DecoyType::BuiltinImage) {
        EnsureWindowCreated(hInst, cfg);
    }
}

void BuiltinDecoyWindow::ApplyModeAndContent(const AppConfig& cfg) {
    if (!hwnd_) return;

    currentType_ = cfg.decoyType;
    simulateTerminal_ = cfg.terminalSimulateActivity;
    imageFullscreen_ = cfg.imageFullscreen;
    sheetTemplate_ = cfg.sheetTemplate;
    simStep_ = 0;

    UINT dpi = utils::GetDpiForWindowSafe(hwnd_);
    if (editFont_) { DeleteObject(editFont_); editFont_ = nullptr; }
    if (editBgBrush_) { DeleteObject(editBgBrush_); editBgBrush_ = nullptr; }

    std::wstring title;
    utils::DecoyIconKind iconKind = utils::DecoyIconKind::Document;

    if (currentType_ == DecoyType::BuiltinNotes) {
        title = cfg.notesTitle.empty() ? L"Informe.txt - Bloc de notas" : cfg.notesTitle;
        darkMode_ = (cfg.notesTheme == 2) || (cfg.notesTheme == 0 && utils::IsSystemDarkMode());
        editFont_ = utils::CreateUIFont(dpi, 11, false, L"Consolas");
        editBgBrush_ = CreateSolidBrush(darkMode_ ? RGB(30, 30, 30) : RGB(255, 255, 255));
        iconKind = utils::DecoyIconKind::Document;

        SetWindowTextW(editCtrl_, cfg.notesContent.c_str());
        SetWindowTextW(statusBar_, L"  Ln 1, Col 1    |    100%    |    Windows (CRLF)    |    UTF-8");
        ShowWindow(editCtrl_, SW_SHOW);
        ShowWindow(statusBar_, SW_SHOW);
    } else if (currentType_ == DecoyType::BuiltinTerminal) {
        title = cfg.terminalTitle.empty() ? L"Windows PowerShell" : cfg.terminalTitle;
        darkMode_ = true;
        editFont_ = utils::CreateUIFont(dpi, 10, false, L"Consolas");
        editBgBrush_ = CreateSolidBrush(RGB(12, 12, 12));
        iconKind = utils::DecoyIconKind::Terminal;

        SetWindowTextW(editCtrl_, cfg.terminalContent.c_str());
        ShowWindow(editCtrl_, SW_SHOW);
        ShowWindow(statusBar_, SW_HIDE);
    } else if (currentType_ == DecoyType::BuiltinSpreadsheet) {
        title = cfg.sheetTitle.empty() ? L"Hoja_de_calculo.xlsx" : cfg.sheetTitle;
        darkMode_ = false;
        iconKind = utils::DecoyIconKind::Spreadsheet;
        ShowWindow(editCtrl_, SW_HIDE);
        ShowWindow(statusBar_, SW_HIDE);
    } else if (currentType_ == DecoyType::BuiltinImage) {
        title = L"Visualizador de documentos";
        darkMode_ = true;
        iconKind = utils::DecoyIconKind::Image;
        LoadImageBitmap(cfg.imagePath);
        ShowWindow(editCtrl_, SW_HIDE);
        ShowWindow(statusBar_, SW_HIDE);
    }

    if (editFont_ && editCtrl_) {
        SendMessageW(editCtrl_, WM_SETFONT, (WPARAM)editFont_, TRUE);
        RECT rcEdit{};
        GetClientRect(editCtrl_, &rcEdit);
        int pad = utils::Scale(14, dpi);
        SendMessageW(editCtrl_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(pad, pad));
    }

    // Window style: borderless fullscreen for Image if requested, else standard overlapped window
    LONG_PTR style = GetWindowLongPtrW(hwnd_, GWL_STYLE);
    if (currentType_ == DecoyType::BuiltinImage && imageFullscreen_) {
        style = (style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP;
    } else {
        style = (style & ~WS_POPUP) | WS_OVERLAPPEDWINDOW;
    }
    SetWindowLongPtrW(hwnd_, GWL_STYLE, style);

    SetWindowTextW(hwnd_, title.c_str());
    utils::ApplyWin11WindowStyle(hwnd_, darkMode_);

    ClearIcons();
    iconBig_ = utils::CreateDecoyIcon(32, iconKind);
    iconSmall_ = utils::CreateDecoyIcon(16, iconKind);
    SendMessageW(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)iconBig_);
    SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)iconSmall_);

    LayoutChildren();
    InvalidateRect(hwnd_, nullptr, TRUE);
}

HWND BuiltinDecoyWindow::Show(HINSTANCE hInst, const AppConfig& cfg) {
    EnsureWindowCreated(hInst, cfg);
    if (!hwnd_) return nullptr;

    if (currentType_ == DecoyType::BuiltinImage && imageFullscreen_) {
        HMONITOR mon = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(mon, &mi);
        SetWindowPos(hwnd_, HWND_TOP,
                     mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        ShowWindow(hwnd_, SW_SHOW);
    } else {
        SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowWindow(hwnd_, SW_SHOWMAXIMIZED);
    }

    utils::ForceForegroundWindow(hwnd_);

    if (currentType_ == DecoyType::BuiltinNotes || currentType_ == DecoyType::BuiltinTerminal) {
        SetFocus(editCtrl_);
        int len = GetWindowTextLengthW(editCtrl_);
        SendMessageW(editCtrl_, EM_SETSEL, len, len);
        SendMessageW(editCtrl_, EM_SCROLLCARET, 0, 0);
    }

    if (currentType_ == DecoyType::BuiltinTerminal && simulateTerminal_) {
        SetTimer(hwnd_, kTerminalTimerId, 1600, nullptr);
    } else {
        KillTimer(hwnd_, kTerminalTimerId);
    }

    InvalidateRect(hwnd_, nullptr, TRUE);
    UpdateWindow(hwnd_);
    return hwnd_;
}

void BuiltinDecoyWindow::Hide() {
    if (!hwnd_) return;
    KillTimer(hwnd_, kTerminalTimerId);
    if (IsZoomed(hwnd_)) {
        ShowWindow(hwnd_, SW_RESTORE);
    }
    ShowWindow(hwnd_, SW_HIDE);
    SetWindowPos(hwnd_, HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
}

void BuiltinDecoyWindow::LayoutChildren() {
    if (!hwnd_) return;
    RECT rc{};
    GetClientRect(hwnd_, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    UINT dpi = utils::GetDpiForWindowSafe(hwnd_);

    if (currentType_ == DecoyType::BuiltinNotes) {
        int menuH = utils::Scale(30, dpi);
        int statusH = utils::Scale(26, dpi);
        MoveWindow(editCtrl_, 0, menuH, w, std::max(0, h - menuH - statusH), TRUE);
        MoveWindow(statusBar_, 0, std::max(0, h - statusH), w, statusH, TRUE);
    } else if (currentType_ == DecoyType::BuiltinTerminal) {
        int pad = utils::Scale(6, dpi);
        MoveWindow(editCtrl_, pad, pad, std::max(0, w - 2 * pad), std::max(0, h - 2 * pad), TRUE);
    }
}

void BuiltinDecoyWindow::AppendSimulatedTerminalLine() {
    if (!editCtrl_ || currentType_ != DecoyType::BuiltinTerminal) return;
    const auto& lines = GetSimulatedTerminalLines();
    if (simStep_ >= (int)lines.size()) {
        KillTimer(hwnd_, kTerminalTimerId);
        return;
    }
    int len = GetWindowTextLengthW(editCtrl_);
    SendMessageW(editCtrl_, EM_SETSEL, len, len);
    SendMessageW(editCtrl_, EM_REPLACESEL, FALSE, (LPARAM)lines[simStep_].c_str());
    SendMessageW(editCtrl_, EM_SCROLLCARET, 0, 0);
    ++simStep_;
}

void BuiltinDecoyWindow::PaintSpreadsheet(HDC hdc, const RECT& rc) {
    UINT dpi = utils::GetDpiForWindowSafe(hwnd_);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    // Background white
    HBRUSH whiteBr = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(memDC, &rc, whiteBr);
    DeleteObject(whiteBr);

    int ribbonH = utils::Scale(44, dpi);
    int formulaH = utils::Scale(28, dpi);
    int headerH = utils::Scale(26, dpi);
    int rowH = utils::Scale(28, dpi);
    int rowNumW = utils::Scale(46, dpi);

    // Green Excel-like header strip
    RECT ribbonRc{0, 0, w, ribbonH};
    HBRUSH greenBr = CreateSolidBrush(RGB(24, 118, 66));
    FillRect(memDC, &ribbonRc, greenBr);
    DeleteObject(greenBr);

    SetBkMode(memDC, TRANSPARENT);
    HGDIOBJ oldFont = SelectObject(memDC, uiBoldFont_);
    SetTextColor(memDC, RGB(255, 255, 255));
    RECT tabRc{utils::Scale(16, dpi), 0, w, ribbonH};
    DrawTextW(memDC, L"Archivo     Inicio     Insertar     Disposición     Fórmulas     Datos     Revisar     Vista",
              -1, &tabRc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

    // Formula bar
    RECT fBar{0, ribbonH, w, ribbonH + formulaH};
    HBRUSH fBg = CreateSolidBrush(RGB(243, 243, 243));
    FillRect(memDC, &fBar, fBg);
    DeleteObject(fBg);

    SheetData data = GetSheetTemplateData(sheetTemplate_);
    wchar_t cellRef[16]{};
    swprintf_s(cellRef, L"%c%d", L'A' + selCol_, selRow_ + 1);

    SelectObject(memDC, uiFont_);
    SetTextColor(memDC, RGB(50, 50, 50));
    RECT cellRefRc{utils::Scale(8, dpi), ribbonH, utils::Scale(76, dpi), ribbonH + formulaH};
    DrawTextW(memDC, cellRef, -1, &cellRefRc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    RECT fxRc{utils::Scale(84, dpi), ribbonH, utils::Scale(116, dpi), ribbonH + formulaH};
    SetTextColor(memDC, RGB(110, 110, 110));
    DrawTextW(memDC, L"fx", -1, &fxRc, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

    RECT fTextRc{utils::Scale(124, dpi), ribbonH + 3, w - utils::Scale(12, dpi), ribbonH + formulaH - 3};
    HBRUSH whiteBox = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(memDC, &fTextRc, whiteBox);
    DeleteObject(whiteBox);
    SetTextColor(memDC, RGB(30, 30, 30));
    RECT fTextPad = fTextRc;
    fTextPad.left += utils::Scale(8, dpi);
    std::wstring formulaDisplay = data.sampleFormula;
    if (selRow_ >= 1 && selRow_ - 1 < (int)data.rows.size() && selCol_ < (int)data.rows[selRow_ - 1].size()) {
        formulaDisplay = data.rows[selRow_ - 1][selCol_];
    }
    DrawTextW(memDC, formulaDisplay.c_str(), -1, &fTextPad, DT_SINGLELINE | DT_VCENTER | DT_LEFT);

    // Column layout
    int numCols = (int)data.headers.size();
    int colW = std::max(utils::Scale(110, dpi), (w - rowNumW - 2) / std::max(1, numCols));
    int gridTop = ribbonH + formulaH;

    // Column headers background
    RECT colHdrBg{0, gridTop, w, gridTop + headerH};
    HBRUSH hdrBr = CreateSolidBrush(RGB(242, 244, 247));
    FillRect(memDC, &colHdrBg, hdrBr);
    RECT rowHdrBg{0, gridTop, rowNumW, h};
    FillRect(memDC, &rowHdrBg, hdrBr);
    DeleteObject(hdrBr);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(217, 221, 226));
    HGDIOBJ oldPen = SelectObject(memDC, gridPen);

    // Draw column letters
    SetTextColor(memDC, RGB(70, 75, 82));
    for (int c = 0; c < numCols + 4; ++c) {
        int x = rowNumW + c * colW;
        if (x > w) break;
        MoveToEx(memDC, x, gridTop, nullptr);
        LineTo(memDC, x, h);
        wchar_t colName[4] = {(wchar_t)(L'A' + c), 0};
        RECT cr{x, gridTop, x + colW, gridTop + headerH};
        DrawTextW(memDC, colName, -1, &cr, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
    }

    // Draw rows
    int maxVisibleRows = (h - gridTop - headerH) / rowH + 2;
    for (int r = 0; r < maxVisibleRows; ++r) {
        int y = gridTop + headerH + r * rowH;
        MoveToEx(memDC, 0, y, nullptr);
        LineTo(memDC, w, y);

        // Row number
        std::wstring rNum = std::to_wstring(r + 1);
        RECT nr{0, y, rowNumW, y + rowH};
        SetTextColor(memDC, RGB(90, 95, 102));
        SelectObject(memDC, uiFont_);
        DrawTextW(memDC, rNum.c_str(), -1, &nr, DT_SINGLELINE | DT_VCENTER | DT_CENTER);

        if (r == 0) {
            // Table header row (subtle tint)
            RECT tr{rowNumW + 1, y + 1, rowNumW + numCols * colW, y + rowH};
            HBRUSH thBr = CreateSolidBrush(RGB(228, 240, 233));
            FillRect(memDC, &tr, thBr);
            DeleteObject(thBr);

            SelectObject(memDC, uiBoldFont_);
            SetTextColor(memDC, RGB(22, 84, 48));
            for (int c = 0; c < numCols; ++c) {
                RECT cell{rowNumW + c * colW + utils::Scale(8, dpi), y, rowNumW + (c + 1) * colW - utils::Scale(6, dpi), y + rowH};
                DrawTextW(memDC, data.headers[c].c_str(), -1, &cell, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
            }
        } else if (r - 1 < (int)data.rows.size()) {
            const auto& row = data.rows[r - 1];
            bool isTotal = (row[0].find(L"TOTAL") != std::wstring::npos);
            if (isTotal) {
                RECT tr{rowNumW + 1, y + 1, rowNumW + numCols * colW, y + rowH};
                HBRUSH totBr = CreateSolidBrush(RGB(240, 244, 248));
                FillRect(memDC, &tr, totBr);
                DeleteObject(totBr);
            }
            SelectObject(memDC, isTotal ? uiBoldFont_ : uiFont_);
            SetTextColor(memDC, RGB(32, 34, 38));
            for (int c = 0; c < (int)row.size() && c < numCols; ++c) {
                RECT cell{rowNumW + c * colW + utils::Scale(8, dpi), y, rowNumW + (c + 1) * colW - utils::Scale(6, dpi), y + rowH};
                UINT align = (c >= 2 && c <= 5) ? DT_RIGHT : DT_LEFT;
                DrawTextW(memDC, row[c].c_str(), -1, &cell, DT_SINGLELINE | DT_VCENTER | align | DT_END_ELLIPSIS);
            }
        }
    }

    // Active cell green highlight
    if (selRow_ >= 0 && selCol_ >= 0) {
        int sx = rowNumW + selCol_ * colW;
        int sy = gridTop + headerH + selRow_ * rowH;
        HPEN selPen = CreatePen(PS_SOLID, 2, RGB(24, 128, 72));
        HGDIOBJ p2 = SelectObject(memDC, selPen);
        HGDIOBJ b2 = SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, sx, sy, sx + colW + 1, sy + rowH + 1);
        SelectObject(memDC, b2);
        SelectObject(memDC, p2);
        DeleteObject(selPen);
    }

    SelectObject(memDC, oldPen);
    DeleteObject(gridPen);
    SelectObject(memDC, oldFont);

    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

void BuiltinDecoyWindow::PaintImage(HDC hdc, const RECT& rc) {
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    HBRUSH bg = CreateSolidBrush(RGB(18, 18, 20));
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);

    if (!imageBmp_ || imageWidth_ <= 0 || imageHeight_ <= 0) {
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(180, 180, 185));
        HGDIOBJ oldF = SelectObject(hdc, uiFont_);
        RECT r = rc;
        DrawTextW(hdc, L"No se ha configurado ninguna imagen válida de camuflaje.\nAbre la configuración del Botón del Pánico para elegir una captura (.png / .jpg / .bmp).",
                  -1, &r, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
        SelectObject(hdc, oldF);
        return;
    }

    HDC srcDC = CreateCompatibleDC(hdc);
    HGDIOBJ oldBmp = SelectObject(srcDC, imageBmp_);

    if (imageWidth_ == w && imageHeight_ == h) {
        BitBlt(hdc, 0, 0, w, h, srcDC, 0, 0, SRCCOPY);
    } else {
        // Aspect-preserving smooth fit (or exact fill if aspect matches screen)
        double scaleX = (double)w / (double)imageWidth_;
        double scaleY = (double)h / (double)imageHeight_;
        double sc = std::min(scaleX, scaleY);
        int dstW = (int)(imageWidth_ * sc);
        int dstH = (int)(imageHeight_ * sc);
        int dstX = (w - dstW) / 2;
        int dstY = (h - dstH) / 2;

        SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, dstX, dstY, dstW, dstH, srcDC, 0, 0, imageWidth_, imageHeight_, SRCCOPY);
    }

    SelectObject(srcDC, oldBmp);
    DeleteDC(srcDC);
}

LRESULT CALLBACK BuiltinDecoyWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    BuiltinDecoyWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<BuiltinDecoyWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<BuiltinDecoyWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) return self->HandleMessage(msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT BuiltinDecoyWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            LayoutChildren();
            if (currentType_ == DecoyType::BuiltinSpreadsheet || currentType_ == DecoyType::BuiltinImage) {
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            return 0;

        case WM_TIMER:
            if (wParam == kTerminalTimerId) {
                AppendSimulatedTerminalLine();
                return 0;
            }
            break;

        case WM_LBUTTONDOWN:
            if (currentType_ == DecoyType::BuiltinSpreadsheet) {
                UINT dpi = utils::GetDpiForWindowSafe(hwnd_);
                int x = (short)LOWORD(lParam);
                int y = (short)HIWORD(lParam);
                int gridTop = utils::Scale(44 + 28 + 26, dpi);
                int rowH = utils::Scale(28, dpi);
                int rowNumW = utils::Scale(46, dpi);
                RECT rc{};
                GetClientRect(hwnd_, &rc);
                int colW = std::max(utils::Scale(110, dpi), (int)(rc.right - rowNumW - 2) / 7);
                if (x >= rowNumW && y >= gridTop) {
                    selCol_ = (x - rowNumW) / colW;
                    selRow_ = (y - gridTop) / rowH;
                    InvalidateRect(hwnd_, nullptr, FALSE);
                }
                return 0;
            }
            break;

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            if (currentType_ == DecoyType::BuiltinTerminal) {
                SetTextColor(hdc, RGB(212, 212, 212));
                SetBkColor(hdc, RGB(12, 12, 12));
            } else if (darkMode_) {
                SetTextColor(hdc, RGB(236, 236, 236));
                SetBkColor(hdc, RGB(30, 30, 30));
            } else {
                SetTextColor(hdc, RGB(28, 28, 30));
                SetBkColor(hdc, RGB(255, 255, 255));
            }
            return (LRESULT)(editBgBrush_ ? editBgBrush_ : GetStockObject(WHITE_BRUSH));
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            if (darkMode_) {
                SetTextColor(hdc, RGB(180, 180, 185));
                SetBkColor(hdc, RGB(38, 38, 42));
                static HBRUSH darkStatus = CreateSolidBrush(RGB(38, 38, 42));
                return (LRESULT)darkStatus;
            } else {
                SetTextColor(hdc, RGB(88, 92, 98));
                SetBkColor(hdc, RGB(243, 243, 243));
                static HBRUSH lightStatus = CreateSolidBrush(RGB(243, 243, 243));
                return (LRESULT)lightStatus;
            }
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd_, &ps);
            RECT rc{};
            GetClientRect(hwnd_, &rc);

            if (currentType_ == DecoyType::BuiltinSpreadsheet) {
                PaintSpreadsheet(hdc, rc);
            } else if (currentType_ == DecoyType::BuiltinImage) {
                PaintImage(hdc, rc);
            } else if (currentType_ == DecoyType::BuiltinNotes) {
                UINT dpi = utils::GetDpiForWindowSafe(hwnd_);
                int menuH = utils::Scale(30, dpi);
                RECT topRc{0, 0, rc.right, menuH};
                HBRUSH topBr = CreateSolidBrush(darkMode_ ? RGB(42, 42, 46) : RGB(243, 244, 246));
                FillRect(hdc, &topRc, topBr);
                DeleteObject(topBr);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, darkMode_ ? RGB(220, 220, 224) : RGB(36, 36, 40));
                HGDIOBJ oldF = SelectObject(hdc, uiFont_);
                RECT txtRc{utils::Scale(14, dpi), 0, rc.right, menuH};
                DrawTextW(hdc, L"Archivo      Editar      Ver", -1, &txtRc, DT_SINGLELINE | DT_VCENTER | DT_LEFT);
                SelectObject(hdc, oldF);

                HPEN sep = CreatePen(PS_SOLID, 1, darkMode_ ? RGB(60, 60, 66) : RGB(226, 228, 232));
                HGDIOBJ oldP = SelectObject(hdc, sep);
                MoveToEx(hdc, 0, menuH - 1, nullptr);
                LineTo(hdc, rc.right, menuH - 1);
                SelectObject(hdc, oldP);
                DeleteObject(sep);
            } else {
                HBRUSH bg = CreateSolidBrush(RGB(12, 12, 12));
                FillRect(hdc, &rc, bg);
                DeleteObject(bg);
            }

            EndPaint(hwnd_, &ps);
            return 0;
        }

        case WM_CLOSE:
            // Clicking X on decoy just hides it cleanly without exiting the background app
            Hide();
            return 0;

        default:
            break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}
