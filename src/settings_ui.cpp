#include "settings_ui.h"

#include <objidl.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace {

constexpr const wchar_t* kSettingsClassName = L"PanicButton_FluentSettingsWnd";
constexpr UINT_PTR kAnimTimerId = 4301;
constexpr UINT_PTR kLiveStatsTimerId = 4302;

// Unscaled logical layout dimensions (96 DPI)
constexpr int kClientW = 886;
constexpr int kClientH = 636;
constexpr int kSidebarW = 222;
constexpr int kContentX = 246;
constexpr int kContentW = 616;

struct Palette {
    Gdiplus::Color sidebarBg;
    Gdiplus::Color canvasBg;
    Gdiplus::Color cardBg;
    Gdiplus::Color cardBorder;
    Gdiplus::Color cardHoverBg;
    Gdiplus::Color inputBg;
    Gdiplus::Color inputBorder;
    Gdiplus::Color divider;

    Gdiplus::Color textPrimary;
    Gdiplus::Color textSecondary;
    Gdiplus::Color textMuted;

    Gdiplus::Color accent;
    Gdiplus::Color accentHover;
    Gdiplus::Color accentPress;
    Gdiplus::Color accentSoftBg;

    Gdiplus::Color danger;
    Gdiplus::Color dangerHover;
    Gdiplus::Color dangerPress;

    Gdiplus::Color success;
    Gdiplus::Color successBg;
    Gdiplus::Color warn;
    Gdiplus::Color warnBg;

    Gdiplus::Color keycapBg;
    Gdiplus::Color keycapBorder;
    Gdiplus::Color btnSecBg;
    Gdiplus::Color btnSecHover;
    Gdiplus::Color btnSecBorder;

    COLORREF captionRef;
    COLORREF borderRef;
    COLORREF inputBgRef;
    COLORREF inputTextRef;
    COLORREF cardBgRef;
};

Palette MakePalette(bool dark) {
    Palette p{};
    if (dark) {
        p.sidebarBg    = Gdiplus::Color(255, 23, 25, 32);
        p.canvasBg     = Gdiplus::Color(255, 29, 32, 41);
        p.cardBg       = Gdiplus::Color(255, 38, 42, 54);
        p.cardBorder   = Gdiplus::Color(255, 55, 61, 78);
        p.cardHoverBg  = Gdiplus::Color(255, 46, 51, 66);
        p.inputBg      = Gdiplus::Color(255, 28, 31, 40);
        p.inputBorder  = Gdiplus::Color(255, 64, 71, 90);
        p.divider      = Gdiplus::Color(255, 48, 53, 68);

        p.textPrimary   = Gdiplus::Color(255, 242, 244, 248);
        p.textSecondary = Gdiplus::Color(255, 162, 170, 188);
        p.textMuted     = Gdiplus::Color(255, 118, 126, 146);

        p.accent       = Gdiplus::Color(255, 59, 130, 246);
        p.accentHover  = Gdiplus::Color(255, 84, 150, 255);
        p.accentPress  = Gdiplus::Color(255, 37, 99, 235);
        p.accentSoftBg = Gdiplus::Color(42, 59, 130, 246);

        p.danger       = Gdiplus::Color(255, 224, 68, 82);
        p.dangerHover  = Gdiplus::Color(255, 238, 84, 98);
        p.dangerPress  = Gdiplus::Color(255, 196, 48, 62);

        p.success      = Gdiplus::Color(255, 52, 211, 153);
        p.successBg    = Gdiplus::Color(36, 52, 211, 153);
        p.warn         = Gdiplus::Color(255, 251, 191, 36);
        p.warnBg       = Gdiplus::Color(38, 251, 191, 36);

        p.keycapBg     = Gdiplus::Color(255, 50, 56, 72);
        p.keycapBorder = Gdiplus::Color(255, 78, 86, 108);
        p.btnSecBg     = Gdiplus::Color(255, 46, 51, 65);
        p.btnSecHover  = Gdiplus::Color(255, 56, 62, 79);
        p.btnSecBorder = Gdiplus::Color(255, 68, 75, 95);

        p.captionRef   = RGB(23, 25, 32);
        p.borderRef    = RGB(52, 58, 74);
        p.inputBgRef   = RGB(28, 31, 40);
        p.inputTextRef = RGB(240, 242, 248);
        p.cardBgRef    = RGB(38, 42, 54);
    } else {
        p.sidebarBg    = Gdiplus::Color(255, 237, 240, 245);
        p.canvasBg     = Gdiplus::Color(255, 246, 247, 250);
        p.cardBg       = Gdiplus::Color(255, 255, 255, 255);
        p.cardBorder   = Gdiplus::Color(255, 222, 227, 236);
        p.cardHoverBg  = Gdiplus::Color(255, 243, 246, 252);
        p.inputBg      = Gdiplus::Color(255, 255, 255, 255);
        p.inputBorder  = Gdiplus::Color(255, 203, 210, 222);
        p.divider      = Gdiplus::Color(255, 226, 230, 238);

        p.textPrimary   = Gdiplus::Color(255, 22, 26, 34);
        p.textSecondary = Gdiplus::Color(255, 90, 99, 115);
        p.textMuted     = Gdiplus::Color(255, 128, 137, 152);

        p.accent       = Gdiplus::Color(255, 0, 103, 192);
        p.accentHover  = Gdiplus::Color(255, 20, 118, 206);
        p.accentPress  = Gdiplus::Color(255, 0, 86, 162);
        p.accentSoftBg = Gdiplus::Color(24, 0, 103, 192);

        p.danger       = Gdiplus::Color(255, 210, 48, 62);
        p.dangerHover  = Gdiplus::Color(255, 224, 62, 76);
        p.dangerPress  = Gdiplus::Color(255, 182, 34, 48);

        p.success      = Gdiplus::Color(255, 16, 156, 98);
        p.successBg    = Gdiplus::Color(28, 16, 156, 98);
        p.warn         = Gdiplus::Color(255, 202, 138, 4);
        p.warnBg       = Gdiplus::Color(30, 202, 138, 4);

        p.keycapBg     = Gdiplus::Color(255, 241, 244, 249);
        p.keycapBorder = Gdiplus::Color(255, 202, 210, 224);
        p.btnSecBg     = Gdiplus::Color(255, 255, 255, 255);
        p.btnSecHover  = Gdiplus::Color(255, 243, 245, 250);
        p.btnSecBorder = Gdiplus::Color(255, 208, 214, 226);

        p.captionRef   = RGB(237, 240, 245);
        p.borderRef    = RGB(212, 218, 228);
        p.inputBgRef   = RGB(255, 255, 255);
        p.inputTextRef = RGB(22, 26, 34);
        p.cardBgRef    = RGB(255, 255, 255);
    }
    return p;
}

float EaseOutCubic(float t) {
    float inv = 1.0f - std::clamp(t, 0.0f, 1.0f);
    return 1.0f - inv * inv * inv;
}

Gdiplus::Color WithAlpha(Gdiplus::Color c, float factor) {
    int a = (int)std::round((float)c.GetAlpha() * std::clamp(factor, 0.0f, 1.0f));
    return Gdiplus::Color((BYTE)std::clamp(a, 0, 255), c.GetR(), c.GetG(), c.GetB());
}

Gdiplus::Color LerpColor(Gdiplus::Color a, Gdiplus::Color b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerp = [t](BYTE x, BYTE y) -> BYTE {
        return (BYTE)std::clamp((int)std::round(x + (y - x) * t), 0, 255);
    };
    return Gdiplus::Color(
        lerp(a.GetA(), b.GetA()),
        lerp(a.GetR(), b.GetR()),
        lerp(a.GetG(), b.GetG()),
        lerp(a.GetB(), b.GetB()));
}

void AddRoundRect(Gdiplus::GraphicsPath& path, float x, float y, float w, float h, float r) {
    r = std::min(r, std::min(w, h) * 0.5f);
    float d = r * 2.0f;
    path.AddArc(x, y, d, d, 180.0f, 90.0f);
    path.AddArc(x + w - d, y, d, d, 270.0f, 90.0f);
    path.AddArc(x + w - d, y + h - d, d, d, 0.0f, 90.0f);
    path.AddArc(x, y + h - d, d, d, 90.0f, 90.0f);
    path.CloseFigure();
}

void FillRoundRect(Gdiplus::Graphics& g, float x, float y, float w, float h, float r, Gdiplus::Color fill) {
    Gdiplus::GraphicsPath p;
    AddRoundRect(p, x, y, w, h, r);
    Gdiplus::SolidBrush br(fill);
    g.FillPath(&br, &p);
}

void DrawRoundRect(Gdiplus::Graphics& g, float x, float y, float w, float h, float r, Gdiplus::Color stroke, float width = 1.0f) {
    Gdiplus::GraphicsPath p;
    AddRoundRect(p, x, y, w, h, r);
    Gdiplus::Pen pen(stroke, width);
    g.DrawPath(&pen, &p);
}

void DrawTextBox(Gdiplus::Graphics& g, const std::wstring& text, const Gdiplus::Font& font,
                 Gdiplus::Color col, float x, float y, float w, float h,
                 Gdiplus::StringAlignment hAlign = Gdiplus::StringAlignmentNear,
                 Gdiplus::StringAlignment vAlign = Gdiplus::StringAlignmentCenter,
                 bool ellipsis = true) {
    Gdiplus::SolidBrush br(col);
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(hAlign);
    fmt.SetLineAlignment(vAlign);
    if (ellipsis) fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    if (vAlign == Gdiplus::StringAlignmentCenter) {
        fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    }
    Gdiplus::RectF rf(x, y, w, h);
    g.DrawString(text.c_str(), (INT)text.size(), &font, rf, &fmt, &br);
}

std::wstring GetEditStr(HWND h) {
    if (!h) return {};
    int len = GetWindowTextLengthW(h);
    if (len <= 0) return {};
    std::wstring s(len, L'\0');
    GetWindowTextW(h, s.data(), len + 1);
    return s;
}

// Vector icons drawn directly with GDI+ at any DPI
void DrawNavIcon(Gdiplus::Graphics& g, int index, float x, float y, float s, Gdiplus::Color col) {
    Gdiplus::Pen pen(col, std::max(1.4f, s * 0.10f));
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);

    if (index == 0) {
        Gdiplus::PointF pts[6] = {
            {x + s * 0.56f, y + s * 0.08f},
            {x + s * 0.18f, y + s * 0.54f},
            {x + s * 0.48f, y + s * 0.54f},
            {x + s * 0.40f, y + s * 0.92f},
            {x + s * 0.82f, y + s * 0.44f},
            {x + s * 0.52f, y + s * 0.44f}
        };
        g.DrawPolygon(&pen, pts, 6);
    } else if (index == 1) {
        DrawRoundRect(g, x + s * 0.10f, y + s * 0.14f, s * 0.62f, s * 0.52f, s * 0.12f, col, pen.GetWidth());
        Gdiplus::GraphicsPath back;
        AddRoundRect(back, x + s * 0.28f, y + s * 0.34f, s * 0.62f, s * 0.52f, s * 0.12f);
        g.DrawPath(&pen, &back);
    } else {
        g.DrawArc(&pen, x + s * 0.10f, y + s * 0.16f, s * 0.80f, s * 0.80f, 150.0f, 240.0f);
        g.DrawLine(&pen, x + s * 0.50f, y + s * 0.58f, x + s * 0.72f, y + s * 0.36f);
        Gdiplus::SolidBrush dot(col);
        g.FillEllipse(&dot, x + s * 0.43f, y + s * 0.51f, s * 0.14f, s * 0.14f);
    }
}

void DrawDecoyTileIcon(Gdiplus::Graphics& g, int decoyIdx, float x, float y, float s, Gdiplus::Color col) {
    Gdiplus::Pen pen(col, std::max(1.4f, s * 0.095f));
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);

    switch (decoyIdx) {
        case 0:
            DrawRoundRect(g, x + s * 0.16f, y + s * 0.08f, s * 0.68f, s * 0.84f, s * 0.12f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.32f, y + s * 0.34f, x + s * 0.68f, y + s * 0.34f);
            g.DrawLine(&pen, x + s * 0.32f, y + s * 0.54f, x + s * 0.68f, y + s * 0.54f);
            g.DrawLine(&pen, x + s * 0.32f, y + s * 0.72f, x + s * 0.54f, y + s * 0.72f);
            break;
        case 1:
            DrawRoundRect(g, x + s * 0.08f, y + s * 0.16f, s * 0.84f, s * 0.68f, s * 0.14f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.26f, y + s * 0.38f, x + s * 0.42f, y + s * 0.50f);
            g.DrawLine(&pen, x + s * 0.42f, y + s * 0.50f, x + s * 0.26f, y + s * 0.62f);
            g.DrawLine(&pen, x + s * 0.50f, y + s * 0.62f, x + s * 0.72f, y + s * 0.62f);
            break;
        case 2:
            DrawRoundRect(g, x + s * 0.10f, y + s * 0.14f, s * 0.80f, s * 0.72f, s * 0.12f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.10f, y + s * 0.40f, x + s * 0.90f, y + s * 0.40f);
            g.DrawLine(&pen, x + s * 0.10f, y + s * 0.64f, x + s * 0.90f, y + s * 0.64f);
            g.DrawLine(&pen, x + s * 0.44f, y + s * 0.14f, x + s * 0.44f, y + s * 0.86f);
            break;
        case 3:
            DrawRoundRect(g, x + s * 0.10f, y + s * 0.16f, s * 0.80f, s * 0.68f, s * 0.12f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.18f, y + s * 0.72f, x + s * 0.42f, y + s * 0.46f);
            g.DrawLine(&pen, x + s * 0.42f, y + s * 0.46f, x + s * 0.60f, y + s * 0.62f);
            g.DrawLine(&pen, x + s * 0.60f, y + s * 0.62f, x + s * 0.74f, y + s * 0.50f);
            g.DrawLine(&pen, x + s * 0.74f, y + s * 0.50f, x + s * 0.84f, y + s * 0.72f);
            break;
        case 4:
            DrawRoundRect(g, x + s * 0.10f, y + s * 0.24f, s * 0.80f, s * 0.60f, s * 0.12f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.36f, y + s * 0.14f, x + s * 0.64f, y + s * 0.14f);
            g.DrawLine(&pen, x + s * 0.50f, y + s * 0.42f, x + s * 0.50f, y + s * 0.66f);
            g.DrawLine(&pen, x + s * 0.38f, y + s * 0.54f, x + s * 0.62f, y + s * 0.54f);
            break;
        case 5:
            g.DrawEllipse(&pen, x + s * 0.12f, y + s * 0.12f, s * 0.76f, s * 0.76f);
            g.DrawEllipse(&pen, x + s * 0.32f, y + s * 0.12f, s * 0.36f, s * 0.76f);
            g.DrawLine(&pen, x + s * 0.14f, y + s * 0.50f, x + s * 0.86f, y + s * 0.50f);
            break;
        case 6:
            DrawRoundRect(g, x + s * 0.18f, y + s * 0.18f, s * 0.64f, s * 0.64f, s * 0.12f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.50f, y + s * 0.06f, x + s * 0.50f, y + s * 0.26f);
            g.DrawLine(&pen, x + s * 0.50f, y + s * 0.74f, x + s * 0.50f, y + s * 0.94f);
            g.DrawLine(&pen, x + s * 0.06f, y + s * 0.50f, x + s * 0.26f, y + s * 0.50f);
            g.DrawLine(&pen, x + s * 0.74f, y + s * 0.50f, x + s * 0.94f, y + s * 0.50f);
            break;
        default:
            DrawRoundRect(g, x + s * 0.10f, y + s * 0.14f, s * 0.80f, s * 0.56f, s * 0.10f, col, pen.GetWidth());
            g.DrawLine(&pen, x + s * 0.50f, y + s * 0.70f, x + s * 0.50f, y + s * 0.86f);
            g.DrawLine(&pen, x + s * 0.30f, y + s * 0.86f, x + s * 0.70f, y + s * 0.86f);
            break;
    }
}

} // namespace

SettingsWindow::~SettingsWindow() {
    Destroy();
}

void SettingsWindow::Destroy() {
    if (hwnd_) {
        KillTimer(hwnd_, kAnimTimerId);
        KillTimer(hwnd_, kLiveStatsTimerId);
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    children_.clear();
    if (fontInput_) { DeleteObject(fontInput_); fontInput_ = nullptr; }
    if (fontMono_) { DeleteObject(fontMono_); fontMono_ = nullptr; }
    if (inputBgBrush_) { DeleteObject(inputBgBrush_); inputBgBrush_ = nullptr; }
    if (cardBgBrush_) { DeleteObject(cardBgBrush_); cardBgBrush_ = nullptr; }
    if (iconBig_) { DestroyIcon(iconBig_); iconBig_ = nullptr; }
    if (iconSmall_) { DestroyIcon(iconSmall_); iconSmall_ = nullptr; }
}

void SettingsWindow::ApplyTheme(bool dark) {
    dark_ = dark;
    Palette pal = MakePalette(dark_);

    if (inputBgBrush_) DeleteObject(inputBgBrush_);
    if (cardBgBrush_) DeleteObject(cardBgBrush_);
    inputBgBrush_ = CreateSolidBrush(pal.inputBgRef);
    cardBgBrush_ = CreateSolidBrush(pal.cardBgRef);

    if (hwnd_) {
        utils::ApplyWin11ModernFrame(hwnd_, dark_, pal.captionRef, pal.borderRef);

        const wchar_t* themeName = dark_ ? L"DarkMode_CFD" : L"Explorer";
        const wchar_t* scrollTheme = dark_ ? L"DarkMode_Explorer" : L"Explorer";
        for (const auto& ch : children_) {
            if (!ch.hwnd) continue;
            wchar_t cls[32]{};
            GetClassNameW(ch.hwnd, cls, 32);
            if (_wcsicmp(cls, L"ComboBox") == 0) {
                SetWindowTheme(ch.hwnd, themeName, nullptr);
            } else {
                SetWindowTheme(ch.hwnd, scrollTheme, nullptr);
            }
            InvalidateRect(ch.hwnd, nullptr, TRUE);
        }
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
}

void SettingsWindow::UpdateComboLanguageTexts() {
    if (comboMouseTrigger_) {
        int cur = (int)SendMessageW(comboMouseTrigger_, CB_GETCURSEL, 0, 0);
        if (cur < 0) cur = (int)workingCfg_.mouseTrigger;
        SendMessageW(comboMouseTrigger_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboMouseTrigger_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::MouseTriggerDisabled));
        SendMessageW(comboMouseTrigger_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::MouseTriggerX1));
        SendMessageW(comboMouseTrigger_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::MouseTriggerX2));
        SendMessageW(comboMouseTrigger_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::MouseTriggerAnyX));
        SendMessageW(comboMouseTrigger_, CB_SETCURSEL, std::clamp(cur, 0, 3), 0);
    }

    if (comboLanguage_) {
        int cur = (int)SendMessageW(comboLanguage_, CB_GETCURSEL, 0, 0);
        if (cur < 0) cur = workingCfg_.language;
        SendMessageW(comboLanguage_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboLanguage_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::LangAuto));
        SendMessageW(comboLanguage_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::LangEnglish));
        SendMessageW(comboLanguage_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::LangSpanish));
        SendMessageW(comboLanguage_, CB_SETCURSEL, std::clamp(cur, 0, 2), 0);
    }

    bool sp = i18n::IsSpanish();
    if (comboNotesTpl_) {
        int cur = std::max(0, (int)SendMessageW(comboNotesTpl_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboNotesTpl_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboNotesTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Informe Trimestral Q4" : L"Q4 Operations Report"));
        SendMessageW(comboNotesTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Acta de Reunión" : L"Meeting Minutes"));
        SendMessageW(comboNotesTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Plan de Proyecto" : L"Sprint Roadmap"));
        SendMessageW(comboNotesTpl_, CB_SETCURSEL, cur, 0);
    }

    if (comboNotesTheme_) {
        int cur = std::max(0, (int)SendMessageW(comboNotesTheme_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboNotesTheme_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboNotesTheme_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyNotesThemeAuto));
        SendMessageW(comboNotesTheme_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyNotesThemeLight));
        SendMessageW(comboNotesTheme_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyNotesThemeDark));
        SendMessageW(comboNotesTheme_, CB_SETCURSEL, cur, 0);
    }

    if (comboTermTpl_) {
        int cur = std::max(0, (int)SendMessageW(comboTermTpl_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboTermTpl_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboTermTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Compilación C++ / CMake" : L"C++ / CMake Build"));
        SendMessageW(comboTermTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Despliegue Docker / K8s" : L"Docker / K8s Rollout"));
        SendMessageW(comboTermTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Auditoría de Sistema" : L"System Health Audit"));
        SendMessageW(comboTermTpl_, CB_SETCURSEL, cur, 0);
    }

    if (comboSheetTpl_) {
        int cur = std::max(0, (int)SendMessageW(comboSheetTpl_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboSheetTpl_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboSheetTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Indicadores Trimestrales (KPIs)" : L"Quarterly Metrics (KPIs)"));
        SendMessageW(comboSheetTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Seguimiento de Proyecto" : L"Project Tracking"));
        SendMessageW(comboSheetTpl_, CB_ADDSTRING, 0, (LPARAM)(sp ? L"Presupuesto Financiero" : L"Financial Budget"));
        SendMessageW(comboSheetTpl_, CB_SETCURSEL, cur, 0);
    }

    if (comboExtRestore_) {
        int cur = std::max(0, (int)SendMessageW(comboExtRestore_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboExtRestore_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboExtRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionKeepHidden));
        SendMessageW(comboExtRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionClose));
        SendMessageW(comboExtRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionLeaveOpen));
        SendMessageW(comboExtRestore_, CB_SETCURSEL, cur, 0);
    }

    if (comboUrlMode_) {
        int cur = std::max(0, (int)SendMessageW(comboUrlMode_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboUrlMode_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboUrlMode_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyUrlModeNewWindow));
        SendMessageW(comboUrlMode_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyUrlModeAppWindow));
        SendMessageW(comboUrlMode_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyUrlModeDefaultBrowser));
        SendMessageW(comboUrlMode_, CB_SETCURSEL, cur, 0);
    }

    if (comboUrlRestore_) {
        int cur = std::max(0, (int)SendMessageW(comboUrlRestore_, CB_GETCURSEL, 0, 0));
        SendMessageW(comboUrlRestore_, CB_RESETCONTENT, 0, 0);
        SendMessageW(comboUrlRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionKeepHidden));
        SendMessageW(comboUrlRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionClose));
        SendMessageW(comboUrlRestore_, CB_ADDSTRING, 0, (LPARAM)i18n::Get(i18n::StrId::DecoyExtActionLeaveOpen));
        SendMessageW(comboUrlRestore_, CB_SETCURSEL, cur, 0);
    }
}

void SettingsWindow::Show(HINSTANCE hInst, const AppConfig& cfg, SettingsCallbacks callbacks) {
    hInst_ = hInst;
    workingCfg_ = cfg;
    cb_ = std::move(callbacks);
    i18n::SetConfiguredLanguage(static_cast<i18n::Language>(cfg.language));

    static bool registered = false;
    if (!registered) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = SettingsWindow::WndProc;
        wc.hInstance = hInst;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = kSettingsClassName;
        RegisterClassExW(&wc);
        registered = true;
    }

    if (!hwnd_) {
        HDC sc = GetDC(nullptr);
        dpi_ = sc ? (UINT)GetDeviceCaps(sc, LOGPIXELSX) : 96;
        if (sc) ReleaseDC(nullptr, sc);

        DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
        RECT wr{0, 0, utils::Scale(kClientW, dpi_), utils::Scale(kClientH, dpi_)};
        AdjustWindowRectExForDpi(&wr, style, FALSE, 0, dpi_);
        int winW = wr.right - wr.left;
        int winH = wr.bottom - wr.top;
        int sx = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
        int sy = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

        hwnd_ = CreateWindowExW(
            0, kSettingsClassName, i18n::Get(i18n::StrId::AppName),
            style, sx, sy, winW, winH,
            nullptr, nullptr, hInst, this);

        iconBig_ = utils::CreateAppIcon(32, false);
        iconSmall_ = utils::CreateAppIcon(16, false);
        SendMessageW(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)iconBig_);
        SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)iconSmall_);

        fontInput_ = utils::CreateUIFont(dpi_, 9, false, L"Segoe UI");
        fontMono_ = utils::CreateUIFont(dpi_, 9, false, L"Consolas");

        CreateChildControls();
    }

    PopulateFromConfig(cfg);
    RefreshOpenWindowsList();

    bool wantDark = (cfg.uiTheme == 2) || (cfg.uiTheme == 0 && utils::IsSystemDarkMode());
    ApplyTheme(wantDark);

    // Subtle entrance slide when opening settings
    pageAnim_ = 0.0f;
    pillY_ = pillTargetY_ = 160.0f + activeTab_ * 50.0f;
    LayoutActiveChildren(utils::Scale(12, dpi_));
    SetTimer(hwnd_, kLiveStatsTimerId, 2000, nullptr);
    StartAnimTimer();

    ShowWindow(hwnd_, SW_SHOWNORMAL);
    utils::ForceForegroundWindow(hwnd_);
}

void SettingsWindow::Hide() {
    if (hwnd_) {
        EndHotkeyCapture();
        KillTimer(hwnd_, kAnimTimerId);
        KillTimer(hwnd_, kLiveStatsTimerId);
        animTimerRunning_ = false;
        if (IsIconic(hwnd_) || IsZoomed(hwnd_)) {
            ShowWindow(hwnd_, SW_RESTORE);
        }
        ShowWindow(hwnd_, SW_HIDE);
        SetWindowPos(hwnd_, HWND_NOTOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
        SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
    }
}

void SettingsWindow::CreateChildControls() {
    auto addEdit = [&](int tab, int decoyFilter, int x, int y, int w, int h,
                       bool multiline, bool mono = false) -> HWND {
        DWORD st = WS_CHILD | WS_TABSTOP | ES_LEFT;
        if (multiline) {
            st |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN;
        } else {
            st |= ES_AUTOHSCROLL;
        }
        HWND ctrl = CreateWindowExW(0, L"EDIT", L"", st, 0, 0, 10, 10, hwnd_, nullptr, hInst_, nullptr);
        SendMessageW(ctrl, WM_SETFONT, (WPARAM)(mono ? fontMono_ : fontInput_), TRUE);
        SendMessageW(ctrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(4, 4));
        children_.push_back({ctrl, tab, decoyFilter, x, y, w, h});
        return ctrl;
    };

    auto addCombo = [&](int tab, int decoyFilter, int x, int y, int w, int dropH) -> HWND {
        HWND ctrl = CreateWindowExW(0, L"COMBOBOX", L"",
                                    WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
                                    0, 0, 10, 10, hwnd_, nullptr, hInst_, nullptr);
        SendMessageW(ctrl, WM_SETFONT, (WPARAM)fontInput_, TRUE);
        children_.push_back({ctrl, tab, decoyFilter, x, y, w, dropH});
        return ctrl;
    };

    // === TAB 0: Mouse Trigger and Language ===
    comboMouseTrigger_ = addCombo(0, -1, 566, 238, 276, 140);
    comboLanguage_ = addCombo(0, -1, 470, 534, 180, 140);

    // === TAB 1: Builtin Notes (decoy 0) ===
    editNotesTitle_ = addEdit(1, 0, 266, 304, 576, 22, false);
    comboNotesTpl_ = addCombo(1, 0, 266, 344, 210, 140);
    comboNotesTheme_ = addCombo(1, 0, 678, 344, 164, 140);
    editNotesContent_ = addEdit(1, 0, 266, 404, 576, 130, true, true);

    // === TAB 1: Builtin Terminal (decoy 1) ===
    editTermTitle_ = addEdit(1, 1, 266, 304, 576, 22, false);
    comboTermTpl_ = addCombo(1, 1, 266, 344, 210, 140);
    editTermContent_ = addEdit(1, 1, 266, 404, 576, 130, true, true);

    // === TAB 1: Builtin Spreadsheet (decoy 2) ===
    editSheetTitle_ = addEdit(1, 2, 266, 314, 576, 22, false);
    comboSheetTpl_ = addCombo(1, 2, 266, 376, 360, 140);

    // === TAB 1: Builtin Image (decoy 3) ===
    editImagePath_ = addEdit(1, 3, 266, 314, 466, 22, false);

    // === TAB 1: External App / File (decoy 4) ===
    editExtPath_ = addEdit(1, 4, 266, 304, 466, 22, false);
    editExtArgs_ = addEdit(1, 4, 266, 360, 576, 22, false);
    comboExtRestore_ = addCombo(1, 4, 424, 468, 250, 140);

    // === TAB 1: External URL (decoy 5) ===
    editUrlAddr_ = addEdit(1, 5, 266, 304, 576, 22, false);
    comboUrlMode_ = addCombo(1, 5, 266, 360, 576, 140);
    comboUrlRestore_ = addCombo(1, 5, 424, 444, 250, 140);

    // === TAB 1: Capture Window (decoy 6) ===
    comboOpenWindows_ = addCombo(1, 6, 266, 310, 464, 220);

    // === TAB 2: Exclusions ===
    editExclusions_ = addEdit(2, -1, 266, 144, 576, 78, true, true);
}

void SettingsWindow::PopulateFromConfig(const AppConfig& cfg) {
    workingCfg_ = cfg;
    uiThemePref_ = cfg.uiTheme;
    i18n::SetConfiguredLanguage(static_cast<i18n::Language>(cfg.language));

    recPanicMods_ = cfg.panicModifiers;
    recPanicVk_ = cfg.panicVk;
    recSettingsMods_ = cfg.settingsModifiers;
    recSettingsVk_ = cfg.settingsVk;

    auto setTog = [](ToggleState& t, bool v) { t.value = v; t.visual = v ? 1.0f : 0.0f; };
    setTog(togMute_, cfg.muteAudioOnPanic);
    setTog(togPauseMedia_, cfg.pauseMediaOnPanic);
    setTog(togStealth_, cfg.stealthTray);
    setTog(togStartWin_, cfg.startWithWindows);
    setTog(togStartTip_, cfg.showStartupNotification);
    setTog(togTermSim_, cfg.terminalSimulateActivity);
    setTog(togImgFull_, cfg.imageFullscreen);
    setTog(togExtMax_, cfg.externalMaximized);
    setTog(togExtPrewarm_, cfg.prewarmExternal);
    setTog(togUrlPrewarm_, cfg.prewarmUrl);
    setTog(togCapKeepVis_, cfg.capturedKeepVisibleUntilPanic);

    selDecoy_ = cfg.decoyType;

    UpdateComboLanguageTexts();

    SetWindowTextW(editNotesTitle_, cfg.notesTitle.c_str());
    SetWindowTextW(editNotesContent_, cfg.notesContent.c_str());

    SetWindowTextW(editTermTitle_, cfg.terminalTitle.c_str());
    SetWindowTextW(editTermContent_, cfg.terminalContent.c_str());

    SetWindowTextW(editSheetTitle_, cfg.sheetTitle.c_str());

    SetWindowTextW(editImagePath_, cfg.imagePath.c_str());

    SetWindowTextW(editExtPath_, cfg.externalPath.c_str());
    SetWindowTextW(editExtArgs_, cfg.externalArgs.c_str());

    SetWindowTextW(editUrlAddr_, cfg.urlAddress.c_str());

    SetWindowTextW(editExclusions_, cfg.excludedProcesses.c_str());
}

AppConfig SettingsWindow::GatherConfig() const {
    AppConfig cfg = workingCfg_;
    cfg.uiTheme = uiThemePref_;
    cfg.language = std::max(0, (int)SendMessageW(comboLanguage_, CB_GETCURSEL, 0, 0));
    cfg.mouseTrigger = static_cast<MouseTrigger>(std::max(0, (int)SendMessageW(comboMouseTrigger_, CB_GETCURSEL, 0, 0)));

    cfg.panicModifiers = recPanicMods_;
    cfg.panicVk = recPanicVk_;
    cfg.settingsModifiers = recSettingsMods_;
    cfg.settingsVk = recSettingsVk_;

    cfg.muteAudioOnPanic = togMute_.value;
    cfg.pauseMediaOnPanic = togPauseMedia_.value;
    cfg.stealthTray = togStealth_.value;
    cfg.startWithWindows = togStartWin_.value;
    cfg.showStartupNotification = togStartTip_.value;

    cfg.decoyType = selDecoy_;

    cfg.notesTitle = GetEditStr(editNotesTitle_);
    cfg.notesTheme = std::max(0, (int)SendMessageW(comboNotesTheme_, CB_GETCURSEL, 0, 0));
    cfg.notesContent = GetEditStr(editNotesContent_);

    cfg.terminalTitle = GetEditStr(editTermTitle_);
    cfg.terminalSimulateActivity = togTermSim_.value;
    cfg.terminalContent = GetEditStr(editTermContent_);

    cfg.sheetTitle = GetEditStr(editSheetTitle_);
    cfg.sheetTemplate = std::max(0, (int)SendMessageW(comboSheetTpl_, CB_GETCURSEL, 0, 0));

    cfg.imagePath = GetEditStr(editImagePath_);
    cfg.imageFullscreen = togImgFull_.value;

    cfg.externalPath = GetEditStr(editExtPath_);
    cfg.externalArgs = GetEditStr(editExtArgs_);
    cfg.externalMaximized = togExtMax_.value;
    cfg.prewarmExternal = togExtPrewarm_.value;

    cfg.urlAddress = GetEditStr(editUrlAddr_);
    int um = (int)SendMessageW(comboUrlMode_, CB_GETCURSEL, 0, 0);
    if (um >= 0 && um <= 2) cfg.urlBrowserMode = (UrlBrowserMode)um;
    cfg.prewarmUrl = togUrlPrewarm_.value;

    if (cfg.decoyType == DecoyType::ExternalUrl) {
        int ra = (int)SendMessageW(comboUrlRestore_, CB_GETCURSEL, 0, 0);
        if (ra >= 0 && ra <= 2) cfg.decoyRestoreAction = (DecoyRestoreAction)ra;
    } else {
        int ra = (int)SendMessageW(comboExtRestore_, CB_GETCURSEL, 0, 0);
        if (ra >= 0 && ra <= 2) cfg.decoyRestoreAction = (DecoyRestoreAction)ra;
    }

    cfg.capturedKeepVisibleUntilPanic = togCapKeepVis_.value;
    cfg.excludedProcesses = GetEditStr(editExclusions_);
    return cfg;
}

void SettingsWindow::SelectTab(int newTab) {
    if (newTab < 0 || newTab > 2 || newTab == activeTab_) return;
    EndHotkeyCapture();
    activeTab_ = newTab;
    pillTargetY_ = 160.0f + activeTab_ * 50.0f;
    pageAnim_ = 0.0f;
    subCardAnim_ = 1.0f;
    if (activeTab_ == 1) RefreshOpenWindowsList();
    LayoutActiveChildren(utils::Scale(14, dpi_));
    StartAnimTimer();
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void SettingsWindow::SelectDecoy(DecoyType newType) {
    if (selDecoy_ == newType) return;
    selDecoy_ = newType;
    subCardAnim_ = 0.0f;
    if (selDecoy_ == DecoyType::CapturedWindow) RefreshOpenWindowsList();
    LayoutActiveChildren(0);
    StartAnimTimer();
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void SettingsWindow::StartAnimTimer() {
    if (!hwnd_) return;
    lastAnimTick_ = GetTickCount64();
    if (!animTimerRunning_) {
        SetTimer(hwnd_, kAnimTimerId, 15, nullptr);
        animTimerRunning_ = true;
    }
}

void SettingsWindow::StopAnimTimerIfIdle() {
    if (animTimerRunning_ && hwnd_) {
        KillTimer(hwnd_, kAnimTimerId);
        animTimerRunning_ = false;
    }
}

void SettingsWindow::ShowToast(const std::wstring& msg, bool isError) {
    toastText_ = msg;
    toastIsError_ = isError;
    toastExpireTick_ = GetTickCount64() + (isError ? 4200 : 2800);
    StartAnimTimer();
}

void SettingsWindow::LayoutActiveChildren(int slideOffsetPx) {
    if (!hwnd_ || children_.empty()) return;

    float subEase = EaseOutCubic(subCardAnim_);
    int subOffsetPx = utils::Scale((int)std::round((1.0f - subEase) * 10.0f), dpi_);

    HDWP hdwp = BeginDeferWindowPos((int)children_.size());
    for (const auto& ch : children_) {
        bool vis = (ch.tab == activeTab_) &&
                   (ch.decoyFilter == -1 || ch.decoyFilter == (int)selDecoy_);
        if (vis) {
            int dy = (ch.decoyFilter != -1) ? (slideOffsetPx + subOffsetPx) : slideOffsetPx;
            int x = utils::Scale(ch.baseX, dpi_);
            int y = utils::Scale(ch.baseY, dpi_) + dy;
            int w = utils::Scale(ch.baseW, dpi_);
            int h = utils::Scale(ch.baseH, dpi_);
            hdwp = DeferWindowPos(hdwp, ch.hwnd, nullptr, x, y, w, h,
                                  SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
        } else {
            if (IsWindowVisible(ch.hwnd)) {
                hdwp = DeferWindowPos(hdwp, ch.hwnd, nullptr, 0, 0, 0, 0,
                                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
            }
        }
    }
    if (hdwp) EndDeferWindowPos(hdwp);
}

void SettingsWindow::TickAnimations() {
    ULONGLONG now = GetTickCount64();
    float dt = std::clamp((float)(now - lastAnimTick_) / 1000.0f, 0.001f, 0.05f);
    lastAnimTick_ = now;

    bool needMore = false;

    // 1. Sliding nav pill
    if (std::fabs(pillY_ - pillTargetY_) > 0.25f) {
        pillY_ += (pillTargetY_ - pillY_) * std::min(1.0f, dt * 18.0f);
        needMore = true;
    } else {
        pillY_ = pillTargetY_;
    }

    // 2. Page entrance transition (~190ms)
    bool layoutMoved = false;
    if (pageAnim_ < 1.0f) {
        pageAnim_ = std::min(1.0f, pageAnim_ + dt * 5.4f);
        layoutMoved = true;
        if (pageAnim_ < 1.0f) needMore = true;
    }

    // 3. Decoy sub-card transition (~160ms)
    if (subCardAnim_ < 1.0f) {
        subCardAnim_ = std::min(1.0f, subCardAnim_ + dt * 6.4f);
        layoutMoved = true;
        if (subCardAnim_ < 1.0f) needMore = true;
    }

    if (layoutMoved) {
        float ease = EaseOutCubic(pageAnim_);
        int slidePx = utils::Scale((int)std::round((1.0f - ease) * 14.0f), dpi_);
        LayoutActiveChildren(slidePx);
    }

    // 4. Toggle switch knobs
    ToggleState* allToggles[] = {
        &togMute_, &togPauseMedia_, &togStealth_, &togStartWin_, &togStartTip_,
        &togTermSim_, &togImgFull_, &togExtMax_, &togExtPrewarm_,
        &togUrlPrewarm_, &togCapKeepVis_
    };
    for (auto* t : allToggles) {
        float target = t->value ? 1.0f : 0.0f;
        if (std::fabs(t->visual - target) > 0.01f) {
            t->visual += (target - t->visual) * std::min(1.0f, dt * 18.0f);
            if (std::fabs(t->visual - target) <= 0.01f) t->visual = target;
            else needMore = true;
        }
    }

    // 5. Toast fade-in / hold / fade-out
    if (!toastText_.empty()) {
        if (now < toastExpireTick_) {
            if (toastAlpha_ < 1.0f) {
                toastAlpha_ = std::min(1.0f, toastAlpha_ + dt * 7.5f);
            }
            needMore = true;
        } else {
            toastAlpha_ = std::max(0.0f, toastAlpha_ - dt * 5.0f);
            if (toastAlpha_ > 0.0f) {
                needMore = true;
            } else {
                toastText_.clear();
            }
        }
    }

    // 6. Hotkey capture pulse
    if (captureSlot_ != 0) {
        needMore = true;
    }

    InvalidateRect(hwnd_, nullptr, FALSE);
    if (!needMore) {
        StopAnimTimerIfIdle();
    }
}

void SettingsWindow::RefreshStatus(const AppConfig& cfg) {
    workingCfg_ = cfg;
    if (hwnd_ && IsWindowVisible(hwnd_)) {
        InvalidateRect(hwnd_, nullptr, FALSE);
    }
}

void SettingsWindow::RefreshOpenWindowsList() {
    if (!comboOpenWindows_) return;
    SendMessageW(comboOpenWindows_, CB_RESETCONTENT, 0, 0);
    openWindowHandles_.clear();
    auto list = WindowManager::EnumerateUserWindows();
    for (const auto& w : list) {
        std::wstring label = w.title;
        if (!w.processName.empty()) label += L"  [" + w.processName + L"]";
        SendMessageW(comboOpenWindows_, CB_ADDSTRING, 0, (LPARAM)label.c_str());
        openWindowHandles_.push_back(w.hwnd);
    }
    if (!openWindowHandles_.empty()) {
        SendMessageW(comboOpenWindows_, CB_SETCURSEL, 0, 0);
    }
}

void SettingsWindow::BrowseForFile(HWND targetEdit, const wchar_t* filter, const wchar_t* title) {
    wchar_t buf[MAX_PATH * 4]{};
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd_;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = (DWORD)std::size(buf);
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&ofn)) {
        SetWindowTextW(targetEdit, buf);
    }
}

void SettingsWindow::BeginHotkeyCapture(int slot) {
    if (captureSlot_ == slot) return;
    bool wasCapturing = (captureSlot_ != 0);
    captureSlot_ = slot;
    if (!wasCapturing && cb_.onBeginHotkeyCapture) {
        cb_.onBeginHotkeyCapture();
    }
    SetFocus(hwnd_);
    StartAnimTimer();
    InvalidateRect(hwnd_, nullptr, FALSE);
}

void SettingsWindow::EndHotkeyCapture() {
    if (captureSlot_ == 0) return;
    captureSlot_ = 0;
    if (cb_.onEndHotkeyCapture) {
        cb_.onEndHotkeyCapture();
    }
    InvalidateRect(hwnd_, nullptr, FALSE);
}

SettingsWindow::HitId SettingsWindow::HitTest(int x, int y) const {
    POINT pt{x, y};
    for (auto it = hitRegions_.rbegin(); it != hitRegions_.rend(); ++it) {
        if (it->enabled && PtInRect(&it->rc, pt)) {
            return it->id;
        }
    }
    return HitId::None;
}

void SettingsWindow::HandleHitClick(HitId id) {
    if (id != HitId::HotkeyPanicBox && id != HitId::HotkeySettingsBox) {
        EndHotkeyCapture();
    }

    auto toggle = [&](ToggleState& t) {
        t.value = !t.value;
        StartAnimTimer();
        InvalidateRect(hwnd_, nullptr, FALSE);
    };

    switch (id) {
        case HitId::NavGeneral: SelectTab(0); break;
        case HitId::NavDecoy:   SelectTab(1); break;
        case HitId::NavEngine:  SelectTab(2); break;

        case HitId::BtnThemeToggle:
            uiThemePref_ = dark_ ? 1 : 2;
            ApplyTheme(!dark_);
            break;

        case HitId::BtnTestPanic: {
            AppConfig newCfg = GatherConfig();
            std::wstring err;
            if (cb_.onApplyConfig && cb_.onApplyConfig(newCfg, err)) {
                workingCfg_ = newCfg;
            }
            if (cb_.onTogglePanic) cb_.onTogglePanic();
            break;
        }

        case HitId::BtnMinimize:
            Hide();
            break;

        case HitId::BtnExit:
            if (cb_.onExitApp) cb_.onExitApp();
            break;

        case HitId::BtnApply: {
            AppConfig newCfg = GatherConfig();
            std::wstring err;
            if (cb_.onApplyConfig) {
                if (cb_.onApplyConfig(newCfg, err)) {
                    workingCfg_ = newCfg;
                    ShowToast(std::wstring(i18n::Get(i18n::StrId::ToastSaved)) +
                              utils::FormatHotkey(newCfg.panicModifiers, newCfg.panicVk), false);
                } else {
                    ShowToast(err.empty() ? i18n::Get(i18n::StrId::ToastErrorApplying) : err, true);
                }
            }
            break;
        }

        // Tab 0
        case HitId::HotkeyPanicBox:    BeginHotkeyCapture(1); break;
        case HitId::HotkeySettingsBox: BeginHotkeyCapture(2); break;
        case HitId::Preset1: recPanicMods_ = MOD_CONTROL | MOD_ALT; recPanicVk_ = 'P'; InvalidateRect(hwnd_, nullptr, FALSE); break;
        case HitId::Preset2: recPanicMods_ = MOD_ALT; recPanicVk_ = 'Z'; InvalidateRect(hwnd_, nullptr, FALSE); break;
        case HitId::Preset3: recPanicMods_ = 0; recPanicVk_ = VK_F9; InvalidateRect(hwnd_, nullptr, FALSE); break;
        case HitId::Preset4: recPanicMods_ = 0; recPanicVk_ = VK_PAUSE; InvalidateRect(hwnd_, nullptr, FALSE); break;

        case HitId::ToggleMute:       toggle(togMute_); break;
        case HitId::TogglePauseMedia: toggle(togPauseMedia_); break;
        case HitId::ToggleStealth:    toggle(togStealth_); break;
        case HitId::ToggleStartWin:   toggle(togStartWin_); break;
        case HitId::ToggleStartTip:   toggle(togStartTip_); break;
        case HitId::BtnRelaunchAdmin:
            if (cb_.onRelaunchAdmin) cb_.onRelaunchAdmin();
            break;

        // Tab 1 Decoy Tiles
        case HitId::DecoyTile0: SelectDecoy(DecoyType::BuiltinNotes); break;
        case HitId::DecoyTile1: SelectDecoy(DecoyType::BuiltinTerminal); break;
        case HitId::DecoyTile2: SelectDecoy(DecoyType::BuiltinSpreadsheet); break;
        case HitId::DecoyTile3: SelectDecoy(DecoyType::BuiltinImage); break;
        case HitId::DecoyTile4: SelectDecoy(DecoyType::ExternalAppOrFile); break;
        case HitId::DecoyTile5: SelectDecoy(DecoyType::ExternalUrl); break;
        case HitId::DecoyTile6: SelectDecoy(DecoyType::CapturedWindow); break;
        case HitId::DecoyTile7: SelectDecoy(DecoyType::CleanDesktop); break;

        // Tab 1 Sub-actions
        case HitId::BtnNotesLoadTpl: {
            int idx = std::max(0, (int)SendMessageW(comboNotesTpl_, CB_GETCURSEL, 0, 0));
            bool sp = i18n::IsSpanish();
            SetWindowTextW(editNotesContent_, AppConfig::GetDefaultNotesTemplate(idx, sp).c_str());
            if (idx == 0) SetWindowTextW(editNotesTitle_, sp ? L"Informe_Trimestral_Q4.txt - Bloc de notas" : L"Q4_Operations_Report.txt - Notepad");
            else if (idx == 1) SetWindowTextW(editNotesTitle_, sp ? L"Acta_Reunion_Semanal.txt - Bloc de notas" : L"Meeting_Minutes.txt - Notepad");
            else SetWindowTextW(editNotesTitle_, sp ? L"Plan_Proyecto_Fase2.txt - Bloc de notas" : L"Project_Sprint_Plan.txt - Notepad");
            ShowToast(i18n::Get(i18n::StrId::ToastTplLoaded), false);
            break;
        }

        case HitId::BtnTermLoadTpl: {
            int idx = std::max(0, (int)SendMessageW(comboTermTpl_, CB_GETCURSEL, 0, 0));
            bool sp = i18n::IsSpanish();
            SetWindowTextW(editTermContent_, AppConfig::GetDefaultTerminalTemplate(idx, sp).c_str());
            if (idx == 0) SetWindowTextW(editTermTitle_, L"Windows PowerShell - core-engine");
            else if (idx == 1) SetWindowTextW(editTermTitle_, L"Windows PowerShell - infra-services");
            else SetWindowTextW(editTermTitle_, L"Windows PowerShell - Diagnostics");
            ShowToast(i18n::Get(i18n::StrId::ToastTplLoaded), false);
            break;
        }

        case HitId::ToggleTermSim:       toggle(togTermSim_); break;
        case HitId::BtnImgBrowse:
            BrowseForFile(editImagePath_,
                          L"Images (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0All (*.*)\0*.*\0",
                          i18n::Get(i18n::StrId::DecoyImagePathLabel));
            break;
        case HitId::ToggleImgFullscreen: toggle(togImgFull_); break;

        case HitId::BtnExtBrowse:
            BrowseForFile(editExtPath_,
                          L"Apps & Documents\0*.exe;*.docx;*.xlsx;*.pptx;*.pdf;*.txt;*.lnk;*.bat\0All (*.*)\0*.*\0",
                          i18n::Get(i18n::StrId::DecoyExtPathLabel));
            break;
        case HitId::ToggleExtMax:        toggle(togExtMax_); break;
        case HitId::ToggleExtPrewarm:    toggle(togExtPrewarm_); break;
        case HitId::ToggleUrlPrewarm:    toggle(togUrlPrewarm_); break;

        case HitId::BtnExtPrepare:
        case HitId::BtnUrlPrepare: {
            AppConfig newCfg = GatherConfig();
            std::wstring err;
            if (cb_.onApplyConfig && cb_.onApplyConfig(newCfg, err)) {
                workingCfg_ = newCfg;
                if (cb_.onPrepareDecoy) cb_.onPrepareDecoy();
                ShowToast(i18n::Get(i18n::StrId::ToastPrewarming), false);
            } else if (!err.empty()) {
                ShowToast(err, true);
            }
            break;
        }

        case HitId::BtnCapRefresh:
            RefreshOpenWindowsList();
            ShowToast(i18n::Get(i18n::StrId::ToastOpenWindowsRefreshed), false);
            break;

        case HitId::ToggleCapKeepVis:
            toggle(togCapKeepVis_);
            break;

        case HitId::BtnCapCapture: {
            int idx = (int)SendMessageW(comboOpenWindows_, CB_GETCURSEL, 0, 0);
            if (idx >= 0 && idx < (int)openWindowHandles_.size()) {
                if (cb_.onCaptureWindow) {
                    cb_.onCaptureWindow(openWindowHandles_[idx], !togCapKeepVis_.value);
                }
                RefreshOpenWindowsList();
                ShowToast(i18n::Get(i18n::StrId::ToastWindowCaptured), false);
            }
            break;
        }

        case HitId::BtnCapRelease:
            if (cb_.onReleaseCapturedWindow) cb_.onReleaseCapturedWindow();
            RefreshOpenWindowsList();
            ShowToast(i18n::Get(i18n::StrId::ToastWindowReleased), false);
            break;

        // Tab 2
        case HitId::BtnDiagRefresh:
            InvalidateRect(hwnd_, nullptr, FALSE);
            ShowToast(i18n::Get(i18n::StrId::ToastMetricsRefreshed), false);
            break;

        case HitId::BtnDiagRestore:
            if (cb_.isPanicActive && cb_.isPanicActive() && cb_.onTogglePanic) {
                cb_.onTogglePanic();
            } else {
                ShowToast(i18n::Get(i18n::StrId::ToastNoHiddenToRestore), false);
            }
            break;

        default:
            break;
    }
}

void SettingsWindow::Render(HDC targetDc) {
    RECT rc{};
    GetClientRect(hwnd_, &rc);
    int W = rc.right - rc.left;
    int H = rc.bottom - rc.top;
    if (W <= 0 || H <= 0) return;

    HDC memDC = CreateCompatibleDC(targetDc);
    HBITMAP memBmp = CreateCompatibleBitmap(targetDc, W, H);
    HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

    Gdiplus::Graphics g(memDC);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);

    auto S = [&](float v) { return v * ((float)dpi_ / 96.0f); };
    Palette pal = MakePalette(dark_);
    hitRegions_.clear();

    auto regHit = [&](HitId id, float x, float y, float w, float h, bool enabled = true) {
        RECT r{(LONG)std::floor(x), (LONG)std::floor(y), (LONG)std::ceil(x + w), (LONG)std::ceil(y + h)};
        hitRegions_.push_back({id, r, enabled});
    };

    Gdiplus::FontFamily ffUi(L"Segoe UI");
    Gdiplus::FontFamily ffMono(L"Consolas");
    Gdiplus::Font fBrand(&ffUi, S(13.5f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font fPageTitle(&ffUi, S(20.5f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font fSection(&ffUi, S(13.5f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font fBodyBold(&ffUi, S(12.5f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font fBody(&ffUi, S(12.5f), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::Font fSub(&ffUi, S(11.0f), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::Font fMicro(&ffUi, S(10.0f), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::Font fStatBig(&ffUi, S(22.0f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font fKeycap(&ffMono, S(11.5f), Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

    auto drawCard = [&](float x, float y, float w, float h, float alpha) {
        FillRoundRect(g, x, y, w, h, S(10), WithAlpha(pal.cardBg, alpha));
        DrawRoundRect(g, x, y, w, h, S(10), WithAlpha(pal.cardBorder, alpha), 1.0f);
    };

    auto drawInputFrame = [&](float x, float y, float w, float h, float alpha) {
        float padX = S(6), padY = S(4);
        FillRoundRect(g, x - padX, y - padY, w + padX * 2, h + padY * 2, S(6), WithAlpha(pal.inputBg, alpha));
        DrawRoundRect(g, x - padX, y - padY, w + padX * 2, h + padY * 2, S(6), WithAlpha(pal.inputBorder, alpha), 1.0f);
    };

    auto drawButton = [&](HitId id, const std::wstring& label, float x, float y, float w, float h,
                          int variant, float alpha = 1.0f, bool enabled = true) {
        regHit(id, x, y, w, h, enabled);
        bool hov = enabled && (hoveredHit_ == id);
        bool prs = enabled && (pressedHit_ == id);

        Gdiplus::Color fill, border, txt;
        float radius = (variant == 4) ? S(13) : S(7);

        if (variant == 0) {
            fill = prs ? pal.accentPress : (hov ? pal.accentHover : pal.accent);
            border = fill;
            txt = Gdiplus::Color(255, 255, 255, 255);
        } else if (variant == 2) {
            fill = prs ? pal.dangerPress : (hov ? pal.dangerHover : pal.danger);
            border = fill;
            txt = Gdiplus::Color(255, 255, 255, 255);
        } else if (variant == 3) {
            fill = prs ? pal.cardBorder : (hov ? pal.cardHoverBg : Gdiplus::Color(0, 0, 0, 0));
            border = hov ? pal.cardBorder : Gdiplus::Color(0, 0, 0, 0);
            txt = pal.textSecondary;
        } else if (variant == 4) {
            fill = prs ? pal.accentSoftBg : (hov ? pal.cardHoverBg : pal.keycapBg);
            border = hov ? pal.accent : pal.keycapBorder;
            txt = hov ? pal.accent : pal.textPrimary;
        } else {
            fill = prs ? pal.divider : (hov ? pal.btnSecHover : pal.btnSecBg);
            border = pal.btnSecBorder;
            txt = enabled ? pal.textPrimary : pal.textMuted;
        }

        if (!enabled) alpha *= 0.55f;

        if (fill.GetA() > 0) FillRoundRect(g, x, y, w, h, radius, WithAlpha(fill, alpha));
        if (border.GetA() > 0 && variant != 0 && variant != 2) {
            DrawRoundRect(g, x, y, w, h, radius, WithAlpha(border, alpha), 1.0f);
        }

        const Gdiplus::Font& bf = (variant == 4) ? fSub : ((variant == 0 || variant == 2) ? fBodyBold : fBody);
        DrawTextBox(g, label, bf, WithAlpha(txt, alpha), x, y, w, h,
                    Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
    };

    auto drawToggle = [&](HitId id, const ToggleState& ts, float rightX, float centerY, float alpha) {
        float tw = S(42), th = S(22);
        float tx = rightX - tw;
        float ty = centerY - th * 0.5f;
        regHit(id, tx - S(4), ty - S(4), tw + S(8), th + S(8), true);

        bool hov = (hoveredHit_ == id);
        Gdiplus::Color offTrack = dark_ ? Gdiplus::Color(255, 58, 64, 82) : Gdiplus::Color(255, 198, 206, 220);
        Gdiplus::Color onTrack = hov ? pal.accentHover : pal.accent;
        Gdiplus::Color trackCol = LerpColor(offTrack, onTrack, ts.visual);
        FillRoundRect(g, tx, ty, tw, th, th * 0.5f, WithAlpha(trackCol, alpha));

        float knobD = S(16);
        float pad = S(3);
        float kx = tx + pad + (tw - knobD - pad * 2) * ts.visual;
        float ky = ty + pad;
        Gdiplus::SolidBrush knobBr(WithAlpha(Gdiplus::Color(255, 255, 255, 255), alpha));
        g.FillEllipse(&knobBr, kx, ky, knobD, knobD);
    };

    auto drawHotkeyBox = [&](HitId id, int slot, UINT mods, UINT vk, float x, float y, float w, float h, float alpha) {
        regHit(id, x, y, w, h, true);
        bool activeCap = (captureSlot_ == slot);
        bool hov = (hoveredHit_ == id);

        float pulse = 0.0f;
        if (activeCap) {
            pulse = 0.5f + 0.5f * std::sin((float)(GetTickCount64() % 1200) * 0.0052359f);
        }

        Gdiplus::Color bg = activeCap ? pal.accentSoftBg : (hov ? pal.cardHoverBg : pal.inputBg);
        Gdiplus::Color brd = activeCap
            ? LerpColor(pal.accent, pal.accentHover, pulse)
            : (hov ? pal.accent : pal.inputBorder);

        FillRoundRect(g, x, y, w, h, S(8), WithAlpha(bg, alpha));
        DrawRoundRect(g, x, y, w, h, S(8), WithAlpha(brd, alpha), activeCap ? 1.8f : 1.0f);

        if (activeCap) {
            DrawTextBox(g, i18n::Get(i18n::StrId::HotkeyPressCombo), fSub, WithAlpha(pal.accent, alpha),
                        x + S(10), y, w - S(20), h, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
        } else {
            auto parts = utils::FormatHotkeyParts(mods, vk);
            if (parts.empty()) {
                DrawTextBox(g, i18n::Get(i18n::StrId::HotkeyClickToAssign), fSub, WithAlpha(pal.textMuted, alpha),
                            x + S(10), y, w - S(20), h, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
            } else {
                float gap = S(16);
                float totalW = 0.0f;
                std::vector<float> capWidths;
                for (const auto& part : parts) {
                    Gdiplus::RectF mr{};
                    g.MeasureString(part.c_str(), (INT)part.size(), &fKeycap, Gdiplus::PointF(0, 0), &mr);
                    float cw = std::max(S(30), mr.Width + S(16));
                    capWidths.push_back(cw);
                    totalW += cw;
                }
                if (parts.size() > 1) totalW += gap * (float)(parts.size() - 1);

                float cx = x + (w - totalW) * 0.5f;
                float ch = S(24);
                float cy = y + (h - ch) * 0.5f;
                for (size_t i = 0; i < parts.size(); ++i) {
                    float cw = capWidths[i];
                    FillRoundRect(g, cx, cy, cw, ch, S(5), WithAlpha(pal.keycapBg, alpha));
                    DrawRoundRect(g, cx, cy, cw, ch, S(5), WithAlpha(pal.keycapBorder, alpha), 1.0f);
                    DrawTextBox(g, parts[i], fKeycap, WithAlpha(pal.textPrimary, alpha),
                                cx, cy, cw, ch, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
                    cx += cw;
                    if (i + 1 < parts.size()) {
                        DrawTextBox(g, L"+", fSub, WithAlpha(pal.textMuted, alpha),
                                    cx, cy, gap, ch, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
                        cx += gap;
                    }
                }
            }
        }
    };

    // 1. Master Canvas & Sidebar
    {
        Gdiplus::SolidBrush canvasBr(pal.canvasBg);
        g.FillRectangle(&canvasBr, 0, 0, W, H);

        Gdiplus::SolidBrush sideBr(pal.sidebarBg);
        g.FillRectangle(&sideBr, 0.0f, 0.0f, S(kSidebarW), (float)H);

        Gdiplus::Pen divPen(pal.divider, 1.0f);
        g.DrawLine(&divPen, S(kSidebarW), 0.0f, S(kSidebarW), (float)H);
    }

    // Sidebar Brand
    {
        float bx = S(18), by = S(20), bs = S(36);
        Gdiplus::GraphicsPath badge;
        AddRoundRect(badge, bx, by, bs, bs, S(9));
        Gdiplus::LinearGradientBrush grad(
            Gdiplus::PointF(bx, by), Gdiplus::PointF(bx, by + bs),
            Gdiplus::Color(255, 232, 62, 78), Gdiplus::Color(255, 178, 28, 46));
        g.FillPath(&grad, &badge);

        Gdiplus::Pen ring(Gdiplus::Color(240, 255, 255, 255), S(2.8f));
        g.DrawEllipse(&ring, bx + S(9), by + S(9), S(18), S(18));
        Gdiplus::SolidBrush dot(Gdiplus::Color(245, 255, 255, 255));
        g.FillEllipse(&dot, bx + S(15), by + S(15), S(6), S(6));

        DrawTextBox(g, i18n::Get(i18n::StrId::AppName), fBrand, pal.textPrimary, S(64), S(20), S(146), S(20));
        DrawTextBox(g, i18n::Get(i18n::StrId::AppTagline), fMicro, pal.textMuted, S(64), S(40), S(146), S(16));
    }

    // Live Status Pill
    bool panicActive = cb_.isPanicActive ? cb_.isPanicActive() : false;
    {
        float px = S(16), py = S(76), pw = S(190), ph = S(34);
        Gdiplus::Color pBg = panicActive ? pal.warnBg : pal.successBg;
        Gdiplus::Color pCol = panicActive ? pal.warn : pal.success;
        FillRoundRect(g, px, py, pw, ph, S(8), pBg);

        Gdiplus::SolidBrush dot(pCol);
        g.FillEllipse(&dot, px + S(12), py + S(13), S(8), S(8));

        std::wostringstream st;
        if (panicActive) {
            st << i18n::Get(i18n::StrId::StatusPanicActive) << L" (" << (cb_.getHiddenCount ? cb_.getHiddenCount() : 0) << L")";
        } else {
            st << i18n::Get(i18n::StrId::StatusReady) << L"  •  " << std::fixed << std::setprecision(1) << utils::GetProcessMemoryMB() << L" MB";
        }
        DrawTextBox(g, st.str(), fSub, pCol, px + S(26), py, pw - S(34), ph);
    }

    DrawTextBox(g, i18n::IsSpanish() ? L"NAVEGACIÓN" : L"NAVIGATION", fMicro, pal.textMuted, S(20), S(128), S(160), S(16));

    // Nav Items + Animated Sliding Indicator Pill
    {
        const wchar_t* navLabels[3] = {
            i18n::Get(i18n::StrId::NavGeneral),
            i18n::Get(i18n::StrId::NavCamouflage),
            i18n::Get(i18n::StrId::NavEngine)
        };
        HitId navIds[3] = { HitId::NavGeneral, HitId::NavDecoy, HitId::NavEngine };

        for (int i = 0; i < 3; ++i) {
            float nx = S(12), ny = S(150.0f + i * 50.0f), nw = S(198), nh = S(42);
            regHit(navIds[i], nx, ny, nw, nh, true);

            bool sel = (activeTab_ == i);
            bool hov = (hoveredHit_ == navIds[i]);
            if (sel) {
                FillRoundRect(g, nx, ny, nw, nh, S(8), pal.cardBg);
                DrawRoundRect(g, nx, ny, nw, nh, S(8), pal.cardBorder, 1.0f);
            } else if (hov) {
                FillRoundRect(g, nx, ny, nw, nh, S(8), pal.cardHoverBg);
            }

            Gdiplus::Color icCol = sel ? pal.accent : (hov ? pal.textPrimary : pal.textSecondary);
            Gdiplus::Color txCol = sel ? pal.textPrimary : (hov ? pal.textPrimary : pal.textSecondary);

            DrawNavIcon(g, i, nx + S(18), ny + S(11), S(19), icCol);
            DrawTextBox(g, navLabels[i], sel ? fBodyBold : fBody, txCol, nx + S(46), ny, nw - S(54), nh);
        }

        // Animated vertical pill
        FillRoundRect(g, S(14.5f), S(pillY_), S(3.5f), S(22), S(2), pal.accent);
    }

    // Sidebar Bottom Dock
    {
        Gdiplus::Pen sep(pal.divider, 1.0f);
        g.DrawLine(&sep, S(16), S(442), S(206), S(442));

        drawButton(HitId::BtnThemeToggle, dark_ ? i18n::Get(i18n::StrId::ThemeToggleLight) : i18n::Get(i18n::StrId::ThemeToggleDark),
                   S(16), S(456), S(190), S(34), 1);

        drawButton(HitId::BtnTestPanic, panicActive ? i18n::Get(i18n::StrId::BtnRestoreAll) : i18n::Get(i18n::StrId::BtnTestPanic),
                   S(16), S(502), S(190), S(40), 2);

        drawButton(HitId::BtnMinimize, i18n::Get(i18n::StrId::BtnHide), S(16), S(554), S(91), S(32), 3);
        drawButton(HitId::BtnExit, i18n::Get(i18n::StrId::BtnExit), S(115), S(554), S(91), S(32), 3);

        std::wstring hkNote = (i18n::IsSpanish() ? L"Atajo: " : L"Shortcut: ") + utils::FormatHotkey(recPanicMods_, recPanicVk_);
        DrawTextBox(g, hkNote, fMicro, pal.textMuted, S(16), S(598), S(190), S(18),
                    Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
    }

    // 2. Right Content Area (Slide + Fade)
    float pageEase = EaseOutCubic(pageAnim_);
    float pageAlpha = 0.25f + 0.75f * pageEase;
    float slideY = S((1.0f - pageEase) * 14.0f);

    g.SetClip(Gdiplus::RectF(S(kSidebarW + 1), 0.0f, (float)W - S(kSidebarW + 1), S(574)));

    {
        const wchar_t* titles[3] = {
            i18n::Get(i18n::StrId::NavGeneral),
            i18n::Get(i18n::StrId::NavCamouflage),
            i18n::Get(i18n::StrId::NavEngine)
        };
        const wchar_t* subs[3] = {
            i18n::Get(i18n::StrId::SectionHotkeysDesc),
            i18n::IsSpanish() ? L"Personaliza qué aparecerá al instante al pulsar el botón del pánico." : L"Customize what instantly appears when the panic button is triggered.",
            i18n::IsSpanish() ? L"Controla procesos protegidos y consulta la eficiencia en tiempo real." : L"Manage protected exclusions and inspect live background telemetry."
        };
        DrawTextBox(g, titles[activeTab_], fPageTitle, WithAlpha(pal.textPrimary, pageAlpha),
                    S(kContentX), S(18) + slideY, S(kContentW), S(28));
        DrawTextBox(g, subs[activeTab_], fSub, WithAlpha(pal.textSecondary, pageAlpha),
                    S(kContentX), S(46) + slideY, S(kContentW), S(18));
    }

    // TAB 0
    if (activeTab_ == 0) {
        // Card 1: Shortcuts & Mouse Trigger
        float c1y = S(74) + slideY;
        drawCard(S(kContentX), c1y, S(kContentW), S(218), pageAlpha);

        // Panic Shortcut
        DrawTextBox(g, i18n::Get(i18n::StrId::PanicHotkeyLabel), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                    S(264), c1y + S(12), S(260), S(20));
        DrawTextBox(g, i18n::Get(i18n::StrId::PanicHotkeyDesc), fMicro, WithAlpha(pal.textSecondary, pageAlpha),
                    S(264), c1y + S(32), S(290), S(16));
        drawHotkeyBox(HitId::HotkeyPanicBox, 1, recPanicMods_, recPanicVk_, S(566), c1y + S(12), S(276), S(38), pageAlpha);

        DrawTextBox(g, i18n::Get(i18n::StrId::PresetQuickLabel), fMicro, WithAlpha(pal.textMuted, pageAlpha), S(264), c1y + S(56), S(58), S(24));
        drawButton(HitId::Preset1, L"Ctrl+Alt+P", S(324), c1y + S(56), S(90), S(25), 4, pageAlpha);
        drawButton(HitId::Preset2, L"Alt+Z", S(422), c1y + S(56), S(64), S(25), 4, pageAlpha);
        drawButton(HitId::Preset3, L"F9", S(494), c1y + S(56), S(48), S(25), 4, pageAlpha);
        drawButton(HitId::Preset4, L"Pausa", S(550), c1y + S(56), S(68), S(25), 4, pageAlpha);

        Gdiplus::Pen sep(WithAlpha(pal.divider, pageAlpha), 1.0f);
        g.DrawLine(&sep, S(264), c1y + S(92), S(844), c1y + S(92));

        // Settings Shortcut
        DrawTextBox(g, i18n::Get(i18n::StrId::SettingsHotkeyLabel), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                    S(264), c1y + S(102), S(260), S(20));
        DrawTextBox(g, i18n::Get(i18n::StrId::SettingsHotkeyDesc), fMicro, WithAlpha(pal.textSecondary, pageAlpha),
                    S(264), c1y + S(122), S(295), S(16));
        drawHotkeyBox(HitId::HotkeySettingsBox, 2, recSettingsMods_, recSettingsVk_, S(566), c1y + S(102), S(276), S(38), pageAlpha);

        g.DrawLine(&sep, S(264), c1y + S(152), S(844), c1y + S(152));

        // Mouse Trigger
        DrawTextBox(g, i18n::Get(i18n::StrId::MouseTriggerLabel), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                    S(264), c1y + S(164), S(260), S(20));
        DrawTextBox(g, i18n::Get(i18n::StrId::MouseTriggerDesc), fMicro, WithAlpha(pal.textSecondary, pageAlpha),
                    S(264), c1y + S(184), S(295), S(16));

        // Card 2: Behavior Toggles (5 rows)
        float c2y = S(302) + slideY;
        drawCard(S(kContentX), c2y, S(kContentW), S(196), pageAlpha);

        struct ToggleRow {
            HitId id;
            const ToggleState* st;
            i18n::StrId title;
            i18n::StrId desc;
        };
        ToggleRow rows[5] = {
            { HitId::ToggleMute, &togMute_, i18n::StrId::TogMuteAudioTitle, i18n::StrId::TogMuteAudioDesc },
            { HitId::TogglePauseMedia, &togPauseMedia_, i18n::StrId::TogPauseMediaTitle, i18n::StrId::TogPauseMediaDesc },
            { HitId::ToggleStealth, &togStealth_, i18n::StrId::TogStealthTitle, i18n::StrId::TogStealthDesc },
            { HitId::ToggleStartWin, &togStartWin_, i18n::StrId::TogStartWinTitle, i18n::StrId::TogStartWinDesc },
            { HitId::ToggleStartTip, &togStartTip_, i18n::StrId::TogStartTipTitle, i18n::StrId::TogStartTipDesc }
        };

        for (int i = 0; i < 5; ++i) {
            float ry = c2y + S(4.0f + i * 38.0f);
            if (i > 0) {
                g.DrawLine(&sep, S(264), ry, S(844), ry);
            }
            DrawTextBox(g, i18n::Get(rows[i].title), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                        S(264), ry + S(4), S(490), S(17));
            DrawTextBox(g, i18n::Get(rows[i].desc), fMicro, WithAlpha(pal.textSecondary, pageAlpha),
                        S(264), ry + S(21), S(490), S(15));
            drawToggle(rows[i].id, *rows[i].st, S(842), ry + S(19), pageAlpha);
        }

        // Card 3: Elevation & Language
        float c3y = S(508) + slideY;
        drawCard(S(kContentX), c3y, S(kContentW), S(56), pageAlpha);
        bool isAdmin = utils::IsRunningAsAdmin();

        DrawTextBox(g, isAdmin ? i18n::Get(i18n::StrId::AdminStatusActive) : i18n::Get(i18n::StrId::AdminStatusInactive),
                    fBodyBold, WithAlpha(isAdmin ? pal.success : pal.textPrimary, pageAlpha),
                    S(264), c3y + S(8), S(380), S(18));
        DrawTextBox(g, i18n::Get(i18n::StrId::LanguageLabel),
                    fMicro, WithAlpha(pal.textSecondary, pageAlpha),
                    S(264), c3y + S(30), S(200), S(18));

        drawButton(HitId::BtnRelaunchAdmin, isAdmin ? i18n::Get(i18n::StrId::BtnRelaunchAdminDone) : i18n::Get(i18n::StrId::BtnRelaunchAdmin),
                   S(676), c3y + S(11), S(166), S(34), 1, pageAlpha, !isAdmin);
    }

    // TAB 1
    else if (activeTab_ == 1) {
        struct TileDef {
            HitId id;
            i18n::StrId name;
            i18n::StrId tag;
        };
        const TileDef tiles[8] = {
            { HitId::DecoyTile0, i18n::StrId::DecoyTitleNotes, i18n::StrId::DecoyTagNotes },
            { HitId::DecoyTile1, i18n::StrId::DecoyTitleTerm, i18n::StrId::DecoyTagTerm },
            { HitId::DecoyTile2, i18n::StrId::DecoyTitleSheet, i18n::StrId::DecoyTagSheet },
            { HitId::DecoyTile3, i18n::StrId::DecoyTitleImage, i18n::StrId::DecoyTagImage },
            { HitId::DecoyTile4, i18n::StrId::DecoyTitleExt, i18n::StrId::DecoyTagExt },
            { HitId::DecoyTile5, i18n::StrId::DecoyTitleUrl, i18n::StrId::DecoyTagUrl },
            { HitId::DecoyTile6, i18n::StrId::DecoyTitleCapture, i18n::StrId::DecoyTagCapture },
            { HitId::DecoyTile7, i18n::StrId::DecoyTitleClean, i18n::StrId::DecoyTagClean }
        };

        float tileW = S(148), tileH = S(64), gapX = S(8), gapY = S(8);
        for (int i = 0; i < 8; ++i) {
            int r = i / 4;
            int c = i % 4;
            float tx = S(kContentX) + c * (tileW + gapX);
            float ty = S(76) + slideY + r * (tileH + gapY);
            regHit(tiles[i].id, tx, ty, tileW, tileH, true);

            bool sel = ((int)selDecoy_ == i);
            bool hov = (hoveredHit_ == tiles[i].id);

            Gdiplus::Color bg = sel ? pal.accentSoftBg : (hov ? pal.cardHoverBg : pal.cardBg);
            Gdiplus::Color brd = sel ? pal.accent : (hov ? pal.inputBorder : pal.cardBorder);
            FillRoundRect(g, tx, ty, tileW, tileH, S(10), WithAlpha(bg, pageAlpha));
            DrawRoundRect(g, tx, ty, tileW, tileH, S(10), WithAlpha(brd, pageAlpha), sel ? 1.8f : 1.0f);

            Gdiplus::Color icCol = sel ? pal.accent : (hov ? pal.textPrimary : pal.textSecondary);
            DrawDecoyTileIcon(g, i, tx + S(12), ty + S(14), S(20), WithAlpha(icCol, pageAlpha));

            if (sel) {
                Gdiplus::SolidBrush chkDot(WithAlpha(pal.accent, pageAlpha));
                g.FillEllipse(&chkDot, tx + tileW - S(16), ty + S(10), S(7), S(7));
            }

            DrawTextBox(g, i18n::Get(tiles[i].name), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                        tx + S(38), ty + S(11), tileW - S(48), S(20));
            DrawTextBox(g, i18n::Get(tiles[i].tag), fMicro, WithAlpha(sel ? pal.accent : pal.textMuted, pageAlpha),
                        tx + S(38), ty + S(34), tileW - S(46), S(16));
        }

        // Live Decoy Status Strip
        {
            float sy = S(220) + slideY;
            FillRoundRect(g, S(kContentX), sy, S(kContentW), S(28), S(7), WithAlpha(pal.cardBg, pageAlpha));
            DrawRoundRect(g, S(kContentX), sy, S(kContentW), S(28), S(7), WithAlpha(pal.cardBorder, pageAlpha), 1.0f);
            Gdiplus::SolidBrush dot(WithAlpha(pal.accent, pageAlpha));
            g.FillEllipse(&dot, S(kContentX + 12), sy + S(10), S(8), S(8));
            std::wstring statusTxt = cb_.getDecoyStatus ? cb_.getDecoyStatus() : i18n::Get(i18n::StrId::DecoyLiveStatusReady);
            DrawTextBox(g, statusTxt, fSub, WithAlpha(pal.textSecondary, pageAlpha),
                        S(kContentX + 28), sy, S(kContentW - 40), S(28));
        }

        // Dynamic Config Card
        float subEase = EaseOutCubic(subCardAnim_);
        float subAlpha = pageAlpha * (0.25f + 0.75f * subEase);
        float dy = slideY + S((1.0f - subEase) * 10.0f);
        float cy = S(258) + dy;
        drawCard(S(kContentX), cy, S(kContentW), S(298), subAlpha);

        if (selDecoy_ == DecoyType::BuiltinNotes) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyNotesDocTitle), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(276) + dy, S(200), S(16));
            drawInputFrame(S(266), S(300) + dy, S(576), S(22), subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyNotesTplLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(326) + dy, S(210), S(16));
            drawButton(HitId::BtnNotesLoadTpl, i18n::Get(i18n::StrId::DecoyNotesLoadTpl), S(486), S(337) + dy, S(124), S(28), 1, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyNotesContentLabel), fMicro,
                        WithAlpha(pal.textSecondary, subAlpha), S(264), S(376) + dy, S(480), S(16));
            drawInputFrame(S(266), S(400) + dy, S(576), S(134), subAlpha);
        } else if (selDecoy_ == DecoyType::BuiltinTerminal) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyTermConsoleTitle), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(276) + dy, S(200), S(16));
            drawInputFrame(S(266), S(300) + dy, S(576), S(22), subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyNotesTplLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(326) + dy, S(210), S(16));
            drawButton(HitId::BtnTermLoadTpl, i18n::Get(i18n::StrId::DecoyTermLoadTpl), S(486), S(337) + dy, S(130), S(28), 1, subAlpha);
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyTermSimActivity), fSub, WithAlpha(pal.textPrimary, subAlpha), S(646), S(341) + dy, S(140), S(20));
            drawToggle(HitId::ToggleTermSim, togTermSim_, S(842), S(351) + dy, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyTermInitialContent), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(376) + dy, S(300), S(16));
            drawInputFrame(S(266), S(400) + dy, S(576), S(134), subAlpha);
        } else if (selDecoy_ == DecoyType::BuiltinSpreadsheet) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoySheetTitle), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(286) + dy, S(250), S(16));
            drawInputFrame(S(266), S(310) + dy, S(576), S(22), subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoySheetTplLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(348) + dy, S(250), S(16));

            FillRoundRect(g, S(264), S(420) + dy, S(580), S(96), S(8), WithAlpha(pal.accentSoftBg, subAlpha));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoySheetNoteTitle), fBodyBold, WithAlpha(pal.accent, subAlpha),
                        S(280), S(434) + dy, S(540), S(20));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoySheetNoteDesc),
                        fSub, WithAlpha(pal.textSecondary, subAlpha),
                        S(280), S(458) + dy, S(548), S(44), Gdiplus::StringAlignmentNear, Gdiplus::StringAlignmentNear, false);
        } else if (selDecoy_ == DecoyType::BuiltinImage) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyImagePathLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(286) + dy, S(350), S(16));
            drawInputFrame(S(266), S(310) + dy, S(466), S(22), subAlpha);
            drawButton(HitId::BtnImgBrowse, i18n::Get(i18n::StrId::DecoyImageBrowse), S(746), S(306) + dy, S(98), S(30), 1, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyImageFullscreen), fBodyBold, WithAlpha(pal.textPrimary, subAlpha), S(264), S(362) + dy, S(350), S(20));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyImageFullscreenDesc), fMicro,
                        WithAlpha(pal.textSecondary, subAlpha), S(264), S(383) + dy, S(480), S(16));
            drawToggle(HitId::ToggleImgFullscreen, togImgFull_, S(842), S(380) + dy, subAlpha);

            FillRoundRect(g, S(264), S(424) + dy, S(580), S(86), S(8), WithAlpha(pal.accentSoftBg, subAlpha));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyImageTipTitle), fBodyBold, WithAlpha(pal.accent, subAlpha), S(280), S(436) + dy, S(540), S(20));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyImageTipDesc), fSub, WithAlpha(pal.textSecondary, subAlpha), S(280), S(458) + dy, S(548), S(40), Gdiplus::StringAlignmentNear, Gdiplus::StringAlignmentNear, false);
        } else if (selDecoy_ == DecoyType::ExternalAppOrFile) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtPathLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(276) + dy, S(420), S(16));
            drawInputFrame(S(266), S(300) + dy, S(466), S(22), subAlpha);
            drawButton(HitId::BtnExtBrowse, i18n::Get(i18n::StrId::DecoyExtBrowse), S(746), S(296) + dy, S(98), S(30), 1, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtArgsLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(334) + dy, S(260), S(16));
            drawInputFrame(S(266), S(356) + dy, S(576), S(22), subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtMaximized), fBody, WithAlpha(pal.textPrimary, subAlpha), S(264), S(398) + dy, S(200), S(22));
            drawToggle(HitId::ToggleExtMax, togExtMax_, S(520), S(409) + dy, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtPrewarm), fBody, WithAlpha(pal.textPrimary, subAlpha), S(564), S(398) + dy, S(210), S(22));
            drawToggle(HitId::ToggleExtPrewarm, togExtPrewarm_, S(842), S(409) + dy, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtRestoreActionLabel), fSub, WithAlpha(pal.textSecondary, subAlpha), S(264), S(468) + dy, S(156), S(22));
            drawButton(HitId::BtnExtPrepare, i18n::Get(i18n::StrId::DecoyExtPrepareBtn), S(690), S(462) + dy, S(154), S(32), 0, subAlpha);
        } else if (selDecoy_ == DecoyType::ExternalUrl) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyUrlAddressLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(276) + dy, S(300), S(16));
            drawInputFrame(S(266), S(300) + dy, S(576), S(22), subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyUrlModeLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(334) + dy, S(300), S(16));

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyUrlPrewarm), fBody, WithAlpha(pal.textPrimary, subAlpha), S(264), S(398) + dy, S(460), S(22));
            drawToggle(HitId::ToggleUrlPrewarm, togUrlPrewarm_, S(842), S(409) + dy, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyExtRestoreActionLabel), fSub, WithAlpha(pal.textSecondary, subAlpha), S(264), S(444) + dy, S(156), S(22));
            drawButton(HitId::BtnUrlPrepare, i18n::Get(i18n::StrId::DecoyUrlPrepareBtn), S(690), S(438) + dy, S(154), S(32), 0, subAlpha);
        } else if (selDecoy_ == DecoyType::CapturedWindow) {
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyCapSelectLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(264), S(282) + dy, S(360), S(16));
            drawButton(HitId::BtnCapRefresh, i18n::Get(i18n::StrId::DecoyCapRefresh), S(742), S(304) + dy, S(102), S(28), 1, subAlpha);

            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyCapKeepVis),
                        fSub, WithAlpha(pal.textPrimary, subAlpha), S(264), S(352) + dy, S(510), S(22));
            drawToggle(HitId::ToggleCapKeepVis, togCapKeepVis_, S(842), S(363) + dy, subAlpha);

            drawButton(HitId::BtnCapCapture, i18n::Get(i18n::StrId::DecoyCapCaptureBtn), S(264), S(396) + dy, S(236), S(34), 0, subAlpha);
            drawButton(HitId::BtnCapRelease, i18n::Get(i18n::StrId::DecoyCapReleaseBtn), S(512), S(396) + dy, S(156), S(34), 1, subAlpha);

            std::wstring capTitle = cb_.getCapturedWindowTitle ? cb_.getCapturedWindowTitle() : i18n::Get(i18n::StrId::DecoyCapNone);
            FillRoundRect(g, S(264), S(448) + dy, S(580), S(62), S(8), WithAlpha(pal.accentSoftBg, subAlpha));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyCapCurrentLabel), fMicro, WithAlpha(pal.textSecondary, subAlpha), S(280), S(456) + dy, S(300), S(16));
            DrawTextBox(g, capTitle, fBodyBold, WithAlpha(pal.textPrimary, subAlpha), S(280), S(476) + dy, S(548), S(22));
        } else {
            // Clean Desktop
            DrawDecoyTileIcon(g, 7, S(524), S(322) + dy, S(52), WithAlpha(pal.accent, subAlpha));
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyCleanTitle), fSection, WithAlpha(pal.textPrimary, subAlpha),
                        S(280), S(390) + dy, S(540), S(24), Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter);
            DrawTextBox(g, i18n::Get(i18n::StrId::DecoyCleanDesc),
                        fSub, WithAlpha(pal.textSecondary, subAlpha),
                        S(310), S(424) + dy, S(480), S(68), Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentNear, false);
        }
    }

    // TAB 2
    else if (activeTab_ == 2) {
        float c1y = S(76) + slideY;
        drawCard(S(kContentX), c1y, S(kContentW), S(158), pageAlpha);
        DrawTextBox(g, i18n::Get(i18n::StrId::EngineExclusionsTitle), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                    S(264), c1y + S(14), S(300), S(20));
        DrawTextBox(g, i18n::Get(i18n::StrId::EngineExclusionsDesc),
                    fMicro, WithAlpha(pal.textSecondary, pageAlpha), S(264), c1y + S(36), S(576), S(16));
        drawInputFrame(S(266), S(144) + slideY, S(576), S(78), pageAlpha);

        float c2y = S(246) + slideY;
        drawCard(S(kContentX), c2y, S(kContentW), S(310), pageAlpha);
        DrawTextBox(g, i18n::Get(i18n::StrId::EngineMetricsTitle), fBodyBold, WithAlpha(pal.textPrimary, pageAlpha),
                    S(264), c2y + S(14), S(400), S(20));

        PanicStats hSt = cb_.getLastHideStats ? cb_.getLastHideStats() : PanicStats{};
        PanicStats rSt = cb_.getLastRestoreStats ? cb_.getLastRestoreStats() : PanicStats{};
        std::wostringstream sRam, sHide, sRest;
        sRam << std::fixed << std::setprecision(1) << utils::GetProcessMemoryMB() << L" MB";
        sHide << std::fixed << std::setprecision(1) << hSt.elapsedMs << L" ms";
        sRest << std::fixed << std::setprecision(1) << rSt.elapsedMs << L" ms";

        struct StatBox { std::wstring val; const wchar_t* lbl; std::wstring sub; Gdiplus::Color col; };
        StatBox stats[3] = {
            { sRam.str(),  i18n::Get(i18n::StrId::EngineStatRamLabel), i18n::Get(i18n::StrId::EngineStatRamSub), pal.success },
            { sHide.str(), i18n::Get(i18n::StrId::EngineStatHideLabel), std::to_wstring(hSt.windowCount) + i18n::Get(i18n::StrId::EngineStatHideSubFormat), pal.accent },
            { sRest.str(), i18n::Get(i18n::StrId::EngineStatRestoreLabel), std::to_wstring(rSt.windowCount) + i18n::Get(i18n::StrId::EngineStatRestoreSubFormat), pal.accent }
        };

        float sw = S(186), sh = S(82), sgap = S(11);
        for (int i = 0; i < 3; ++i) {
            float sx = S(264) + i * (sw + sgap);
            float sy = c2y + S(44);
            FillRoundRect(g, sx, sy, sw, sh, S(8), WithAlpha(pal.inputBg, pageAlpha));
            DrawRoundRect(g, sx, sy, sw, sh, S(8), WithAlpha(pal.cardBorder, pageAlpha), 1.0f);
            DrawTextBox(g, stats[i].lbl, fMicro, WithAlpha(pal.textSecondary, pageAlpha), sx + S(12), sy + S(10), sw - S(24), S(15));
            DrawTextBox(g, stats[i].val, fStatBig, WithAlpha(stats[i].col, pageAlpha), sx + S(12), sy + S(27), sw - S(24), S(28));
            DrawTextBox(g, stats[i].sub, fMicro, WithAlpha(pal.textMuted, pageAlpha), sx + S(12), sy + S(58), sw - S(24), S(16));
        }

        struct InfoRow { const wchar_t* k; std::wstring v; };
        InfoRow infos[4] = {
            { i18n::Get(i18n::StrId::EngineArchLabel), i18n::Get(i18n::StrId::EngineArchValue) },
            { i18n::Get(i18n::StrId::EngineCamouflageLabel), cb_.getDecoyStatus ? cb_.getDecoyStatus() : L"Ready" },
            { i18n::Get(i18n::StrId::EngineConfigPathLabel), AppConfig::GetConfigPath() },
            { i18n::Get(i18n::StrId::EngineRecoveryPathLabel), WindowManager::GetStateFilePath() }
        };
        for (int i = 0; i < 4; ++i) {
            float iy = c2y + S(140.0f + i * 24.0f);
            DrawTextBox(g, infos[i].k, fMicro, WithAlpha(pal.textMuted, pageAlpha), S(264), iy, S(114), S(20));
            DrawTextBox(g, infos[i].v, fMicro, WithAlpha(pal.textSecondary, pageAlpha), S(382), iy, S(462), S(20));
        }

        drawButton(HitId::BtnDiagRefresh, i18n::Get(i18n::StrId::EngineRefreshMetricsBtn), S(264), c2y + S(258), S(162), S(34), 1, pageAlpha);
        drawButton(HitId::BtnDiagRestore, i18n::Get(i18n::StrId::EngineRestoreHiddenBtn), S(438), c2y + S(258), S(222), S(34), 1, pageAlpha);
    }

    g.ResetClip();

    // 3. Bottom Action Bar
    {
        Gdiplus::Pen sep(pal.divider, 1.0f);
        g.DrawLine(&sep, S(kSidebarW + 1), S(574), (float)W, S(574));

        drawButton(HitId::BtnApply, i18n::Get(i18n::StrId::BtnSaveApply), S(kContentX), S(587), S(176), S(36), 0);

        if (!toastText_.empty() && toastAlpha_ > 0.01f) {
            float tx = S(436);
            float ty = S(589) + S((1.0f - toastAlpha_) * 8.0f);
            float tw = S(426);
            float th = S(32);

            Gdiplus::Color tBg = toastIsError_ ? pal.warnBg : pal.successBg;
            Gdiplus::Color tCol = toastIsError_ ? pal.warn : pal.success;
            FillRoundRect(g, tx, ty, tw, th, S(8), WithAlpha(tBg, toastAlpha_));
            DrawRoundRect(g, tx, ty, tw, th, S(8), WithAlpha(tCol, toastAlpha_ * 0.6f), 1.0f);

            std::wstring prefix = toastIsError_ ? L"⚠  " : L"✓  ";
            DrawTextBox(g, prefix + toastText_, fSub, WithAlpha(tCol, toastAlpha_),
                        tx + S(12), ty, tw - S(24), th);
        }
    }

    BitBlt(targetDc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<SettingsWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<SettingsWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT SettingsWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd_, &ps);
            Render(hdc);
            EndPaint(hwnd_, &ps);
            return 0;
        }

        case WM_TIMER:
            if (wParam == kAnimTimerId) {
                TickAnimations();
                return 0;
            } else if (wParam == kLiveStatsTimerId && IsWindowVisible(hwnd_)) {
                InvalidateRect(hwnd_, nullptr, FALSE);
                return 0;
            }
            break;

        case WM_COMMAND: {
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                HWND ctrl = (HWND)lParam;
                if (ctrl == comboLanguage_) {
                    int selLang = (int)SendMessageW(comboLanguage_, CB_GETCURSEL, 0, 0);
                    i18n::SetConfiguredLanguage(static_cast<i18n::Language>(selLang));
                    workingCfg_.language = selLang;
                    UpdateComboLanguageTexts();
                    InvalidateRect(hwnd_, nullptr, FALSE);
                } else if (ctrl == comboMouseTrigger_) {
                    int selTrig = (int)SendMessageW(comboMouseTrigger_, CB_GETCURSEL, 0, 0);
                    workingCfg_.mouseTrigger = static_cast<MouseTrigger>(selTrig);
                }
            }
            break;
        }

        case WM_MOUSEMOVE: {
            if (!mouseTracked_) {
                TRACKMOUSEEVENT tme{sizeof(tme), TME_LEAVE, hwnd_, 0};
                TrackMouseEvent(&tme);
                mouseTracked_ = true;
            }
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            HitId h = HitTest(x, y);
            if (h != hoveredHit_) {
                hoveredHit_ = h;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            return 0;
        }

        case WM_MOUSELEAVE:
            mouseTracked_ = false;
            if (hoveredHit_ != HitId::None || pressedHit_ != HitId::None) {
                hoveredHit_ = HitId::None;
                pressedHit_ = HitId::None;
                InvalidateRect(hwnd_, nullptr, FALSE);
            }
            return 0;

        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                POINT pt{};
                GetCursorPos(&pt);
                ScreenToClient(hwnd_, &pt);
                HitId h = HitTest(pt.x, pt.y);
                SetCursor(LoadCursorW(nullptr, h != HitId::None ? IDC_HAND : IDC_ARROW));
                return TRUE;
            }
            break;

        case WM_LBUTTONDOWN: {
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            HitId h = HitTest(x, y);
            if (h != HitId::None) {
                pressedHit_ = h;
                SetCapture(hwnd_);
                InvalidateRect(hwnd_, nullptr, FALSE);
            } else {
                EndHotkeyCapture();
                SetFocus(hwnd_);
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            if (GetCapture() == hwnd_) ReleaseCapture();
            int x = (short)LOWORD(lParam);
            int y = (short)HIWORD(lParam);
            HitId h = HitTest(x, y);
            HitId wasPressed = pressedHit_;
            pressedHit_ = HitId::None;
            InvalidateRect(hwnd_, nullptr, FALSE);
            if (wasPressed != HitId::None && wasPressed == h) {
                HandleHitClick(h);
            }
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            if (captureSlot_ != 0) {
                UINT vk = (UINT)wParam;
                if (vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL ||
                    vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT ||
                    vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU ||
                    vk == VK_LWIN || vk == VK_RWIN) {
                    return 0;
                }
                if (vk == VK_ESCAPE) {
                    EndHotkeyCapture();
                    return 0;
                }

                UINT mods = 0;
                if (GetKeyState(VK_CONTROL) & 0x8000) mods |= MOD_CONTROL;
                if (GetKeyState(VK_MENU) & 0x8000)    mods |= MOD_ALT;
                if (GetKeyState(VK_SHIFT) & 0x8000)   mods |= MOD_SHIFT;
                if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) mods |= MOD_WIN;

                if (captureSlot_ == 1) {
                    recPanicMods_ = mods;
                    recPanicVk_ = vk;
                } else {
                    recSettingsMods_ = mods;
                    recSettingsVk_ = vk;
                }
                EndHotkeyCapture();
                ShowToast(std::wstring(i18n::Get(i18n::StrId::ToastHotkeyCaptured)) +
                          utils::FormatHotkey(mods, vk), false);
                return 0;
            }
            break;
        }

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            Palette pal = MakePalette(dark_);
            SetTextColor(hdc, pal.inputTextRef);
            SetBkColor(hdc, pal.inputBgRef);
            return (LRESULT)(inputBgBrush_ ? inputBgBrush_ : GetStockObject(WHITE_BRUSH));
        }

        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_MINIMIZE) {
                Hide();
                return 0;
            }
            break;

        case WM_CLOSE:
            Hide();
            return 0;

        default:
            break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}
