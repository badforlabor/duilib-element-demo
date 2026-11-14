#include "ElementControls.h"
#include "UiScale.h"

#include <algorithm>
#include <cmath>
#include <commctrl.h>
#include <cwctype>
#include <gdiplus.h>
#include <objbase.h>

namespace el {
using namespace DuiLib;

namespace {

DWORD MixColor(DWORD base, DWORD target, int percentTarget)
{
    if (percentTarget <= 0) {
        return base;
    }
    if (percentTarget >= 100) {
        return target;
    }
    const int keep = 100 - percentTarget;
    const DWORD a = ((base >> 24) & 0xFF) * keep / 100 + ((target >> 24) & 0xFF) * percentTarget / 100;
    const DWORD r = ((base >> 16) & 0xFF) * keep / 100 + ((target >> 16) & 0xFF) * percentTarget / 100;
    const DWORD g = ((base >> 8) & 0xFF) * keep / 100 + ((target >> 8) & 0xFF) * percentTarget / 100;
    const DWORD b = (base & 0xFF) * keep / 100 + (target & 0xFF) * percentTarget / 100;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

COLORREF GdiColor(DWORD color)
{
    return RGB(GetBValue(color), GetGValue(color), GetRValue(color));
}

void EnsureGdiplus()
{
    static Gdiplus::GdiplusStartupInput input;
    static ULONG_PTR token = 0;
    static const Gdiplus::Status status = Gdiplus::GdiplusStartup(&token, &input, nullptr);
    (void)status;
}

Gdiplus::Color GpColor(DWORD color)
{
    return Gdiplus::Color(static_cast<BYTE>((color >> 24) & 0xFF), static_cast<BYTE>((color >> 16) & 0xFF),
        static_cast<BYTE>((color >> 8) & 0xFF), static_cast<BYTE>(color & 0xFF));
}

void FillRoundRectColor(HDC hDC, const RECT& rc, int diameter, DWORD color)
{
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) {
        return;
    }
    diameter = (std::max)(2, (std::min)(diameter, (std::min)(width, height)));

    EnsureGdiplus();
    const Gdiplus::REAL radius = diameter / 2.0f;
    const Gdiplus::REAL right = static_cast<Gdiplus::REAL>(rc.left + width);
    const Gdiplus::REAL bottom = static_cast<Gdiplus::REAL>(rc.top + height);

    Gdiplus::GraphicsPath path;
    path.AddArc(static_cast<Gdiplus::REAL>(rc.left), static_cast<Gdiplus::REAL>(rc.top), radius * 2, radius * 2, 180, 90);
    path.AddArc(right - radius * 2, static_cast<Gdiplus::REAL>(rc.top), radius * 2, radius * 2, 270, 90);
    path.AddArc(right - radius * 2, bottom - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(static_cast<Gdiplus::REAL>(rc.left), bottom - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();

    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush brush(GpColor(color));
    graphics.FillPath(&brush, &path);
}

void FillEllipseColor(HDC hDC, const RECT& rc, DWORD fill, DWORD border, int borderWidth)
{
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    const bool stroked = ((border >> 24) & 0xFF) != 0 && borderWidth > 0;
    const Gdiplus::REAL penWidth = stroked ? static_cast<Gdiplus::REAL>((std::max)(1, borderWidth)) : 0.0f;
    const Gdiplus::REAL inset = penWidth / 2.0f;

    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    const Gdiplus::RectF oval(static_cast<Gdiplus::REAL>(rc.left) + inset,
        static_cast<Gdiplus::REAL>(rc.top) + inset, width - penWidth, height - penWidth);
    Gdiplus::SolidBrush brush(GpColor(fill));
    graphics.FillEllipse(&brush, oval);
    if (stroked) {
        Gdiplus::Pen pen(GpColor(border), penWidth);
        graphics.DrawEllipse(&pen, oval);
    }
}

void DrawCheckMark(HDC hDC, const RECT& box, DWORD color)
{
    HPEN pen = ::CreatePen(PS_SOLID, std::max(1, UiScale::Px(2)), GdiColor(color));
    HGDIOBJ oldPen = ::SelectObject(hDC, pen);
    const int x = box.left;
    const int cy = (box.top + box.bottom) / 2;
    const int w = box.right - box.left;
    ::MoveToEx(hDC, x + w * 24 / 100, cy, nullptr);
    ::LineTo(hDC, x + w * 42 / 100, cy + w * 20 / 100);
    ::LineTo(hDC, x + w * 76 / 100, cy - w * 22 / 100);
    ::SelectObject(hDC, oldPen);
    ::DeleteObject(pen);
}

void MakeRoundRectPath(Gdiplus::GraphicsPath& path, const RECT& rc, int diameter)
{
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    const int clamped = (std::max)(2, (std::min)(diameter, (std::min)(width, height)));
    const Gdiplus::REAL radius = clamped / 2.0f;
    const Gdiplus::REAL right = static_cast<Gdiplus::REAL>(rc.right);
    const Gdiplus::REAL bottom = static_cast<Gdiplus::REAL>(rc.bottom);
    const Gdiplus::REAL left = static_cast<Gdiplus::REAL>(rc.left);
    const Gdiplus::REAL top = static_cast<Gdiplus::REAL>(rc.top);

    path.Reset();
    path.AddArc(left, top, radius * 2, radius * 2, 180, 90);
    path.AddArc(right - radius * 2, top, radius * 2, radius * 2, 270, 90);
    path.AddArc(right - radius * 2, bottom - radius * 2, radius * 2, radius * 2, 0, 90);
    path.AddArc(left, bottom - radius * 2, radius * 2, radius * 2, 90, 90);
    path.CloseFigure();
}

void MakeEllipsePath(Gdiplus::GraphicsPath& path, const RECT& rc)
{
    path.Reset();
    path.AddEllipse(static_cast<Gdiplus::REAL>(rc.left), static_cast<Gdiplus::REAL>(rc.top),
        static_cast<Gdiplus::REAL>(rc.right - rc.left), static_cast<Gdiplus::REAL>(rc.bottom - rc.top));
}

void StrokePathAa(HDC hDC, const Gdiplus::GraphicsPath& path, DWORD color, int width)
{
    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::Pen pen(GpColor(color), static_cast<Gdiplus::REAL>(std::max(1, width)));
    graphics.DrawPath(&pen, &path);
}

void DrawLinesAa(HDC hDC, const Gdiplus::PointF* points, int count, DWORD color, float width)
{
    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::Pen pen(GpColor(color), width);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    graphics.DrawLines(&pen, points, count);
}

void FillPolygonAa(HDC hDC, const Gdiplus::PointF* points, int count, DWORD color)
{
    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush brush(GpColor(color));
    graphics.FillPolygon(&brush, points, count);
}

bool IsHot(UINT state)
{
    return (state & UISTATE_HOT) != 0;
}

bool IsPushed(UINT state)
{
    return (state & UISTATE_PUSHED) != 0;
}

} // namespace

//////////////////////////////////////////////////////////////////////////
// EButtonUI

EButtonUI::EButtonUI()
    : m_variant(Variant::Default)
    , m_plain(false)
    , m_size(Size::Normal)
{
    m_cxyBorderRound.cx = 4;
    m_cxyBorderRound.cy = 4;
    SetFixedHeight(32);
    SetFont(font::Body);
    ApplyStyle();
}

LPCTSTR EButtonUI::GetClass() const
{
    return _T("EButton");
}

LPVOID EButtonUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EButton")) == 0) {
        return static_cast<EButtonUI*>(this);
    }
    return CButtonUI::GetInterface(pstrName);
}

void EButtonUI::SetVariant(Variant variant)
{
    m_variant = variant;
    ApplyStyle();
    Invalidate();
}

void EButtonUI::SetPlain(bool plain)
{
    m_plain = plain;
    ApplyStyle();
    Invalidate();
}

void EButtonUI::SetButtonSize(Size size)
{
    m_size = size;
    const int height = size == Size::Small ? 24 : (size == Size::Large ? 40 : 32);
    SetFixedHeight(height);
    SetFont(size == Size::Small ? font::Small : (size == Size::Large ? font::Body14 : font::Body));
    Invalidate();
}

void EButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("variant")) == 0) {
        Variant variant = Variant::Default;
        if (_tcsicmp(pstrValue, _T("primary")) == 0) variant = Variant::Primary;
        else if (_tcsicmp(pstrValue, _T("success")) == 0) variant = Variant::Success;
        else if (_tcsicmp(pstrValue, _T("warning")) == 0) variant = Variant::Warning;
        else if (_tcsicmp(pstrValue, _T("danger")) == 0) variant = Variant::Danger;
        else if (_tcsicmp(pstrValue, _T("info")) == 0) variant = Variant::Info;
        else if (_tcsicmp(pstrValue, _T("text")) == 0) variant = Variant::Text;
        SetVariant(variant);
    }
    else if (_tcsicmp(pstrName, _T("plain")) == 0) {
        SetPlain(_tcsicmp(pstrValue, _T("true")) == 0);
    }
    else if (_tcsicmp(pstrName, _T("size")) == 0) {
        Size size = Size::Normal;
        if (_tcsicmp(pstrValue, _T("small")) == 0) size = Size::Small;
        else if (_tcsicmp(pstrValue, _T("large")) == 0) size = Size::Large;
        SetButtonSize(size);
    }
    else {
        CButtonUI::SetAttribute(pstrName, pstrValue);
    }
}

void EButtonUI::ApplyStyle()
{
    const bool outlined = (m_variant == Variant::Default) || m_plain;
    const int width = outlined ? 1 : 0;
    m_rcBorderSize = { width, width, width, width };
}

DWORD EButtonUI::BackgroundColor() const
{
    if (!IsEnabled()) {
        if (m_variant == Variant::Default || m_variant == Variant::Text) {
            return 0;
        }
        if (m_plain) {
            return theme::PrimaryBg;
        }
        return MixColor(theme::Primary, theme::White, 60);
    }

    const bool hot = IsHot(m_uButtonState);
    const bool pushed = IsPushed(m_uButtonState);

    switch (m_variant) {
    case Variant::Default:
        if (pushed || hot) {
            return theme::PrimaryBg;
        }
        return theme::White;
    case Variant::Primary:
        if (m_plain) {
            if (pushed) return theme::PrimaryDark;
            if (hot) return theme::Primary;
            return theme::PrimaryBg;
        }
        if (pushed) return theme::PrimaryDark;
        if (hot) return theme::PrimaryLight;
        return theme::Primary;
    case Variant::Success:
        if (m_plain) {
            if (pushed) return theme::SuccessDark;
            if (hot) return theme::Success;
            return theme::SuccessBg;
        }
        if (pushed) return theme::SuccessDark;
        if (hot) return theme::SuccessLight;
        return theme::Success;
    case Variant::Warning:
        if (m_plain) {
            if (pushed) return theme::WarningDark;
            if (hot) return theme::Warning;
            return theme::WarningBg;
        }
        if (pushed) return theme::WarningDark;
        if (hot) return theme::WarningLight;
        return theme::Warning;
    case Variant::Danger:
        if (m_plain) {
            if (pushed) return theme::DangerDark;
            if (hot) return theme::Danger;
            return theme::DangerBg;
        }
        if (pushed) return theme::DangerDark;
        if (hot) return theme::DangerLight;
        return theme::Danger;
    case Variant::Info:
        if (m_plain) {
            if (pushed) return theme::InfoDark;
            if (hot) return theme::Info;
            return theme::InfoBg;
        }
        if (pushed) return theme::InfoDark;
        if (hot) return theme::InfoLight;
        return theme::Info;
    case Variant::Text:
        return 0;
    }
    return theme::White;
}

DWORD EButtonUI::BorderColor() const
{
    if (m_variant == Variant::Default) {
        if (!IsEnabled()) {
            return theme::BorderLight;
        }
        if (IsHot(m_uButtonState) || IsPushed(m_uButtonState)) {
            return theme::Primary;
        }
        return theme::BorderBase;
    }
    if (!m_plain) {
        return 0;
    }

    DWORD base = theme::PrimaryBorder;
    DWORD main = theme::Primary;
    switch (m_variant) {
    case Variant::Success:
        base = theme::SuccessBorder;
        main = theme::Success;
        break;
    case Variant::Warning:
        base = theme::WarningBorder;
        main = theme::Warning;
        break;
    case Variant::Danger:
        base = theme::DangerBorder;
        main = theme::Danger;
        break;
    case Variant::Info:
        base = theme::InfoBorder;
        main = theme::Info;
        break;
    default:
        break;
    }
    if (!IsEnabled()) {
        return MixColor(base, theme::White, 30);
    }
    if (IsHot(m_uButtonState) || IsPushed(m_uButtonState)) {
        return main;
    }
    return base;
}

DWORD EButtonUI::TextColor() const
{
    if (!IsEnabled()) {
        if (m_variant == Variant::Default || m_variant == Variant::Text) {
            return theme::TextPlaceholder;
        }
        return m_plain ? MixColor(theme::Primary, theme::White, 40) : theme::White;
    }

    switch (m_variant) {
    case Variant::Default:
        return (IsHot(m_uButtonState) || IsPushed(m_uButtonState)) ? theme::PrimaryDark : theme::TextRegular;
    case Variant::Text:
        if (IsPushed(m_uButtonState)) return theme::PrimaryDark;
        if (IsHot(m_uButtonState)) return theme::PrimaryLight;
        return theme::Primary;
    case Variant::Primary:
    case Variant::Success:
    case Variant::Warning:
    case Variant::Danger:
    case Variant::Info:
        if (m_plain) {
            if (IsHot(m_uButtonState) || IsPushed(m_uButtonState)) {
                return theme::White;
            }
            switch (m_variant) {
            case Variant::Success: return theme::Success;
            case Variant::Warning: return theme::Warning;
            case Variant::Danger: return theme::Danger;
            case Variant::Info: return theme::Info;
            default: return theme::Primary;
            }
        }
        return theme::White;
    }
    return theme::TextRegular;
}

void EButtonUI::PaintBkColor(HDC hDC)
{
    const DWORD color = BackgroundColor();
    if (color != 0) {
        CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(color));
    }
}

void EButtonUI::PaintStatusImage(HDC hDC)
{
    (void)hDC;
}

void EButtonUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, TextColor(), m_iFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void EButtonUI::PaintBorder(HDC hDC)
{
    const DWORD color = BorderColor();
    if (color == 0 || m_rcBorderSize.left <= 0) {
        return;
    }
    CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_rcBorderSize.left,
        m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(color), m_nBorderStyle);
}

//////////////////////////////////////////////////////////////////////////
// ELinkUI

ELinkUI::ELinkUI()
    : m_type(Type::Default)
    , m_underline(false)
{
    SetFixedHeight(24);
    SetFont(font::Body);
    m_rcTextPadding = { 0, 0, 0, 0 };
    SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

LPCTSTR ELinkUI::GetClass() const
{
    return _T("ELink");
}

LPVOID ELinkUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ELink")) == 0) {
        return static_cast<ELinkUI*>(this);
    }
    return CButtonUI::GetInterface(pstrName);
}

void ELinkUI::SetType(Type type)
{
    m_type = type;
    Invalidate();
}

void ELinkUI::SetUnderline(bool underline)
{
    m_underline = underline;
    Invalidate();
}

void ELinkUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("type")) == 0) {
        Type type = Type::Default;
        if (_tcsicmp(pstrValue, _T("primary")) == 0) type = Type::Primary;
        else if (_tcsicmp(pstrValue, _T("success")) == 0) type = Type::Success;
        else if (_tcsicmp(pstrValue, _T("warning")) == 0) type = Type::Warning;
        else if (_tcsicmp(pstrValue, _T("danger")) == 0) type = Type::Danger;
        else if (_tcsicmp(pstrValue, _T("info")) == 0) type = Type::Info;
        SetType(type);
    }
    else if (_tcsicmp(pstrName, _T("underline")) == 0) {
        SetUnderline(_tcsicmp(pstrValue, _T("true")) == 0);
    }
    else {
        CButtonUI::SetAttribute(pstrName, pstrValue);
    }
}

void ELinkUI::PaintBkColor(HDC hDC)
{
    (void)hDC;
}

void ELinkUI::PaintStatusImage(HDC hDC)
{
    (void)hDC;
}

void ELinkUI::PaintBorder(HDC hDC)
{
    (void)hDC;
}

DWORD ELinkUI::TextColor() const
{
    if (!IsEnabled()) {
        return theme::TextPlaceholder;
    }
    const bool hover = IsHot(m_uButtonState) || IsPushed(m_uButtonState);
    switch (m_type) {
    case Type::Success: return hover ? theme::SuccessLight : theme::Success;
    case Type::Warning: return hover ? theme::WarningLight : theme::Warning;
    case Type::Danger: return hover ? theme::DangerLight : theme::Danger;
    case Type::Info: return hover ? theme::InfoLight : theme::Info;
    case Type::Default:
    case Type::Primary:
    default: return hover ? theme::PrimaryLight : theme::Primary;
    }
}

void ELinkUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    const DWORD color = TextColor();
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, color, m_iFont,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    if (!IsEnabled() || !m_underline || !IsHot(m_uButtonState)) {
        return;
    }
    const SIZE size = CRenderEngine::GetTextSize(hDC, m_pManager, m_sText, m_iFont,
        DT_LEFT | DT_SINGLELINE);
    if (size.cx <= 0) {
        return;
    }
    const int y = rc.top + (rc.bottom - rc.top + size.cy) / 2;
    RECT line = { rc.left, y, rc.left + size.cx, y };
    CRenderEngine::DrawLine(hDC, line, std::max(1, UiScale::Px(1)), color);
}

//////////////////////////////////////////////////////////////////////////
// ENavItemUI

ENavItemUI::ENavItemUI()
    : m_selected(false)
{
    SetFixedHeight(UiScale::Px(36));
    SetFont(font::Body);
    m_rcTextPadding = { UiScale::Px(16), 0, UiScale::Px(10), 0 };
    m_cxyBorderRound.cx = UiScale::Px(4);
    m_cxyBorderRound.cy = UiScale::Px(4);
    SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

LPCTSTR ENavItemUI::GetClass() const
{
    return _T("ENavItem");
}

LPVOID ENavItemUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ENavItem")) == 0) {
        return static_cast<ENavItemUI*>(this);
    }
    return CButtonUI::GetInterface(pstrName);
}

void ENavItemUI::SetSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    Invalidate();
}

bool ENavItemUI::IsSelected() const
{
    return m_selected;
}

void ENavItemUI::PaintBkColor(HDC hDC)
{
    if (m_selected) {
        CRenderEngine::DrawColor(hDC, m_rcItem, theme::PrimaryBg);
        RECT bar = { m_rcItem.left, m_rcItem.top + UiScale::Px(9), m_rcItem.left + UiScale::Px(3), m_rcItem.bottom - UiScale::Px(9) };
        CRenderEngine::DrawColor(hDC, bar, theme::Primary);
    }
    else if (IsHot(m_uButtonState)) {
        CRenderEngine::DrawColor(hDC, m_rcItem, theme::FillLight);
    }
}

void ENavItemUI::PaintStatusImage(HDC hDC)
{
    (void)hDC;
}

void ENavItemUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    const DWORD color = m_selected ? theme::Primary : theme::TextRegular;
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, color, m_iFont,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void ENavItemUI::PaintBorder(HDC hDC)
{
    (void)hDC;
}

//////////////////////////////////////////////////////////////////////////
// ETabButtonUI

ETabButtonUI::ETabButtonUI()
    : m_selected(false)
{
    SetFixedHeight(38);
    SetFont(font::Body);
    m_rcTextPadding = { 18, 0, 18, 0 };
    SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

LPCTSTR ETabButtonUI::GetClass() const
{
    return _T("ETabButton");
}

LPVOID ETabButtonUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ETabButton")) == 0) {
        return static_cast<ETabButtonUI*>(this);
    }
    return CButtonUI::GetInterface(pstrName);
}

void ETabButtonUI::SetSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    Invalidate();
}

bool ETabButtonUI::IsSelected() const
{
    return m_selected;
}

void ETabButtonUI::PaintBkColor(HDC hDC)
{
    (void)hDC;
}

void ETabButtonUI::PaintStatusImage(HDC hDC)
{
    if (!m_selected) {
        return;
    }
    RECT bar = { m_rcItem.left, m_rcItem.bottom - UiScale::Px(2), m_rcItem.right, m_rcItem.bottom };
    CRenderEngine::DrawColor(hDC, bar, theme::Primary);
}

void ETabButtonUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    DWORD color = theme::TextRegular;
    if (!IsEnabled()) {
        color = theme::TextPlaceholder;
    }
    else if (m_selected) {
        color = theme::Primary;
    }
    else if (IsHot(m_uButtonState)) {
        color = theme::PrimaryLight;
    }
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, color, m_iFont,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void ETabButtonUI::PaintBorder(HDC hDC)
{
    (void)hDC;
}

//////////////////////////////////////////////////////////////////////////
// EOptionUI

EOptionUI::EOptionUI()
{
    SetFixedHeight(30);
    SetFont(font::Body);
    m_uTextStyle = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
}

LPCTSTR EOptionUI::GetClass() const
{
    return _T("ERadio");
}

LPVOID EOptionUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ERadio")) == 0) {
        return static_cast<EOptionUI*>(this);
    }
    return COptionUI::GetInterface(pstrName);
}

void EOptionUI::PaintStatusImage(HDC hDC)
{
    const int diameter = UiScale::Px(14);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    RECT circle = { m_rcItem.left, cy - diameter / 2, m_rcItem.left + diameter, cy - diameter / 2 + diameter };

    const bool selected = IsSelected();
    DWORD border = theme::BorderBase;
    if (!IsEnabled()) {
        border = theme::BorderLight;
    }
    else if (selected || IsHot(m_uButtonState)) {
        border = theme::Primary;
    }

    DWORD fill = IsEnabled() ? theme::White : theme::FillLight;
    FillEllipseColor(hDC, circle, fill, border, std::max(1, UiScale::Px(1)));

    if (selected) {
        const int dot = UiScale::Px(6);
        RECT inner = { circle.left + (diameter - dot) / 2, circle.top + (diameter - dot) / 2,
            circle.left + (diameter - dot) / 2 + dot, circle.top + (diameter - dot) / 2 + dot };
        FillEllipseColor(hDC, inner, IsEnabled() ? theme::Primary : theme::TextPlaceholder, 0, 1);
    }
}

void EOptionUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += UiScale::Px(22);
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText,
        IsEnabled() ? theme::TextRegular : theme::TextPlaceholder, m_iFont,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

//////////////////////////////////////////////////////////////////////////
// ECheckBoxUI

ECheckBoxUI::ECheckBoxUI()
{
    SetFixedHeight(30);
    SetFont(font::Body);
    m_uTextStyle = DT_LEFT | DT_VCENTER | DT_SINGLELINE;
}

LPCTSTR ECheckBoxUI::GetClass() const
{
    return _T("ECheckBox");
}

LPVOID ECheckBoxUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ECheckBox")) == 0) {
        return static_cast<ECheckBoxUI*>(this);
    }
    return CCheckBoxUI::GetInterface(pstrName);
}

void ECheckBoxUI::PaintStatusImage(HDC hDC)
{
    const int size = UiScale::Px(14);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    RECT box = { m_rcItem.left, cy - size / 2, m_rcItem.left + size, cy - size / 2 + size };

    const bool checked = IsSelected();
    DWORD fill = theme::White;
    DWORD border = theme::BorderBase;
    if (!IsEnabled()) {
        fill = checked ? theme::TextPlaceholder : theme::FillLight;
        border = theme::BorderLight;
    }
    else if (checked) {
        fill = theme::Primary;
        border = theme::Primary;
    }
    else if (IsHot(m_uButtonState)) {
        border = theme::Primary;
    }

    HBRUSH brush = ::CreateSolidBrush(GdiColor(fill));
    HPEN pen = ::CreatePen(PS_SOLID, std::max(1, UiScale::Px(1)), GdiColor(border));
    HGDIOBJ oldBrush = ::SelectObject(hDC, brush);
    HGDIOBJ oldPen = ::SelectObject(hDC, pen);
    ::RoundRect(hDC, box.left, box.top, box.right, box.bottom, UiScale::Px(4), UiScale::Px(4));
    ::SelectObject(hDC, oldBrush);
    ::SelectObject(hDC, oldPen);
    ::DeleteObject(brush);
    ::DeleteObject(pen);

    if (checked) {
        DrawCheckMark(hDC, box, theme::White);
    }
}

void ECheckBoxUI::PaintText(HDC hDC)
{
    if (m_sText.IsEmpty() || m_pManager == nullptr) {
        return;
    }
    RECT rc = m_rcItem;
    rc.left += UiScale::Px(22);
    CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText,
        IsEnabled() ? theme::TextRegular : theme::TextPlaceholder, m_iFont,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

//////////////////////////////////////////////////////////////////////////
// EProgressUI

EProgressUI::EProgressUI()
{
    SetMinValue(0);
    SetMaxValue(100);
    SetFixedHeight(6);
}

LPCTSTR EProgressUI::GetClass() const
{
    return _T("EProgress");
}

LPVOID EProgressUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EProgress")) == 0) {
        return static_cast<EProgressUI*>(this);
    }
    return CProgressUI::GetInterface(pstrName);
}

void EProgressUI::PaintStatusImage(HDC hDC)
{
    const int height = m_rcItem.bottom - m_rcItem.top;
    if (height <= 0 || m_rcItem.right <= m_rcItem.left) {
        return;
    }

    FillRoundRectColor(hDC, m_rcItem, height, theme::BorderLighter);

    const int range = GetMaxValue() - GetMinValue();
    if (range <= 0) {
        return;
    }
    int width = (GetValue() - GetMinValue()) * (m_rcItem.right - m_rcItem.left) / range;
    if (width <= 0) {
        return;
    }
    RECT fill = { m_rcItem.left, m_rcItem.top, m_rcItem.left + width, m_rcItem.bottom };
    FillRoundRectColor(hDC, fill, std::min(height, width), theme::Primary);
}

//////////////////////////////////////////////////////////////////////////
// ESliderUI

ESliderUI::ESliderUI()
{
    SetMinValue(0);
    SetMaxValue(100);
    SetThumbSize({ UiScale::Px(16), UiScale::Px(16) });
    SetFixedHeight(24);
}

LPCTSTR ESliderUI::GetClass() const
{
    return _T("ESlider");
}

LPVOID ESliderUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ESlider")) == 0) {
        return static_cast<ESliderUI*>(this);
    }
    return CSliderUI::GetInterface(pstrName);
}

void ESliderUI::PaintStatusImage(HDC hDC)
{
    const int trackHeight = UiScale::Px(6);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    RECT track = { m_rcItem.left, cy - trackHeight / 2, m_rcItem.right, cy - trackHeight / 2 + trackHeight };
    if (track.right <= track.left) {
        return;
    }

    FillRoundRectColor(hDC, track, trackHeight, theme::BorderLight);

    const int range = GetMaxValue() - GetMinValue();
    RECT thumb = GetThumbRect();
    const int thumbCenter = (thumb.left + thumb.right) / 2;
    if (range > 0 && thumbCenter > track.left) {
        RECT fill = { track.left, track.top, (std::min)(static_cast<LONG>(thumbCenter), track.right), track.bottom };
        FillRoundRectColor(hDC, fill, (std::min)(trackHeight, static_cast<int>(fill.right - fill.left)),
            IsEnabled() ? theme::Primary : theme::TextPlaceholder);
    }

    FillEllipseColor(hDC, thumb, theme::White,
        IsEnabled() ? theme::Primary : theme::TextPlaceholder, std::max(2, UiScale::Px(2)));
}

//////////////////////////////////////////////////////////////////////////
// EInputNumberUI

EInputNumberUI::EInputNumberUI()
    : m_edit(nullptr)
    , m_value(0)
    , m_min(-999999)
    , m_max(999999)
    , m_step(1)
{
    SetFixedHeight(32);
    SetInset({ UiScale::Px(32), 1, UiScale::Px(32), 1 });
    m_cxyBorderRound.cx = 4;
    m_cxyBorderRound.cy = 4;
    m_rcBorderSize = { 1, 1, 1, 1 };

    m_edit = new EInputNumberEditUI(this);
    m_edit->SetFont(font::Body);
    m_edit->SetTextColor(theme::TextPrimary);
    m_edit->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_edit->SetTextPadding({ 0, 0, 0, 0 });
    m_edit->SetNumberOnly(true);
    m_edit->SetBorderSize({ 0, 0, 0, 0 });
    m_edit->SetBkColor(theme::White);
    m_edit->SetNativeEditBkColor(theme::White);
    Add(m_edit);
    UpdateEditText();
}

LPCTSTR EInputNumberUI::GetClass() const
{
    return _T("EInputNumber");
}

LPVOID EInputNumberUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EInputNumber")) == 0) {
        return static_cast<EInputNumberUI*>(this);
    }
    return CHorizontalLayoutUI::GetInterface(pstrName);
}

UINT EInputNumberUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void EInputNumberUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("value")) == 0) {
        SetValue(_wtoi(pstrValue));
    }
    else if (_tcsicmp(pstrName, _T("min")) == 0) {
        m_min = _wtoi(pstrValue);
        SetValue(m_value);
    }
    else if (_tcsicmp(pstrName, _T("max")) == 0) {
        m_max = _wtoi(pstrValue);
        SetValue(m_value);
    }
    else if (_tcsicmp(pstrName, _T("step")) == 0) {
        m_step = std::max(1, _wtoi(pstrValue));
    }
    else {
        CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
    }
}

void EInputNumberUI::SetEnabled(bool bEnable)
{
    CHorizontalLayoutUI::SetEnabled(bEnable);
    if (m_edit != nullptr) {
        m_edit->SetEnabled(bEnable);
    }
    Invalidate();
}

void EInputNumberUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_SETCURSOR && IsEnabled()) {
        const POINT pt = event.ptMouse;
        if (pt.x < m_rcItem.left + m_rcInset.left || pt.x > m_rcItem.right - m_rcInset.right) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
            return;
        }
    }
    if (event.Type == UIEVENT_BUTTONUP && IsEnabled()) {
        if (event.ptMouse.x < m_rcItem.left + m_rcInset.left) {
            Step(-1);
            return;
        }
        if (event.ptMouse.x > m_rcItem.right - m_rcInset.right) {
            Step(1);
            return;
        }
    }
    CHorizontalLayoutUI::DoEvent(event);
}

void EInputNumberUI::PaintBkColor(HDC hDC)
{
    const DWORD background = IsEnabled() ? theme::White : theme::FillLight;
    FillRoundRectColor(hDC, m_rcItem, m_cxyBorderRound.cx, background);

    const DWORD glyphColor = IsEnabled() ? theme::TextRegular : theme::TextPlaceholder;
    RECT left = { m_rcItem.left, m_rcItem.top, m_rcItem.left + m_rcInset.left, m_rcItem.bottom };
    RECT right = { m_rcItem.right - m_rcInset.right, m_rcItem.top, m_rcItem.right, m_rcItem.bottom };
    CRenderEngine::DrawText(hDC, m_pManager, left, _T("-"), glyphColor, font::Body14,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    CRenderEngine::DrawText(hDC, m_pManager, right, _T("+"), glyphColor, font::Body14,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    const int lineTop = m_rcItem.top + UiScale::Px(6);
    const int lineBottom = m_rcItem.bottom - UiScale::Px(6);
    RECT leftSep = { m_rcItem.left + m_rcInset.left, lineTop, m_rcItem.left + m_rcInset.left, lineBottom };
    RECT rightSep = { m_rcItem.right - m_rcInset.right, lineTop, m_rcItem.right - m_rcInset.right, lineBottom };
    CRenderEngine::DrawLine(hDC, leftSep, 1, theme::BorderLight);
    CRenderEngine::DrawLine(hDC, rightSep, 1, theme::BorderLight);
}

void EInputNumberUI::PaintBorder(HDC hDC)
{
    const DWORD color = IsEnabled() ? theme::BorderBase : theme::BorderLight;
    CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_rcBorderSize.left,
        m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(color), m_nBorderStyle);
}

int EInputNumberUI::GetValue() const
{
    return m_value;
}

int EInputNumberUI::Clamp(int value) const
{
    if (value < m_min) {
        return m_min;
    }
    if (value > m_max) {
        return m_max;
    }
    return value;
}

void EInputNumberUI::SetValue(int value)
{
    m_value = Clamp(value);
    UpdateEditText();
    Invalidate();
}

void EInputNumberUI::UpdateEditText()
{
    if (m_edit != nullptr) {
        m_edit->SetText(std::to_wstring(m_value).c_str());
    }
}

void EInputNumberUI::CommitEdit()
{
    if (m_edit == nullptr || !IsEnabled()) {
        return;
    }
    const int value = Clamp(_wtoi(m_edit->GetText()));
    if (value == m_value) {
        UpdateEditText();
        return;
    }
    SetValue(value);
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
    }
}

void EInputNumberUI::Step(int delta)
{
    if (!IsEnabled()) {
        return;
    }
    const int value = Clamp(m_value + delta * m_step);
    if (value == m_value) {
        return;
    }
    SetValue(value);
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
    }
}

EInputNumberEditUI::EInputNumberEditUI(EInputNumberUI* owner)
    : m_owner(owner)
{
}

LPCTSTR EInputNumberEditUI::GetClass() const
{
    return _T("EInputNumberEdit");
}

LPVOID EInputNumberEditUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EInputNumberEdit")) == 0) {
        return static_cast<EInputNumberEditUI*>(this);
    }
    return CEditUI::GetInterface(pstrName);
}

void EInputNumberEditUI::DoEvent(TEventUI& event)
{
    const bool commit = (event.Type == UIEVENT_KILLFOCUS);
    CEditUI::DoEvent(event);
    if (commit && m_owner != nullptr) {
        m_owner->CommitEdit();
    }
}

void EInputNumberEditUI::Commit()
{
    if (m_owner != nullptr) {
        m_owner->CommitEdit();
    }
}

//////////////////////////////////////////////////////////////////////////
// ECascaderUI

namespace {

bool CascaderContains(const std::wstring& text, const std::wstring& query)
{
    if (query.empty()) {
        return true;
    }
    std::wstring lowerText = text;
    std::wstring lowerQuery = query;
    for (wchar_t& ch : lowerText) {
        ch = static_cast<wchar_t>(towlower(ch));
    }
    for (wchar_t& ch : lowerQuery) {
        ch = static_cast<wchar_t>(towlower(ch));
    }
    return lowerText.find(lowerQuery) != std::wstring::npos;
}

int CascaderTextWidth(DuiLib::CPaintManagerUI* manager, const std::wstring& text, int fontId)
{
    if (manager == nullptr || text.empty()) {
        return 0;
    }
    HDC hDC = ::GetDC(manager->GetPaintWindow());
    const SIZE size = DuiLib::CRenderEngine::GetTextSize(hDC, manager, text.c_str(), fontId, DT_SINGLELINE);
    ::ReleaseDC(manager->GetPaintWindow(), hDC);
    return size.cx;
}

std::wstring JoinLabels(const std::vector<std::wstring>& labels, const wchar_t* separator)
{
    std::wstring text;
    for (size_t i = 0; i < labels.size(); ++i) {
        if (i > 0) {
            text += separator;
        }
        text += labels[i];
    }
    return text;
}

std::wstring JoinSelectedLabels(const std::vector<std::wstring>& labels)
{
    std::wstring text;
    for (size_t i = 0; i < labels.size(); ++i) {
        if (!text.empty()) {
            text += L"、";
        }
        text += labels[i];
    }
    return text;
}

void DrawCascaderCheck(HDC hDC, int right, int cy, DWORD color)
{
    const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(4));
    Gdiplus::PointF points[3] = {
        { static_cast<Gdiplus::REAL>(right) - arm * 2.0f, static_cast<Gdiplus::REAL>(cy) + arm * 0.1f },
        { static_cast<Gdiplus::REAL>(right) - arm * 1.1f, static_cast<Gdiplus::REAL>(cy) + arm * 0.9f },
        { static_cast<Gdiplus::REAL>(right) - arm * 0.1f, static_cast<Gdiplus::REAL>(cy) - arm * 0.9f } };
    DrawLinesAa(hDC, points, 3, color, 1.6f);
}

void DrawCascaderChevron(HDC hDC, int right, int cy, DWORD color)
{
    const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
    Gdiplus::PointF points[3] = {
        { static_cast<Gdiplus::REAL>(right) - arm, static_cast<Gdiplus::REAL>(cy) - arm },
        { static_cast<Gdiplus::REAL>(right), static_cast<Gdiplus::REAL>(cy) },
        { static_cast<Gdiplus::REAL>(right) - arm, static_cast<Gdiplus::REAL>(cy) + arm } };
    DrawLinesAa(hDC, points, 3, color, 1.4f);
}

void DrawCascaderCheckbox(HDC hDC, const RECT& rc, bool checked, bool enabled)
{
    RECT box = rc;
    const DWORD border = !enabled ? theme::BorderLight : (checked ? theme::Primary : theme::BorderBase);
    const DWORD fill = checked ? (enabled ? theme::Primary : theme::PrimaryDisabled) : theme::White;
    FillRoundRectColor(hDC, box, UiScale::Px(4), fill);
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, box, UiScale::Px(4));
    StrokePathAa(hDC, path, border, 1);
    if (checked) {
        const int cy = (box.top + box.bottom) / 2;
        DrawCascaderCheck(hDC, box.right - UiScale::Px(3), cy, theme::White);
    }
}

void DrawCascaderRadio(HDC hDC, int cx, int cy, bool checked, bool enabled)
{
    const Gdiplus::REAL radius = static_cast<Gdiplus::REAL>(UiScale::Px(7));
    const Gdiplus::RectF outer(static_cast<Gdiplus::REAL>(cx) - radius, static_cast<Gdiplus::REAL>(cy) - radius,
        radius * 2.0f, radius * 2.0f);
    const DWORD color = !enabled ? theme::BorderLight : (checked ? theme::Primary : theme::BorderBase);
    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 255, 255));
    graphics.FillEllipse(&brush, outer);
    Gdiplus::Pen pen(GpColor(color), 1.0f);
    graphics.DrawEllipse(&pen, outer);
    if (checked) {
        const Gdiplus::REAL dot = radius * 0.45f;
        const Gdiplus::RectF inner(static_cast<Gdiplus::REAL>(cx) - dot, static_cast<Gdiplus::REAL>(cy) - dot,
            dot * 2.0f, dot * 2.0f);
        Gdiplus::SolidBrush dotBrush(GpColor(color));
        graphics.FillEllipse(&dotBrush, inner);
    }
}

} // namespace

ECascaderUI::ECascaderUI()
    : m_multiple(false)
    , m_clearable(false)
    , m_checkStrictly(false)
    , m_filterable(false)
    , m_lazy(false)
    , m_showAllLevels(true)
    , m_collapseTags(false)
    , m_open(false)
    , m_hover(false)
    , m_placeholder(L"请选择")
{
    SetFixedHeight(32);
    m_cxyBorderRound.cx = 4;
    m_cxyBorderRound.cy = 4;
}

LPCTSTR ECascaderUI::GetClass() const
{
    return _T("ECascader");
}

LPVOID ECascaderUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ECascader")) == 0) {
        return static_cast<ECascaderUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

UINT ECascaderUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void ECascaderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("placeholder")) == 0) {
        m_placeholder = pstrValue != nullptr ? pstrValue : L"";
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("multiple")) == 0) {
        m_multiple = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("clearable")) == 0) {
        m_clearable = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("checkstrictly")) == 0) {
        m_checkStrictly = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("filterable")) == 0) {
        m_filterable = _tcsicmp(pstrValue, _T("true")) == 0;
    }
    else if (_tcsicmp(pstrName, _T("lazy")) == 0) {
        m_lazy = _tcsicmp(pstrValue, _T("true")) == 0;
    }
    else if (_tcsicmp(pstrName, _T("showall")) == 0) {
        m_showAllLevels = _tcsicmp(pstrValue, _T("false")) != 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("collapsetags")) == 0) {
        m_collapseTags = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("demo")) == 0) {
        m_demo = pstrValue != nullptr ? pstrValue : L"";
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

void ECascaderUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    Invalidate();
}

void ECascaderUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_SETCURSOR && IsEnabled()) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return;
    }
    if (event.Type == UIEVENT_MOUSEENTER || event.Type == UIEVENT_MOUSEMOVE) {
        if (!m_hover) {
            m_hover = true;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hover) {
            m_hover = false;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_BUTTONUP && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse)) {
        RECT clear = ClearRect();
        if (m_clearable && HasSelection() && ::PtInRect(&clear, event.ptMouse)) {
            m_paths.clear();
            m_labels.clear();
            NotifyChanged();
            if (m_pManager != nullptr) {
                m_pManager->SendNotify(this, _T("cascaderclear"));
            }
            return;
        }
        if (m_pManager != nullptr) {
            m_pManager->SendNotify(this, _T("cascadertoggle"));
        }
        return;
    }
    CControlUI::DoEvent(event);
}

void ECascaderUI::PaintBkColor(HDC hDC)
{
    FillRoundRectColor(hDC, m_rcItem, m_cxyBorderRound.cx,
        IsEnabled() ? theme::White : theme::FillLight);
}

void ECascaderUI::PaintBorder(HDC hDC)
{
    DWORD color = theme::BorderBase;
    if (!IsEnabled()) {
        color = theme::BorderLight;
    }
    else if (m_open) {
        color = theme::Primary;
    }
    else if (m_hover) {
        color = theme::TextPlaceholder;
    }

    RECT rc = m_rcItem;
    rc.left += 1;
    rc.top += 1;
    rc.right -= 1;
    rc.bottom -= 1;
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, rc, m_cxyBorderRound.cx);
    StrokePathAa(hDC, path, color, 1);
}

void ECascaderUI::PaintStatusImage(HDC hDC)
{
    if (m_pManager == nullptr) {
        return;
    }

    const bool enabled = IsEnabled();
    const DWORD valueColor = enabled ? theme::TextPrimary : theme::TextPlaceholder;

    if (m_multiple && !m_labels.empty()) {
        std::vector<RECT> tags;
        std::vector<RECT> closes;
        std::vector<int> indexes;
        int plusWidth = 0;
        BuildTagLayout(tags, closes, indexes, plusWidth);
        for (size_t i = 0; i < tags.size(); ++i) {
            FillRoundRectColor(hDC, tags[i], UiScale::Px(4), theme::FillLight);
            Gdiplus::GraphicsPath border;
            MakeRoundRectPath(border, tags[i], UiScale::Px(4));
            StrokePathAa(hDC, border, theme::BorderLight, 1);

            const std::vector<std::wstring>& labels = m_labels[indexes[i]];
            const std::wstring& leaf = labels.empty() ? m_placeholder : labels.back();
            RECT textRc = { tags[i].left + UiScale::Px(8), tags[i].top,
                tags[i].right - UiScale::Px(18), tags[i].bottom };
            CRenderEngine::DrawText(hDC, m_pManager, textRc, leaf.c_str(), valueColor,
                font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            const int cx = (closes[i].left + closes[i].right) / 2;
            const int cy = (closes[i].top + closes[i].bottom) / 2;
            const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
            Gdiplus::PointF cross[4] = {
                { cx - arm, cy - arm }, { cx + arm, cy + arm },
                { cx - arm, cy + arm }, { cx + arm, cy - arm } };
            DrawLinesAa(hDC, cross, 2, theme::TextPlaceholder, 1.0f);
            DrawLinesAa(hDC, cross + 2, 2, theme::TextPlaceholder, 1.0f);
        }
        if (plusWidth > 0 && !tags.empty()) {
            RECT plusRc = { tags.back().right + UiScale::Px(4), m_rcItem.top,
                tags.back().right + UiScale::Px(4) + plusWidth, m_rcItem.bottom };
            const int hidden = static_cast<int>(m_labels.size() - tags.size());
            const std::wstring text = L"+" + std::to_wstring(hidden);
            CRenderEngine::DrawText(hDC, m_pManager, plusRc, text.c_str(), theme::TextRegular,
                font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }
    else {
        std::wstring text;
        DWORD color = valueColor;
        if (!m_labels.empty()) {
            const std::vector<std::wstring>& labels = m_labels.front();
            if (labels.empty()) {
                text = m_placeholder;
                color = theme::TextPlaceholder;
            }
            else if (m_showAllLevels) {
                text = JoinLabels(labels, L" / ");
            }
            else {
                text = labels.back();
            }
        }
        else {
            text = m_placeholder;
            color = theme::TextPlaceholder;
        }
        RECT rc = { m_rcItem.left + UiScale::Px(15), m_rcItem.top,
            m_rcItem.right - UiScale::Px(32), m_rcItem.bottom };
        CRenderEngine::DrawText(hDC, m_pManager, rc, text.c_str(), color, font::Body,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    if (m_clearable && HasSelection() && m_hover && enabled) {
        RECT clear = ClearRect();
        FillEllipseColor(hDC, clear, theme::TextPlaceholder, 0, 0);
        const int cx = (clear.left + clear.right) / 2;
        const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
        Gdiplus::PointF cross[4] = {
            { cxF - arm, cyF - arm }, { cxF + arm, cyF + arm },
            { cxF - arm, cyF + arm }, { cxF + arm, cyF - arm } };
        DrawLinesAa(hDC, cross, 2, theme::White, 1.2f);
        DrawLinesAa(hDC, cross + 2, 2, theme::White, 1.2f);
    }
    else {
        const Gdiplus::REAL cx = static_cast<Gdiplus::REAL>(m_rcItem.right - UiScale::Px(22));
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        const Gdiplus::REAL half = static_cast<Gdiplus::REAL>(UiScale::Px(4));
        const Gdiplus::REAL drop = static_cast<Gdiplus::REAL>(UiScale::Px(2));
        const DWORD arrowColor = !enabled ? theme::TextPlaceholder
            : (m_open ? theme::Primary : theme::TextPlaceholder);
        Gdiplus::PointF points[3] = {};
        if (m_open) {
            points[0] = Gdiplus::PointF(cx - half, cyF + drop);
            points[1] = Gdiplus::PointF(cx, cyF - drop);
            points[2] = Gdiplus::PointF(cx + half, cyF + drop);
        }
        else {
            points[0] = Gdiplus::PointF(cx - half, cyF - drop);
            points[1] = Gdiplus::PointF(cx, cyF + drop);
            points[2] = Gdiplus::PointF(cx + half, cyF - drop);
        }
        DrawLinesAa(hDC, points, 3, arrowColor, 1.4f);
    }
}

void ECascaderUI::SetOptions(const std::vector<CascaderNode>& options)
{
    m_options = options;
    m_paths.clear();
    m_labels.clear();
    Invalidate();
}

const std::vector<CascaderNode>& ECascaderUI::GetOptions() const
{
    return m_options;
}

std::vector<std::vector<int>> ECascaderUI::GetSelection() const
{
    return m_paths;
}

std::vector<std::vector<std::wstring>> ECascaderUI::GetSelectionLabels() const
{
    return m_labels;
}

void ECascaderUI::ApplySelection(const std::vector<std::vector<int>>& paths,
    const std::vector<std::vector<std::wstring>>& labels)
{
    m_paths.clear();
    m_labels.clear();
    for (size_t i = 0; i < paths.size(); ++i) {
        if (paths[i].empty()) {
            continue;
        }
        if (!m_multiple && !m_paths.empty()) {
            break;
        }
        m_paths.push_back(paths[i]);
        m_labels.push_back(i < labels.size() ? labels[i] : std::vector<std::wstring>());
    }
    Invalidate();
}

void ECascaderUI::ClearSelection()
{
    m_paths.clear();
    m_labels.clear();
    Invalidate();
}

bool ECascaderUI::IsMultiple() const
{
    return m_multiple;
}

bool ECascaderUI::IsClearable() const
{
    return m_clearable;
}

bool ECascaderUI::IsCheckStrictly() const
{
    return m_checkStrictly;
}

bool ECascaderUI::IsFilterable() const
{
    return m_filterable;
}

bool ECascaderUI::IsLazy() const
{
    return m_lazy;
}

bool ECascaderUI::IsShowAllLevels() const
{
    return m_showAllLevels;
}

bool ECascaderUI::IsCollapseTags() const
{
    return m_collapseTags;
}

void ECascaderUI::SetOpen(bool open)
{
    if (m_open == open) {
        return;
    }
    m_open = open;
    Invalidate();
}

std::wstring ECascaderUI::GetDisplayText() const
{
    if (m_labels.empty()) {
        return L"（未选择）";
    }
    if (m_multiple) {
        std::vector<std::wstring> leaves;
        for (const std::vector<std::wstring>& labels : m_labels) {
            if (!labels.empty()) {
                leaves.push_back(labels.back());
            }
        }
        return leaves.empty() ? L"（未选择）" : JoinSelectedLabels(leaves);
    }
    const std::vector<std::wstring>& labels = m_labels.front();
    if (labels.empty()) {
        return L"（未选择）";
    }
    return m_showAllLevels ? JoinLabels(labels, L" / ") : labels.back();
}

std::wstring ECascaderUI::GetDemoTitle() const
{
    return m_demo;
}

bool ECascaderUI::HasSelection() const
{
    return !m_paths.empty();
}

RECT ECascaderUI::ClearRect() const
{
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    const int diameter = UiScale::Px(14);
    const int right = m_rcItem.right - UiScale::Px(15);
    return { right - diameter, cy - diameter / 2, right, cy + diameter / 2 };
}

int ECascaderUI::TextWidth(const std::wstring& text, int fontId) const
{
    return CascaderTextWidth(m_pManager, text, fontId);
}

void ECascaderUI::BuildTagLayout(std::vector<RECT>& tags, std::vector<RECT>& closes,
    std::vector<int>& indexes, int& plusWidth) const
{
    tags.clear();
    closes.clear();
    indexes.clear();
    plusWidth = 0;
    if (m_labels.empty()) {
        return;
    }

    const int height = m_rcItem.bottom - m_rcItem.top;
    const int tagHeight = (std::max)(UiScale::Px(20), height - UiScale::Px(10));
    const int tagTop = m_rcItem.top + (height - tagHeight) / 2;
    const int limit = m_rcItem.right - UiScale::Px(34);
    int x = m_rcItem.left + UiScale::Px(8);
    int hidden = 0;

    for (size_t i = 0; i < m_labels.size(); ++i) {
        const std::wstring& leaf = m_labels[i].empty() ? std::wstring() : m_labels[i].back();
        const int width = TextWidth(leaf, font::Body) + UiScale::Px(30);
        if (x + width > limit) {
            hidden = static_cast<int>(m_labels.size() - i);
            break;
        }
        RECT tag = { x, tagTop, x + width, tagTop + tagHeight };
        RECT close = { tag.right - UiScale::Px(18), tag.top, tag.right - UiScale::Px(4), tag.bottom };
        tags.push_back(tag);
        closes.push_back(close);
        indexes.push_back(static_cast<int>(i));
        x += width + UiScale::Px(6);
    }

    if (hidden > 0) {
        plusWidth = TextWidth(L"+" + std::to_wstring(hidden), font::Body) + UiScale::Px(4);
    }
}

void ECascaderUI::NotifyChanged()
{
    Invalidate();
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
    }
}

//////////////////////////////////////////////////////////////////////////
// ECascaderPopupUI

ECascaderPopupUI::ECascaderPopupUI()
    : m_multiple(false)
    , m_checkStrictly(false)
    , m_filterable(false)
    , m_lazy(false)
    , m_loading(false)
    , m_hoverColumn(-1)
    , m_hoverRow(-1)
    , m_rowHeight(UiScale::Px(34))
    , m_colWidth(UiScale::Px(180))
    , m_pad(UiScale::Px(6))
    , m_searchHeight(UiScale::Px(34))
    , m_edit(nullptr)
{
    SetBkColor(0);
    SetBorderSize({ 0, 0, 0, 0 });
    SetVisible(false);

    m_edit = new CEditUI;
    m_edit->SetName(_T("cascaderSearchEdit"));
    m_edit->SetFont(font::Body);
    m_edit->SetTextColor(theme::TextPrimary);
    m_edit->SetBkColor(theme::White);
    m_edit->SetNativeEditBkColor(theme::White);
    m_edit->SetBorderSize({ 0, 0, 0, 0 });
    m_edit->SetTextPadding({ 0, 0, 0, 0 });
    m_edit->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    m_edit->SetVisible(false);
    Add(m_edit);
}

LPCTSTR ECascaderPopupUI::GetClass() const
{
    return _T("ECascaderPopup");
}

LPVOID ECascaderPopupUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ECascaderPopup")) == 0) {
        return static_cast<ECascaderPopupUI*>(this);
    }
    return CContainerUI::GetInterface(pstrName);
}

void ECascaderPopupUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    LayoutSearchEdit();
}

void ECascaderPopupUI::SetVisible(bool bVisible)
{
    CContainerUI::SetVisible(bVisible);
    if (m_edit != nullptr) {
        m_edit->SetVisible(bVisible && m_filterable);
    }
    if (!bVisible && m_pManager != nullptr) {
        m_pManager->KillTimer(this, 1);
        m_loading = false;
    }
}

void ECascaderPopupUI::Open(const std::vector<CascaderNode>& options,
    const std::vector<std::vector<int>>& paths,
    const std::vector<std::vector<std::wstring>>& labels,
    bool multiple, bool checkStrictly, bool filterable, bool lazy)
{
    if (m_pManager != nullptr) {
        m_pManager->KillTimer(this, 1);
    }
    m_options = options;
    m_paths = paths;
    m_labels = labels;
    m_multiple = multiple;
    m_checkStrictly = checkStrictly;
    m_filterable = filterable;
    m_lazy = lazy;
    m_loading = false;
    m_loadingPath.clear();
    m_query.clear();
    m_browse.clear();
    m_results.clear();
    m_resultLabels.clear();
    m_scrolls.clear();
    m_hoverColumn = -1;
    m_hoverRow = -1;
    if (m_edit != nullptr) {
        m_edit->SetText(L"");
        m_edit->SetVisible(m_filterable);
    }
    UpdatePanelSize();
    Invalidate();
}

void ECascaderPopupUI::SetColumnWidth(int width)
{
    m_colWidth = (std::max)(UiScale::Px(180), width);
}

int ECascaderPopupUI::ColumnsTop() const
{
    return m_rcItem.top + m_pad + SearchAreaHeight();
}

int ECascaderPopupUI::SearchAreaHeight() const
{
    return m_filterable ? m_searchHeight : 0;
}

int ECascaderPopupUI::VisibleColumns() const
{
    if (m_filterable && !m_query.empty()) {
        return 1;
    }
    return (std::max)(1, static_cast<int>(m_browse.size()) + 1);
}

int ECascaderPopupUI::ColumnX(int column) const
{
    return m_rcItem.left + column * m_colWidth;
}

RECT ECascaderPopupUI::ColumnRect(int column) const
{
    return { ColumnX(column), ColumnsTop(), ColumnX(column) + m_colWidth, m_rcItem.bottom - m_pad };
}

int ECascaderPopupUI::RowHeight() const
{
    return m_rowHeight;
}

int ECascaderPopupUI::ViewRows() const
{
    return 6;
}

int ECascaderPopupUI::MaxRows() const
{
    if (m_filterable && !m_query.empty()) {
        return static_cast<int>(m_results.size()) + 1;
    }
    int rows = 1;
    const int columns = VisibleColumns();
    for (int column = 0; column < columns; ++column) {
        const std::vector<CascaderNode>* nodes = ColumnNodes(column);
        if (nodes != nullptr) {
            rows = (std::max)(rows, static_cast<int>(nodes->size()));
        }
    }
    return rows;
}

int ECascaderPopupUI::ContentWidth() const
{
    if (m_filterable && !m_query.empty()) {
        return (std::max)(m_colWidth, UiScale::Px(220));
    }
    return VisibleColumns() * m_colWidth;
}

int ECascaderPopupUI::ContentHeight() const
{
    const int rows = (std::min)(MaxRows(), ViewRows());
    return m_pad * 2 + SearchAreaHeight() + (std::max)(1, rows) * m_rowHeight;
}

int ECascaderPopupUI::ScrollAt(int column) const
{
    if (column < 0 || column >= static_cast<int>(m_scrolls.size())) {
        return 0;
    }
    return m_scrolls[column];
}

void ECascaderPopupUI::SetScroll(int column, int value)
{
    if (column < 0 || column >= static_cast<int>(m_scrolls.size())) {
        return;
    }
    m_scrolls[column] = value;
}

const std::vector<CascaderNode>* ECascaderPopupUI::NodesAt(const std::vector<int>& path) const
{
    const std::vector<CascaderNode>* nodes = &m_options;
    for (int index : path) {
        if (nodes == nullptr || index < 0 || index >= static_cast<int>(nodes->size())) {
            return nullptr;
        }
        nodes = &(*nodes)[index].children;
    }
    return nodes;
}

std::vector<CascaderNode>* ECascaderPopupUI::MutableNodesAt(const std::vector<int>& path)
{
    std::vector<CascaderNode>* nodes = &m_options;
    for (int index : path) {
        if (nodes == nullptr || index < 0 || index >= static_cast<int>(nodes->size())) {
            return nullptr;
        }
        nodes = &(*nodes)[index].children;
    }
    return nodes;
}

const std::vector<CascaderNode>* ECascaderPopupUI::ColumnNodes(int column) const
{
    const std::vector<CascaderNode>* nodes = &m_options;
    for (int level = 0; level < column; ++level) {
        if (level >= static_cast<int>(m_browse.size()) || nodes == nullptr
            || m_browse[level] < 0 || m_browse[level] >= static_cast<int>(nodes->size())) {
            return nullptr;
        }
        nodes = &(*nodes)[m_browse[level]].children;
    }
    return nodes;
}

std::vector<CascaderNode>* ECascaderPopupUI::MutableColumnNodes(int column)
{
    std::vector<CascaderNode>* nodes = &m_options;
    for (int level = 0; level < column; ++level) {
        if (level >= static_cast<int>(m_browse.size()) || nodes == nullptr
            || m_browse[level] < 0 || m_browse[level] >= static_cast<int>(nodes->size())) {
            return nullptr;
        }
        nodes = &(*nodes)[m_browse[level]].children;
    }
    return nodes;
}

bool ECascaderPopupUI::HasChildren(const CascaderNode& node) const
{
    return !node.children.empty() || node.lazy;
}

void ECascaderPopupUI::BrowseTo(const std::vector<int>& path)
{
    m_browse = path;
    m_scrolls.assign(path.size() + 2, 0);
    m_hoverColumn = -1;
    m_hoverRow = -1;
    const CascaderNode* node = nullptr;
    if (!path.empty()) {
        const std::vector<CascaderNode>* parents = NodesAt(
            std::vector<int>(path.begin(), path.end() - 1));
        if (parents != nullptr && path.back() >= 0 && path.back() < static_cast<int>(parents->size())) {
            node = &(*parents)[path.back()];
        }
    }
    if (node != nullptr && node->children.empty() && node->lazy && m_lazy) {
        StartLazy(path);
    }
    UpdatePanelSize();
    Invalidate();
}

void ECascaderPopupUI::TogglePath(const std::vector<int>& path, const std::vector<std::wstring>& labels)
{
    for (size_t i = 0; i < m_paths.size(); ++i) {
        if (m_paths[i] == path) {
            m_paths.erase(m_paths.begin() + i);
            if (i < m_labels.size()) {
                m_labels.erase(m_labels.begin() + i);
            }
            Invalidate();
            return;
        }
    }
    m_paths.push_back(path);
    m_labels.push_back(labels);
    Invalidate();
}

bool ECascaderPopupUI::IsPathSelected(const std::vector<int>& path) const
{
    for (const std::vector<int>& selected : m_paths) {
        if (selected == path) {
            return true;
        }
    }
    return false;
}

void ECascaderPopupUI::CollectResults(const std::vector<CascaderNode>& nodes,
    std::vector<int> prefix, std::vector<std::wstring> names)
{
    if (m_results.size() >= 20) {
        return;
    }
    for (size_t i = 0; i < nodes.size(); ++i) {
        const CascaderNode& node = nodes[i];
        if (node.disabled) {
            continue;
        }
        std::vector<int> path = prefix;
        path.push_back(static_cast<int>(i));
        std::vector<std::wstring> labels = names;
        labels.push_back(node.name);
        if (node.children.empty()) {
            if (!node.lazy && CascaderContains(JoinLabels(labels, L" / "), m_query)) {
                m_results.push_back(path);
                m_resultLabels.push_back(labels);
            }
            continue;
        }
        CollectResults(node.children, path, labels);
    }
}

void ECascaderPopupUI::BuildSearchResults()
{
    m_results.clear();
    m_resultLabels.clear();
    if (m_query.empty()) {
        return;
    }
    CollectResults(m_options, {}, {});
    m_hoverRow = -1;
}

void ECascaderPopupUI::SetQuery(const std::wstring& query)
{
    if (m_query == query) {
        return;
    }
    m_query = query;
    BuildSearchResults();
    UpdatePanelSize();
    Invalidate();
}

void ECascaderPopupUI::StartLazy(const std::vector<int>& path)
{
    m_loading = true;
    m_loadingPath = path;
    if (m_pManager != nullptr) {
        m_pManager->SetTimer(this, 1, 500);
    }
}

void ECascaderPopupUI::FinishLazy()
{
    if (!m_loading) {
        return;
    }
    std::vector<CascaderNode>* nodes = MutableNodesAt(m_loadingPath);
    if (nodes != nullptr && m_loadingPath.size() >= 1) {
        std::vector<CascaderNode>* parents = MutableNodesAt(
            std::vector<int>(m_loadingPath.begin(), m_loadingPath.end() - 1));
        if (parents != nullptr && m_loadingPath.back() < static_cast<int>(parents->size())) {
            CascaderNode& node = (*parents)[m_loadingPath.back()];
            const std::wstring name = node.name;
            struct LazyGroup { const wchar_t* parent; const wchar_t* children[3]; };
            static const LazyGroup groups[] = {
                { L"浙江", { L"杭州", L"宁波", nullptr } },
                { L"江苏", { L"南京", L"苏州", nullptr } },
                { L"广东", { L"广州", L"深圳", nullptr } },
                { L"杭州", { L"西湖区", L"滨江区", L"余杭区" } },
                { L"宁波", { L"海曙区", L"鄞州区", nullptr } },
                { L"南京", { L"玄武区", L"鼓楼区", nullptr } },
                { L"苏州", { L"姑苏区", L"工业园区", nullptr } },
                { L"广州", { L"天河区", L"越秀区", nullptr } },
                { L"深圳", { L"南山区", L"福田区", nullptr } },
            };
            for (const LazyGroup& group : groups) {
                if (name != group.parent) {
                    continue;
                }
                node.children.clear();
                for (const wchar_t* child : group.children) {
                    if (child == nullptr) {
                        break;
                    }
                    CascaderNode item;
                    item.name = child;
                    item.lazy = true;
                    node.children.push_back(item);
                }
                break;
            }
        }
    }
    m_loading = false;
    m_loadingPath.clear();
    UpdatePanelSize();
    Invalidate();
}

void ECascaderPopupUI::NotifyPick()
{
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, _T("cascaderpick"));
    }
}

ECascaderPopupUI::HitRow ECascaderPopupUI::HitTest(const POINT& pt) const
{
    HitRow hit;
    if (!::PtInRect(&m_rcItem, pt)) {
        return hit;
    }
    const int top = ColumnsTop();
    const int bottom = m_rcItem.bottom - m_pad;
    if (pt.y < top || pt.y >= bottom) {
        return hit;
    }

    if (m_filterable && !m_query.empty()) {
        const int headerHeight = UiScale::Px(30);
        if (pt.y < top + headerHeight) {
            return hit;
        }
        const int row = (pt.y - top - headerHeight) / m_rowHeight;
        if (row >= 0 && row < static_cast<int>(m_results.size())) {
            hit.column = 0;
            hit.row = row;
            hit.path = m_results[row];
        }
        return hit;
    }

    const int columns = VisibleColumns();
    for (int column = 0; column < columns; ++column) {
        RECT rect = ColumnRect(column);
        if (pt.x < rect.left || pt.x >= rect.right) {
            continue;
        }
        const std::vector<CascaderNode>* nodes = ColumnNodes(column);
        if (nodes == nullptr) {
            return hit;
        }
        const int row = (pt.y - top + ScrollAt(column)) / m_rowHeight;
        if (row < 0 || row >= static_cast<int>(nodes->size())) {
            return hit;
        }
        hit.column = column;
        hit.row = row;
        hit.nodeIndex = row;
        for (int level = 0; level < column; ++level) {
            hit.path.push_back(m_browse[level]);
        }
        hit.path.push_back(row);
        return hit;
    }
    return hit;
}

void ECascaderPopupUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_TIMER && event.wParam == 1) {
        if (m_pManager != nullptr) {
            m_pManager->KillTimer(this, 1);
        }
        FinishLazy();
        return;
    }
    if (event.Type == UIEVENT_SETCURSOR) {
        HitRow hit = HitTest(event.ptMouse);
        if (hit.column >= 0) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
            return;
        }
    }
    else if (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSEENTER) {
        HitRow hit = HitTest(event.ptMouse);
        if (hit.column != m_hoverColumn || hit.row != m_hoverRow) {
            m_hoverColumn = hit.column;
            m_hoverRow = hit.row;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hoverColumn != -1 || m_hoverRow != -1) {
            m_hoverColumn = -1;
            m_hoverRow = -1;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_BUTTONUP && ::PtInRect(&m_rcItem, event.ptMouse)) {
        HitRow hit = HitTest(event.ptMouse);
        if (hit.column < 0) {
            return;
        }
        std::vector<std::wstring> labels;
        const CascaderNode* node = nullptr;
        if (m_filterable && !m_query.empty()) {
            if (hit.row >= 0 && hit.row < static_cast<int>(m_resultLabels.size())) {
                labels = m_resultLabels[hit.row];
            }
        }
        const std::vector<CascaderNode>* walk = &m_options;
        for (size_t i = 0; i < hit.path.size(); ++i) {
            if (hit.path[i] < 0 || hit.path[i] >= static_cast<int>(walk->size())) {
                return;
            }
            if (labels.size() <= i) {
                labels.push_back((*walk)[hit.path[i]].name);
            }
            walk = &(*walk)[hit.path[i]].children;
        }
        if (!hit.path.empty()) {
            const std::vector<CascaderNode>* parents = NodesAt(
                std::vector<int>(hit.path.begin(), hit.path.end() - 1));
            if (parents != nullptr && hit.path.back() < static_cast<int>(parents->size())) {
                node = &(*parents)[hit.path.back()];
            }
        }
        if (node == nullptr || node->disabled) {
            return;
        }

        if (m_checkStrictly) {
            m_paths.assign(1, hit.path);
            m_labels.assign(1, labels);
            Invalidate();
            NotifyPick();
            return;
        }
        if (HasChildren(*node)) {
            BrowseTo(hit.path);
            return;
        }
        if (m_multiple) {
            TogglePath(hit.path, labels);
            NotifyPick();
        }
        else {
            m_paths.assign(1, hit.path);
            m_labels.assign(1, labels);
            Invalidate();
            NotifyPick();
        }
        return;
    }
    else if (event.Type == UIEVENT_SCROLLWHEEL) {
        HitRow hit = HitTest(event.ptMouse);
        if (hit.column < 0) {
            return;
        }
        const std::vector<CascaderNode>* nodes = ColumnNodes(hit.column);
        if (nodes == nullptr) {
            return;
        }
        const int view = ViewRows();
        const int maxScroll = (std::max)(0, static_cast<int>(nodes->size()) - view) * m_rowHeight;
        if (maxScroll <= 0) {
            return;
        }
        const int delta = LOWORD(event.wParam) == SB_LINEDOWN ? m_rowHeight * 2 : -m_rowHeight * 2;
        const int current = ScrollAt(hit.column);
        const int next = (std::max)(0, (std::min)(current + delta, maxScroll));
        if (next != current) {
            SetScroll(hit.column, next);
            Invalidate();
        }
        return;
    }
    CContainerUI::DoEvent(event);
}

void ECascaderPopupUI::PaintBkColor(HDC hDC)
{
    FillRoundRectColor(hDC, m_rcItem, UiScale::Px(4), theme::White);
}

void ECascaderPopupUI::PaintBorder(HDC hDC)
{
    RECT rc = m_rcItem;
    rc.left += 1;
    rc.top += 1;
    rc.right -= 1;
    rc.bottom -= 1;
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, rc, UiScale::Px(4));
    StrokePathAa(hDC, path, theme::BorderLight, 1);
}

void ECascaderPopupUI::PaintStatusImage(HDC hDC)
{
    if (m_pManager == nullptr) {
        return;
    }

    if (m_filterable) {
        const int cy = m_rcItem.top + m_pad + m_searchHeight / 2;
        const int cx = m_rcItem.left + UiScale::Px(20);
        const Gdiplus::REAL radius = static_cast<Gdiplus::REAL>(UiScale::Px(5));
        const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        {
            EnsureGdiplus();
            Gdiplus::Graphics graphics(hDC);
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            Gdiplus::Pen pen(GpColor(theme::TextSecondary), 1.4f);
            graphics.DrawEllipse(&pen, Gdiplus::RectF(cxF - radius, cyF - radius, radius * 2, radius * 2));
        }
        Gdiplus::PointF handle[2] = {
            { cxF + radius * 0.7f, cyF + radius * 0.7f },
            { cxF + radius * 1.6f, cyF + radius * 1.6f } };
        DrawLinesAa(hDC, handle, 2, theme::TextSecondary, 1.4f);

        RECT separator = { m_rcItem.left + UiScale::Px(10), m_rcItem.top + m_pad + m_searchHeight - 1,
            m_rcItem.right - UiScale::Px(10), m_rcItem.top + m_pad + m_searchHeight - 1 };
        CRenderEngine::DrawLine(hDC, separator, 1, theme::BorderLighter);
    }

    const int top = ColumnsTop();
    const int bottom = m_rcItem.bottom - m_pad;

    if (m_filterable && !m_query.empty()) {
        RECT header = { m_rcItem.left + UiScale::Px(12), top, m_rcItem.right - UiScale::Px(12),
            top + UiScale::Px(30) };
        CRenderEngine::DrawText(hDC, m_pManager, header, L"搜索建议", theme::TextSecondary,
            font::Small, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        int y = top + UiScale::Px(30);
        const int saved = ::SaveDC(hDC);
        ::IntersectClipRect(hDC, m_rcItem.left, y, m_rcItem.right, bottom);
        for (size_t i = 0; i < m_results.size(); ++i) {
            if (y >= bottom) {
                break;
            }
            RECT row = { m_rcItem.left, y, m_rcItem.right, y + m_rowHeight };
            if (static_cast<int>(i) == m_hoverRow) {
                CRenderEngine::DrawColor(hDC, row, theme::FillLight);
            }
            int textLeft = m_rcItem.left + UiScale::Px(12);
            if (m_multiple) {
                RECT box = { m_rcItem.left + UiScale::Px(12), y + (m_rowHeight - UiScale::Px(14)) / 2,
                    m_rcItem.left + UiScale::Px(26), y + (m_rowHeight + UiScale::Px(14)) / 2 };
                DrawCascaderCheckbox(hDC, box, IsPathSelected(m_results[i]), true);
                textLeft = box.right + UiScale::Px(10);
            }
            RECT textRc = { textLeft, y, m_rcItem.right - UiScale::Px(12), y + m_rowHeight };
            CRenderEngine::DrawText(hDC, m_pManager, textRc,
                JoinLabels(m_resultLabels[i], L" / ").c_str(), theme::TextRegular,
                font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            y += m_rowHeight;
        }
        ::RestoreDC(hDC, saved);
        if (m_results.empty()) {
            RECT rc = { m_rcItem.left, top + UiScale::Px(30), m_rcItem.right, top + UiScale::Px(30) + m_rowHeight };
            CRenderEngine::DrawText(hDC, m_pManager, rc, L"无匹配数据", theme::TextSecondary,
                font::Small, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        return;
    }

    const int columns = VisibleColumns();
    for (int column = 0; column < columns; ++column) {
        RECT colRect = ColumnRect(column);
        if (column + 1 < columns) {
            RECT divider = { colRect.right - 1, top, colRect.right, bottom };
            CRenderEngine::DrawColor(hDC, divider, theme::BorderLighter);
        }
        if (m_loading && column == static_cast<int>(m_browse.size())) {
            RECT rc = { colRect.left, top, colRect.right, top + m_rowHeight };
            CRenderEngine::DrawText(hDC, m_pManager, rc, L"加载中…", theme::TextSecondary,
                font::Small, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            continue;
        }
        const std::vector<CascaderNode>* nodes = ColumnNodes(column);
        if (nodes == nullptr || nodes->empty()) {
            continue;
        }
        const int saved = ::SaveDC(hDC);
        ::IntersectClipRect(hDC, colRect.left, top, colRect.right, bottom);
        const int scroll = ScrollAt(column);
        for (size_t row = 0; row < nodes->size(); ++row) {
            const int y = top + static_cast<int>(row) * m_rowHeight - scroll;
            if (y + m_rowHeight <= top || y >= bottom) {
                continue;
            }
            const CascaderNode& node = (*nodes)[row];
            const bool onPath = column < static_cast<int>(m_browse.size())
                && m_browse[column] == static_cast<int>(row);
            const bool hot = m_hoverColumn == column && m_hoverRow == static_cast<int>(row);
            RECT rowRect = { colRect.left, y, colRect.right, y + m_rowHeight };
            if (hot) {
                CRenderEngine::DrawColor(hDC, rowRect, theme::FillLight);
            }

            std::vector<int> path;
            for (int level = 0; level < column; ++level) {
                path.push_back(m_browse[level]);
            }
            path.push_back(static_cast<int>(row));
            const bool selected = IsPathSelected(path);

            const DWORD textColor = node.disabled ? theme::TextPlaceholder
                : (onPath ? theme::Primary : theme::TextRegular);
            int reserve = UiScale::Px(40);
            if (!node.desc.empty()) {
                const int descWidth = CascaderTextWidth(m_pManager, node.desc, font::Small);
                RECT descRc = { colRect.left, y, colRect.right - UiScale::Px(34), y + m_rowHeight };
                descRc.left = descRc.right - descWidth;
                CRenderEngine::DrawText(hDC, m_pManager, descRc, node.desc.c_str(), theme::TextSecondary,
                    font::Small, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                reserve += descWidth + UiScale::Px(12);
            }
            RECT textRc = { colRect.left + UiScale::Px(20), y, colRect.right - reserve, y + m_rowHeight };
            CRenderEngine::DrawText(hDC, m_pManager, textRc, node.name.c_str(), textColor,
                font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            const int cy = y + m_rowHeight / 2;
            if (m_checkStrictly) {
                DrawCascaderRadio(hDC, colRect.right - UiScale::Px(18), cy, selected, !node.disabled);
            }
            else if (selected) {
                DrawCascaderCheck(hDC, colRect.right - UiScale::Px(12), cy,
                    node.disabled ? theme::TextPlaceholder : theme::Primary);
            }
            else if (!node.children.empty() || node.lazy) {
                DrawCascaderChevron(hDC, colRect.right - UiScale::Px(12), cy, theme::TextPlaceholder);
            }
        }
        ::RestoreDC(hDC, saved);

        if (static_cast<int>(nodes->size()) > ViewRows()) {
            const int viewHeight = (bottom - top);
            const int thumbHeight = (std::max)(UiScale::Px(12),
                viewHeight * viewHeight / (static_cast<int>(nodes->size()) * m_rowHeight));
            const int maxScroll = (static_cast<int>(nodes->size()) - ViewRows()) * m_rowHeight;
            const int thumbTop = top + (viewHeight - thumbHeight) * scroll / (maxScroll > 0 ? maxScroll : 1);
            RECT track = { colRect.right - UiScale::Px(6), top, colRect.right - UiScale::Px(3), bottom };
            CRenderEngine::DrawColor(hDC, track, theme::FillLight);
            RECT thumb = { track.left, thumbTop, track.right, thumbTop + thumbHeight };
            FillRoundRectColor(hDC, thumb, UiScale::Px(2), theme::TextPlaceholder);
        }
    }
}

void ECascaderPopupUI::LayoutSearchEdit()
{
    if (m_edit == nullptr) {
        return;
    }
    RECT rc = { m_rcItem.left + UiScale::Px(40), m_rcItem.top + UiScale::Px(7),
        m_rcItem.right - UiScale::Px(12), m_rcItem.top + UiScale::Px(7) + UiScale::Px(24) };
    m_edit->SetPos(rc);
}

void ECascaderPopupUI::FocusSearch()
{
    if (m_edit == nullptr || !m_filterable) {
        return;
    }
    m_edit->SetVisible(true);
    if (m_pManager != nullptr) {
        m_pManager->SetFocus(m_edit);
    }
}

DuiLib::CEditUI* ECascaderPopupUI::SearchEdit() const
{
    return m_edit;
}

std::vector<std::vector<int>> ECascaderPopupUI::GetSelection() const
{
    return m_paths;
}

std::vector<std::vector<std::wstring>> ECascaderPopupUI::GetSelectionLabels() const
{
    return m_labels;
}

bool ECascaderPopupUI::IsMultiple() const
{
    return m_multiple;
}

void ECascaderPopupUI::UpdatePanelSize()
{
    SetFixedWidth(ContentWidth());
    SetFixedHeight(ContentHeight());
    if (m_pManager != nullptr) {
        NeedParentUpdate();
    }
}

//////////////////////////////////////////////////////////////////////////
// ESwitchUI

ESwitchUI::ESwitchUI()
    : m_checked(false)
{
    SetFixedWidth(44);
    SetFixedHeight(22);
}

LPCTSTR ESwitchUI::GetClass() const
{
    return _T("ESwitch");
}

LPVOID ESwitchUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ESwitch")) == 0) {
        return static_cast<ESwitchUI*>(this);
    }
    return CButtonUI::GetInterface(pstrName);
}

void ESwitchUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("checked")) == 0) {
        SetChecked(_tcsicmp(pstrValue, _T("true")) == 0);
    }
    else {
        CButtonUI::SetAttribute(pstrName, pstrValue);
    }
}

void ESwitchUI::SetChecked(bool checked)
{
    if (m_checked == checked) {
        return;
    }
    m_checked = checked;
    Invalidate();
}

bool ESwitchUI::IsChecked() const
{
    return m_checked;
}

void ESwitchUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_BUTTONUP && IsEnabled()
        && ::PtInRect(&m_rcItem, event.ptMouse) && (m_uButtonState & UISTATE_PUSHED) != 0) {
        SetChecked(!m_checked);
        if (m_pManager != nullptr) {
            m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
        }
    }
    CButtonUI::DoEvent(event);
}

void ESwitchUI::PaintBkColor(HDC hDC)
{
    const int height = m_rcItem.bottom - m_rcItem.top;
    DWORD track = m_checked ? theme::Primary : theme::BorderBase;
    if (!IsEnabled()) {
        track = m_checked ? theme::PrimaryDisabled : theme::BorderLight;
    }
    FillRoundRectColor(hDC, m_rcItem, height, track);

    const int margin = std::max(1, UiScale::Px(2));
    const int diameter = height - margin * 2;
    const int left = m_checked ? m_rcItem.right - margin - diameter : m_rcItem.left + margin;
    RECT thumb = { left, m_rcItem.top + margin, left + diameter, m_rcItem.top + margin + diameter };
    FillEllipseColor(hDC, thumb, theme::White, theme::White, 1);
}

void ESwitchUI::PaintStatusImage(HDC hDC)
{
    (void)hDC;
}

//////////////////////////////////////////////////////////////////////////
// ERateUI

ERateUI::ERateUI()
    : m_max(5)
    , m_value(0)
    , m_hover(0)
    , m_allowHalf(false)
{
    SetFixedHeight(24);
    SetMax(5);
}

LPCTSTR ERateUI::GetClass() const
{
    return _T("ERate");
}

LPVOID ERateUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ERate")) == 0) {
        return static_cast<ERateUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

UINT ERateUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void ERateUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("value")) == 0) {
        SetValue(_wtof(pstrValue));
    }
    else if (_tcsicmp(pstrName, _T("max")) == 0) {
        SetMax(_wtoi(pstrValue));
    }
    else if (_tcsicmp(pstrName, _T("allowhalf")) == 0
        || _tcsicmp(pstrName, _T("half")) == 0) {
        SetAllowHalf(_tcsicmp(pstrValue, _T("true")) == 0);
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

int ERateUI::StarSize() const
{
    return UiScale::Px(20);
}

int ERateUI::StarGap() const
{
    return UiScale::Px(6);
}

void ERateUI::SetMax(int max)
{
    m_max = std::max(1, max);
    const int width = m_max * StarSize() + (m_max - 1) * StarGap();
    SetFixedWidth(width);
    if (m_value > m_max) {
        m_value = m_max;
    }
    Invalidate();
}

void ERateUI::SetValue(double value)
{
    if (value < 0) {
        value = 0;
    }
    if (value > m_max) {
        value = m_max;
    }
    m_value = value;
    Invalidate();
}

void ERateUI::SetAllowHalf(bool allowHalf)
{
    m_allowHalf = allowHalf;
    Invalidate();
}

double ERateUI::GetValue() const
{
    return m_value;
}

double ERateUI::ValueAt(int x) const
{
    if (m_max <= 0) {
        return 0;
    }
    const int size = StarSize();
    const int step = size + StarGap();
    const int rel = x - m_rcItem.left;
    if (rel <= 0) {
        return m_allowHalf ? 0.5 : 1;
    }
    int index = rel / step;
    if (index >= m_max) {
        return m_max;
    }
    if (index < 0) {
        index = 0;
    }
    const int offset = rel - index * step;
    if (m_allowHalf && offset < size / 2) {
        return index + 0.5;
    }
    return std::min<double>(index + 1, m_max);
}

void ERateUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_MOUSEENTER || event.Type == UIEVENT_MOUSEMOVE) {
        if (IsEnabled()) {
            m_hover = ValueAt(event.ptMouse.x);
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        m_hover = 0;
        Invalidate();
    }
    else if (event.Type == UIEVENT_BUTTONUP && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse)) {
        const double value = ValueAt(event.ptMouse.x);
        if (value != m_value) {
            m_value = value;
            Invalidate();
            if (m_pManager != nullptr) {
                m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
            }
        }
    }
    else if (event.Type == UIEVENT_SETCURSOR && IsEnabled()) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return;
    }
    CControlUI::DoEvent(event);
}

void ERateUI::DrawStar(HDC hDC, const RECT& star, double fill) const
{
    const double cx = (star.left + star.right) / 2.0;
    const double cy = (star.top + star.bottom) / 2.0;
    const double outer = std::min(star.right - star.left, star.bottom - star.top) / 2.0;
    const double inner = outer * 0.382;
    Gdiplus::PointF points[10] = {};
    for (int index = 0; index < 10; ++index) {
        const double angle = -90.0 + index * 36.0;
        const double radius = (index % 2 == 0) ? outer : inner;
        const double rad = angle * 3.14159265358979323846 / 180.0;
        points[index].X = static_cast<Gdiplus::REAL>(cx + radius * std::cos(rad));
        points[index].Y = static_cast<Gdiplus::REAL>(cy + radius * std::sin(rad));
    }

    Gdiplus::GraphicsPath path;
    path.AddPolygon(points, 10);

    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    Gdiplus::SolidBrush voidBrush(GpColor(theme::RateVoid));
    graphics.FillPath(&voidBrush, &path);

    if (fill > 0) {
        const Gdiplus::REAL clipWidth = static_cast<Gdiplus::REAL>((star.right - star.left) * fill);
        graphics.SetClip(Gdiplus::RectF(static_cast<Gdiplus::REAL>(star.left),
            static_cast<Gdiplus::REAL>(star.top), clipWidth,
            static_cast<Gdiplus::REAL>(star.bottom - star.top)));
        Gdiplus::SolidBrush fillBrush(GpColor(IsEnabled() ? theme::RateGold : theme::RateVoid));
        graphics.FillPath(&fillBrush, &path);
    }
}

void ERateUI::PaintBkColor(HDC hDC)
{
    const double shown = (m_hover > 0 && IsEnabled()) ? m_hover : m_value;
    const int size = StarSize();
    const int step = size + StarGap();
    const int height = m_rcItem.bottom - m_rcItem.top;
    const int top = m_rcItem.top + (height - size) / 2;
    for (int index = 0; index < m_max; ++index) {
        RECT star = { m_rcItem.left + index * step, top, m_rcItem.left + index * step + size, top + size };
        const double fill = std::max(0.0, std::min(1.0, shown - index));
        DrawStar(hDC, star, fill);
    }
}

void ERateUI::PaintStatusImage(HDC hDC)
{
    (void)hDC;
}

void ERateUI::PaintText(HDC hDC)
{
    (void)hDC;
}

//////////////////////////////////////////////////////////////////////////
// EPaginationUI

EPaginationUI::EPaginationUI()
    : m_total(100)
    , m_pageSize(10)
    , m_current(1)
    , m_hover(-1)
{
    SetFixedHeight(28);
}

LPCTSTR EPaginationUI::GetClass() const
{
    return _T("EPagination");
}

LPVOID EPaginationUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EPagination")) == 0) {
        return static_cast<EPaginationUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

UINT EPaginationUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void EPaginationUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("total")) == 0) {
        SetTotal(_wtoi(pstrValue));
    }
    else if (_tcsicmp(pstrName, _T("pagesize")) == 0) {
        m_pageSize = (std::max)(1, _wtoi(pstrValue));
        SetCurrentPage(m_current);
    }
    else if (_tcsicmp(pstrName, _T("page")) == 0 || _tcsicmp(pstrName, _T("current")) == 0) {
        SetCurrentPage(_wtoi(pstrValue));
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

int EPaginationUI::GetCurrentPage() const
{
    return m_current;
}

void EPaginationUI::SetCurrentPage(int page)
{
    const int count = PageCount();
    m_current = (std::max)(1, (std::min)(page, count));
    Invalidate();
}

void EPaginationUI::SetTotal(int total)
{
    m_total = (std::max)(0, total);
    SetCurrentPage(m_current);
}

int EPaginationUI::GetPageCount() const
{
    return PageCount();
}

int EPaginationUI::PageCount() const
{
    if (m_pageSize <= 0) {
        return 1;
    }
    const int count = (m_total + m_pageSize - 1) / m_pageSize;
    return (std::max)(1, count);
}

void EPaginationUI::BuildCells(std::vector<Cell>& cells) const
{
    const int count = PageCount();
    const int size = UiScale::Px(28);
    const int gap = UiScale::Px(6);
    const int top = (m_rcItem.top + m_rcItem.bottom - size) / 2;
    int x = m_rcItem.left;

    auto addCell = [&](int page, bool nav, bool next) {
        Cell cell = {};
        cell.rc = { x, top, x + size, top + size };
        cell.page = page;
        cell.nav = nav;
        cell.next = next;
        cells.push_back(cell);
        x += size + gap;
    };

    addCell(0, true, false);

    std::vector<int> pages;
    if (count <= 7) {
        for (int page = 1; page <= count; ++page) {
            pages.push_back(page);
        }
    }
    else {
        pages.push_back(1);
        const int start = (std::max)(2, m_current - 2);
        const int end = (std::min)(count - 1, m_current + 2);
        if (start > 2) {
            pages.push_back(0);
        }
        for (int page = start; page <= end; ++page) {
            pages.push_back(page);
        }
        if (end < count - 1) {
            pages.push_back(0);
        }
        pages.push_back(count);
    }

    for (int page : pages) {
        addCell(page, false, false);
    }
    addCell(0, true, true);
}

int EPaginationUI::HitTest(const POINT& pt) const
{
    std::vector<Cell> cells;
    BuildCells(cells);
    for (size_t index = 0; index < cells.size(); ++index) {
        if (::PtInRect(&cells[index].rc, pt)) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

void EPaginationUI::ChangePage(int page)
{
    const int count = PageCount();
    page = (std::max)(1, (std::min)(page, count));
    if (page == m_current) {
        return;
    }
    m_current = page;
    Invalidate();
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
    }
}

void EPaginationUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_SETCURSOR) {
        if (IsEnabled() && HitTest(event.ptMouse) >= 0) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
            return;
        }
    }
    else if (event.Type == UIEVENT_MOUSEMOVE) {
        const int hover = IsEnabled() ? HitTest(event.ptMouse) : -1;
        if (hover != m_hover) {
            m_hover = hover;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hover != -1) {
            m_hover = -1;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_BUTTONUP && IsEnabled()) {
        const int index = HitTest(event.ptMouse);
        if (index >= 0) {
            std::vector<Cell> cells;
            BuildCells(cells);
            const Cell& cell = cells[index];
            const int count = PageCount();
            if (cell.nav) {
                if (cell.next) {
                    ChangePage(m_current + 1);
                }
                else {
                    ChangePage(m_current - 1);
                }
            }
            else if (cell.page == 0) {
                const bool backward = cell.rc.left < cells[0].rc.left + (m_current - 1) * (UiScale::Px(28) + UiScale::Px(6));
                ChangePage(m_current + (backward ? -5 : 5));
            }
            else {
                ChangePage(cell.page);
            }
            return;
        }
    }
    CControlUI::DoEvent(event);
}

void EPaginationUI::PaintBkColor(HDC hDC)
{
    (void)hDC;
}

void EPaginationUI::PaintStatusImage(HDC hDC)
{
    if (m_pManager == nullptr) {
        return;
    }
    std::vector<Cell> cells;
    BuildCells(cells);
    const int count = PageCount();

    for (size_t index = 0; index < cells.size(); ++index) {
        Cell cell = cells[index];
        cell.hover = static_cast<int>(index) == m_hover;
        const bool enabledCell = IsEnabled();

        if (cell.nav) {
            if (cell.hover) {
                FillRoundRectColor(hDC, cell.rc, UiScale::Px(4), theme::FillLight);
            }
            const bool active = cell.next ? (m_current < count) : (m_current > 1);
            const DWORD color = (enabledCell && active) ? theme::TextSecondary : theme::TextPlaceholder;
            const int cx = (cell.rc.left + cell.rc.right) / 2;
            const int cy = (cell.rc.top + cell.rc.bottom) / 2;
            const Gdiplus::REAL half = static_cast<Gdiplus::REAL>(UiScale::Px(3));
            const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(5));
            const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
            const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
            Gdiplus::PointF points[3] = {};
            if (cell.next) {
                points[0] = Gdiplus::PointF(cxF - half, cyF - arm);
                points[1] = Gdiplus::PointF(cxF + half, cyF);
                points[2] = Gdiplus::PointF(cxF - half, cyF + arm);
            }
            else {
                points[0] = Gdiplus::PointF(cxF + half, cyF - arm);
                points[1] = Gdiplus::PointF(cxF - half, cyF);
                points[2] = Gdiplus::PointF(cxF + half, cyF + arm);
            }
            DrawLinesAa(hDC, points, 3, color, static_cast<float>(std::max(1, UiScale::Px(1))) * 1.4f);
            continue;
        }

        if (cell.page == 0) {
            CRenderEngine::DrawText(hDC, m_pManager, cell.rc, _T("···"),
                cell.hover ? theme::Primary : theme::TextSecondary, font::Body,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            continue;
        }

        const bool current = cell.page == m_current;
        if (current) {
            FillRoundRectColor(hDC, cell.rc, UiScale::Px(4), theme::Primary);
        }
        else if (cell.hover) {
            FillRoundRectColor(hDC, cell.rc, UiScale::Px(4), theme::PrimaryBg);
        }

        wchar_t text[16] = {};
        ::_snwprintf_s(text, _TRUNCATE, L"%d", cell.page);
        DWORD color = theme::TextRegular;
        if (!IsEnabled()) {
            color = theme::TextPlaceholder;
        }
        else if (current) {
            color = theme::White;
        }
        else if (cell.hover) {
            color = theme::Primary;
        }
        CRenderEngine::DrawText(hDC, m_pManager, cell.rc, text, color, font::Body,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

//////////////////////////////////////////////////////////////////////////
// EBadgeUI

EBadgeUI::EBadgeUI()
    : m_value(0)
    , m_numeric(true)
    , m_max(99)
    , m_dot(false)
    , m_type(0)
    , m_contentWidth(0)
    , m_contentHeight(0)
    , m_rcContent{}
{
}

LPCTSTR EBadgeUI::GetClass() const
{
    return _T("EBadge");
}

LPVOID EBadgeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EBadge")) == 0) {
        return static_cast<EBadgeUI*>(this);
    }
    return CContainerUI::GetInterface(pstrName);
}

void EBadgeUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CContainerUI::SetManager(pManager, pParent, bInit);
    UpdateOuterSize();
}

void EBadgeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("value")) == 0) {
        m_valueText = pstrValue;
        m_value = _wtoi(pstrValue);
        m_numeric = !m_valueText.empty()
            && m_valueText.find_first_not_of(L"0123456789") == std::wstring::npos;
        UpdateOuterSize();
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("max")) == 0) {
        m_max = _wtoi(pstrValue);
        UpdateOuterSize();
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("dot")) == 0) {
        m_dot = _tcsicmp(pstrValue, _T("true")) == 0;
        UpdateOuterSize();
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("type")) == 0) {
        if (_tcsicmp(pstrValue, _T("primary")) == 0) m_type = 1;
        else if (_tcsicmp(pstrValue, _T("success")) == 0) m_type = 2;
        else if (_tcsicmp(pstrValue, _T("warning")) == 0) m_type = 3;
        else if (_tcsicmp(pstrValue, _T("info")) == 0) m_type = 4;
        else m_type = 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("width")) == 0) {
        m_contentWidth = _wtoi(pstrValue);
        UpdateOuterSize();
    }
    else if (_tcsicmp(pstrName, _T("height")) == 0) {
        m_contentHeight = _wtoi(pstrValue);
        UpdateOuterSize();
    }
    else {
        CContainerUI::SetAttribute(pstrName, pstrValue);
    }
}

void EBadgeUI::ScaleForDpi()
{
    if (m_contentWidth > 0) {
        m_contentWidth = UiScale::Px(m_contentWidth);
    }
    if (m_contentHeight > 0) {
        m_contentHeight = UiScale::Px(m_contentHeight);
    }
    UpdateOuterSize();
}

void EBadgeUI::UpdateOuterSize()
{
    const int extraWidth = (std::max)(0, BadgeWidth() - BadgeOverlap());
    CControlUI::SetFixedWidth(m_contentWidth > 0 ? m_contentWidth + extraWidth : 0);
    CControlUI::SetFixedHeight(m_contentHeight > 0 ? m_contentHeight + BadgeOverhead() : 0);
}

DWORD EBadgeUI::BadgeColor() const
{
    switch (m_type) {
    case 1: return theme::Primary;
    case 2: return theme::Success;
    case 3: return theme::Warning;
    case 4: return theme::Info;
    default: return theme::Danger;
    }
}

std::wstring EBadgeUI::BadgeText() const
{
    if (!m_numeric) {
        return m_valueText;
    }
    wchar_t text[16] = {};
    if (m_max > 0 && m_value > m_max) {
        ::_snwprintf_s(text, _TRUNCATE, L"%d+", m_max);
    }
    else {
        ::_snwprintf_s(text, _TRUNCATE, L"%d", m_value);
    }
    return text;
}

bool EBadgeUI::HasBadge() const
{
    if (m_dot) {
        return true;
    }
    return m_numeric ? m_value > 0 : !m_valueText.empty();
}

int EBadgeUI::BadgeWidth() const
{
    if (!HasBadge()) {
        return 0;
    }
    if (m_dot) {
        return UiScale::Px(8);
    }
    SIZE size = { 0, 0 };
    if (m_pManager != nullptr) {
        HDC hDC = ::GetDC(nullptr);
        if (hDC != nullptr) {
            size = CRenderEngine::GetTextSize(hDC, m_pManager, BadgeText().c_str(),
                font::Group, DT_SINGLELINE);
            ::ReleaseDC(nullptr, hDC);
        }
    }
    return (std::max)(UiScale::Px(18), static_cast<int>(size.cx) + UiScale::Px(10));
}

int EBadgeUI::BadgeHeight() const
{
    if (m_dot) {
        return UiScale::Px(8);
    }
    return HasBadge() ? UiScale::Px(18) : 0;
}

int EBadgeUI::BadgeOverlap() const
{
    if (BadgeHeight() <= 0) {
        return 0;
    }
    return UiScale::Px(m_dot ? 5 : 10);
}

int EBadgeUI::BadgeOverhead() const
{
    return BadgeHeight() > 0 ? UiScale::Px(9) : 0;
}

SIZE EBadgeUI::EstimateSize(SIZE szAvailable)
{
    int cx = m_contentWidth;
    int cy = m_contentHeight;
    if (cx <= 0 || cy <= 0) {
        for (int i = 0; i < GetCount(); ++i) {
            CControlUI* child = GetItemAt(i);
            if (child == nullptr || !child->IsVisible()) {
                continue;
            }
            const SIZE sz = child->EstimateSize(szAvailable);
            cx = (std::max)(cx, static_cast<int>(sz.cx));
            cy = (std::max)(cy, static_cast<int>(sz.cy));
        }
    }
    const int extraWidth = (std::max)(0, BadgeWidth() - BadgeOverlap());
    SIZE size = { cx + extraWidth, cy + BadgeOverhead() };
    return size;
}

void EBadgeUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);

    RECT inner = m_rcItem;
    inner.left += m_rcInset.left;
    inner.top += m_rcInset.top;
    inner.right -= m_rcInset.right;
    inner.bottom -= m_rcInset.bottom;

    const int innerWidth = (std::max)(0, static_cast<int>(inner.right - inner.left));
    const int innerHeight = (std::max)(0, static_cast<int>(inner.bottom - inner.top));
    const int extraWidth = (std::max)(0, BadgeWidth() - BadgeOverlap());
    const int topOverhead = BadgeOverhead();

    int cx = m_contentWidth > 0 ? m_contentWidth : innerWidth - extraWidth;
    int cy = m_contentHeight > 0 ? m_contentHeight : innerHeight - topOverhead;
    cx = (std::max)(0, (std::min)(cx, innerWidth - extraWidth));
    cy = (std::max)(0, (std::min)(cy, innerHeight - topOverhead));

    m_rcContent.left = inner.left;
    m_rcContent.top = inner.top + topOverhead;
    m_rcContent.right = inner.left + cx;
    m_rcContent.bottom = m_rcContent.top + cy;

    for (int i = 0; i < GetCount(); ++i) {
        CControlUI* child = GetItemAt(i);
        if (child != nullptr) {
            child->SetPos(m_rcContent, bNeedInvalidate);
        }
    }
}

bool EBadgeUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
    CContainerUI::DoPaint(hDC, rcPaint, pStopControl);

    const int height = BadgeHeight();
    if (height <= 0) {
        return true;
    }
    const int width = BadgeWidth();
    RECT badge = { m_rcContent.right - BadgeOverlap(), m_rcContent.top - height / 2,
        m_rcContent.right - BadgeOverlap() + width, m_rcContent.top - height / 2 + height };

    if (m_dot) {
        FillEllipseColor(hDC, badge, BadgeColor(), 0, 0);
        return true;
    }

    if (m_pManager == nullptr) {
        return true;
    }
    FillRoundRectColor(hDC, badge, height, BadgeColor());
    CRenderEngine::DrawText(hDC, m_pManager, badge, BadgeText().c_str(), theme::White,
        font::Group, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// EAvatarUI

EAvatarUI::EAvatarUI()
    : m_type(0)
    , m_square(false)
    , m_icon(false)
    , m_bitmap(nullptr)
    , m_bitmapSize(0)
{
    SetFixedWidth(40);
    SetFixedHeight(40);
}

EAvatarUI::~EAvatarUI()
{
    delete m_bitmap;
}

LPCTSTR EAvatarUI::GetClass() const
{
    return _T("EAvatar");
}

LPVOID EAvatarUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EAvatar")) == 0) {
        return static_cast<EAvatarUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

void EAvatarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("type")) == 0) {
        if (_tcsicmp(pstrValue, _T("primary")) == 0) m_type = 1;
        else if (_tcsicmp(pstrValue, _T("success")) == 0) m_type = 2;
        else if (_tcsicmp(pstrValue, _T("warning")) == 0) m_type = 3;
        else if (_tcsicmp(pstrValue, _T("danger")) == 0) m_type = 4;
        else if (_tcsicmp(pstrValue, _T("info")) == 0) m_type = 5;
        else m_type = 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("shape")) == 0) {
        m_square = _tcsicmp(pstrValue, _T("square")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("src")) == 0) {
        m_src = pstrValue != nullptr ? pstrValue : L"";
        delete m_bitmap;
        m_bitmap = nullptr;
        m_bitmapSize = 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("icons")) == 0) {
        m_icon = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

DWORD EAvatarUI::BackgroundColor() const
{
    switch (m_type) {
    case 1: return theme::Primary;
    case 2: return theme::Success;
    case 3: return theme::Warning;
    case 4: return theme::Danger;
    case 5: return theme::Info;
    default: return theme::TextPlaceholder;
    }
}

Gdiplus::Bitmap* EAvatarUI::BitmapFor(int size)
{
    if (m_src.empty() || size <= 0) {
        return nullptr;
    }
    if (m_bitmap != nullptr && m_bitmapSize == size) {
        return m_bitmap;
    }

    delete m_bitmap;
    m_bitmap = nullptr;
    m_bitmapSize = 0;

    std::wstring file = CPaintManagerUI::GetResourcePath().GetData();
    if (file.empty()) {
        return nullptr;
    }
    if (file.back() != L'\\' && file.back() != L'/') {
        file += L'\\';
    }
    file += m_src;

    EnsureGdiplus();
    Gdiplus::Bitmap source(file.c_str());
    if (source.GetLastStatus() != Gdiplus::Ok) {
        return nullptr;
    }

    auto* scaled = new Gdiplus::Bitmap(size, size, PixelFormat32bppPARGB);
    if (scaled->GetLastStatus() != Gdiplus::Ok) {
        delete scaled;
        return nullptr;
    }
    {
        Gdiplus::Graphics graphics(scaled);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        const int srcWidth = static_cast<int>(source.GetWidth());
        const int srcHeight = static_cast<int>(source.GetHeight());
        const int side = (std::min)(srcWidth, srcHeight);
        const int srcX = (srcWidth - side) / 2;
        const int srcY = (srcHeight - side) / 2;
        graphics.DrawImage(&source, Gdiplus::Rect(0, 0, size, size),
            srcX, srcY, side, side, Gdiplus::UnitPixel);
    }
    if (scaled->GetLastStatus() != Gdiplus::Ok) {
        delete scaled;
        return nullptr;
    }
    m_bitmap = scaled;
    m_bitmapSize = size;
    return m_bitmap;
}

void EAvatarUI::PaintStatusImage(HDC hDC)
{
    const int width = m_rcItem.right - m_rcItem.left;
    const int height = m_rcItem.bottom - m_rcItem.top;
    const int size = (std::min)(width, height);
    if (size <= 0) {
        return;
    }

    RECT box = { m_rcItem.left, m_rcItem.top, m_rcItem.left + size, m_rcItem.top + size };
    Gdiplus::GraphicsPath path;
    if (m_square) {
        MakeRoundRectPath(path, box, UiScale::Px(8));
    }
    else {
        MakeEllipsePath(path, box);
    }

    EnsureGdiplus();
    {
        Gdiplus::Graphics graphics(hDC);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        Gdiplus::Bitmap* bitmap = BitmapFor(size);
        if (bitmap != nullptr) {
            Gdiplus::TextureBrush brush(bitmap);
            brush.SetWrapMode(Gdiplus::WrapModeClamp);
            brush.TranslateTransform(static_cast<Gdiplus::REAL>(box.left),
                static_cast<Gdiplus::REAL>(box.top));
            graphics.FillPath(&brush, &path);
        }
        else {
            Gdiplus::SolidBrush brush(GpColor(BackgroundColor()));
            graphics.FillPath(&brush, &path);
        }
    }

    if (m_pManager == nullptr) {
        return;
    }
    if (m_icon) {
        CRenderEngine::DrawText(hDC, m_pManager, box, m_sText, theme::White, font::Icon,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else if (!m_sText.IsEmpty()) {
        CRenderEngine::DrawText(hDC, m_pManager, box, m_sText, theme::White, font::Body14,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

//////////////////////////////////////////////////////////////////////////
// EIconImageUI

EIconImageUI::EIconImageUI()
    : m_resourceId(0)
    , m_tint(0)
    , m_loaded(false)
    , m_bitmap(nullptr)
    , m_stream(nullptr)
{
}

EIconImageUI::~EIconImageUI()
{
    delete m_bitmap;
    if (m_stream != nullptr) {
        m_stream->Release();
    }
}

LPCTSTR EIconImageUI::GetClass() const
{
    return _T("EIconImage");
}

LPVOID EIconImageUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EIconImage")) == 0) {
        return static_cast<EIconImageUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

void EIconImageUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("resid")) == 0) {
        SetResourceId(_ttoi(pstrValue));
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

void EIconImageUI::SetTintColor(DWORD argb)
{
    if (m_tint == argb) {
        return;
    }
    m_tint = argb;
    Invalidate();
}

void EIconImageUI::SetResourceId(int resourceId)
{
    if (resourceId == m_resourceId && m_loaded) {
        return;
    }
    delete m_bitmap;
    m_bitmap = nullptr;
    if (m_stream != nullptr) {
        m_stream->Release();
        m_stream = nullptr;
    }
    m_resourceId = resourceId;
    m_loaded = false;
    Invalidate();
}

Gdiplus::Bitmap* EIconImageUI::DecodeFromStream()
{
    if (m_stream == nullptr) {
        return nullptr;
    }
    LARGE_INTEGER offset = {};
    m_stream->Seek(offset, STREAM_SEEK_SET, nullptr);
    EnsureGdiplus();
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(m_stream, FALSE);
    if (bitmap == nullptr || bitmap->GetLastStatus() != Gdiplus::Ok) {
        delete bitmap;
        return nullptr;
    }
    return bitmap;
}

Gdiplus::Bitmap* EIconImageUI::EnsureBitmap()
{
    const int width = m_rcItem.right - m_rcItem.left;
    const int height = m_rcItem.bottom - m_rcItem.top;
    if (m_loaded && m_bitmap != nullptr) {
        if (static_cast<int>(m_bitmap->GetWidth()) == width
            && static_cast<int>(m_bitmap->GetHeight()) == height) {
            return m_bitmap;
        }
    }
    if (m_loaded) {
        return m_bitmap;
    }

    if (m_stream == nullptr) {
        m_loaded = true;
        if (m_resourceId <= 0) {
            return nullptr;
        }

        HINSTANCE instance = CPaintManagerUI::GetInstance();
        HRSRC resource = ::FindResourceW(instance, MAKEINTRESOURCEW(m_resourceId), RT_RCDATA);
        if (resource == nullptr) {
            return nullptr;
        }
        const DWORD size = ::SizeofResource(instance, resource);
        HGLOBAL loaded = ::LoadResource(instance, resource);
        const void* data = loaded != nullptr ? ::LockResource(loaded) : nullptr;
        if (data == nullptr || size == 0) {
            return nullptr;
        }

        HGLOBAL memory = ::GlobalAlloc(GMEM_MOVEABLE, size);
        if (memory == nullptr) {
            return nullptr;
        }
        void* target = ::GlobalLock(memory);
        if (target == nullptr) {
            ::GlobalFree(memory);
            return nullptr;
        }
        ::memcpy(target, data, size);
        ::GlobalUnlock(memory);

        IStream* stream = nullptr;
        if (FAILED(::CreateStreamOnHGlobal(memory, TRUE, &stream)) || stream == nullptr) {
            ::GlobalFree(memory);
            return nullptr;
        }
        m_stream = stream;
    }

    // 缩放到控件尺寸缓存，避免大量图标同时保留 4 倍大图。
    Gdiplus::Bitmap* source = DecodeFromStream();
    if (source == nullptr) {
        m_loaded = true;
        return nullptr;
    }
    delete m_bitmap;
    m_bitmap = nullptr;
    m_loaded = true;

    const int sourceWidth = static_cast<int>(source->GetWidth());
    const int sourceHeight = static_cast<int>(source->GetHeight());
    if (width <= 0 || height <= 0
        || (width == sourceWidth && height == sourceHeight)) {
        m_bitmap = source;
        return m_bitmap;
    }

    EnsureGdiplus();
    Gdiplus::Bitmap* scaled = new Gdiplus::Bitmap(width, height, PixelFormat32bppPARGB);
    if (scaled->GetLastStatus() != Gdiplus::Ok) {
        delete scaled;
        m_bitmap = source;
        return m_bitmap;
    }
    Gdiplus::Graphics graphics(scaled);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
    graphics.DrawImage(source, Gdiplus::Rect(0, 0, width, height),
        0, 0, sourceWidth, sourceHeight, Gdiplus::UnitPixel);
    delete source;
    m_bitmap = scaled;
    return m_bitmap;
}

void EIconImageUI::PaintStatusImage(HDC hDC)
{
    Gdiplus::Bitmap* bitmap = EnsureBitmap();
    if (bitmap == nullptr) {
        return;
    }
    const int width = m_rcItem.right - m_rcItem.left;
    const int height = m_rcItem.bottom - m_rcItem.top;
    if (width <= 0 || height <= 0) {
        return;
    }

    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::ImageAttributes attributes;
    attributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    Gdiplus::ColorMatrix tintMatrix = {};
    if (m_tint != 0) {
        // 白色图标按目标色缩放 RGB，保留原始 alpha 边缘。
        const Gdiplus::Color tint(m_tint);
        tintMatrix.m[0][0] = tint.GetR() / 255.0f;
        tintMatrix.m[1][1] = tint.GetG() / 255.0f;
        tintMatrix.m[2][2] = tint.GetB() / 255.0f;
        tintMatrix.m[3][3] = 1.0f;
        tintMatrix.m[4][4] = 1.0f;
        attributes.SetColorMatrix(&tintMatrix);
    }
    graphics.DrawImage(bitmap,
        Gdiplus::Rect(m_rcItem.left, m_rcItem.top, width, height),
        0, 0, static_cast<INT>(bitmap->GetWidth()), static_cast<INT>(bitmap->GetHeight()),
        Gdiplus::UnitPixel, &attributes);
}

//////////////////////////////////////////////////////////////////////////
// ELoadingUI

ELoadingUI::ELoadingUI()
    : m_angle(0)
    , m_timerStarted(false)
{
    SetFixedWidth(20);
    SetFixedHeight(20);
}

LPCTSTR ELoadingUI::GetClass() const
{
    return _T("ELoading");
}

LPVOID ELoadingUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ELoading")) == 0) {
        return static_cast<ELoadingUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

void ELoadingUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CControlUI::SetManager(pManager, pParent, bInit);
    if (pManager != nullptr && !m_timerStarted) {
        m_timerStarted = pManager->SetTimer(this, 1, 50);
    }
}

void ELoadingUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_TIMER && event.wParam == 1) {
        m_angle = (m_angle + 24) % 360;
        Invalidate();
        return;
    }
    CControlUI::DoEvent(event);
}

void ELoadingUI::PaintBkColor(HDC hDC)
{
    (void)hDC;
}

void ELoadingUI::PaintStatusImage(HDC hDC)
{
    const int width = m_rcItem.right - m_rcItem.left;
    const int height = m_rcItem.bottom - m_rcItem.top;
    const int size = (std::min)(width, height);
    if (size <= 0) {
        return;
    }

    const Gdiplus::REAL penWidth = static_cast<Gdiplus::REAL>((std::max)(2, UiScale::Px(2)));
    const Gdiplus::REAL left = static_cast<Gdiplus::REAL>(m_rcItem.left + (width - size) / 2) + penWidth / 2;
    const Gdiplus::REAL top = static_cast<Gdiplus::REAL>(m_rcItem.top + (height - size) / 2) + penWidth / 2;
    const Gdiplus::REAL span = static_cast<Gdiplus::REAL>(size) - penWidth;

    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::Pen pen(GpColor(IsEnabled() ? theme::Primary : theme::TextPlaceholder), penWidth);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    graphics.DrawArc(&pen, Gdiplus::RectF(left, top, span, span),
        static_cast<Gdiplus::REAL>(m_angle), 280.0f);
}

//////////////////////////////////////////////////////////////////////////
// EBreadcrumbUI

namespace {

class EBreadcrumbItemUI : public CButtonUI
{
public:
    EBreadcrumbItemUI()
        : m_current(false)
    {
        SetFont(font::Body);
        m_rcTextPadding = { 0, 0, 0, 0 };
    }

    LPCTSTR GetClass() const override
    {
        return _T("ECrumbItem");
    }

    void SetCurrent(bool current)
    {
        if (m_current != current) {
            m_current = current;
            Invalidate();
        }
    }

    void PaintText(HDC hDC) override
    {
        if (m_sText.IsEmpty() || m_pManager == nullptr) {
            return;
        }
        DWORD color = m_current ? theme::TextRegular : theme::TextSecondary;
        if (!m_current && IsEnabled() && IsHot(m_uButtonState)) {
            color = theme::Primary;
        }
        CRenderEngine::DrawText(hDC, m_pManager, m_rcItem, m_sText, color, m_iFont,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

private:
    bool m_current;
};

} // namespace

EBreadcrumbUI::EBreadcrumbUI()
    : m_current(0)
{
    SetChildPadding(4);
    SetFixedHeight(24);
}

LPCTSTR EBreadcrumbUI::GetClass() const
{
    return _T("EBreadcrumb");
}

LPVOID EBreadcrumbUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EBreadcrumb")) == 0) {
        return static_cast<EBreadcrumbUI*>(this);
    }
    return CHorizontalLayoutUI::GetInterface(pstrName);
}

void EBreadcrumbUI::SetItems(const std::vector<std::wstring>& items)
{
    m_items = items;
    m_current = items.empty() ? 0 : static_cast<int>(items.size()) - 1;
    Rebuild();
}

void EBreadcrumbUI::SetCurrent(int index)
{
    if (m_items.empty()) {
        return;
    }
    m_current = (std::max)(0, (std::min)(index, static_cast<int>(m_items.size()) - 1));
    for (size_t item = 0; item < m_itemControls.size() && item < m_items.size(); ++item) {
        auto* control = dynamic_cast<EBreadcrumbItemUI*>(m_itemControls[item]);
        if (control != nullptr) {
            control->SetCurrent(static_cast<int>(item) == m_current);
        }
    }
}

int EBreadcrumbUI::GetCurrent() const
{
    return m_current;
}

void EBreadcrumbUI::Rebuild()
{
    RemoveAll();
    m_itemControls.clear();
    if (m_pManager == nullptr) {
        return;
    }

    const HDC hDC = m_pManager->GetPaintDC();
    for (size_t index = 0; index < m_items.size(); ++index) {
        if (index > 0) {
            CLabelUI* separator = MakeLabel(_T("/"), font::Body, theme::TextPlaceholder);
            separator->SetFixedWidth(UiScale::Px(12));
            separator->SetFixedHeight(UiScale::Px(24));
            separator->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            Add(separator);
        }

        auto* item = new EBreadcrumbItemUI;
        item->SetName((L"crumb:" + std::to_wstring(index)).c_str());
        item->SetText(m_items[index].c_str());
        item->SetCurrent(static_cast<int>(index) == m_current);
        item->SetFixedHeight(UiScale::Px(24));
        const SIZE textSize = CRenderEngine::GetTextSize(hDC, m_pManager, m_items[index].c_str(),
            font::Body, DT_SINGLELINE);
        item->SetFixedWidth(static_cast<int>(textSize.cx) + UiScale::Px(2));
        Add(item);
        m_itemControls.push_back(item);
    }
}

//////////////////////////////////////////////////////////////////////////
// EStepsUI

EStepsUI::EStepsUI()
    : m_active(0)
{
    SetFixedHeight(48);
}

LPCTSTR EStepsUI::GetClass() const
{
    return _T("ESteps");
}

LPVOID EStepsUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ESteps")) == 0) {
        return static_cast<EStepsUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

void EStepsUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("steps")) == 0) {
        m_steps.clear();
        std::wstring text = pstrValue != nullptr ? pstrValue : L"";
        size_t begin = 0;
        while (begin <= text.size()) {
            const size_t end = text.find(L'|', begin);
            if (end == std::wstring::npos) {
                m_steps.push_back(text.substr(begin));
                break;
            }
            m_steps.push_back(text.substr(begin, end - begin));
            begin = end + 1;
        }
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("active")) == 0) {
        SetActive(_wtoi(pstrValue));
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

void EStepsUI::SetActive(int active)
{
    m_active = (std::max)(0, (std::min)(active, static_cast<int>(m_steps.size())));
    Invalidate();
}

int EStepsUI::GetActive() const
{
    return m_active;
}

void EStepsUI::PaintStatusImage(HDC hDC)
{
    const int count = static_cast<int>(m_steps.size());
    if (count <= 0 || m_pManager == nullptr) {
        return;
    }

    const int width = m_rcItem.right - m_rcItem.left;
    const int cellWidth = width / count;
    if (cellWidth <= 0) {
        return;
    }

    const int diameter = UiScale::Px(24);
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    const int lineWidth = (std::max)(1, UiScale::Px(1));

    for (int index = 0; index < count; ++index) {
        const int cx = m_rcItem.left + index * cellWidth + diameter / 2;
        RECT circle = { cx - diameter / 2, cy - diameter / 2, cx + diameter / 2, cy + diameter / 2 };

        if (index + 1 < count) {
            const int nextCx = m_rcItem.left + (index + 1) * cellWidth + diameter / 2;
            const SIZE titleSize = CRenderEngine::GetTextSize(hDC, m_pManager,
                m_steps[index].c_str(), font::Body14, DT_SINGLELINE);
            const int titleRight = cx + diameter / 2 + UiScale::Px(10) + static_cast<int>(titleSize.cx);
            const int lineLeft = (std::max)(cx + diameter / 2 + UiScale::Px(8), titleRight + UiScale::Px(8));
            const int lineRight = nextCx - diameter / 2 - UiScale::Px(8);
            if (lineRight > lineLeft) {
                Gdiplus::PointF points[2] = {
                    { static_cast<Gdiplus::REAL>(lineLeft), static_cast<Gdiplus::REAL>(cy) },
                    { static_cast<Gdiplus::REAL>(lineRight), static_cast<Gdiplus::REAL>(cy) } };
                DrawLinesAa(hDC, points, 2, index < m_active ? theme::Primary : theme::BorderLight,
                    static_cast<float>(lineWidth));
            }
        }

        const bool finished = index < m_active;
        const bool current = index == m_active;
        DWORD titleColor = theme::TextSecondary;
        if (!IsEnabled()) {
            titleColor = theme::TextPlaceholder;
        }
        else if (finished) {
            titleColor = theme::TextRegular;
        }
        else if (current) {
            titleColor = theme::Primary;
        }

        if (!IsEnabled()) {
            FillEllipseColor(hDC, circle, theme::FillLight, theme::BorderLight, lineWidth);
        }
        else if (finished) {
            FillEllipseColor(hDC, circle, theme::Primary, 0, 0);
            const Gdiplus::REAL size = static_cast<Gdiplus::REAL>(diameter);
            Gdiplus::PointF check[3] = {
                { cx - size * 0.17f, cy + size * 0.02f },
                { cx - size * 0.04f, cy + size * 0.15f },
                { cx + size * 0.19f, cy - size * 0.14f } };
            DrawLinesAa(hDC, check, 3, theme::White,
                static_cast<float>((std::max)(1, UiScale::Px(2))));
        }
        else if (current) {
            FillEllipseColor(hDC, circle, theme::White, theme::Primary,
                (std::max)(2, UiScale::Px(2)));
        }
        else {
            FillEllipseColor(hDC, circle, theme::White, theme::BorderBase, lineWidth);
        }

        if (!finished) {
            wchar_t number[8] = {};
            ::_snwprintf_s(number, _TRUNCATE, L"%d", index + 1);
            DWORD numberColor = theme::TextPlaceholder;
            if (!IsEnabled()) {
                numberColor = theme::TextPlaceholder;
            }
            else if (current) {
                numberColor = theme::Primary;
            }
            CRenderEngine::DrawText(hDC, m_pManager, circle, number, numberColor, font::Small,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        RECT title = { cx + diameter / 2 + UiScale::Px(10), m_rcItem.top,
            m_rcItem.left + (index + 1) * cellWidth, m_rcItem.bottom };
        CRenderEngine::DrawText(hDC, m_pManager, title, m_steps[index].c_str(), titleColor, font::Body14,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
}

//////////////////////////////////////////////////////////////////////////
// EPopoverUI

EPopoverUI::EPopoverUI()
    : m_arrow(24)
{
    SetBkColor(0);
}

LPCTSTR EPopoverUI::GetClass() const
{
    return _T("EPopover");
}

LPVOID EPopoverUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EPopover")) == 0) {
        return static_cast<EPopoverUI*>(this);
    }
    return CContainerUI::GetInterface(pstrName);
}

void EPopoverUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("arrow")) == 0) {
        m_arrow = _wtoi(pstrValue);
        Invalidate();
    }
    else {
        CContainerUI::SetAttribute(pstrName, pstrValue);
    }
}

bool EPopoverUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
    const int arrowHeight = UiScale::Px(7);
    const int arrowHalf = UiScale::Px(7);
    const int radius = UiScale::Px(4);
    RECT body = { m_rcItem.left, m_rcItem.top + arrowHeight, m_rcItem.right, m_rcItem.bottom };
    if (body.bottom > body.top && body.right > body.left) {
        Gdiplus::GraphicsPath bodyPath;
        MakeRoundRectPath(bodyPath, body, radius);
        EnsureGdiplus();
        {
            Gdiplus::Graphics graphics(hDC);
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            Gdiplus::SolidBrush brush(GpColor(theme::White));
            graphics.FillPath(&brush, &bodyPath);
            Gdiplus::Pen pen(GpColor(theme::BorderLight), 1.0f);
            graphics.DrawPath(&pen, &bodyPath);
        }

        const int ax = m_rcItem.left + UiScale::Px(m_arrow);
        const int ay = m_rcItem.top;
        Gdiplus::PointF triangle[3] = {
            { static_cast<Gdiplus::REAL>(ax - arrowHalf), static_cast<Gdiplus::REAL>(ay + arrowHeight) },
            { static_cast<Gdiplus::REAL>(ax), static_cast<Gdiplus::REAL>(ay) },
            { static_cast<Gdiplus::REAL>(ax + arrowHalf), static_cast<Gdiplus::REAL>(ay + arrowHeight) } };
        FillPolygonAa(hDC, triangle, 3, theme::White);

        Gdiplus::PointF edges[3] = {
            { static_cast<Gdiplus::REAL>(ax - arrowHalf), static_cast<Gdiplus::REAL>(ay + arrowHeight) },
            { static_cast<Gdiplus::REAL>(ax), static_cast<Gdiplus::REAL>(ay) },
            { static_cast<Gdiplus::REAL>(ax + arrowHalf), static_cast<Gdiplus::REAL>(ay + arrowHeight) } };
        DrawLinesAa(hDC, edges, 3, theme::BorderLight, 1.0f);
    }

    return CContainerUI::DoPaint(hDC, rcPaint, pStopControl);
}

//////////////////////////////////////////////////////////////////////////
// ECollapseUI

namespace {

class ECollapseHeaderUI : public CButtonUI
{
public:
    explicit ECollapseHeaderUI(ECollapseUI* owner)
        : m_owner(owner)
    {
        SetFont(font::BodyBold);
    }

    LPCTSTR GetClass() const override
    {
        return _T("ECollapseHeader");
    }

    UINT GetControlFlags() const override
    {
        return UIFLAG_SETCURSOR;
    }

    void DoEvent(TEventUI& event) override
    {
        if (event.Type == UIEVENT_SETCURSOR) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
            return;
        }
        if (event.Type == UIEVENT_BUTTONUP && ::PtInRect(&m_rcItem, event.ptMouse)
            && (m_uButtonState & UISTATE_PUSHED) != 0 && m_owner != nullptr) {
            m_owner->Toggle();
        }
        CButtonUI::DoEvent(event);
    }

    void PaintBkColor(HDC hDC) override
    {
        if (IsHot(m_uButtonState)) {
            CRenderEngine::DrawColor(hDC, m_rcItem, theme::FillLighter);
        }
    }

    void PaintText(HDC hDC) override
    {
        if (m_sText.IsEmpty() || m_pManager == nullptr) {
            return;
        }
        RECT rc = m_rcItem;
        rc.left += UiScale::Px(16);
        rc.right -= UiScale::Px(36);
        CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText,
            IsEnabled() ? theme::TextPrimary : theme::TextPlaceholder, m_iFont,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    void PaintStatusImage(HDC hDC) override
    {
        if (m_owner == nullptr) {
            return;
        }
        const int cx = m_rcItem.right - UiScale::Px(20);
        const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
        const Gdiplus::REAL unit = static_cast<Gdiplus::REAL>(UiScale::Px(4));
        const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        Gdiplus::PointF points[3] = {};
        if (m_owner->IsExpanded()) {
            points[0] = Gdiplus::PointF(cxF - unit, cyF - unit / 2);
            points[1] = Gdiplus::PointF(cxF, cyF + unit / 2);
            points[2] = Gdiplus::PointF(cxF + unit, cyF - unit / 2);
        }
        else {
            points[0] = Gdiplus::PointF(cxF - unit / 2, cyF - unit);
            points[1] = Gdiplus::PointF(cxF + unit / 2, cyF);
            points[2] = Gdiplus::PointF(cxF - unit / 2, cyF + unit);
        }
        DrawLinesAa(hDC, points, 3, theme::TextSecondary,
            static_cast<float>(std::max(1, UiScale::Px(1))) * 1.4f);
    }

private:
    ECollapseUI* m_owner;
};

} // namespace

ECollapseUI::ECollapseUI()
    : m_expanded(true)
{
    SetBkColor(theme::White);
    SetBorderColor(theme::BorderLight);
    SetBorderSize({ UiScale::Px(1), UiScale::Px(1), UiScale::Px(1), UiScale::Px(1) });
    SetBorderRound({ UiScale::Px(4), UiScale::Px(4) });

    m_header = new ECollapseHeaderUI(this);
    m_header->SetFixedHeight(UiScale::Px(44));

    m_body = new CVerticalLayoutUI;
    m_body->SetInset({ UiScale::Px(16), 0, UiScale::Px(16), UiScale::Px(16) });
    m_body->SetChildPadding(UiScale::Px(6));

    CVerticalLayoutUI::Add(m_header);
    CVerticalLayoutUI::Add(m_body);
    UpdateHeight();
}

LPCTSTR ECollapseUI::GetClass() const
{
    return _T("ECollapse");
}

LPVOID ECollapseUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ECollapse")) == 0) {
        return static_cast<ECollapseUI*>(this);
    }
    return CVerticalLayoutUI::GetInterface(pstrName);
}

void ECollapseUI::SetTitle(const std::wstring& title)
{
    m_title = title;
    if (m_header != nullptr) {
        m_header->SetText(title.c_str());
    }
}

void ECollapseUI::SetExpanded(bool expanded)
{
    m_expanded = expanded;
    if (m_body != nullptr) {
        m_body->SetVisible(expanded);
    }
    if (m_header != nullptr) {
        m_header->Invalidate();
    }
    UpdateHeight();
    NeedParentUpdate();
}

void ECollapseUI::UpdateHeight()
{
    if (m_header == nullptr || m_body == nullptr) {
        return;
    }
    int height = m_header->GetFixedHeight();
    if (m_expanded) {
        const RECT inset = m_body->GetInset();
        height += inset.top + inset.bottom;
        const int count = m_body->GetCount();
        for (int index = 0; index < count; ++index) {
            CControlUI* child = m_body->GetItemAt(index);
            if (child == nullptr || !child->IsVisible()) {
                continue;
            }
            height += child->GetFixedHeight();
            if (index + 1 < count) {
                height += m_body->GetChildPadding();
            }
        }
    }
    SetFixedHeight(height);
}

bool ECollapseUI::IsExpanded() const
{
    return m_expanded;
}

void ECollapseUI::Toggle()
{
    SetExpanded(!m_expanded);
}

CVerticalLayoutUI* ECollapseUI::Body() const
{
    return m_body;
}

void ECollapseUI::SetHeaderHeight(int height)
{
    if (m_header != nullptr) {
        m_header->SetFixedHeight(height);
    }
}

//////////////////////////////////////////////////////////////////////////
// EInfiniteListUI

LPCTSTR EInfiniteListUI::GetClass() const
{
    return _T("EInfiniteList");
}

LPVOID EInfiniteListUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EInfiniteList")) == 0) {
        return static_cast<EInfiniteListUI*>(this);
    }
    return CListUI::GetInterface(pstrName);
}

void EInfiniteListUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CListUI::SetManager(pManager, pParent, bInit);
    if (pManager != nullptr && GetVerticalScrollBar() == nullptr) {
        EnableScrollBar(true, false);
        CListHeaderUI* header = GetHeader();
        if (header != nullptr && header->GetCount() == 0) {
            header->SetVisible(false);
        }
    }
}

void EInfiniteListUI::DoEvent(TEventUI& event)
{
    CListUI::DoEvent(event);
    if (event.Type == UIEVENT_SCROLLWHEEL) {
        CheckNearBottom();
    }
}

void EInfiniteListUI::CheckNearBottom()
{
    if (m_pManager == nullptr) {
        return;
    }
    CScrollBarUI* bar = GetVerticalScrollBar();
    if (bar == nullptr || !bar->IsVisible()) {
        return;
    }
    const int range = bar->GetScrollRange();
    const int pos = bar->GetScrollPos();
    if (range > 0 && range - pos <= UiScale::Px(8)) {
        m_pManager->SendNotify(this, _T("loadmore"));
    }
}

//////////////////////////////////////////////////////////////////////////
// ESelectUI

namespace {

bool ContainsText(const std::wstring& text, const std::wstring& query)
{
    if (query.empty()) {
        return true;
    }
    std::wstring lowerText = text;
    std::wstring lowerQuery = query;
    for (wchar_t& ch : lowerText) {
        ch = static_cast<wchar_t>(towlower(ch));
    }
    for (wchar_t& ch : lowerQuery) {
        ch = static_cast<wchar_t>(towlower(ch));
    }
    return lowerText.find(lowerQuery) != std::wstring::npos;
}

int MeasureText(DuiLib::CPaintManagerUI* manager, const std::wstring& text, int fontId)
{
    if (manager == nullptr || text.empty()) {
        return 0;
    }
    HDC hDC = ::GetDC(manager->GetPaintWindow());
    const SIZE size = DuiLib::CRenderEngine::GetTextSize(hDC, manager, text.c_str(), fontId, DT_SINGLELINE);
    ::ReleaseDC(manager->GetPaintWindow(), hDC);
    return size.cx;
}

} // namespace

ESelectUI::ESelectUI()
    : m_multiple(false)
    , m_clearable(false)
    , m_filterable(false)
    , m_remote(false)
    , m_allowCreate(false)
    , m_open(false)
    , m_hover(false)
    , m_size(1)
    , m_placeholder(L"请选择")
{
    SetFixedHeight(32);
    m_cxyBorderRound.cx = 4;
    m_cxyBorderRound.cy = 4;
}

LPCTSTR ESelectUI::GetClass() const
{
    return _T("ESelect");
}

LPVOID ESelectUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ESelect")) == 0) {
        return static_cast<ESelectUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

UINT ESelectUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void ESelectUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if (_tcsicmp(pstrName, _T("placeholder")) == 0) {
        m_placeholder = pstrValue != nullptr ? pstrValue : L"";
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("multiple")) == 0) {
        m_multiple = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("clearable")) == 0) {
        m_clearable = _tcsicmp(pstrValue, _T("true")) == 0;
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("filterable")) == 0) {
        m_filterable = _tcsicmp(pstrValue, _T("true")) == 0;
    }
    else if (_tcsicmp(pstrName, _T("remote")) == 0) {
        m_remote = _tcsicmp(pstrValue, _T("true")) == 0;
    }
    else if (_tcsicmp(pstrName, _T("allowcreate")) == 0) {
        m_allowCreate = _tcsicmp(pstrValue, _T("true")) == 0;
    }
    else if (_tcsicmp(pstrName, _T("size")) == 0) {
        m_size = 1;
        if (_tcsicmp(pstrValue, _T("small")) == 0) m_size = 0;
        else if (_tcsicmp(pstrValue, _T("large")) == 0) m_size = 2;
        SetFixedHeight(m_size == 0 ? 24 : (m_size == 2 ? 40 : 32));
        Invalidate();
    }
    else if (_tcsicmp(pstrName, _T("demo")) == 0) {
        m_demo = pstrValue != nullptr ? pstrValue : L"";
    }
    else {
        CControlUI::SetAttribute(pstrName, pstrValue);
    }
}

void ESelectUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    Invalidate();
}

void ESelectUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_SETCURSOR && IsEnabled()) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return;
    }
    if (event.Type == UIEVENT_MOUSEENTER || event.Type == UIEVENT_MOUSEMOVE) {
        if (!m_hover) {
            m_hover = true;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hover) {
            m_hover = false;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_BUTTONUP && IsEnabled() && ::PtInRect(&m_rcItem, event.ptMouse)) {
        if (m_multiple) {
            std::vector<RECT> tags;
            std::vector<RECT> closes;
            std::vector<int> indexes;
            int plusWidth = 0;
            BuildTagLayout(tags, closes, indexes, plusWidth);
            for (size_t i = 0; i < closes.size(); ++i) {
                if (::PtInRect(&closes[i], event.ptMouse) && i < indexes.size()) {
                    const int position = indexes[i];
                    if (position >= 0 && position < static_cast<int>(m_selected.size())) {
                        m_selected.erase(m_selected.begin() + position);
                        NotifyChanged();
                        if (m_pManager != nullptr) {
                            m_pManager->SendNotify(this, _T("selectclear"));
                        }
                    }
                    return;
                }
            }
        }
        if (m_clearable && HasSelection() && ::PtInRect(&ClearRect(), event.ptMouse)) {
            ClearSelection();
            NotifyChanged();
            if (m_pManager != nullptr) {
                m_pManager->SendNotify(this, _T("selectclear"));
            }
            return;
        }
        if (m_pManager != nullptr) {
            m_pManager->SendNotify(this, _T("selecttoggle"));
        }
        return;
    }
    CControlUI::DoEvent(event);
}

void ESelectUI::PaintBkColor(HDC hDC)
{
    FillRoundRectColor(hDC, m_rcItem, m_cxyBorderRound.cx,
        IsEnabled() ? theme::White : theme::FillLight);
}

void ESelectUI::PaintBorder(HDC hDC)
{
    DWORD color = theme::BorderBase;
    if (!IsEnabled()) {
        color = theme::BorderLight;
    }
    else if (m_open) {
        color = theme::Primary;
    }
    else if (m_hover) {
        color = theme::TextPlaceholder;
    }

    Gdiplus::GraphicsPath path;
    RECT rc = m_rcItem;
    rc.left += 1;
    rc.top += 1;
    rc.right -= 1;
    rc.bottom -= 1;
    MakeRoundRectPath(path, rc, m_cxyBorderRound.cx);
    StrokePathAa(hDC, path, color, 1);
}

void ESelectUI::PaintStatusImage(HDC hDC)
{
    if (m_pManager == nullptr) {
        return;
    }

    const bool enabled = IsEnabled();
    const int font = FontForBox();
    const DWORD valueColor = enabled ? theme::TextPrimary : theme::TextPlaceholder;

    if (m_multiple) {
        if (m_selected.empty()) {
            RECT rc = { m_rcItem.left + UiScale::Px(15), m_rcItem.top,
                m_rcItem.right - UiScale::Px(32), m_rcItem.bottom };
            CRenderEngine::DrawText(hDC, m_pManager, rc, m_placeholder.c_str(), theme::TextPlaceholder,
                font, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        else {
            std::vector<RECT> tags;
            std::vector<RECT> closes;
            std::vector<int> indexes;
            int plusWidth = 0;
            BuildTagLayout(tags, closes, indexes, plusWidth);
            for (size_t i = 0; i < tags.size(); ++i) {
                FillRoundRectColor(hDC, tags[i], UiScale::Px(4), theme::FillLight);
                Gdiplus::GraphicsPath border;
                MakeRoundRectPath(border, tags[i], UiScale::Px(4));
                StrokePathAa(hDC, border, theme::BorderLight, 1);

                RECT textRc = { tags[i].left + UiScale::Px(8), tags[i].top,
                    tags[i].right - UiScale::Px(18), tags[i].bottom };
                const SelectOption* option = i < indexes.size()
                    ? OptionAt(m_selected[indexes[i]]) : nullptr;
                if (option != nullptr) {
                    CRenderEngine::DrawText(hDC, m_pManager, textRc, option->label.c_str(),
                        valueColor, font, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                }

                const int cx = (closes[i].left + closes[i].right) / 2;
                const int cy = (closes[i].top + closes[i].bottom) / 2;
                const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
                Gdiplus::PointF cross[4] = {
                    { cx - arm, cy - arm }, { cx + arm, cy + arm },
                    { cx - arm, cy + arm }, { cx + arm, cy - arm } };
                DrawLinesAa(hDC, cross, 2, theme::TextPlaceholder, 1.0f);
                DrawLinesAa(hDC, cross + 2, 2, theme::TextPlaceholder, 1.0f);
            }
            if (plusWidth > 0 && !tags.empty()) {
                RECT plusRc = { tags.back().right + UiScale::Px(4), m_rcItem.top,
                    tags.back().right + UiScale::Px(4) + plusWidth, m_rcItem.bottom };
                const int hidden = static_cast<int>(m_selected.size() - tags.size());
                const std::wstring text = L"+" + std::to_wstring(hidden);
                CRenderEngine::DrawText(hDC, m_pManager, plusRc, text.c_str(), theme::TextRegular,
                    font, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
    else {
        std::wstring text;
        DWORD color = valueColor;
        if (!m_selected.empty()) {
            const SelectOption* option = OptionAt(m_selected.front());
            if (option != nullptr) {
                text = option->label;
            }
        }
        else {
            text = m_placeholder;
            color = theme::TextPlaceholder;
        }
        RECT rc = { m_rcItem.left + UiScale::Px(15), m_rcItem.top,
            m_rcItem.right - UiScale::Px(32), m_rcItem.bottom };
        CRenderEngine::DrawText(hDC, m_pManager, rc, text.c_str(), color, font,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    if (m_clearable && HasSelection() && m_hover && enabled) {
        RECT clear = ClearRect();
        FillEllipseColor(hDC, clear, theme::TextPlaceholder, 0, 0);
        const int cx = (clear.left + clear.right) / 2;
        const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
        Gdiplus::PointF cross[4] = {
            { cxF - arm, cyF - arm }, { cxF + arm, cyF + arm },
            { cxF - arm, cyF + arm }, { cxF + arm, cyF - arm } };
        DrawLinesAa(hDC, cross, 2, theme::White, 1.2f);
        DrawLinesAa(hDC, cross + 2, 2, theme::White, 1.2f);
    }
    else {
        const Gdiplus::REAL cx = static_cast<Gdiplus::REAL>(m_rcItem.right - UiScale::Px(22));
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        const Gdiplus::REAL half = static_cast<Gdiplus::REAL>(UiScale::Px(4));
        const Gdiplus::REAL drop = static_cast<Gdiplus::REAL>(UiScale::Px(2));
        const DWORD arrowColor = !enabled ? theme::TextPlaceholder
            : (m_open ? theme::Primary : theme::TextPlaceholder);
        Gdiplus::PointF points[3] = {};
        if (m_open) {
            points[0] = Gdiplus::PointF(cx - half, cyF + drop);
            points[1] = Gdiplus::PointF(cx, cyF - drop);
            points[2] = Gdiplus::PointF(cx + half, cyF + drop);
        }
        else {
            points[0] = Gdiplus::PointF(cx - half, cyF - drop);
            points[1] = Gdiplus::PointF(cx, cyF + drop);
            points[2] = Gdiplus::PointF(cx + half, cyF - drop);
        }
        DrawLinesAa(hDC, points, 3, arrowColor, 1.4f);
    }
}

void ESelectUI::AddOption(const std::wstring& label, const std::wstring& desc, bool disabled)
{
    SelectOption option;
    option.label = label;
    option.desc = desc;
    option.disabled = disabled;
    m_options.push_back(option);
    Invalidate();
}

void ESelectUI::AddGroup(const std::wstring& label)
{
    SelectOption option;
    option.label = label;
    option.group = true;
    m_options.push_back(option);
    Invalidate();
}

const std::vector<SelectOption>& ESelectUI::GetOptions() const
{
    return m_options;
}

std::vector<int> ESelectUI::GetSelection() const
{
    return m_selected;
}

void ESelectUI::ApplySelection(const std::vector<int>& selection)
{
    m_selected.clear();
    for (int index : selection) {
        if (OptionAt(index) == nullptr) {
            continue;
        }
        if (!m_multiple) {
            m_selected.assign(1, index);
            break;
        }
        if (std::find(m_selected.begin(), m_selected.end(), index) == m_selected.end()) {
            m_selected.push_back(index);
        }
    }
    Invalidate();
}

void ESelectUI::ClearSelection()
{
    m_selected.clear();
    Invalidate();
}

void ESelectUI::AddCreated(const std::wstring& label)
{
    if (label.empty()) {
        return;
    }
    int index = -1;
    for (size_t i = 0; i < m_options.size(); ++i) {
        if (!m_options[i].group && m_options[i].label == label) {
            index = static_cast<int>(i);
            break;
        }
    }
    if (index < 0) {
        AddOption(label);
        index = static_cast<int>(m_options.size()) - 1;
    }
    if (m_multiple) {
        if (std::find(m_selected.begin(), m_selected.end(), index) == m_selected.end()) {
            m_selected.push_back(index);
        }
    }
    else {
        m_selected.assign(1, index);
    }
    NotifyChanged();
}

bool ESelectUI::IsMultiple() const
{
    return m_multiple;
}

bool ESelectUI::IsClearable() const
{
    return m_clearable;
}

bool ESelectUI::IsSearchable() const
{
    return m_filterable || m_remote;
}

bool ESelectUI::IsRemote() const
{
    return m_remote;
}

bool ESelectUI::IsAllowCreate() const
{
    return m_allowCreate;
}

void ESelectUI::SetOpen(bool open)
{
    if (m_open == open) {
        return;
    }
    m_open = open;
    Invalidate();
}

std::wstring ESelectUI::GetDisplayText() const
{
    std::wstring text;
    for (size_t i = 0; i < m_selected.size(); ++i) {
        const SelectOption* option = OptionAt(m_selected[i]);
        if (option == nullptr) {
            continue;
        }
        if (!text.empty()) {
            text += L"、";
        }
        text += option->label;
    }
    return text.empty() ? L"（未选择）" : text;
}

std::wstring ESelectUI::GetDemoTitle() const
{
    return m_demo;
}

int ESelectUI::BoxHeight() const
{
    const int height = m_rcItem.bottom - m_rcItem.top;
    return height > 0 ? height : UiScale::Px(32);
}

int ESelectUI::FontForBox() const
{
    if (m_size == 0) {
        return font::Small;
    }
    if (m_size == 2) {
        return font::Body14;
    }
    return font::Body;
}

int ESelectUI::TextWidth(const std::wstring& text, int fontId) const
{
    return MeasureText(m_pManager, text, fontId);
}

const SelectOption* ESelectUI::OptionAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_options.size())) {
        return nullptr;
    }
    return &m_options[index];
}

bool ESelectUI::HasSelection() const
{
    return !m_selected.empty();
}

RECT ESelectUI::ClearRect() const
{
    const int cy = (m_rcItem.top + m_rcItem.bottom) / 2;
    const int diameter = UiScale::Px(14);
    const int right = m_rcItem.right - UiScale::Px(15);
    return { right - diameter, cy - diameter / 2, right, cy + diameter / 2 };
}

void ESelectUI::BuildTagLayout(std::vector<RECT>& tags, std::vector<RECT>& closes,
    std::vector<int>& indexes, int& plusWidth) const
{
    tags.clear();
    closes.clear();
    indexes.clear();
    plusWidth = 0;
    if (!m_multiple || m_selected.empty()) {
        return;
    }

    const int font = FontForBox();
    const int height = BoxHeight();
    const int tagHeight = (std::max)(UiScale::Px(20), height - UiScale::Px(10));
    const int tagTop = m_rcItem.top + (height - tagHeight) / 2;
    const int limit = m_rcItem.right - UiScale::Px(34);
    int x = m_rcItem.left + UiScale::Px(8);
    int hidden = 0;

    for (size_t i = 0; i < m_selected.size(); ++i) {
        const SelectOption* option = OptionAt(m_selected[i]);
        if (option == nullptr) {
            continue;
        }
        const int width = TextWidth(option->label, font) + UiScale::Px(30);
        if (x + width > limit) {
            hidden = static_cast<int>(m_selected.size() - i);
            break;
        }
        RECT tag = { x, tagTop, x + width, tagTop + tagHeight };
        RECT close = { tag.right - UiScale::Px(18), tag.top, tag.right - UiScale::Px(4), tag.bottom };
        tags.push_back(tag);
        closes.push_back(close);
        indexes.push_back(static_cast<int>(i));
        x += width + UiScale::Px(6);
    }

    if (hidden > 0) {
        plusWidth = TextWidth(L"+" + std::to_wstring(hidden), font) + UiScale::Px(4);
    }
}

void ESelectUI::NotifyChanged()
{
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
    }
}

//////////////////////////////////////////////////////////////////////////
// ESelectPopupUI

ESelectPopupUI::ESelectPopupUI()
    : m_multiple(false)
    , m_searchable(false)
    , m_remote(false)
    , m_loading(false)
    , m_scroll(0)
    , m_hover(-1)
    , m_rowHeight(UiScale::Px(34))
    , m_groupHeight(UiScale::Px(30))
    , m_searchHeight(UiScale::Px(34))
    , m_edit(nullptr)
{
    SetBkColor(0);
    SetBorderSize({ 0, 0, 0, 0 });
    SetVisible(false);

    m_edit = new CEditUI;
    m_edit->SetName(_T("selectSearchEdit"));
    m_edit->SetFont(font::Body);
    m_edit->SetTextColor(theme::TextPrimary);
    m_edit->SetBkColor(theme::White);
    m_edit->SetNativeEditBkColor(theme::White);
    m_edit->SetBorderSize({ 0, 0, 0, 0 });
    m_edit->SetTextPadding({ 0, 0, 0, 0 });
    m_edit->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    m_edit->SetVisible(false);
    Add(m_edit);
}

LPCTSTR ESelectPopupUI::GetClass() const
{
    return _T("ESelectPopup");
}

LPVOID ESelectPopupUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ESelectPopup")) == 0) {
        return static_cast<ESelectPopupUI*>(this);
    }
    return CContainerUI::GetInterface(pstrName);
}

void ESelectPopupUI::SetPos(RECT rc, bool bNeedInvalidate)
{
    CControlUI::SetPos(rc, bNeedInvalidate);
    LayoutSearchEdit();
}

void ESelectPopupUI::SetVisible(bool bVisible)
{
    CContainerUI::SetVisible(bVisible);
    if (m_edit != nullptr) {
        m_edit->SetVisible(bVisible && m_searchable);
    }
}

void ESelectPopupUI::Open(const std::vector<SelectOption>& options, const std::vector<int>& selected,
    bool multiple, bool searchable, bool remote)
{
    if (m_pManager != nullptr) {
        m_pManager->KillTimer(this, 1);
    }
    m_options = options;
    m_selected = selected;
    m_multiple = multiple;
    m_searchable = searchable;
    m_remote = remote;
    m_loading = false;
    m_query.clear();
    m_pendingQuery.clear();
    m_scroll = 0;
    m_hover = -1;
    if (m_edit != nullptr) {
        m_edit->SetText(L"");
        m_edit->SetVisible(m_searchable);
    }
    RebuildRows();
    UpdatePanelSize();
    Invalidate();
}

int ESelectPopupUI::ContentHeight() const
{
    const int total = UiScale::Px(6) * 2 + (m_searchable ? m_searchHeight : 0) + RowsHeight();
    const int cap = UiScale::Px(288);
    return (std::min)(total, cap);
}

void ESelectPopupUI::SetQuery(const std::wstring& query)
{
    if (m_query == query && !m_loading) {
        return;
    }
    if (m_remote) {
        m_pendingQuery = query;
        m_loading = true;
        m_rows.clear();
        m_scroll = 0;
        m_hover = -1;
        if (m_pManager != nullptr) {
            m_pManager->SetTimer(this, 1, 400);
        }
    }
    else {
        m_query = query;
        RebuildRows();
    }
    UpdatePanelSize();
    Invalidate();
}

void ESelectPopupUI::FinishRemote()
{
    if (!m_loading) {
        return;
    }
    m_loading = false;
    m_query = m_pendingQuery;
    RebuildRows();
    UpdatePanelSize();
    Invalidate();
}

void ESelectPopupUI::FocusSearch()
{
    if (m_edit == nullptr || !m_searchable) {
        return;
    }
    m_edit->SetVisible(true);
    if (m_pManager != nullptr) {
        m_pManager->SetFocus(m_edit);
    }
}

DuiLib::CEditUI* ESelectPopupUI::SearchEdit() const
{
    return m_edit;
}

std::vector<int> ESelectPopupUI::GetSelection() const
{
    return m_selected;
}

bool ESelectPopupUI::IsMultiple() const
{
    return m_multiple;
}

void ESelectPopupUI::RebuildRows()
{
    m_rows.clear();
    if (m_remote && m_query.empty()) {
        return;
    }
    int pendingGroup = -1;
    for (size_t i = 0; i < m_options.size(); ++i) {
        const SelectOption& option = m_options[i];
        if (option.group) {
            pendingGroup = static_cast<int>(i);
            continue;
        }
        if (!ContainsText(option.label, m_query)) {
            continue;
        }
        if (pendingGroup >= 0) {
            m_rows.push_back(pendingGroup);
            pendingGroup = -1;
        }
        m_rows.push_back(static_cast<int>(i));
    }
}

void ESelectPopupUI::UpdatePanelSize()
{
    SetFixedHeight(ContentHeight());
}

void ESelectPopupUI::LayoutSearchEdit()
{
    if (m_edit == nullptr) {
        return;
    }
    RECT rc = { m_rcItem.left + UiScale::Px(40), m_rcItem.top + UiScale::Px(7),
        m_rcItem.right - UiScale::Px(12), m_rcItem.top + UiScale::Px(7) + UiScale::Px(24) };
    m_edit->SetPos(rc);
}

bool ESelectPopupUI::IsLoading() const
{
    return m_loading;
}

bool ESelectPopupUI::Matches(const std::wstring& label) const
{
    return ContainsText(label, m_query);
}

int ESelectPopupUI::RowsHeight() const
{
    int height = 0;
    for (int index : m_rows) {
        if (index >= 0 && index < static_cast<int>(m_options.size()) && m_options[index].group) {
            height += m_groupHeight;
        }
        else {
            height += m_rowHeight;
        }
    }
    return height > 0 ? height : m_rowHeight;
}

int ESelectPopupUI::RowsTop() const
{
    return m_rcItem.top + UiScale::Px(6) + (m_searchable ? m_searchHeight : 0);
}

int ESelectPopupUI::RowsViewHeight() const
{
    return (std::max)(0, static_cast<int>(m_rcItem.bottom - UiScale::Px(6) - RowsTop()));
}

int ESelectPopupUI::RowAt(const POINT& pt) const
{
    if (!::PtInRect(&m_rcItem, pt)) {
        return -1;
    }
    const int top = RowsTop();
    if (pt.y < top || pt.y >= m_rcItem.bottom - UiScale::Px(6)) {
        return -1;
    }
    int y = top - m_scroll;
    for (size_t i = 0; i < m_rows.size(); ++i) {
        const int index = m_rows[i];
        const bool group = index >= 0 && index < static_cast<int>(m_options.size())
            && m_options[index].group;
        const int height = group ? m_groupHeight : m_rowHeight;
        if (pt.y >= y && pt.y < y + height) {
            return static_cast<int>(i);
        }
        y += height;
    }
    return -1;
}

void ESelectPopupUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_TIMER && event.wParam == 1) {
        if (m_pManager != nullptr) {
            m_pManager->KillTimer(this, 1);
        }
        FinishRemote();
        return;
    }
    if (event.Type == UIEVENT_SETCURSOR) {
        const int row = RowAt(event.ptMouse);
        if (row >= 0 && row < static_cast<int>(m_rows.size())) {
            const int index = m_rows[row];
            if (index >= 0 && index < static_cast<int>(m_options.size())
                && !m_options[index].group && !m_options[index].disabled) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }
    }
    else if (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSEENTER) {
        const int row = RowAt(event.ptMouse);
        if (row != m_hover) {
            m_hover = row;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hover != -1) {
            m_hover = -1;
            Invalidate();
        }
    }
    else if (event.Type == UIEVENT_BUTTONUP && ::PtInRect(&m_rcItem, event.ptMouse)) {
        const int row = RowAt(event.ptMouse);
        if (row >= 0 && row < static_cast<int>(m_rows.size())) {
            const int index = m_rows[row];
            if (index >= 0 && index < static_cast<int>(m_options.size())
                && !m_options[index].group && !m_options[index].disabled) {
                if (m_multiple) {
                    auto found = std::find(m_selected.begin(), m_selected.end(), index);
                    if (found == m_selected.end()) {
                        m_selected.push_back(index);
                    }
                    else {
                        m_selected.erase(found);
                    }
                }
                else {
                    m_selected.assign(1, index);
                }
                Invalidate();
                NotifyPick(index);
                return;
            }
        }
    }
    else if (event.Type == UIEVENT_SCROLLWHEEL) {
        const int view = RowsViewHeight();
        const int maxScroll = (std::max)(0, RowsHeight() - view);
        const int delta = LOWORD(event.wParam) == SB_LINEDOWN ? m_rowHeight * 2 : -m_rowHeight * 2;
        const int next = (std::max)(0, (std::min)(m_scroll + delta, maxScroll));
        if (next != m_scroll) {
            m_scroll = next;
            Invalidate();
        }
        return;
    }
    CContainerUI::DoEvent(event);
}

void ESelectPopupUI::PaintBkColor(HDC hDC)
{
    FillRoundRectColor(hDC, m_rcItem, UiScale::Px(4), theme::White);
}

void ESelectPopupUI::PaintBorder(HDC hDC)
{
    RECT rc = m_rcItem;
    rc.left += 1;
    rc.top += 1;
    rc.right -= 1;
    rc.bottom -= 1;
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, rc, UiScale::Px(4));
    StrokePathAa(hDC, path, theme::BorderLight, 1);
}

void ESelectPopupUI::PaintStatusImage(HDC hDC)
{
    if (m_pManager == nullptr) {
        return;
    }

    if (m_searchable) {
        const int cy = m_rcItem.top + UiScale::Px(6) + m_searchHeight / 2;
        const int cx = m_rcItem.left + UiScale::Px(20);
        const Gdiplus::REAL radius = static_cast<Gdiplus::REAL>(UiScale::Px(5));
        const Gdiplus::REAL cxF = static_cast<Gdiplus::REAL>(cx);
        const Gdiplus::REAL cyF = static_cast<Gdiplus::REAL>(cy);
        Gdiplus::RectF circle(cxF - radius, cyF - radius, radius * 2, radius * 2);
        {
            EnsureGdiplus();
            Gdiplus::Graphics graphics(hDC);
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            Gdiplus::Pen pen(GpColor(theme::TextSecondary), 1.4f);
            graphics.DrawEllipse(&pen, circle);
        }
        Gdiplus::PointF handle[2] = {
            { cxF + radius * 0.7f, cyF + radius * 0.7f },
            { cxF + radius * 1.6f, cyF + radius * 1.6f } };
        DrawLinesAa(hDC, handle, 2, theme::TextSecondary, 1.4f);

        RECT separator = { m_rcItem.left + UiScale::Px(10), m_rcItem.top + UiScale::Px(6) + m_searchHeight - 1,
            m_rcItem.right - UiScale::Px(10), m_rcItem.top + UiScale::Px(6) + m_searchHeight - 1 };
        CRenderEngine::DrawLine(hDC, separator, 1, theme::BorderLighter);
    }

    const int top = RowsTop();
    const int bottom = m_rcItem.bottom - UiScale::Px(6);
    const int rowsHeight = RowsHeight();

    if (m_rows.empty()) {
        std::wstring hint;
        if (m_loading) {
            hint = L"加载中…";
        }
        else if (m_remote && m_query.empty()) {
            hint = L"输入关键字搜索";
        }
        else {
            hint = L"无匹配数据";
        }
        RECT rc = { m_rcItem.left, top, m_rcItem.right, top + m_rowHeight };
        CRenderEngine::DrawText(hDC, m_pManager, rc, hint.c_str(), theme::TextSecondary,
            font::Small, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    const int saved = ::SaveDC(hDC);
    ::IntersectClipRect(hDC, m_rcItem.left, top, m_rcItem.right, bottom);
    int y = top - m_scroll;
    for (size_t i = 0; i < m_rows.size(); ++i) {
        const int index = m_rows[i];
        if (index < 0 || index >= static_cast<int>(m_options.size())) {
            continue;
        }
        const SelectOption& option = m_options[index];
        const int height = option.group ? m_groupHeight : m_rowHeight;
        if (y + height > top && y < bottom) {
            if (option.group) {
                RECT rc = { m_rcItem.left + UiScale::Px(20), y, m_rcItem.right - UiScale::Px(16), y + height };
                CRenderEngine::DrawText(hDC, m_pManager, rc, option.label.c_str(), theme::TextSecondary,
                    font::Small, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            }
            else {
                const bool selected = std::find(m_selected.begin(), m_selected.end(), index)
                    != m_selected.end();
                const bool hot = static_cast<int>(i) == m_hover && !option.disabled;
                RECT row = { m_rcItem.left + UiScale::Px(6), y, m_rcItem.right - UiScale::Px(6), y + height };
                if (hot) {
                    FillRoundRectColor(hDC, row, UiScale::Px(4), theme::FillLight);
                }

                int rightReserve = UiScale::Px(20);
                RECT descRc = { m_rcItem.left, y, m_rcItem.right - UiScale::Px(36), y + height };
                if (!option.desc.empty()) {
                    const int descWidth = MeasureText(m_pManager, option.desc, font::Small);
                    descRc.left = descRc.right - descWidth;
                    CRenderEngine::DrawText(hDC, m_pManager, descRc, option.desc.c_str(),
                        theme::TextSecondary, font::Small,
                        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                    rightReserve += descWidth + UiScale::Px(16);
                }

                RECT textRc = { m_rcItem.left + UiScale::Px(20), y,
                    m_rcItem.right - rightReserve - UiScale::Px(20), y + height };
                const DWORD color = option.disabled ? theme::TextPlaceholder
                    : (selected ? theme::Primary : theme::TextRegular);
                CRenderEngine::DrawText(hDC, m_pManager, textRc, option.label.c_str(), color,
                    font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                if (selected) {
                    RECT check = { m_rcItem.right - UiScale::Px(30), y,
                        m_rcItem.right - UiScale::Px(12), y + height };
                    CRenderEngine::DrawText(hDC, m_pManager, check, _T("\xE73E"), theme::Primary,
                        font::IconSmall, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
            }
        }
        y += height;
    }
    ::RestoreDC(hDC, saved);

    const int view = RowsViewHeight();
    if (rowsHeight > view && view > 0) {
        const int trackHeight = view;
        const int thumbHeight = (std::max)(UiScale::Px(12), view * view / rowsHeight);
        const int maxScroll = rowsHeight - view;
        const int thumbTop = top + (trackHeight - thumbHeight) * m_scroll / (maxScroll > 0 ? maxScroll : 1);
        RECT track = { m_rcItem.right - UiScale::Px(6), top, m_rcItem.right - UiScale::Px(3), top + trackHeight };
        CRenderEngine::DrawColor(hDC, track, theme::FillLight);
        RECT thumb = { track.left, thumbTop, track.right, thumbTop + thumbHeight };
        FillRoundRectColor(hDC, thumb, UiScale::Px(2), theme::TextPlaceholder);
    }
}

void ESelectPopupUI::NotifyPick(int optionIndex)
{
    if (m_pManager != nullptr) {
        m_pManager->SendNotify(this, _T("selectpick"), static_cast<WPARAM>(optionIndex));
    }
}

//////////////////////////////////////////////////////////////////////////
// EDateTimeUI

LPCTSTR EDateTimeUI::GetClass() const
{
    return _T("EDateTime");
}

LPVOID EDateTimeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EDateTime")) == 0) {
        return static_cast<EDateTimeUI*>(this);
    }
    return CDateTimeUI::GetInterface(pstrName);
}

void EDateTimeUI::DoEvent(TEventUI& event)
{
    const bool hadWindow = GetNativeWindow() != nullptr;
    CDateTimeUI::DoEvent(event);

    const HWND hwnd = GetNativeWindow();
    if (hadWindow || hwnd == nullptr) {
        return;
    }
    ::SendMessage(hwnd, DTM_SETFORMAT, 0, reinterpret_cast<LPARAM>(_T("   yyyy-MM-dd")));
}

//////////////////////////////////////////////////////////////////////////
// ETreeUI

namespace {

void DrawTreeCheckbox(HDC hDC, const RECT& rc, bool checked, bool indeterminate, bool enabled)
{
    const DWORD border = !enabled ? theme::BorderLight
        : ((checked || indeterminate) ? theme::Primary : theme::BorderBase);
    const DWORD fill = (checked || indeterminate)
        ? (enabled ? theme::Primary : theme::PrimaryDisabled) : theme::White;
    FillRoundRectColor(hDC, rc, UiScale::Px(4), fill);
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, rc, UiScale::Px(4));
    StrokePathAa(hDC, path, border, 1);

    const int cy = (rc.top + rc.bottom) / 2;
    if (checked) {
        DrawCascaderCheck(hDC, rc.right - UiScale::Px(3), cy, theme::White);
    }
    else if (indeterminate) {
        const int thickness = (std::max)(1, UiScale::Px(2));
        RECT line = { rc.left + UiScale::Px(3), cy - thickness / 2,
            rc.right - UiScale::Px(3), cy - thickness / 2 + thickness };
        FillRoundRectColor(hDC, line, thickness, theme::White);
    }
}

void DrawTreeArrow(HDC hDC, int cx, int cy, bool expanded)
{
    const Gdiplus::REAL arm = static_cast<Gdiplus::REAL>(UiScale::Px(3));
    Gdiplus::PointF points[3];
    if (expanded) {
        points[0] = { static_cast<Gdiplus::REAL>(cx) - arm, static_cast<Gdiplus::REAL>(cy) - arm * 0.6f };
        points[1] = { static_cast<Gdiplus::REAL>(cx) + arm, static_cast<Gdiplus::REAL>(cy) - arm * 0.6f };
        points[2] = { static_cast<Gdiplus::REAL>(cx), static_cast<Gdiplus::REAL>(cy) + arm * 0.9f };
    }
    else {
        points[0] = { static_cast<Gdiplus::REAL>(cx) - arm * 0.6f, static_cast<Gdiplus::REAL>(cy) - arm };
        points[1] = { static_cast<Gdiplus::REAL>(cx) + arm * 0.9f, static_cast<Gdiplus::REAL>(cy) };
        points[2] = { static_cast<Gdiplus::REAL>(cx) - arm * 0.6f, static_cast<Gdiplus::REAL>(cy) + arm };
    }
    FillPolygonAa(hDC, points, 3, theme::TextPlaceholder);
}

std::wstring LowerText(const std::wstring& text)
{
    std::wstring lower = text;
    for (wchar_t& ch : lower) {
        ch = static_cast<wchar_t>(std::towlower(ch));
    }
    return lower;
}

} // namespace

ETreeUI::ETreeUI()
    : m_root(new TreeNode)
    , m_selected(nullptr)
    , m_hover(nullptr)
    , m_drag(nullptr)
    , m_drop(nullptr)
    , m_dragging(false)
    , m_dragMoved(false)
    , m_showCheckbox(false)
    , m_accordion(false)
    , m_draggable(false)
    , m_lazy(false)
    , m_showPlus(false)
    , m_loadingActive(false)
    , m_rowHeight(UiScale::Px(26))
    , m_appended(0)
{
    m_root->depth = -1;
    m_dragStart = { 0, 0 };
    SetBkColor(theme::White);
    SetFixedHeight(m_rowHeight * 3);
}

ETreeUI::~ETreeUI()
{
    if (m_loadingActive && m_pManager != nullptr) {
        m_pManager->KillTimer(this, 1);
    }
    std::vector<TreeNode*> stack;
    stack.push_back(m_root);
    while (!stack.empty()) {
        TreeNode* node = stack.back();
        stack.pop_back();
        for (TreeNode* child : node->children) {
            stack.push_back(child);
        }
        delete node;
    }
}

LPCTSTR ETreeUI::GetClass() const
{
    return _T("ETree");
}

LPVOID ETreeUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("ETree")) == 0) {
        return static_cast<ETreeUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

UINT ETreeUI::GetControlFlags() const
{
    return IsEnabled() ? UIFLAG_SETCURSOR : 0;
}

void ETreeUI::AddNode(const std::wstring& id, const std::wstring& parentId, const std::wstring& label,
    bool disabled, bool isLeaf)
{
    TreeNode* parent = parentId.empty() ? m_root : FindNode(m_root, parentId);
    if (parent == nullptr) {
        return;
    }
    auto* node = new TreeNode;
    node->id = id;
    node->label = label;
    node->disabled = disabled;
    node->isLeaf = isLeaf;
    node->parent = parent;
    node->depth = parent->depth + 1;
    if (m_lazy && parent != m_root) {
        parent->loaded = false;
    }
    parent->children.push_back(node);
}

void ETreeUI::ExpandKey(const std::wstring& id)
{
    TreeNode* node = FindNode(m_root, id);
    if (node == nullptr) {
        return;
    }
    node->expanded = true;
    node->loaded = true;
    UpdateHeight();
    Invalidate();
}

void ETreeUI::CheckKey(const std::wstring& id)
{
    TreeNode* node = FindNode(m_root, id);
    if (node == nullptr || node->disabled) {
        return;
    }
    SetSubtreeChecked(node, true);
    UpdateAncestors(node);
    Invalidate();
}

void ETreeUI::SelectKey(const std::wstring& id)
{
    m_selected = FindNode(m_root, id);
    Invalidate();
}

void ETreeUI::SetFilterText(const std::wstring& text)
{
    m_filter = LowerText(text);
    UpdateHeight();
    Invalidate();
}

void ETreeUI::SetShowCheckbox(bool show)
{
    m_showCheckbox = show;
    Invalidate();
}

void ETreeUI::SetAccordion(bool accordion)
{
    m_accordion = accordion;
}

void ETreeUI::SetDraggable(bool draggable)
{
    m_draggable = draggable;
}

void ETreeUI::SetLazy(bool lazy)
{
    m_lazy = lazy;
}

void ETreeUI::SetShowPlus(bool show)
{
    m_showPlus = show;
}

void ETreeUI::SetRowHeight(int height)
{
    m_rowHeight = (std::max)(UiScale::Px(20), height);
    UpdateHeight();
}

void ETreeUI::UpdateHeight()
{
    RebuildRows();
    const int rows = (std::max)(1, static_cast<int>(m_rows.size()));
    const int height = rows * m_rowHeight + UiScale::Px(4);
    if (GetFixedHeight() != height) {
        SetFixedHeight(height);
        if (m_pManager != nullptr) {
            m_pManager->SendNotify(this, _T("treesize"));
        }
    }
}

ETreeUI::TreeNode* ETreeUI::FindNode(TreeNode* node, const std::wstring& id) const
{
    if (node == nullptr) {
        return nullptr;
    }
    if (node->id == id) {
        return node;
    }
    for (TreeNode* child : node->children) {
        if (TreeNode* found = FindNode(child, id)) {
            return found;
        }
    }
    return nullptr;
}

bool ETreeUI::LabelMatches(TreeNode* node) const
{
    if (m_filter.empty()) {
        return true;
    }
    return LowerText(node->label).find(m_filter) != std::wstring::npos;
}

bool ETreeUI::SubtreeMatches(TreeNode* node) const
{
    if (node == nullptr) {
        return false;
    }
    if (LabelMatches(node)) {
        return true;
    }
    for (TreeNode* child : node->children) {
        if (SubtreeMatches(child)) {
            return true;
        }
    }
    return false;
}

void ETreeUI::CollectRows(TreeNode* node, std::vector<TreeRow>& rows) const
{
    if (node == nullptr) {
        return;
    }
    if (!m_filter.empty()) {
        if (!SubtreeMatches(node)) {
            return;
        }
        rows.push_back({ node, false });
        for (TreeNode* child : node->children) {
            CollectRows(child, rows);
        }
        return;
    }

    rows.push_back({ node, false });
    if (node->loading) {
        rows.push_back({ node, true });
        return;
    }
    if (!node->expanded) {
        return;
    }
    if (m_lazy && !node->loaded) {
        return;
    }
    for (TreeNode* child : node->children) {
        CollectRows(child, rows);
    }
}

void ETreeUI::RebuildRows()
{
    m_rows.clear();
    for (TreeNode* child : m_root->children) {
        CollectRows(child, m_rows);
    }
}

int ETreeUI::FindRowAt(const POINT& pt) const
{
    if (pt.x < m_rcItem.left || pt.x >= m_rcItem.right || pt.y < m_rcItem.top) {
        return -1;
    }
    const int index = (pt.y - m_rcItem.top) / m_rowHeight;
    return index >= 0 && index < static_cast<int>(m_rows.size()) ? index : -1;
}

RECT ETreeUI::RowRect(int index) const
{
    RECT rc = { m_rcItem.left, m_rcItem.top + index * m_rowHeight, m_rcItem.right,
        m_rcItem.top + (index + 1) * m_rowHeight };
    return rc;
}

void ETreeUI::PaintStatusImage(HDC hDC)
{
    RebuildRows();
    for (int index = 0; index < static_cast<int>(m_rows.size()); ++index) {
        PaintRow(hDC, m_rows[index], index);
    }
}

void ETreeUI::PaintRow(HDC hDC, const TreeRow& row, int index)
{
    const RECT rc = RowRect(index);
    TreeNode* node = row.node;
    const int contentLeft = m_rcItem.left + node->depth * UiScale::Px(18);

    if (row.loading) {
        RECT text = { contentLeft + UiScale::Px(22), rc.top, m_rcItem.right, rc.bottom };
        CRenderEngine::DrawText(hDC, m_pManager, text, L"加载中…", theme::TextSecondary,
            font::Body, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    if (m_dragging && m_drop == node) {
        FillRoundRectColor(hDC, rc, 0, theme::PrimaryBg);
        Gdiplus::GraphicsPath path;
        MakeRoundRectPath(path, rc, UiScale::Px(2));
        StrokePathAa(hDC, path, theme::Primary, 1);
    }
    else if (m_selected == node) {
        FillRoundRectColor(hDC, rc, 0, theme::PrimaryBg);
    }
    else if (m_hover == node) {
        FillRoundRectColor(hDC, rc, 0, theme::FillLight);
    }

    const int cy = (rc.top + rc.bottom) / 2;
    const bool filtered = !m_filter.empty();
    const bool expanded = filtered ? !node->children.empty() : node->expanded;
    if (!node->isLeaf && !node->children.empty()) {
        DrawTreeArrow(hDC, contentLeft + UiScale::Px(9), cy, expanded);
    }

    int labelLeft = contentLeft + UiScale::Px(22);
    if (m_showCheckbox) {
        RECT box = { contentLeft + UiScale::Px(22), cy - UiScale::Px(7),
            contentLeft + UiScale::Px(36), cy + UiScale::Px(7) };
        DrawTreeCheckbox(hDC, box, node->checked, node->indeterminate, !node->disabled);
        labelLeft = contentLeft + UiScale::Px(44);
    }

    const DWORD color = node->disabled ? theme::TextPlaceholder
        : (m_selected == node ? theme::Primary : theme::TextRegular);
    RECT text = { labelLeft, rc.top, m_rcItem.right - UiScale::Px(m_showPlus ? 38 : 8), rc.bottom };
    CRenderEngine::DrawText(hDC, m_pManager, text, node->label.c_str(), color, font::Body,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    if (m_showPlus) {
        RECT box = { m_rcItem.right - UiScale::Px(30), cy - UiScale::Px(10),
            m_rcItem.right - UiScale::Px(10), cy + UiScale::Px(10) };
        FillRoundRectColor(hDC, box, UiScale::Px(4), theme::White);
        Gdiplus::GraphicsPath path;
        MakeRoundRectPath(path, box, UiScale::Px(4));
        StrokePathAa(hDC, path, theme::BorderBase, 1);
        CRenderEngine::DrawText(hDC, m_pManager, box, L"+", theme::TextRegular, font::Body14,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

void ETreeUI::ToggleExpand(TreeNode* node)
{
    if (node == nullptr || node->isLeaf) {
        return;
    }
    if (!node->expanded && m_lazy && !node->loaded) {
        node->expanded = true;
        node->loading = true;
        if (!m_loadingActive) {
            m_loadingActive = true;
            if (m_pManager != nullptr) {
                m_pManager->SetTimer(this, 1, 600);
            }
        }
        UpdateHeight();
        Invalidate();
        return;
    }

    node->expanded = !node->expanded;
    if (node->expanded && m_accordion) {
        std::vector<TreeNode*>& siblings =
            node->parent != nullptr ? node->parent->children : m_root->children;
        for (TreeNode* sibling : siblings) {
            if (sibling != node) {
                sibling->expanded = false;
            }
        }
    }
    UpdateHeight();
    Invalidate();
}

void ETreeUI::SetSubtreeChecked(TreeNode* node, bool checked)
{
    if (node == nullptr) {
        return;
    }
    node->checked = checked;
    node->indeterminate = false;
    for (TreeNode* child : node->children) {
        if (!child->disabled) {
            SetSubtreeChecked(child, checked);
        }
    }
}

void ETreeUI::UpdateAncestors(TreeNode* node)
{
    for (TreeNode* parent = node != nullptr ? node->parent : nullptr;
         parent != nullptr; parent = parent->parent) {
        bool allChecked = true;
        bool anyChecked = false;
        int enabled = 0;
        for (TreeNode* child : parent->children) {
            if (child->disabled) {
                continue;
            }
            ++enabled;
            anyChecked = anyChecked || child->checked || child->indeterminate;
            if (!child->checked || child->indeterminate) {
                allChecked = false;
            }
        }
        if (enabled == 0) {
            continue;
        }
        parent->indeterminate = anyChecked && !allChecked;
        parent->checked = allChecked;
    }
}

void ETreeUI::AppendChild(TreeNode* parent)
{
    if (parent == nullptr) {
        return;
    }
    auto* node = new TreeNode;
    ++m_appended;
    node->id = L"new-" + std::to_wstring(m_appended);
    node->label = L"新增节点 " + std::to_wstring(m_appended);
    node->parent = parent;
    node->depth = parent->depth + 1;
    parent->children.push_back(node);
    parent->expanded = true;
    parent->loaded = true;
    UpdateHeight();
    Invalidate();
}

void ETreeUI::FinishLazyLoad()
{
    if (m_pManager != nullptr) {
        m_pManager->KillTimer(this, 1);
    }
    m_loadingActive = false;

    std::vector<TreeNode*> stack;
    for (TreeNode* child : m_root->children) {
        stack.push_back(child);
    }
    while (!stack.empty()) {
        TreeNode* node = stack.back();
        stack.pop_back();
        if (node->loading) {
            node->loading = false;
            node->loaded = true;
            node->expanded = true;
        }
        for (TreeNode* child : node->children) {
            stack.push_back(child);
        }
    }
    UpdateHeight();
    Invalidate();
}

bool ETreeUI::IsAncestor(TreeNode* ancestor, TreeNode* node) const
{
    for (TreeNode* parent = node != nullptr ? node->parent : nullptr;
         parent != nullptr; parent = parent->parent) {
        if (parent == ancestor) {
            return true;
        }
    }
    return false;
}

ETreeUI::TreeNode* ETreeUI::Detach(TreeNode* node)
{
    if (node == nullptr) {
        return nullptr;
    }
    std::vector<TreeNode*>& siblings = node->parent != nullptr ? node->parent->children : m_root->children;
    auto it = std::find(siblings.begin(), siblings.end(), node);
    if (it == siblings.end()) {
        return nullptr;
    }
    siblings.erase(it);
    return node;
}

void ETreeUI::UpdateDepths(TreeNode* node, int depth)
{
    if (node == nullptr) {
        return;
    }
    node->depth = depth;
    for (TreeNode* child : node->children) {
        UpdateDepths(child, depth + 1);
    }
}

void ETreeUI::MoveNode(TreeNode* node, TreeNode* target)
{
    if (node == nullptr || target == nullptr || node == target || IsAncestor(node, target)) {
        return;
    }
    if (Detach(node) == nullptr) {
        return;
    }
    node->parent = target;
    target->children.push_back(node);
    target->expanded = true;
    target->loaded = true;
    UpdateDepths(node, target->depth + 1);
    UpdateHeight();
    Invalidate();
}

void ETreeUI::DoEvent(TEventUI& event)
{
    if (!IsEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
        CControlUI::DoEvent(event);
        return;
    }

    if (event.Type == UIEVENT_SETCURSOR) {
        ::SetCursor(::LoadCursor(nullptr, m_dragging ? IDC_SIZEALL : IDC_ARROW));
        return;
    }

    if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hover != nullptr) {
            m_hover = nullptr;
            Invalidate();
        }
        return;
    }

    if (event.Type == UIEVENT_MOUSEMOVE) {
        if (m_drag != nullptr && m_draggable) {
            if (!m_dragMoved) {
                const int dx = event.ptMouse.x - m_dragStart.x;
                const int dy = event.ptMouse.y - m_dragStart.y;
                const int threshold = UiScale::Px(4);
                if (dx * dx + dy * dy >= threshold * threshold) {
                    m_dragMoved = true;
                    m_dragging = true;
                }
            }
            if (m_dragMoved) {
                RebuildRows();
                const int index = FindRowAt(event.ptMouse);
                TreeNode* target = index >= 0 ? m_rows[index].node : nullptr;
                if (target == m_drag || IsAncestor(m_drag, target)) {
                    target = nullptr;
                }
                if (target != m_drop) {
                    m_drop = target;
                    Invalidate();
                }
                return;
            }
        }

        RebuildRows();
        const int index = FindRowAt(event.ptMouse);
        TreeNode* hover = index >= 0 ? m_rows[index].node : nullptr;
        if (hover != m_hover) {
            m_hover = hover;
            Invalidate();
        }
        return;
    }

    if (event.Type == UIEVENT_BUTTONDOWN) {
        RebuildRows();
        const int index = FindRowAt(event.ptMouse);
        if (index < 0) {
            CControlUI::DoEvent(event);
            return;
        }
        const TreeRow& row = m_rows[index];
        if (row.loading) {
            return;
        }
        TreeNode* node = row.node;
        const RECT rc = RowRect(index);
        const int cy = (rc.top + rc.bottom) / 2;
        const int contentLeft = m_rcItem.left + node->depth * UiScale::Px(18);

        if (!node->isLeaf && !node->children.empty()) {
            RECT arrow = { contentLeft, rc.top, contentLeft + UiScale::Px(18), rc.bottom };
            if (::PtInRect(&arrow, event.ptMouse)) {
                ToggleExpand(node);
                return;
            }
        }
        if (m_showCheckbox) {
            RECT box = { contentLeft + UiScale::Px(22), cy - UiScale::Px(7),
                contentLeft + UiScale::Px(36), cy + UiScale::Px(7) };
            if (::PtInRect(&box, event.ptMouse)) {
                if (!node->disabled) {
                    const bool target = !(node->checked || node->indeterminate);
                    SetSubtreeChecked(node, target);
                    UpdateAncestors(node);
                    if (m_pManager != nullptr) {
                        m_pManager->SendNotify(this, _T("treecheck"));
                    }
                    Invalidate();
                }
                return;
            }
        }
        if (m_showPlus) {
            RECT box = { m_rcItem.right - UiScale::Px(30), cy - UiScale::Px(10),
                m_rcItem.right - UiScale::Px(10), cy + UiScale::Px(10) };
            if (::PtInRect(&box, event.ptMouse)) {
                if (!node->disabled) {
                    AppendChild(node);
                }
                return;
            }
        }

        if (!node->disabled) {
            m_selected = node;
            Invalidate();
        }
        if (m_draggable) {
            m_drag = node;
            m_dragStart = event.ptMouse;
            m_dragMoved = false;
            m_dragging = false;
        }
        return;
    }

    if (event.Type == UIEVENT_BUTTONUP) {
        if (m_draggable && m_drag != nullptr) {
            if (m_dragMoved && m_drop != nullptr && m_drop != m_drag) {
                MoveNode(m_drag, m_drop);
            }
            m_drag = nullptr;
            m_drop = nullptr;
            m_dragMoved = false;
            m_dragging = false;
            Invalidate();
            return;
        }
        CControlUI::DoEvent(event);
        return;
    }

    if (event.Type == UIEVENT_TIMER && event.wParam == 1) {
        FinishLazyLoad();
        return;
    }

    CControlUI::DoEvent(event);
}

//////////////////////////////////////////////////////////////////////////
// EControlFactory

CControlUI* EControlFactory::CreateControl(LPCTSTR pstrClass)
{
    if (_tcsicmp(pstrClass, _T("EButton")) == 0) {
        return new EButtonUI;
    }
    if (_tcsicmp(pstrClass, _T("ELink")) == 0) {
        return new ELinkUI;
    }
    if (_tcsicmp(pstrClass, _T("EInputNumber")) == 0) {
        return new EInputNumberUI;
    }
    if (_tcsicmp(pstrClass, _T("ECascader")) == 0) {
        return new ECascaderUI;
    }
    if (_tcsicmp(pstrClass, _T("ESwitch")) == 0) {
        return new ESwitchUI;
    }
    if (_tcsicmp(pstrClass, _T("ERate")) == 0) {
        return new ERateUI;
    }
    if (_tcsicmp(pstrClass, _T("ENavItem")) == 0) {
        return new ENavItemUI;
    }
    if (_tcsicmp(pstrClass, _T("ETabButton")) == 0) {
        return new ETabButtonUI;
    }
    if (_tcsicmp(pstrClass, _T("ERadio")) == 0) {
        return new EOptionUI;
    }
    if (_tcsicmp(pstrClass, _T("ECheckBox")) == 0) {
        return new ECheckBoxUI;
    }
    if (_tcsicmp(pstrClass, _T("EProgress")) == 0) {
        return new EProgressUI;
    }
    if (_tcsicmp(pstrClass, _T("ESlider")) == 0) {
        return new ESliderUI;
    }
    if (_tcsicmp(pstrClass, _T("EPagination")) == 0) {
        return new EPaginationUI;
    }
    if (_tcsicmp(pstrClass, _T("EBadge")) == 0) {
        return new EBadgeUI;
    }
    if (_tcsicmp(pstrClass, _T("EAvatar")) == 0) {
        return new EAvatarUI;
    }
    if (_tcsicmp(pstrClass, _T("ELoading")) == 0) {
        return new ELoadingUI;
    }
    if (_tcsicmp(pstrClass, _T("EBreadcrumb")) == 0) {
        return new EBreadcrumbUI;
    }
    if (_tcsicmp(pstrClass, _T("ESteps")) == 0) {
        return new EStepsUI;
    }
    if (_tcsicmp(pstrClass, _T("EPopover")) == 0) {
        return new EPopoverUI;
    }
    if (_tcsicmp(pstrClass, _T("ECollapse")) == 0) {
        return new ECollapseUI;
    }
    if (_tcsicmp(pstrClass, _T("EInfiniteList")) == 0) {
        return new EInfiniteListUI;
    }
    if (_tcsicmp(pstrClass, _T("ESelect")) == 0) {
        return new ESelectUI;
    }
    if (_tcsicmp(pstrClass, _T("ESelectPopup")) == 0) {
        return new ESelectPopupUI;
    }
    if (_tcsicmp(pstrClass, _T("EDateTime")) == 0) {
        return new EDateTimeUI;
    }
    if (_tcsicmp(pstrClass, _T("ETree")) == 0) {
        return new ETreeUI;
    }
    return nullptr;
}

void ScaleTree(CControlUI* control)
{
    if (control == nullptr) {
        return;
    }
    if (auto* badge = dynamic_cast<EBadgeUI*>(control)) {
        badge->ScaleForDpi();
    }
    else {
        const int fixedWidth = control->GetFixedWidth();
        if (fixedWidth > 0) {
            control->SetFixedWidth(UiScale::Px(fixedWidth));
        }
        const int fixedHeight = control->GetFixedHeight();
        if (fixedHeight > 0) {
            control->SetFixedHeight(UiScale::Px(fixedHeight));
        }
    }

    RECT padding = control->GetPadding();
    if (padding.left != 0 || padding.top != 0 || padding.right != 0 || padding.bottom != 0) {
        padding.left = UiScale::Px(padding.left);
        padding.top = UiScale::Px(padding.top);
        padding.right = UiScale::Px(padding.right);
        padding.bottom = UiScale::Px(padding.bottom);
        control->SetPadding(padding);
    }

    RECT borderSize = control->GetBorderSize();
    if (borderSize.left != 0 || borderSize.top != 0 || borderSize.right != 0 || borderSize.bottom != 0) {
        borderSize.left = UiScale::Px(borderSize.left);
        borderSize.top = UiScale::Px(borderSize.top);
        borderSize.right = UiScale::Px(borderSize.right);
        borderSize.bottom = UiScale::Px(borderSize.bottom);
        control->SetBorderSize(borderSize);
    }

    SIZE borderRound = control->GetBorderRound();
    if (borderRound.cx > 0 || borderRound.cy > 0) {
        borderRound.cx = UiScale::Px(borderRound.cx);
        borderRound.cy = UiScale::Px(borderRound.cy);
        control->SetBorderRound(borderRound);
    }

    if (auto* label = dynamic_cast<CLabelUI*>(control)) {
        RECT textPadding = label->GetTextPadding();
        if (textPadding.left != 0 || textPadding.top != 0 || textPadding.right != 0 || textPadding.bottom != 0) {
            textPadding.left = UiScale::Px(textPadding.left);
            textPadding.top = UiScale::Px(textPadding.top);
            textPadding.right = UiScale::Px(textPadding.right);
            textPadding.bottom = UiScale::Px(textPadding.bottom);
            label->SetTextPadding(textPadding);
        }
    }

    if (auto* tile = dynamic_cast<CTileLayoutUI*>(control)) {
        SIZE itemSize = tile->GetItemSize();
        if (itemSize.cx > 0 || itemSize.cy > 0) {
            itemSize.cx = UiScale::Px(itemSize.cx);
            itemSize.cy = UiScale::Px(itemSize.cy);
            tile->SetItemSize(itemSize);
        }
        const int childVPadding = tile->GetChildVPadding();
        if (childVPadding != 0) {
            tile->SetChildVPadding(UiScale::Px(childVPadding));
        }
    }

    if (auto* container = dynamic_cast<CContainerUI*>(control)) {
        RECT inset = container->GetInset();
        if (inset.left != 0 || inset.top != 0 || inset.right != 0 || inset.bottom != 0) {
            inset.left = UiScale::Px(inset.left);
            inset.top = UiScale::Px(inset.top);
            inset.right = UiScale::Px(inset.right);
            inset.bottom = UiScale::Px(inset.bottom);
            container->SetInset(inset);
        }
        const int childPadding = container->GetChildPadding();
        if (childPadding != 0) {
            container->SetChildPadding(UiScale::Px(childPadding));
        }
        for (int index = 0; index < container->GetCount(); ++index) {
            ScaleTree(container->GetItemAt(index));
        }
    }
}

CLabelUI* MakeLabel(const std::wstring& text, int fontId, DWORD color)
{
    CLabelUI* label = new CLabelUI;
    label->SetText(text.c_str());
    label->SetFont(fontId);
    label->SetTextColor(color);
    label->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    return label;
}

CLabelUI* MakeLabelW(const std::wstring& text, int fontId, DWORD color, int width)
{
    CLabelUI* label = MakeLabel(text, fontId, color);
    label->SetFixedWidth(UiScale::Px(width));
    return label;
}

CLabelUI* MakeLabelH(const std::wstring& text, int fontId, DWORD color, int height)
{
    CLabelUI* label = MakeLabel(text, fontId, color);
    label->SetFixedHeight(UiScale::Px(height));
    return label;
}

CLabelUI* MakeLabelWH(const std::wstring& text, int fontId, DWORD color, int width, int height)
{
    CLabelUI* label = MakeLabel(text, fontId, color);
    label->SetFixedWidth(UiScale::Px(width));
    label->SetFixedHeight(UiScale::Px(height));
    return label;
}

//////////////////////////////////////////////////////////////////////////
// EThemePreviewUI

namespace {

constexpr DWORD TP_PrimaryBg = 0xFFECF5FF;
constexpr DWORD TP_PrimaryBorderTag = 0xFFD9ECFF;
constexpr DWORD TP_PrimaryBorderBtn = 0xFFB3D8FF;
constexpr DWORD TP_SuccessBg = 0xFFF0F9EB;
constexpr DWORD TP_SuccessBorder = 0xFFC2E7B0;
constexpr DWORD TP_InfoBg = 0xFFF4F4F5;
constexpr DWORD TP_InfoBorder = 0xFFD3D4D6;
constexpr DWORD TP_WarningBg = 0xFFFDF6EC;
constexpr DWORD TP_WarningBorder = 0xFFF5DAB1;
constexpr DWORD TP_DangerBg = 0xFFFEF0F0;
constexpr DWORD TP_DangerBorder = 0xFFFBC4C4;
constexpr DWORD TP_Paragraph = 0xFF5E6D82;
constexpr DWORD TP_Desc = 0xFF8391A5;
constexpr DWORD TP_CarouselA = 0xFF99A9BF;
constexpr DWORD TP_CarouselB = 0xFFD3DCE6;
constexpr DWORD TP_Gold = 0xFFFF9900;
constexpr DWORD TP_RateVoid = 0xFFC6D1DE;
constexpr DWORD TP_WhiteSoft = 0x33FFFFFF;

enum : int {
    K_NONE = 0,
    K_BUTTON,
    K_RADIO,
    K_CHECK,
    K_SWITCH,
    K_SLIDER,
    K_INPUT,
    K_INPUT_MINUS,
    K_INPUT_PLUS,
    K_SELECT_FIELD,
    K_SELECT_OPTION,
    K_CASCADER_FIELD,
    K_CASCADER_PROVINCE,
    K_CASCADER_CITY,
    K_DATE_FIELD,
    K_CALENDAR_PREV,
    K_CALENDAR_NEXT,
    K_CALENDAR_DAY,
    K_RATE,
    K_TAB,
    K_MENU_H,
    K_MENU_V,
    K_PAGE,
    K_COLLAPSE,
    K_CAROUSEL_IND,
    K_TAG_CLOSE,
    K_TREE,
    K_TREE_ARROW,
};

struct TreePreviewNode
{
    const wchar_t* label;
    int parent;
    int depth;
};

const TreePreviewNode kTreePreviewNodes[] = {
    { L"Level one 1", -1, 0 },
    { L"Level two 1-1", 0, 1 },
    { L"Level three 1-1-1", 1, 2 },
    { L"Level one 2", -1, 0 },
    { L"Level two 2-1", 3, 1 },
    { L"Level three 2-1-1", 4, 2 },
    { L"Level two 2-2", 3, 1 },
    { L"Level three 2-2-1", 6, 2 },
    { L"Level one 3", -1, 0 },
    { L"Level two 3-1", 8, 1 },
    { L"Level three 3-1-1", 9, 2 },
    { L"Level two 3-2", 8, 1 },
    { L"Level three 3-2-1", 11, 2 },
};

constexpr int kTreePreviewCount = static_cast<int>(sizeof(kTreePreviewNodes) / sizeof(kTreePreviewNodes[0]));

bool TreePreviewHasChildren(int node)
{
    for (int i = 0; i < kTreePreviewCount; ++i) {
        if (kTreePreviewNodes[i].parent == node) {
            return true;
        }
    }
    return false;
}

bool TreePreviewVisible(int node, int openMask)
{
    for (int p = kTreePreviewNodes[node].parent; p >= 0; p = kTreePreviewNodes[p].parent) {
        if ((openMask & (1 << p)) == 0) {
            return false;
        }
    }
    return true;
}

int TreePreviewVisibleCount(int openMask)
{
    int count = 0;
    for (int i = 0; i < kTreePreviewCount; ++i) {
        if (TreePreviewVisible(i, openMask)) {
            ++count;
        }
    }
    return count;
}

int DaysInMonth(int year, int month)
{
    static const int days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
        return 29;
    }
    return days[(std::max)(1, (std::min)(12, month)) - 1];
}

int FirstWeekday(int year, int month)
{
    SYSTEMTIME st = {};
    st.wYear = static_cast<WORD>(year);
    st.wMonth = static_cast<WORD>(month);
    st.wDay = 1;
    FILETIME ft = {};
    SYSTEMTIME out = {};
    if (!::SystemTimeToFileTime(&st, &ft) || !::FileTimeToSystemTime(&ft, &out)) {
        return 0;
    }
    return out.wDayOfWeek;
}

std::wstring FormatDateText(int year, int month, int day)
{
    wchar_t buffer[32] = {};
    swprintf_s(buffer, L"%04d-%02d-%02d", year, month, day);
    return buffer;
}

void FillSolidRect(HDC hDC, const RECT& rc, DWORD color)
{
    HBRUSH brush = ::CreateSolidBrush(GdiColor(color));
    ::FillRect(hDC, &rc, brush);
    ::DeleteObject(brush);
}

void FillAlphaRect(HDC hDC, const RECT& rc, DWORD color)
{
    EnsureGdiplus();
    Gdiplus::Graphics graphics(hDC);
    Gdiplus::SolidBrush brush(GpColor(color));
    graphics.FillRectangle(&brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

void FillRoundRectBorder(HDC hDC, const RECT& rc, int radius, DWORD fill, DWORD border, int borderWidth)
{
    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        return;
    }
    EnsureGdiplus();
    Gdiplus::GraphicsPath path;
    MakeRoundRectPath(path, rc, (std::max)(2, radius * 2));
    Gdiplus::Graphics graphics(hDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    if (((fill >> 24) & 0xFF) != 0) {
        Gdiplus::SolidBrush brush(GpColor(fill));
        graphics.FillPath(&brush, &path);
    }
    if (((border >> 24) & 0xFF) != 0 && borderWidth > 0) {
        Gdiplus::Pen pen(GpColor(border), static_cast<Gdiplus::REAL>((std::max)(1, borderWidth)));
        graphics.DrawPath(&pen, &path);
    }
}

void FillTriangle(HDC hDC, int x1, int y1, int x2, int y2, int x3, int y3, DWORD color)
{
    Gdiplus::PointF points[3] = {
        Gdiplus::PointF(static_cast<Gdiplus::REAL>(x1), static_cast<Gdiplus::REAL>(y1)),
        Gdiplus::PointF(static_cast<Gdiplus::REAL>(x2), static_cast<Gdiplus::REAL>(y2)),
        Gdiplus::PointF(static_cast<Gdiplus::REAL>(x3), static_cast<Gdiplus::REAL>(y3)),
    };
    FillPolygonAa(hDC, points, 3, color);
}

} // namespace

EThemePreviewUI::EThemePreviewUI()
    : m_hoverKind(K_NONE)
    , m_hoverGroup(-1)
    , m_hoverIndex(-1)
    , m_pressKind(K_NONE)
    , m_pressGroup(-1)
    , m_pressIndex(-1)
    , m_pressed(false)
    , m_dragSlider(false)
    , m_radioBasic(0)
    , m_radioCity(1)
    , m_radioBordered(0)
    , m_checkBasic(true)
    , m_checkCityMask(1)
    , m_checkBordered(true)
    , m_switchYear(true)
    , m_switchMonth(true)
    , m_sliderValue(28)
    , m_inputFocused(false)
    , m_number(1)
    , m_selectOpen(false)
    , m_selectIndex(-1)
    , m_cascaderOpen(false)
    , m_cascaderProvince(-1)
    , m_cascaderCity(-1)
    , m_dateOpen(false)
    , m_calYear(2026)
    , m_calMonth(11)
    , m_rateValue(0)
    , m_rateHoverRow(-1)
    , m_rateHoverValue(0)
    , m_tabTop(0)
    , m_tabCard(0)
    , m_menuH(0)
    , m_menuV(1)
    , m_page(1)
    , m_collapseMask(1)
    , m_carousel(0)
    , m_extraHeight(0)
    , m_pageExtra(0)
    , m_treeOpenMask(0)
    , m_treeSelNode(-1)
    , m_treeExtra(0)
    , m_menuExtra(0)
    , m_menuHOpen(false)
    , m_menuHSubOpen(false)
    , m_menuHPopup(-1)
    , m_menuHSub(-1)
    , m_menuVOpenMask(0)
{
    for (bool& visible : m_tagVisible) {
        visible = true;
    }
    SetKeyboardEnabled(true);
    SetFixedHeight(PreferredHeight());
}

LPCTSTR EThemePreviewUI::GetClass() const
{
    return _T("EThemePreview");
}

LPVOID EThemePreviewUI::GetInterface(LPCTSTR pstrName)
{
    if (_tcsicmp(pstrName, _T("EThemePreview")) == 0) {
        return static_cast<EThemePreviewUI*>(this);
    }
    return CControlUI::GetInterface(pstrName);
}

SIZE EThemePreviewUI::EstimateSize(SIZE szAvailable)
{
    const int height = GetFixedHeight() > 0 ? GetFixedHeight() : PreferredHeight();
    SIZE size = { szAvailable.cx > 0 ? szAvailable.cx : UiScale::Px(845), height };
    return size;
}

int EThemePreviewUI::PreferredHeight()
{
    return UiScale::Px(7524);
}

void EThemePreviewUI::AddHotspot(const RECT& rc, int kind, int group, int index)
{
    Hotspot hot = { rc, kind, group, index };
    m_hotspots.push_back(hot);
}

const EThemePreviewUI::Hotspot* EThemePreviewUI::HitTestPoint(const POINT& pt) const
{
    for (auto it = m_hotspots.rbegin(); it != m_hotspots.rend(); ++it) {
        if (::PtInRect(&it->rc, pt)) {
            return &(*it);
        }
    }
    return nullptr;
}

bool EThemePreviewUI::IsHovered(int kind, int group, int index) const
{
    return m_hoverKind == kind && m_hoverGroup == group && m_hoverIndex == index;
}

bool EThemePreviewUI::IsPressed(int kind, int group, int index) const
{
    return m_pressed && m_pressKind == kind && m_pressGroup == group && m_pressIndex == index;
}

void EThemePreviewUI::UpdateHover(const POINT& pt)
{
    const Hotspot* hot = HitTestPoint(pt);
    const int kind = hot != nullptr ? hot->kind : K_NONE;
    const int group = hot != nullptr ? hot->group : -1;
    const int index = hot != nullptr ? hot->index : -1;
    const int rateRow = (hot != nullptr && hot->kind == K_RATE) ? hot->group : -1;
    const int rateValue = rateRow >= 0 ? index + 1 : 0;
    if (kind != m_hoverKind || group != m_hoverGroup || index != m_hoverIndex
        || rateRow != m_rateHoverRow || rateValue != m_rateHoverValue) {
        m_hoverKind = kind;
        m_hoverGroup = group;
        m_hoverIndex = index;
        m_rateHoverRow = rateRow;
        m_rateHoverValue = rateValue;
        Invalidate();
    }
}

void EThemePreviewUI::ClosePopups()
{
    if (m_selectOpen || m_cascaderOpen || m_dateOpen || m_menuHOpen) {
        m_selectOpen = false;
        m_cascaderOpen = false;
        m_dateOpen = false;
        m_menuHOpen = false;
        m_menuHSubOpen = false;
        Invalidate();
    }
}

void EThemePreviewUI::SyncHeight()
{
    const int collapseExtra = ((m_collapseMask & 1) != 0 ? 0 : -51)
        + ((m_collapseMask & 2) != 0 ? 51 : 0);
    const int total = collapseExtra + m_treeExtra + m_menuExtra;
    const bool shiftChanged = collapseExtra != m_extraHeight;
    const bool heightChanged = total != m_pageExtra;
    m_extraHeight = collapseExtra;
    m_pageExtra = total;
    if (heightChanged) {
        SetFixedHeight(UiScale::Px(7524 + total));
        NeedParentUpdate();
    }
    if (shiftChanged || heightChanged) {
        Invalidate();
    }
}

void EThemePreviewUI::DoEvent(TEventUI& event)
{
    if (event.Type == UIEVENT_SETCURSOR) {
        const Hotspot* hot = HitTestPoint(event.ptMouse);
        const bool disabled = hot != nullptr
            && ((hot->kind == K_MENU_H && hot->group == 1 && hot->index == 2)
                || (hot->kind == K_MENU_V && hot->index == 2));
        if (hot != nullptr && !disabled) {
            ::SetCursor(::LoadCursor(nullptr, IDC_HAND));
            return;
        }
    }
    if (event.Type == UIEVENT_MOUSEMOVE) {
        if (m_dragSlider) {
            const int contentWidth = UiScale::Px(845);
            const int available = m_rcItem.right - m_rcItem.left;
            const int left = m_rcItem.left + (std::max)(0, (available - contentWidth) / 2);
            int value = (event.ptMouse.x - left) * 100 / UiScale::Px(380);
            value = (std::max)(0, (std::min)(100, value));
            if (value != m_sliderValue) {
                m_sliderValue = value;
                Invalidate();
            }
        }
        else {
            UpdateHover(event.ptMouse);
        }
        return;
    }
    if (event.Type == UIEVENT_MOUSELEAVE) {
        if (m_hoverKind != K_NONE || m_rateHoverRow != -1) {
            m_hoverKind = K_NONE;
            m_hoverGroup = -1;
            m_hoverIndex = -1;
            m_rateHoverRow = -1;
            m_rateHoverValue = 0;
            Invalidate();
        }
        return;
    }
    if (event.Type == UIEVENT_BUTTONDOWN) {
        const Hotspot* hot = HitTestPoint(event.ptMouse);
        if (hot != nullptr) {
            m_pressed = true;
            m_pressKind = hot->kind;
            m_pressGroup = hot->group;
            m_pressIndex = hot->index;
            if (hot->kind == K_INPUT) {
                m_inputFocused = true;
            }
            if (hot->kind == K_SLIDER) {
                m_dragSlider = true;
                const int contentWidth = UiScale::Px(845);
                const int available = m_rcItem.right - m_rcItem.left;
                const int left = m_rcItem.left + (std::max)(0, (available - contentWidth) / 2);
                int value = (event.ptMouse.x - left) * 100 / UiScale::Px(380);
                m_sliderValue = (std::max)(0, (std::min)(100, value));
            }
            Invalidate();
        }
        return;
    }
    if (event.Type == UIEVENT_BUTTONUP) {
        const Hotspot* hot = HitTestPoint(event.ptMouse);
        const bool same = hot != nullptr && m_pressed && hot->kind == m_pressKind
            && hot->group == m_pressGroup && hot->index == m_pressIndex;
        m_dragSlider = false;
        if (same) {
            HandleClick(*hot);
        }
        else {
            ClosePopups();
            if (m_inputFocused) {
                m_inputFocused = false;
            }
        }
        m_pressed = false;
        m_pressKind = K_NONE;
        m_pressGroup = -1;
        m_pressIndex = -1;
        Invalidate();
        return;
    }
    if (event.Type == UIEVENT_CHAR) {
        if (m_inputFocused) {
            const wchar_t ch = static_cast<wchar_t>(event.chKey);
            if (ch == 8) {
                if (!m_inputText.empty()) {
                    m_inputText.pop_back();
                }
            }
            else if (ch == 27) {
                m_inputFocused = false;
            }
            else if (ch >= 32) {
                m_inputText.push_back(ch);
            }
            Invalidate();
            return;
        }
    }
    CControlUI::DoEvent(event);
}

void EThemePreviewUI::HandleClick(const Hotspot& hot)
{
    if (m_inputFocused && hot.kind != K_INPUT) {
        m_inputFocused = false;
    }
    if (hot.kind == K_SELECT_FIELD) {
        const bool open = m_selectOpen;
        ClosePopups();
        m_selectOpen = !open;
        Invalidate();
        return;
    }
    if (hot.kind == K_CASCADER_FIELD) {
        const bool open = m_cascaderOpen;
        ClosePopups();
        m_cascaderOpen = !open;
        Invalidate();
        return;
    }
    if (hot.kind == K_DATE_FIELD) {
        const bool open = m_dateOpen;
        ClosePopups();
        m_dateOpen = !open;
        Invalidate();
        return;
    }
    if (hot.kind == K_SELECT_OPTION) {
        m_selectIndex = hot.index;
        ClosePopups();
        Invalidate();
        return;
    }
    if (hot.kind == K_CASCADER_PROVINCE) {
        m_cascaderProvince = hot.index;
        m_cascaderCity = -1;
        Invalidate();
        return;
    }
    if (hot.kind == K_CASCADER_CITY) {
        m_cascaderCity = hot.index;
        ClosePopups();
        Invalidate();
        return;
    }
    if (hot.kind == K_CALENDAR_PREV || hot.kind == K_CALENDAR_NEXT) {
        if (hot.kind == K_CALENDAR_PREV) {
            if (--m_calMonth < 1) {
                m_calMonth = 12;
                --m_calYear;
            }
        }
        else if (++m_calMonth > 12) {
            m_calMonth = 1;
            ++m_calYear;
        }
        Invalidate();
        return;
    }
    if (hot.kind == K_CALENDAR_DAY) {
        m_dateText = FormatDateText(m_calYear, m_calMonth, hot.index);
        ClosePopups();
        Invalidate();
        return;
    }
    if (hot.kind == K_MENU_H) {
        if (hot.group == 1) {
            if (hot.index == 1) {
                const bool open = m_menuHOpen;
                ClosePopups();
                m_menuHOpen = !open;
                if (!m_menuHOpen) {
                    m_menuHSubOpen = false;
                }
            }
            else if (hot.index != 2) {
                ClosePopups();
                m_menuH = hot.index;
            }
        }
        else if (hot.group == 2) {
            if (hot.index == 3) {
                m_menuHSubOpen = !m_menuHSubOpen;
            }
            else {
                m_menuHPopup = hot.index;
            }
        }
        else {
            m_menuHSub = hot.index;
        }
        Invalidate();
        return;
    }
    ClosePopups();
    switch (hot.kind) {
    case K_RADIO:
        if (hot.group == 1) {
            m_radioBasic = hot.index;
        }
        else if (hot.group == 2) {
            m_radioBordered = hot.index;
        }
        else {
            m_radioCity = hot.index;
        }
        break;
    case K_CHECK:
        if (hot.group == 1) {
            m_checkBasic = !m_checkBasic;
        }
        else if (hot.group == 2) {
            m_checkCityMask ^= (1 << hot.index);
        }
        else {
            m_checkBordered = !m_checkBordered;
        }
        break;
    case K_SWITCH:
        if (hot.group == 1) {
            m_switchYear = !m_switchYear;
        }
        else {
            m_switchMonth = !m_switchMonth;
        }
        break;
    case K_INPUT_MINUS:
        if (m_number > 1) {
            --m_number;
        }
        break;
    case K_INPUT_PLUS:
        ++m_number;
        break;
    case K_RATE:
        m_rateValue = hot.index + 1;
        break;
    case K_TAB:
        if (hot.group == 1) {
            m_tabTop = hot.index;
        }
        else {
            m_tabCard = hot.index;
        }
        break;
    case K_MENU_V:
        if (hot.index == 0) {
            m_menuVOpenMask ^= 1;
            if ((m_menuVOpenMask & 1) == 0) {
                m_menuVOpenMask &= ~2;
            }
            m_menuExtra = ((m_menuVOpenMask & 1) != 0 ? 292 : 0)
                + ((m_menuVOpenMask & 2) != 0 ? 56 : 0);
            SyncHeight();
        }
        else if (hot.index == 30) {
            if ((m_menuVOpenMask & 1) != 0) {
                m_menuVOpenMask ^= 2;
                m_menuExtra = 292 + (((m_menuVOpenMask & 2) != 0) ? 56 : 0);
                SyncHeight();
            }
        }
        else if (hot.index != 2) {
            m_menuV = hot.index;
        }
        break;
    case K_PAGE:
        if (hot.index == -1) {
            if (m_page == 100) {
                m_page = 6;
            }
            else if (m_page > 1) {
                --m_page;
            }
        }
        else if (hot.index == -2) {
            if (m_page < 6) {
                ++m_page;
            }
            else if (m_page == 6) {
                m_page = 100;
            }
        }
        else {
            m_page = hot.index;
        }
        break;
    case K_COLLAPSE:
        m_collapseMask ^= (1 << hot.index);
        SyncHeight();
        break;
    case K_CAROUSEL_IND:
        m_carousel = hot.index;
        break;
    case K_TREE_ARROW:
        m_treeOpenMask ^= (1 << hot.group);
        m_treeExtra = (TreePreviewVisibleCount(m_treeOpenMask) - 3) * 26;
        SyncHeight();
        break;
    case K_TREE:
        m_treeSelNode = hot.group;
        break;
    case K_TAG_CLOSE:
        if (hot.index >= 0 && hot.index < 5) {
            m_tagVisible[hot.index] = false;
        }
        break;
    default:
        break;
    }
    Invalidate();
}

bool EThemePreviewUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
{
    (void)pStopControl;

    auto P = [](int value) { return UiScale::Px(value); };
    const int contentWidth = P(845);
    const int available = m_rcItem.right - m_rcItem.left;
    const int left = m_rcItem.left + (std::max)(0, (available - contentWidth) / 2);
    const int top = m_rcItem.top;

    auto X = [&](int cssX) { return left + P(cssX); };
    auto Y = [&](int cssY) {
        int y = top + P(70 + cssY - 190);
        if (cssY >= 3860) {
            y += P(m_treeExtra);
        }
        if (cssY >= 5810) {
            y += P(m_menuExtra);
        }
        return y;
    };
    auto RC = [&](int x, int y, int w, int h) {
        RECT rc = { X(x), Y(y), X(x + w), Y(y + h) };
        return rc;
    };
    auto textW = [&](const wchar_t* text, int fontId) {
        const SIZE size = CRenderEngine::GetTextSize(hDC, m_pManager, text, fontId, DT_SINGLELINE);
        return static_cast<int>(size.cx);
    };
    auto txt = [&](RECT rc, const wchar_t* text, int fontId, DWORD color, UINT flags) {
        CRenderEngine::DrawText(hDC, m_pManager, rc, text, color, fontId, flags);
    };
    auto label = [&](int x, int y, int w, int h, const wchar_t* text, int fontId, DWORD color,
                     UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
        RECT rc = RC(x, y, w, h);
        txt(rc, text, fontId, color, flags);
    };
    auto heading = [&](const wchar_t* text, int cssY) {
        label(0, cssY, 600, 23, text, font::Th20, theme::TextSecondary);
    };
    auto inView = [&](int cssTop, int cssBottom) {
        return Y(cssBottom) >= rcPaint.top && Y(cssTop) <= rcPaint.bottom;
    };
    auto shadow = [&](const RECT& rc, int radius) {
        for (int i = 3; i >= 1; --i) {
            RECT s = { rc.left - P(i), rc.top - P(1), rc.right + P(i), rc.bottom + P(i + 2) };
            const DWORD alpha = static_cast<DWORD>(0x0C + (4 - i) * 0x06) << 24;
            FillRoundRectBorder(hDC, s, radius + P(i), alpha, 0, 0);
        }
    };

    const DWORD fills[6] = { theme::White, theme::Primary, theme::Success, theme::Info,
        theme::Warning, theme::Danger };
    const DWORD lightFills[6] = { theme::White, TP_PrimaryBg, TP_SuccessBg, TP_InfoBg,
        TP_WarningBg, TP_DangerBg };
    const DWORD lightBorders[6] = { theme::BorderBase, TP_PrimaryBorderBtn, TP_SuccessBorder,
        TP_InfoBorder, TP_WarningBorder, TP_DangerBorder };
    const DWORD strongText[6] = { theme::TextRegular, theme::Primary, theme::Success,
        theme::Info, theme::Warning, theme::Danger };

    const wchar_t* selectOptions[5] = { L"Option1", L"Option2", L"Option3", L"Option4", L"Option5" };
    const wchar_t* provinces[3] = { L"Zhejiang", L"Jiangsu", L"Shanghai" };
    const wchar_t* cascaderCities[3][3] = {
        { L"Hangzhou", L"Ningbo", L"Wenzhou" },
        { L"Nanjing", L"Suzhou", L"Wuxi" },
        { L"Putuo", L"Pudong", L"Xuhui" },
    };

    const DWORD hoverColors[6] = { theme::Primary, 0xFF66B1FF, 0xFF85CE61, 0xFFA6A9AD,
        0xFFEBB563, 0xFFF78989 };
    const DWORD activeColors[6] = { 0xFF3A8EE6, 0xFF3A8EE6, 0xFF5DAF34, 0xFF82848A,
        0xFFCF9236, 0xFFDD6161 };

    auto drawBtn = [&](int x, int y, int w, int h, const wchar_t* caption, int variant,
                       bool plain, bool round, int fontId, int group, int index) {
        DWORD fill = fills[variant];
        DWORD border = fills[variant];
        DWORD color = theme::White;
        if (variant == 0) {
            fill = theme::White;
            border = theme::BorderBase;
            color = theme::TextRegular;
        }
        if (plain) {
            if (variant == 0) {
                fill = theme::White;
                border = theme::BorderBase;
                color = theme::TextRegular;
            }
            else {
                fill = lightFills[variant];
                border = lightBorders[variant];
                color = strongText[variant];
            }
        }
        const bool hovered = IsHovered(K_BUTTON, group, index);
        const bool pressed = IsPressed(K_BUTTON, group, index);
        if (variant == 0) {
            if (hovered) {
                fill = 0xFFECF5FF;
                border = 0xFFC6E2FF;
                color = theme::Primary;
            }
            if (pressed) {
                fill = 0xFFECF5FF;
                border = activeColors[0];
                color = activeColors[0];
            }
        }
        else if (plain) {
            if (hovered) {
                border = hoverColors[variant];
                color = hoverColors[variant];
            }
            if (pressed) {
                border = activeColors[variant];
                color = activeColors[variant];
            }
        }
        else {
            if (hovered) {
                fill = hoverColors[variant];
                border = hoverColors[variant];
            }
            if (pressed) {
                fill = activeColors[variant];
                border = activeColors[variant];
            }
        }
        RECT rc = RC(x, y, w, h);
        FillRoundRectBorder(hDC, rc, round ? h / 2 : P(4), fill, border, 1);
        if (caption != nullptr && caption[0] != L'\0') {
            txt(rc, caption, fontId, color, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        AddHotspot(rc, K_BUTTON, group, index);
    };
    auto buttonWidth = [&](const wchar_t* caption, int fontId) {
        return textW(caption, fontId) + P(40);
    };

    m_hotspots.clear();
    FillSolidRect(hDC, m_rcItem, theme::White);

    // Color
    if (inView(190, 630)) {
        heading(L"Color", 190);
        const wchar_t* names[5] = { L"Primary", L"Success", L"Warning", L"Danger", L"Info" };
        const wchar_t* values[5] = { L"#409EFF", L"#67C23A", L"#E6A23C", L"#F56C6C", L"#909399" };
        const DWORD colors[5] = { theme::Primary, theme::Success, theme::Warning, theme::Danger, theme::Info };
        for (int i = 0; i < 5; ++i) {
            RECT box = RC(i * 1278 / 8, 234, 148, 114);
            FillRoundRectBorder(hDC, box, P(4), colors[i], 0, 0);
            label(i * 1278 / 8 + 20, 254, 110, 20, names[i], font::Body14, theme::White);
            label(i * 1278 / 8 + 20, 270, 110, 24, values[i], font::Small,
                MixColor(colors[i], theme::White, 69));
            RECT strip = { box.left, box.top + P(74), box.right, box.bottom };
            FillAlphaRect(hDC, strip, TP_WhiteSoft);
        }

        const wchar_t* names2[12] = { L"Primary Text", L"Regular Text", L"Secondary Text", L"Placeholder",
            L"Border Base", L"Border Light", L"Border Lighter", L"Border Extralight",
            L"Background B", L"Background W", L"Background", L"" };
        const wchar_t* values2[12] = { L"#303133", L"#606266", L"#909399", L"#C0C4CC",
            L"#DCDFE6", L"#E4E7ED", L"#EBEEF5", L"#F2F6FC",
            L"#000000", L"#FFFFFF", L"Transparent", L"" };
        const DWORD colors2[12] = { 0xFF303133, 0xFF606266, 0xFF909399, 0xFFC0C4CC,
            0xFFDCDFE6, 0xFFE4E7ED, 0xFFEBEEF5, 0xFFF2F6FC,
            0xFF000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
        for (int row = 0; row < 3; ++row) {
            const int count = row == 2 ? 3 : 4;
            const int y = 358 + row * 94;
            for (int i = 0; i < count; ++i) {
                const int index = row * 4 + i;
                const int x = i * 1278 / 8;
                const bool lite = row == 1;
                const DWORD textColor = lite ? theme::TextPrimary : theme::White;
                RECT box = RC(x, y, 148, 74);
                if (index == 9) {
                    FillRoundRectBorder(hDC, box, P(4), theme::White, 0xFFEEEEEE, 1);
                }
                else if (index == 10) {
                    FillRoundRectBorder(hDC, box, P(4), theme::White, 0xFFFCC3C3, 1);
                    for (int d = -74; d < 148; d += 12) {
                        Gdiplus::PointF pts[2] = {
                            Gdiplus::PointF(static_cast<Gdiplus::REAL>(box.left + d),
                                static_cast<Gdiplus::REAL>(box.bottom)),
                            Gdiplus::PointF(static_cast<Gdiplus::REAL>(box.left + d + 74),
                                static_cast<Gdiplus::REAL>(box.top)) };
                        DrawLinesAa(hDC, pts, 2, 0xFFF6D5D5, 1.0f);
                    }
                }
                else {
                    FillRoundRectBorder(hDC, box, P(4), colors2[index], 0, 0);
                }
                label(x + 20, y + 15, 110, 20, names2[index], font::Body14, textColor);
                const DWORD valueColor = (index == 8) ? MixColor(0xFF000000, theme::White, 69)
                    : (lite ? theme::TextPrimary : MixColor(colors2[index], theme::White, 69));
                label(x + 20, y + 31, 110, 24, values2[index], font::Small, valueColor);
            }
        }
    }

    // Typography
    if (inView(670, 911)) {
        heading(L"Typography", 670);
        const int sizes[6] = { 20, 18, 16, 14, 13, 12 };
        const int fontIds[6] = { font::Th20, font::Th18, font::Th16, font::Body14, font::Body, font::Small };
        int lineY = 713;
        for (int i = 0; i < 6; ++i) {
            const int lineHeight = sizes[i] + 7;
            label(6, lineY, 190, lineHeight, sizes[i] == 20 ? L"Heading1" : sizes[i] == 18 ? L"Heading2"
                : sizes[i] == 16 ? L"Heading3" : sizes[i] == 14 ? L"Heading4" : sizes[i] == 13 ? L"Heading5"
                : L"Heading6", fontIds[i], theme::TextPrimary);
            lineY += lineHeight;
        }
        label(240, 713, 269, 21, L"Example body text", font::Th18, theme::TextPrimary);
        RECT paragraph = RC(240, 748, 269, 120);
        txt(paragraph, L"With MySpace becoming more popular every day, there is the constant need to be "
            L"different. There are millions of users. If MySpace layouts are chosen well, then you can "
            L"enhance your profile a great deal.", font::Body14, TP_Paragraph,
            DT_LEFT | DT_TOP | DT_WORDBREAK);
        label(559, 713, 259, 21, L"Example small text", font::Th18, theme::TextPrimary);
        RECT paragraph2 = RC(559, 748, 259, 110);
        txt(paragraph2, L"Computers have become ubiquitous in almost every facet of our lives. At work, "
            L"desk jockeys spend hours in front of their desktops, while delivery people scan bar codes "
            L"with handhelds and workers in the field stay in touch with the central office via their "
            L"notebooks. Computer hardware weaves itself through the fabric of our lives.",
            font::Body, TP_Paragraph, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }

    // Button
    if (inView(951, 1253)) {
        heading(L"Button", 951);
        const wchar_t* captions[6] = { L"Default", L"Primary", L"Success", L"Info", L"Warning", L"Danger" };
        int x = 0;
        for (int i = 0; i < 6; ++i) {
            const int w = buttonWidth(captions[i], font::Body14);
            drawBtn(x, 994, w, 40, captions[i], i, false, false, font::Body14, 10, i);
            x += w + P(10);
        }
        x = 0;
        for (int i = 0; i < 6; ++i) {
            const int w = buttonWidth(captions[i], font::Body14);
            drawBtn(x, 1049, w, 40, captions[i], i, true, false, font::Body14, 20, i);
            x += w + P(10);
        }
        x = 0;
        const wchar_t* roundCaptions[6] = { L"Round", L"Primary", L"Success", L"Info", L"Warning", L"Danger" };
        for (int i = 0; i < 6; ++i) {
            const int w = buttonWidth(roundCaptions[i], font::Body14);
            drawBtn(x, 1104, w, 40, roundCaptions[i], i, false, true, font::Body14, 30, i);
            x += w + P(10);
        }
        const wchar_t* icons[6] = { L"\uE721", L"\uE70F", L"\uE73E", L"\uE8BD", L"\uE734", L"\uE74D" };
        for (int i = 0; i < 6; ++i) {
            drawBtn(i * 50, 1159, 40, 40, icons[i], i, false, true, font::IconSmall, 40, i);
        }
        const wchar_t* sizeCaptions[4] = { L"Default", L"Medium", L"Small", L"Mini" };
        const int heights[4] = { 40, 36, 32, 28 };
        const int fontIds[4] = { font::Body14, font::Body14, font::Small, font::Small };
        const int padX[4] = { 40, 40, 30, 30 };
        x = 0;
        for (int i = 0; i < 4; ++i) {
            const int w = textW(sizeCaptions[i], fontIds[i]) + P(padX[i]);
            drawBtn(x, 1214, w, heights[i], sizeCaptions[i], 0, false, false, fontIds[i], 50, i);
            x += w + P(10);
        }
    }

    // Radio
    if (inView(1293, 1465)) {
        heading(L"Radio", 1293);

        auto drawRadio = [&](int x, int y, bool checked, bool hovered) {
            RECT rc = RC(x, y + 3, 14, 14);
            if (checked) {
                FillEllipseColor(hDC, rc, theme::Primary, theme::Primary, 1);
                RECT dot = { rc.left + P(4), rc.top + P(4), rc.right - P(4), rc.bottom - P(4) };
                FillEllipseColor(hDC, dot, theme::White, 0, 0);
            }
            else {
                FillEllipseColor(hDC, rc, theme::White,
                    hovered ? theme::Primary : theme::BorderBase, 1);
            }
        };
        int x = 0;
        const bool hoverA = IsHovered(K_RADIO, 1, 0);
        const bool hoverB = IsHovered(K_RADIO, 1, 1);
        drawRadio(x, 1336, m_radioBasic == 0, hoverA);
        label(x + 24, 1336, 90, 20, L"Option A", font::Body14,
            (m_radioBasic == 0 || hoverA) ? theme::Primary : theme::TextRegular);
        AddHotspot(RC(x, 1334, 24 + textW(L"Option A", font::Body14) + 6, 22), K_RADIO, 1, 0);
        x += 24 + textW(L"Option A", font::Body14) + P(30);
        drawRadio(x, 1336, m_radioBasic == 1, hoverB);
        label(x + 24, 1336, 90, 20, L"Option B", font::Body14,
            (m_radioBasic == 1 || hoverB) ? theme::Primary : theme::TextRegular);
        AddHotspot(RC(x, 1334, 24 + textW(L"Option B", font::Body14) + 6, 22), K_RADIO, 1, 1);

        const wchar_t* cities[4] = { L"New York", L"Washington", L"Los Angeles", L"Chicago" };
        x = 0;
        for (int i = 0; i < 4; ++i) {
            const int w = textW(cities[i], font::Body14) + P(40);
            RECT rc = RC(x, 1371, w, 40);
            const bool active = m_radioCity == i;
            const bool hovered = IsHovered(K_RADIO, 3, i);
            if (active) {
                FillRoundRectBorder(hDC, rc, 0, theme::Primary, theme::Primary, 1);
            }
            else {
                FillRoundRectBorder(hDC, rc, 0, hovered ? TP_PrimaryBg : theme::White,
                    theme::BorderBase, 1);
            }
            label(x, 1371, w, 40, cities[i], font::Body14,
                active ? theme::White : (hovered ? theme::Primary : theme::TextRegular),
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            AddHotspot(rc, K_RADIO, 3, i);
            x += w;
        }

        const wchar_t* options[2] = { L"Option A", L"Option B" };
        x = 0;
        for (int i = 0; i < 2; ++i) {
            const bool checked = m_radioBordered == i;
            const bool hovered = IsHovered(K_RADIO, 2, i);
            const int w = textW(options[i], font::Body14) + P(15 + 15 + 14 + 10);
            RECT rc = RC(x, 1426, w, 40);
            FillRoundRectBorder(hDC, rc, P(4), theme::White,
                (checked || hovered) ? theme::Primary : theme::BorderBase, 1);
            drawRadio(x + 15, 1426 + 13, checked, hovered);
            label(x + 39, 1426, w - 39, 40, options[i], font::Body14,
                (checked || hovered) ? theme::Primary : theme::TextRegular);
            AddHotspot(rc, K_RADIO, 2, i);
            x += w + P(10);
        }
    }

    // Checkbox
    if (inView(1505, 1704)) {
        heading(L"Checkbox", 1505);

        auto drawCheck = [&](int x, int y, bool checked, bool hovered) {
            RECT box = RC(x, y + 3, 14, 14);
            if (checked) {
                FillRoundRectBorder(hDC, box, P(2), theme::Primary, theme::Primary, 1);
                DrawCheckMark(hDC, box, theme::White);
            }
            else {
                FillRoundRectBorder(hDC, box, P(2), theme::White,
                    hovered ? theme::Primary : theme::BorderBase, 1);
            }
        };

        const bool basicHover = IsHovered(K_CHECK, 1, 0);
        drawCheck(0, 1548, m_checkBasic, basicHover);
        label(24, 1548, 100, 20, L"Option", font::Body14,
            (m_checkBasic || basicHover) ? theme::Primary : theme::TextRegular);
        AddHotspot(RC(0, 1544, 120, 28), K_CHECK, 1, 0);

        const wchar_t* cities[4] = { L"Shanghai", L"Beijing", L"Guangzhou", L"Shenzhen" };
        int x = 0;
        for (int i = 0; i < 4; ++i) {
            const bool checked = (m_checkCityMask & (1 << i)) != 0;
            const bool hovered = IsHovered(K_CHECK, 2, i);
            const int w = textW(cities[i], font::Body14) + P(40);
            RECT rc = RC(x, 1583, w, 40);
            if (checked) {
                FillRoundRectBorder(hDC, rc, 0, theme::Primary, theme::Primary, 1);
                label(x, 1583, w, 40, cities[i], font::Body14, theme::White,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            else {
                FillRoundRectBorder(hDC, rc, 0, hovered ? TP_PrimaryBg : theme::White,
                    hovered ? theme::Primary : theme::BorderBase, 1);
                label(x, 1583, w, 40, cities[i], font::Body14,
                    hovered ? theme::Primary : theme::TextRegular,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            AddHotspot(rc, K_CHECK, 2, i);
            x += w;
        }

        const bool borderHover = IsHovered(K_CHECK, 3, 0);
        const int w = textW(L"Option1", font::Body14) + P(15 + 15 + 14 + 10);
        RECT rc = RC(0, 1638, w, 40);
        FillRoundRectBorder(hDC, rc, P(4), theme::White,
            (m_checkBordered || borderHover) ? theme::Primary : theme::BorderBase, 1);
        drawCheck(15, 1638 + 13, m_checkBordered, borderHover);
        label(39, 1638, w - 39, 40, L"Option1", font::Body14,
            (m_checkBordered || borderHover) ? theme::Primary : theme::TextRegular);
        AddHotspot(rc, K_CHECK, 3, 0);
    }

    auto drawInputBox = [&](int x, int y, int w, int h, const std::wstring& text,
                            const wchar_t* prefixIcon, const wchar_t* suffixIcon,
                            int kind, int group, int index, bool focused, bool placeholder) {
        RECT rc = RC(x, y, w, h);
        FillRoundRectBorder(hDC, rc, P(4), theme::White,
            focused ? theme::Primary : theme::BorderBase, 1);
        int textX = 15;
        if (prefixIcon != nullptr) {
            label(x + 12, y + (h - 14) / 2, 16, 14, prefixIcon, font::IconSmall,
                focused ? theme::Primary : theme::TextPlaceholder);
            textX = 36;
        }
        if (suffixIcon != nullptr) {
            label(x + w - 29, y + (h - 12) / 2, 16, 12, suffixIcon, font::ThIcon12,
                m_selectOpen && kind == K_SELECT_FIELD ? theme::Primary : theme::TextPlaceholder);
        }
        RECT textRc = RC(x + textX, y, w - textX - (suffixIcon != nullptr ? 30 : 10), h);
        txt(textRc, text.c_str(), font::Body14,
            placeholder ? theme::TextPlaceholder : theme::TextRegular,
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        if (focused) {
            RECT caret = { X(x + textX), Y(y + 12), X(x + textX) + 1, Y(y + 28) };
            if (!placeholder && !text.empty()) {
                const int offset = textW(text.c_str(), font::Body14);
                caret.left += offset;
                caret.right += offset;
            }
            FillSolidRect(hDC, caret, theme::Primary);
        }
        if (kind != K_NONE) {
            AddHotspot(rc, kind, group, index);
        }
    };

    // Input
    if (inView(1717, 1803)) {
        heading(L"Input", 1717);
        const std::wstring value = m_inputText.empty()
            ? std::wstring(L"Please input") : m_inputText;
        drawInputBox(0, 1760, 180, 40, value, nullptr, nullptr,
            K_INPUT, 1, 0, m_inputFocused, m_inputText.empty());
    }

    // InputNumber
    if (inView(1841, 1927)) {
        heading(L"InputNumber", 1841);
        RECT rc = RC(0, 1884, 180, 40);
        FillRoundRectBorder(hDC, rc, P(4), theme::White, theme::BorderBase, 1);
        FillSolidRect(hDC, RC(40, 1885, 1, 38), theme::BorderLight);
        FillSolidRect(hDC, RC(139, 1885, 1, 38), theme::BorderLight);
        const bool minusHover = IsHovered(K_INPUT_MINUS, 1, 0) && m_number > 1;
        const bool plusHover = IsHovered(K_INPUT_PLUS, 1, 0);
        label(0, 1884, 40, 40, L"-", font::Th16,
            m_number > 1 ? (minusHover ? theme::Primary : theme::TextRegular)
                         : theme::TextPlaceholder,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        label(140, 1884, 40, 40, L"+", font::Th16,
            plusHover ? theme::Primary : theme::TextRegular,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        wchar_t numberText[16] = {};
        swprintf_s(numberText, L"%d", m_number);
        label(40, 1884, 100, 40, numberText, font::Body14, theme::TextRegular,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        AddHotspot(RC(0, 1884, 40, 40), K_INPUT_MINUS, 1, 0);
        AddHotspot(RC(140, 1884, 40, 40), K_INPUT_PLUS, 1, 0);
    }

    // Select
    if (inView(1964, 2050)) {
        heading(L"Select", 1964);
        const std::wstring value = m_selectIndex >= 0
            ? std::wstring(selectOptions[m_selectIndex]) : std::wstring(L"Select");
        drawInputBox(0, 2007, 221, 40, value, nullptr, m_selectOpen ? L"\uE70E" : L"\uE70D",
            K_SELECT_FIELD, 1, 0, m_selectOpen, m_selectIndex < 0);
    }

    // Cascader
    if (inView(2087, 2173)) {
        heading(L"Cascader", 2087);
        std::wstring value = L"请选择";
        if (m_cascaderProvince >= 0) {
            value = provinces[m_cascaderProvince];
            if (m_cascaderCity >= 0) {
                value += L" / ";
                value += cascaderCities[m_cascaderProvince][m_cascaderCity];
            }
        }
        drawInputBox(0, 2130, 221, 40, value, nullptr, m_cascaderOpen ? L"\uE70E" : L"\uE70D",
            K_CASCADER_FIELD, 1, 0, m_cascaderOpen, m_cascaderProvince < 0);
    }

    // Switch
    if (inView(2210, 2296)) {
        heading(L"Switch", 2210);
        auto drawSwitch = [&](int x, int y, bool on, int group, int index) {
            const bool hovered = IsHovered(K_SWITCH, group, index);
            const DWORD color = on ? (hovered ? 0xFF66B1FF : theme::Primary) : 0xFFDCDFE6;
            RECT core = RC(x, y, 40, 20);
            FillRoundRectBorder(hDC, core, P(10), color, color, 1);
            const int knobX = on ? x + 22 : x + 2;
            RECT knob = RC(knobX, y + 2, 16, 16);
            FillEllipseColor(hDC, knob, theme::White, 0, 0);
            AddHotspot(RC(x, y, 40, 20), K_SWITCH, group, index);
        };
        drawSwitch(0, 2254, m_switchYear, 1, 0);
        const int leftW = textW(L"Pay by year", font::Body14);
        label(80, 2254 - 3, leftW + 4, 26, L"Pay by year", font::Body14, theme::TextPrimary);
        drawSwitch(80 + leftW + 10, 2254, m_switchMonth, 2, 0);
        label(80 + leftW + 10 + 40 + 10, 2254 - 3, 100, 26, L"Pay by month", font::Body14,
            m_switchMonth ? theme::Primary : theme::TextSecondary);
    }

    // Slider
    if (inView(2314, 2400)) {
        heading(L"Slider", 2314);
        RECT runway = RC(0, 2373, 380, 6);
        FillRoundRectBorder(hDC, runway, P(3), 0xFFE4E7ED, 0, 0);
        const int barW = 380 * m_sliderValue / 100;
        if (barW > 0) {
            RECT bar = RC(0, 2373, barW, 6);
            FillRoundRectBorder(hDC, bar, P(3), theme::Primary, 0, 0);
        }
        const bool active = IsHovered(K_SLIDER, 1, 0) || m_dragSlider;
        const int knobR = active ? 14 : 12;
        RECT knob = RC(barW - knobR, 2376 - knobR, knobR * 2, knobR * 2);
        FillEllipseColor(hDC, knob, theme::White, theme::Primary, 2);
        AddHotspot(RC(0, 2358, 380, 36), K_SLIDER, 1, 0);
    }

    // DatePicker
    if (inView(2435, 2521)) {
        heading(L"DatePicker", 2435);
        const std::wstring value = m_dateText.empty()
            ? std::wstring(L"Pick a day") : m_dateText;
        drawInputBox(0, 2478, 220, 40, value, L"\uE787", nullptr,
            K_DATE_FIELD, 1, 0, m_dateOpen, m_dateText.empty());
    }

    // Rate
    if (inView(2558, 2644)) {
        heading(L"Rate", 2558);
        for (int row = 0; row < 2; ++row) {
            const int y = 2617 + row * 32;
            for (int i = 0; i < 5; ++i) {
                const bool filled = i < m_rateValue
                    || (m_rateHoverRow == row + 1 && i < m_rateHoverValue);
                label(i * 24, y, 24, 18, L"\uE735", font::ThIcon18,
                    filled ? TP_Gold : TP_RateVoid,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                AddHotspot(RC(i * 24, y, 24, 18), K_RATE, row + 1, i);
            }
            if (row == 1) {
                wchar_t buffer[32] = {};
                swprintf_s(buffer, L"%d points", m_rateValue);
                label(130, y, 90, 18, buffer, font::Body14, TP_Gold);
            }
        }
    }

    // Transfer
    if (inView(2726, 3112)) {
        heading(L"Transfer", 2726);
        RECT rc = RC(0, 2769, 845, 343);
        FillRoundRectBorder(hDC, rc, P(4), 0xFFFAFAFA, theme::BorderLight, 1);
        label(0, 2769, 845, 343, L"TODO: Transfer 穿梭框暂未支持（duilib 无对应控件）",
            font::Th16, theme::Danger, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Table
    if (inView(3152, 3434)) {
        heading(L"Table", 3152);
        const wchar_t* headers[3] = { L"Date", L"Name", L"Address" };
        const int widths[3] = { 180, 180, 232 };
        int x = 0;
        for (int i = 0; i < 3; ++i) {
            label(x + 10, 3195, widths[i] - 20, 48, headers[i], font::Body14, theme::TextSecondary);
            x += widths[i];
        }
        FillSolidRect(hDC, RC(0, 3195 + 47, 592, 1), theme::BorderLighter);
        const wchar_t* rows[4][3] = {
            { L"2016-05-03", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-02", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-04", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-01", L"Tom", L"No. 189, Grove St, Los Angeles" },
        };
        for (int r = 0; r < 4; ++r) {
            const int y = 3243 + r * 48;
            x = 0;
            for (int i = 0; i < 3; ++i) {
                label(x + 10, y, widths[i] - 20, 48, rows[r][i], font::Body14, theme::TextRegular);
                x += widths[i];
            }
            FillSolidRect(hDC, RC(0, y + 47, 592, 1), theme::BorderLighter);
        }
    }

    // Tag
    if (inView(3474, 3637)) {
        heading(L"Tag", 3474);
        const wchar_t* texts[5] = { L"Tag One", L"Tag Two", L"Tag Three", L"Tag Four", L"Tag Five" };
        const int ws[5] = { 83, 82, 91, 85, 83 };
        const DWORD bgs[5] = { TP_PrimaryBg, TP_SuccessBg, TP_InfoBg, TP_WarningBg, TP_DangerBg };
        const DWORD borders[5] = { TP_PrimaryBorderTag, TP_SuccessBorder, TP_InfoBorder,
            TP_WarningBorder, TP_DangerBorder };
        const DWORD colors[5] = { theme::Primary, theme::Success, theme::Info, theme::Warning, theme::Danger };
        int x = 0;
        for (int i = 0; i < 5; ++i) {
            if (!m_tagVisible[i]) {
                continue;
            }
            RECT rc = RC(x, 3517, ws[i], 32);
            FillRoundRectBorder(hDC, rc, P(4), bgs[i], borders[i], 1);
            label(x + 10, 3517, ws[i] - 30, 32, texts[i], font::Small, colors[i]);
            const bool hovering = IsHovered(K_TAG_CLOSE, 1, i);
            label(x + ws[i] - 22, 3517, 12, 32, L"\uE711", font::ThIcon12,
                hovering ? theme::TextPrimary : colors[i],
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            AddHotspot(RC(x + ws[i] - 26, 3517, 20, 32), K_TAG_CLOSE, 1, i);
            x += ws[i] + 40;
        }
    }

    // Progress
    if (inView(3599, 3762)) {
        heading(L"Progress", 3599);
        const int rows[3] = { 3643, 3659, 3676 };
        const int percents[3] = { 20, 60, 100 };
        const DWORD barColors[3] = { theme::Primary, theme::Danger, theme::Success };
        for (int i = 0; i < 3; ++i) {
            const int y = rows[i];
            RECT runway = RC(0, y + 5, 330, 6);
            FillRoundRectBorder(hDC, runway, P(3), theme::BorderLighter, 0, 0);
            RECT bar = RC(0, y + 5, 330 * percents[i] / 100, 6);
            FillRoundRectBorder(hDC, bar, P(3), barColors[i], 0, 0);
            if (i == 1) {
                label(340, y, 40, 17, L"\uEA39", font::IconSmall, theme::Danger);
            }
            else if (i == 2) {
                label(340, y, 40, 17, L"\uE73E", font::IconSmall, theme::Success);
            }
            else {
                label(340, y, 40, 17, L"20%", font::Body14, theme::TextRegular);
            }
        }
    }

    // Tree
    {
        const int rowHeight = 26;
        const int visibleCount = TreePreviewVisibleCount(m_treeOpenMask);
        const int treeTop = Y(3776);
        const int treeBottom = treeTop + P(visibleCount * rowHeight);
        if (treeBottom >= rcPaint.top && Y(3733) <= rcPaint.bottom) {
            heading(L"Tree", 3733);
            int rowY = treeTop;
            for (int i = 0; i < kTreePreviewCount; ++i) {
                if (!TreePreviewVisible(i, m_treeOpenMask)) {
                    continue;
                }
                const TreePreviewNode& node = kTreePreviewNodes[i];
                const int indent = node.depth * 18;
                const bool expandable = TreePreviewHasChildren(i);
                const bool open = (m_treeOpenMask & (1 << i)) != 0;
                const bool selected = m_treeSelNode == i;
                const bool hoverArrow = IsHovered(K_TREE_ARROW, i, -1);
                const bool hovered = hoverArrow || IsHovered(K_TREE, i, -1);
                if (selected || hovered) {
                    RECT row = { X(0), rowY, X(500), rowY + P(rowHeight) };
                    FillSolidRect(hDC, row, theme::FillLight);
                }
                RECT rowRect = { X(expandable ? indent + 16 : 0), rowY, X(500),
                    rowY + P(rowHeight) };
                if (expandable) {
                    RECT arrow = { X(indent), rowY, X(indent + 16), rowY + P(rowHeight) };
                    txt(arrow, open ? L"\uE70D" : L"\uE76C", font::ThIcon12,
                        hovered ? theme::TextSecondary : theme::TextPlaceholder,
                        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    AddHotspot(arrow, K_TREE_ARROW, i, -1);
                }
                RECT text = { X(indent + 18), rowY, X(indent + 318), rowY + P(rowHeight) };
                txt(text, node.label, font::Body14, theme::TextRegular,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                AddHotspot(rowRect, K_TREE, i, -1);
                rowY += P(rowHeight);
            }
        }
    }

    // Pagination
    if (inView(3894, 3972)) {
        heading(L"Pagination", 3894);
        auto pageCell = [&](int x, int index, const wchar_t* text) {
            RECT cell = RC(x, 3937, 30, 28);
            const bool active = m_page == index;
            const bool hovered = IsHovered(K_PAGE, 1, index);
            if (active) {
                FillRoundRectBorder(hDC, cell, P(2), theme::Primary, theme::Primary, 1);
            }
            else if (hovered) {
                FillSolidRect(hDC, cell, TP_PrimaryBg);
            }
            label(x, 3937, 30, 28, text, font::Body,
                active ? theme::White : (hovered ? theme::Primary : theme::TextRegular),
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            AddHotspot(cell, K_PAGE, 1, index);
        };
        RECT prev = RC(0, 3937, 30, 28);
        const bool prevHover = IsHovered(K_PAGE, 1, -1);
        FillSolidRect(hDC, prev, prevHover ? TP_PrimaryBg : theme::FillLight);
        label(0, 3937, 30, 28, L"\uE76B", font::ThIcon12,
            prevHover ? theme::Primary : theme::TextPlaceholder,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        AddHotspot(prev, K_PAGE, 1, -1);
        for (int i = 0; i < 6; ++i) {
            wchar_t buffer[8] = {};
            swprintf_s(buffer, L"%d", i + 1);
            pageCell(30 + i * 30, i + 1, buffer);
        }
        label(210, 3937, 30, 28, L"\uE712", font::ThIcon12, theme::TextPlaceholder,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pageCell(240, 100, L"100");
        RECT next = RC(270, 3937, 30, 28);
        const bool nextHover = IsHovered(K_PAGE, 1, -2);
        if (nextHover) {
            FillSolidRect(hDC, next, TP_PrimaryBg);
        }
        label(270, 3937, 30, 28, L"\uE76C", font::ThIcon12,
            nextHover ? theme::Primary : theme::TextRegular,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        AddHotspot(next, K_PAGE, 1, -2);
    }

    // Badge
    if (inView(4009, 4112)) {
        heading(L"Badge", 4009);
        const wchar_t* captions[4] = { L"comments", L"replies", L"comments", L"replies" };
        const wchar_t* values[4] = { L"12", L"3", L"1", L"2" };
        const int variants[4] = { 5, 5, 1, 4 };
        const int dpi = UiScale::Dpi();
        auto cssWidth = [&](const wchar_t* text, int fontId) {
            return MulDiv(textW(text, fontId), 96, dpi);
        };
        int x = 0;
        for (int i = 0; i < 4; ++i) {
            const int w = cssWidth(captions[i], font::Small) + 32;
            drawBtn(x, 4062, w, 32, captions[i], 0, false, false, font::Small, 60, i);
            const int badgeW = (std::max)(18, cssWidth(values[i], font::Small) + 14);
            const int badgeX = x + w - 10;
            RECT pill = RC(badgeX, 4053, badgeW, 18);
            FillRoundRectBorder(hDC, pill, P(9), fills[variants[i]], theme::White, 1);
            label(badgeX, 4053, badgeW, 18, values[i], font::Small, theme::White,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            x += w + 40;
        }
        const int dataWidth = cssWidth(L"data", font::Body14);
        label(x, 4062, 120, 21, L"data", font::Body14, theme::TextRegular);
        RECT dot = RC(x + dataWidth, 4058, 8, 8);
        FillEllipseColor(hDC, dot, theme::Danger, theme::Danger, 1);
    }

    // Alert
    if (inView(4134, 4406)) {
        heading(L"Alert", 4134);
        const wchar_t* texts[4] = { L"成功提示的文案", L"消息提示的文案", L"警告提示的文案", L"错误提示的文案" };
        const wchar_t* description = L"这是一段说明文字，说明文字会换行展示。";
        const int variants[4] = { 2, 3, 4, 5 };
        const wchar_t* icons[4] = { L"\uE73E", L"\uE946", L"\uE7BA", L"\uEA39" };
        const int ys[4] = { 4187, 4232, 4277, 4321 };
        for (int i = 0; i < 4; ++i) {
            const int h = i == 3 ? 58 : 35;
            RECT rc = RC(0, ys[i], 380, h);
            FillRoundRectBorder(hDC, rc, P(4), lightFills[variants[i]], lightBorders[variants[i]], 1);
            label(10, ys[i], 20, i == 3 ? 30 : h, icons[i], font::IconSmall, strongText[variants[i]]);
            label(36, ys[i], 320, i == 3 ? 30 : h, texts[i], font::Body14, strongText[variants[i]]);
            if (i == 3) {
                label(36, ys[i] + 30, 320, 24, description, font::Small, strongText[variants[i]]);
            }
        }
    }

    // Loading
    if (inView(4419, 4754)) {
        heading(L"Loading", 4419);
        RECT table = RC(0, 4462, 761, 239);
        FillRoundRectBorder(hDC, table, P(4), theme::White, theme::BorderLighter, 1);
        FillSolidRect(hDC, RC(0, 4462, 761, 48), theme::FillLight);
        const wchar_t* headers[3] = { L"Date", L"Name", L"Address" };
        const int widths[3] = { 200, 200, 361 };
        int x = 0;
        for (int i = 0; i < 3; ++i) {
            label(x + 12, 4462, widths[i] - 24, 48, headers[i], font::Small, theme::TextSecondary);
            x += widths[i];
        }
        FillSolidRect(hDC, RC(0, 4509, 761, 1), theme::BorderLighter);
        const wchar_t* rows[4][3] = {
            { L"2016-05-03", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-02", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-04", L"Tom", L"No. 189, Grove St, Los Angeles" },
            { L"2016-05-01", L"Tom", L"No. 189, Grove St, Los Angeles" },
        };
        for (int r = 0; r < 4; ++r) {
            const int y = 4510 + r * 48;
            x = 0;
            for (int c = 0; c < 3; ++c) {
                label(x + 12, y, widths[c] - 24, 48, rows[r][c], font::Small, theme::TextRegular);
                x += widths[c];
            }
            FillSolidRect(hDC, RC(0, y + 47, 761, 1), theme::BorderLighter);
        }
        FillAlphaRect(hDC, table, 0xB3FFFFFF);
        EnsureGdiplus();
        {
            Gdiplus::Graphics g(hDC);
            g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            const int cx = X(380);
            const int cy = Y(4462 + 120);
            Gdiplus::Pen pen(GpColor(theme::Primary), 2.0f);
            Gdiplus::RectF arc(static_cast<Gdiplus::REAL>(cx - P(14)), static_cast<Gdiplus::REAL>(cy - P(14)),
                static_cast<Gdiplus::REAL>(P(28)), static_cast<Gdiplus::REAL>(P(28)));
            g.DrawArc(&pen, arc, -90.0f, 270.0f);
        }
    }

    // Message
    if (inView(4741, 5050)) {
        heading(L"Message", 4741);
        const wchar_t* texts[4] = { L"成功提示的文案", L"消息提示的文案", L"警告提示的文案", L"错误提示的文案" };
        const int variants[4] = { 2, 3, 4, 5 };
        const wchar_t* icons[4] = { L"\uE73E", L"\uE946", L"\uE7BA", L"\uEA39" };
        const int ys[4] = { 4795, 4857, 4920, 4982 };
        for (int i = 0; i < 4; ++i) {
            RECT rc = RC(0, ys[i], 380, 53);
            FillRoundRectBorder(hDC, rc, P(4), lightFills[variants[i]], lightBorders[variants[i]], 1);
            label(16, ys[i], 22, 53, icons[i], font::IconSmall, strongText[variants[i]]);
            label(48, ys[i], 300, 53, texts[i], font::Body14, strongText[variants[i]]);
        }
    }

    // MessageBox
    if (inView(5075, 5270)) {
        heading(L"MessageBox", 5075);
        RECT box = RC(0, 5118, 422, 140);
        shadow(box, P(4));
        FillRoundRectBorder(hDC, box, P(4), theme::White, 0, 0);
        label(15, 5132, 330, 20, L"Warning", font::Th18, theme::TextPrimary);
        label(391, 5132, 16, 16, L"\uE711", font::ThIcon12, theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        label(15, 5171, 24, 24, L"\uE7BA", font::Icon24, theme::Warning);
        label(51, 5171, 340, 24, L"This is a warning message.", font::Body14, theme::TextRegular);
        const int dpi = UiScale::Dpi();
        const int okW = MulDiv(textW(L"OK", font::Small), 96, dpi) + 30;
        const int cancelW = MulDiv(textW(L"Cancel", font::Small), 96, dpi) + 30;
        drawBtn(422 - 15 - okW, 5215, okW, 32, L"OK", 1, false, false, font::Small, 70, 0);
        drawBtn(422 - 15 - okW - 10 - cancelW, 5215, cancelW, 32, L"Cancel", 0, false, false,
            font::Small, 70, 1);
    }

    // Notification
    if (inView(5290, 5470)) {
        heading(L"Notification", 5290);
        RECT box = RC(0, 5334, 330, 75);
        shadow(box, P(4));
        FillRoundRectBorder(hDC, box, P(4), theme::White, theme::BorderLighter, 1);
        label(16, 5346, 240, 22, L"Notification", font::Body14Bold, theme::TextPrimary);
        label(16, 5372, 270, 20, L"This is a notification message", font::Small, theme::TextRegular);
        label(298, 5346, 16, 16, L"\uE711", font::ThIcon12, theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Menu
    if (inView(5449, 5810 + m_menuExtra)) {
        heading(L"Menu", 5449);
        const int dpi = UiScale::Dpi();
        auto cssW = [&](const wchar_t* text, int fontId) {
            return MulDiv(textW(text, fontId), 96, dpi);
        };
        auto MY = [&](int cssY) {
            return top + P(70 + cssY - 190) + (cssY >= 3860 ? P(m_treeExtra) : 0);
        };
        auto MR = [&](int x, int y, int w, int h) {
            RECT rc = { X(x), MY(y), X(x + w), MY(y + h) };
            return rc;
        };
        auto mLabel = [&](int x, int y, int w, int h, const wchar_t* text, int fontId, DWORD color,
                          UINT flags = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
            txt(MR(x, y, w, h), text, fontId, color, flags);
        };
        {
            const wchar_t* items[4] = { L"Processing Center", L"Workspace", L"Info", L"Orders" };
            const bool disabled[4] = { false, false, true, false };
            FillSolidRect(hDC, MR(0, 5492, 845, 60), theme::White);
            FillSolidRect(hDC, MR(0, 5492 + 60, 845, 1), theme::BorderLight);
            int x = 0;
            for (int i = 0; i < 4; ++i) {
                const int w = cssW(items[i], font::Body14) + 40;
                const bool active = m_menuH == i;
                const bool hovered = !disabled[i] && IsHovered(K_MENU_H, 1, i);
                if (hovered && !active) {
                    FillSolidRect(hDC, MR(x, 5492, w, 60), TP_PrimaryBg);
                }
                const DWORD color = disabled[i] ? theme::TextPlaceholder
                    : ((active || hovered) ? theme::Primary : theme::TextPrimary);
                mLabel(x, 5492, w - (i == 1 ? 18 : 0), 60, items[i], font::Body14, color,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                if (i == 1) {
                    mLabel(x + w - 26, 5492, 16, 60, L"\uE70D", font::ThIcon12,
                        hovered ? theme::Primary : theme::TextSecondary,
                        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                if (active) {
                    FillSolidRect(hDC, MR(x, 5492 + 58, w, 2), theme::Primary);
                }
                AddHotspot(MR(x, 5492, w, 60), K_MENU_H, 1, i);
                x += w;
            }
        }
        {
            const bool nav1Open = (m_menuVOpenMask & 1) != 0;
            const bool item4Open = nav1Open && (m_menuVOpenMask & 2) != 0;
            const int menuHeight = 224 + (nav1Open ? 292 : 0) + (item4Open ? 56 : 0);
            FillSolidRect(hDC, MR(0, 5568, 200, menuHeight), theme::White);

            auto topItem = [&](int y, int id, const wchar_t* glyph, const wchar_t* text,
                               bool disabled, bool submenu, bool open) {
                RECT item = MR(0, y, 200, 56);
                const bool active = m_menuV == id;
                const bool hovered = !disabled && IsHovered(K_MENU_V, 1, id);
                if (hovered) {
                    FillSolidRect(hDC, item, TP_PrimaryBg);
                }
                const DWORD color = disabled ? theme::TextPlaceholder
                    : ((active || hovered) ? theme::Primary : theme::TextPrimary);
                mLabel(20, y, 16, 56, glyph, font::IconSmall, color);
                mLabel(42, y, 118, 56, text, font::Body14, color);
                if (submenu) {
                    mLabel(172, y, 16, 56, open ? L"\uE70D" : L"\uE76C", font::ThIcon12,
                        (active || hovered) ? theme::Primary : theme::TextSecondary,
                        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                AddHotspot(item, K_MENU_V, 1, id);
            };
            auto subItem = [&](int y, int id, const wchar_t* text, int indent, bool arrow, bool open) {
                RECT item = MR(0, y, 200, 56);
                const bool active = m_menuV == id;
                const bool hovered = IsHovered(K_MENU_V, 1, id);
                if (hovered) {
                    FillSolidRect(hDC, item, TP_PrimaryBg);
                }
                const DWORD color = (active || hovered) ? theme::Primary : theme::TextPrimary;
                mLabel(indent, y, 200 - indent - 24, 56, text, font::Body14, color);
                if (arrow) {
                    mLabel(172, y, 16, 56, open ? L"\uE70D" : L"\uE76C", font::ThIcon12,
                        (active || hovered) ? theme::Primary : theme::TextSecondary,
                        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }
                AddHotspot(item, K_MENU_V, 1, id);
            };
            auto groupTitle = [&](int y, const wchar_t* text) {
                mLabel(40, y, 140, 34, text, font::Small, theme::TextSecondary);
            };

            int y = 5568;
            topItem(y, 0, L"\uE707", L"Navigator One", false, true, nav1Open);
            y += 56;
            if (nav1Open) {
                groupTitle(y, L"Group One");
                y += 34;
                subItem(y, 11, L"Item one", 40, false, false);
                y += 56;
                subItem(y, 12, L"Item two", 40, false, false);
                y += 56;
                groupTitle(y, L"Group Two");
                y += 34;
                subItem(y, 21, L"Item three", 40, false, false);
                y += 56;
                subItem(y, 30, L"Item four", 40, true, item4Open);
                y += 56;
                if (item4Open) {
                    subItem(y, 31, L"Item one", 60, false, false);
                    y += 56;
                }
            }
            topItem(y, 1, L"\uE700", L"Navigator Two", false, false, false);
            y += 56;
            topItem(y, 2, L"\uE8A5", L"Navigator Three", true, false, false);
            y += 56;
            topItem(y, 3, L"\uE713", L"Navigator Four", false, false, false);
        }
    }

    // Tabs
    if (inView(5847, 6070)) {
        heading(L"Tabs", 5847);
        {
            const wchar_t* items[4] = { L"User", L"Config", L"Role", L"Task" };
            FillSolidRect(hDC, RC(0, 5900 + 39, 805, 1), theme::BorderLight);
            int x = 0;
            for (int i = 0; i < 4; ++i) {
                const int w = textW(items[i], font::Body14) + P(32);
                const bool active = m_tabTop == i;
                const bool hovered = IsHovered(K_TAB, 1, i);
                label(x, 5900, w, 40, items[i], font::Body14,
                    (active || hovered) ? theme::Primary : theme::TextPrimary,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                if (active) {
                    FillSolidRect(hDC, RC(x, 5900 + 37, w, 2), theme::Primary);
                }
                AddHotspot(RC(x, 5900, w, 40), K_TAB, 1, i);
                x += w;
            }
        }
        {
            const wchar_t* items[4] = { L"User", L"Config", L"Role", L"Task" };
            int x = 0;
            for (int i = 0; i < 4; ++i) {
                const int w = textW(items[i], font::Body14) + P(32);
                RECT cell = RC(x, 5982, w, 40);
                const bool active = m_tabCard == i;
                const bool hovered = IsHovered(K_TAB, 2, i);
                FillRoundRectBorder(hDC, cell, 0,
                    (active || hovered) ? theme::White : theme::FillLight,
                    theme::BorderBase, 1);
                if (active) {
                    FillSolidRect(hDC, RC(x + 1, 5982, w - 1, 2), theme::Primary);
                }
                label(x, 5982, w, 40, items[i], font::Body14,
                    (active || hovered) ? theme::Primary : theme::TextPrimary,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                AddHotspot(cell, K_TAB, 2, i);
                x += w;
            }
        }
    }

    // Dialog
    if (inView(6096, 6380)) {
        heading(L"Dialog", 6096);
        RECT box = RC(0, 6139, 423, 200);
        shadow(box, P(4));
        FillRoundRectBorder(hDC, box, P(4), theme::White, 0, 0);
        label(20, 6159, 300, 24, L"Tips", font::Section, theme::TextPrimary);
        label(388, 6159, 16, 16, L"\uE711", font::ThIcon12, theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        label(20, 6205, 382, 22, L"This is a dialog", font::Body14, theme::TextRegular);
        const int okW = textW(L"OK", font::Body14) + P(40);
        const int cancelW = textW(L"Cancel", font::Body14) + P(40);
        const int footerY = 6139 + 200 - 32 - 20;
        drawBtn(423 - 20 - okW, footerY, okW, 32, L"OK", 1, false, false, font::Body14, 80, 0);
        drawBtn(423 - 20 - okW - 10 - cancelW, footerY, cancelW, 32, L"Cancel", 0, false, false,
            font::Body14, 80, 1);
    }

    // Tooltip
    if (inView(6380, 6530)) {
        heading(L"Tooltip", 6380);
        {
            RECT rc = RC(0, 6423, 60, 34);
            shadow(rc, P(4));
            FillRoundRectBorder(hDC, rc, P(4), 0xFF303133, 0, 0);
            label(0, 6423, 60, 34, L"Dark", font::Small, theme::White,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            FillTriangle(hDC, X(26), Y(6457), X(34), Y(6457), X(30), Y(6462), 0xFF303133);
        }
        {
            RECT rc = RC(120, 6469, 62, 36);
            shadow(rc, P(4));
            FillRoundRectBorder(hDC, rc, P(4), theme::White, theme::BorderLight, 1);
            label(120, 6469, 62, 36, L"Light", font::Small, theme::TextPrimary,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            FillTriangle(hDC, X(146), Y(6505), X(154), Y(6505), X(150), Y(6510), theme::White);
        }
    }

    // Popover
    if (inView(6557, 6730)) {
        heading(L"Popover", 6557);
        RECT box = RC(0, 6601, 242, 105);
        shadow(box, P(4));
        FillRoundRectBorder(hDC, box, P(4), theme::White, theme::BorderLighter, 1);
        label(15, 6613, 200, 22, L"Title", font::Body14Bold, theme::TextPrimary);
        label(15, 6641, 212, 50, L"This is a popover content description.", font::Small,
            theme::TextRegular, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }

    // Card
    if (inView(6757, 6930)) {
        heading(L"Card", 6757);
        RECT box = RC(0, 6801, 845, 97);
        FillRoundRectBorder(hDC, box, P(4), theme::White, theme::BorderLighter, 1);
        FillSolidRect(hDC, RC(0, 6801 + 48, 845, 1), theme::BorderLighter);
        label(20, 6801, 400, 48, L"Card name", font::Th16, theme::TextPrimary);
        label(845 - 20 - 140, 6801, 140, 48, L"Operation button", font::Small, theme::Primary,
            DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        label(20, 6861, 400, 22, L"List item one", font::Body14, theme::TextRegular);
        label(434, 6861, 400, 22, L"List item two", font::Body14, theme::TextRegular);
    }

    // Carousel
    if (inView(6937, 7160)) {
        heading(L"Carousel", 6937);
        const DWORD slideColors[4] = { TP_CarouselA, 0xFFB3C0CE, TP_CarouselB, 0xFFE5E9F2 };
        const wchar_t* slideTexts[4] = { L"2", L"1", L"3", L"4" };
        RECT slide = RC(0, 6981, 845, 150);
        FillRoundRectBorder(hDC, slide, 0, slideColors[m_carousel], 0, 0);
        label(0, 6981, 845, 150, slideTexts[m_carousel], font::H1, theme::White,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        const int indicatorWidth = 30;
        const int gap = 10;
        const int total = 4 * indicatorWidth + 3 * gap;
        int x = (845 - total) / 2;
        for (int i = 0; i < 4; ++i) {
            FillSolidRect(hDC, RC(x, 6981 + 150 - 14, indicatorWidth, 4),
                i == m_carousel ? theme::White : 0x66FFFFFF);
            AddHotspot(RC(x, 6981 + 150 - 20, indicatorWidth, 16), K_CAROUSEL_IND, 1, i);
            x += indicatorWidth + gap;
        }
    }

    // Collapse
    const bool collapseOpen1 = (m_collapseMask & 1) != 0;
    const bool collapseOpen2 = (m_collapseMask & 2) != 0;
    if (inView(7171, 7390 + m_extraHeight)) {
        heading(L"Collapse", 7171);
        const int collapseHeight = 48 + (collapseOpen1 ? 51 : 0) + 48 + (collapseOpen2 ? 51 : 0);
        RECT box = RC(0, 7214, 845, collapseHeight);
        FillRoundRectBorder(hDC, box, P(4), theme::White, theme::BorderLighter, 1);
        int y = 7214;
        const bool hover1 = IsHovered(K_COLLAPSE, 1, 0);
        label(20, y, 16, 48, collapseOpen1 ? L"\uE70D" : L"\uE76C", font::ThIcon12,
            hover1 ? theme::Primary : theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        label(52, y, 600, 48, L"Consistent", font::Body14,
            hover1 ? theme::Primary : theme::TextPrimary);
        AddHotspot(RC(0, y, 845, 48), K_COLLAPSE, 1, 0);
        y += 48;
        if (collapseOpen1) {
            label(20, y, 760, 51, L"Consistent with real-life usage conventions.", font::Small,
                theme::TextRegular, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            y += 51;
        }
        FillSolidRect(hDC, RC(0, y, 845, 1), theme::BorderLighter);
        const bool hover2 = IsHovered(K_COLLAPSE, 1, 1);
        label(20, y, 16, 48, collapseOpen2 ? L"\uE70D" : L"\uE76C", font::ThIcon12,
            hover2 ? theme::Primary : theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        label(52, y, 600, 48, L"Feedback", font::Body14,
            hover2 ? theme::Primary : theme::TextPrimary);
        AddHotspot(RC(0, y, 845, 48), K_COLLAPSE, 1, 1);
        y += 48;
        if (collapseOpen2) {
            label(20, y, 760, 51, L"Feedback is the breakfast of champions.", font::Small,
                theme::TextRegular, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
    }

    // Avatar
    const int avatarY = 7444 + m_extraHeight;
    if (inView(7401 + m_extraHeight, 7520 + m_extraHeight)) {
        heading(L"Avatar", 7401 + m_extraHeight);
        const wchar_t* names[3] = { L"large", L"medium", L"small" };
        const int sizes[3] = { 40, 36, 28 };
        int x = 0;
        RECT circle = RC(x, avatarY, 40, 40);
        FillEllipseColor(hDC, circle, 0xFFC0C4CC, 0, 0);
        label(x, avatarY, 40, 40, L"\uE77B", font::Icon, theme::White,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        x += 60;
        RECT circle2 = RC(x, avatarY, 40, 40);
        FillEllipseColor(hDC, circle2, 0xFFC0C4CC, 0, 0);
        label(x, avatarY, 40, 40, L"avatar", font::Th11, theme::White,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        x += 60;
        RECT square = RC(x, avatarY, 40, 40);
        FillRoundRectBorder(hDC, square, P(4), 0xFFC0C4CC, 0, 0);
        label(x, avatarY, 40, 40, L"\uE91B", font::Icon, theme::White,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        x += 60;
        for (int i = 0; i < 3; ++i) {
            const int size = sizes[i];
            RECT avatar = RC(x, avatarY + (40 - size) / 2, size, size);
            FillEllipseColor(hDC, avatar, 0xFFC0C4CC, 0, 0);
            label(x, avatarY + (40 - size) / 2, size, size, L"\uE77B",
                i == 0 ? font::Icon : font::IconSmall, theme::White,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            label(x + size + 8, avatarY, 70, 40, names[i], font::Small, theme::TextSecondary);
            x += size + 8 + textW(names[i], font::Small) + P(20);
        }
    }

    // Popups: drawn last so hit-testing gives them priority over the page below.
    if (m_selectOpen) {
        RECT panel = RC(0, 2055, 221, 182);
        shadow(panel, P(4));
        FillRoundRectBorder(hDC, panel, P(4), theme::White, theme::BorderLighter, 1);
        for (int i = 0; i < 5; ++i) {
            const int rowY = 2061 + i * 34;
            const bool selected = m_selectIndex == i;
            const bool hovered = IsHovered(K_SELECT_OPTION, 1, i);
            if (selected) {
                FillSolidRect(hDC, RC(5, rowY, 211, 34), TP_PrimaryBg);
            }
            else if (hovered) {
                FillSolidRect(hDC, RC(5, rowY, 211, 34), theme::FillLight);
            }
            label(20, rowY, 180, 34, selectOptions[i], font::Body14,
                (selected || hovered) ? theme::Primary : theme::TextRegular);
            AddHotspot(RC(0, rowY, 221, 34), K_SELECT_OPTION, 1, i);
        }
    }

    if (m_cascaderOpen) {
        RECT panel = RC(0, 2178, 320, 116);
        shadow(panel, P(4));
        FillRoundRectBorder(hDC, panel, P(4), theme::White, theme::BorderLighter, 1);
        for (int i = 0; i < 3; ++i) {
            const int rowY = 2184 + i * 34;
            const bool selected = m_cascaderProvince == i;
            const bool hovered = IsHovered(K_CASCADER_PROVINCE, 1, i);
            if (selected) {
                FillSolidRect(hDC, RC(4, rowY, 152, 34), TP_PrimaryBg);
            }
            label(20, rowY, 130, 34, provinces[i], font::Body14,
                (selected || hovered) ? theme::Primary : theme::TextRegular);
            AddHotspot(RC(0, rowY, 160, 34), K_CASCADER_PROVINCE, 1, i);
        }
        if (m_cascaderProvince >= 0) {
            FillSolidRect(hDC, RC(160, 2180, 1, 112), theme::BorderLighter);
            for (int i = 0; i < 3; ++i) {
                const int rowY = 2184 + i * 34;
                const bool selected = m_cascaderCity == i;
                const bool hovered = IsHovered(K_CASCADER_CITY, 1, i);
                if (selected) {
                    FillSolidRect(hDC, RC(164, rowY, 152, 34), TP_PrimaryBg);
                }
                label(180, rowY, 130, 34, cascaderCities[m_cascaderProvince][i], font::Body14,
                    (selected || hovered) ? theme::Primary : theme::TextRegular);
                AddHotspot(RC(160, rowY, 160, 34), K_CASCADER_CITY, 1, i);
            }
        }
    }

    if (m_dateOpen) {
        RECT panel = RC(0, 2526, 240, 226);
        shadow(panel, P(4));
        FillRoundRectBorder(hDC, panel, P(4), theme::White, theme::BorderLighter, 1);
        const bool prevHover = IsHovered(K_CALENDAR_PREV, 1, 0);
        label(10, 2534, 20, 20, L"\uE76B", font::ThIcon12,
            prevHover ? theme::Primary : theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        AddHotspot(RC(6, 2532, 28, 24), K_CALENDAR_PREV, 1, 0);
        wchar_t monthText[32] = {};
        swprintf_s(monthText, L"%d 年 %d 月", m_calYear, m_calMonth);
        label(50, 2534, 140, 20, monthText, font::Body14, theme::TextPrimary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        const bool nextHover = IsHovered(K_CALENDAR_NEXT, 1, 0);
        label(210, 2534, 20, 20, L"\uE76C", font::ThIcon12,
            nextHover ? theme::Primary : theme::TextSecondary,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        AddHotspot(RC(206, 2532, 28, 24), K_CALENDAR_NEXT, 1, 0);
        const wchar_t* weekdays[7] = { L"Mo", L"Tu", L"We", L"Th", L"Fr", L"Sa", L"Su" };
        for (int i = 0; i < 7; ++i) {
            label(15 + i * 30, 2562, 30, 18, weekdays[i], font::Small, theme::TextSecondary,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        const int lead = (FirstWeekday(m_calYear, m_calMonth) + 6) % 7;
        const int days = DaysInMonth(m_calYear, m_calMonth);
        for (int day = 1; day <= days; ++day) {
            const int cellIndex = lead + day - 1;
            const int x = 15 + (cellIndex % 7) * 30;
            const int y = 2584 + (cellIndex / 7) * 26;
            const bool selected = m_dateText == FormatDateText(m_calYear, m_calMonth, day);
            const bool hovered = IsHovered(K_CALENDAR_DAY, 1, day);
            if (selected) {
                FillRoundRectBorder(hDC, RC(x, y, 30, 24), P(2), theme::Primary, theme::Primary, 1);
            }
            else if (hovered) {
                FillRoundRectBorder(hDC, RC(x, y, 30, 24), P(2), TP_PrimaryBg, 0, 0);
            }
            wchar_t dayText[8] = {};
            swprintf_s(dayText, L"%d", day);
            label(x, y, 30, 24, dayText, font::Body14,
                selected ? theme::White : (hovered ? theme::Primary : theme::TextPrimary),
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            AddHotspot(RC(x, y, 30, 26), K_CALENDAR_DAY, 1, day);
        }
    }

    if (m_menuHOpen) {
        const int dpi = UiScale::Dpi();
        const int workspaceX = MulDiv(textW(L"Processing Center", font::Body14), 96, dpi) + 40;
        const wchar_t* popupItems[4] = { L"item one", L"item two", L"item three", L"item four" };
        RECT panel = RC(workspaceX, 5553, 200, 168);
        shadow(panel, P(4));
        FillRoundRectBorder(hDC, panel, P(4), theme::White, theme::BorderLight, 1);
        for (int i = 0; i < 4; ++i) {
            const int rowY = 5557 + i * 40;
            const bool active = m_menuHPopup == i;
            const bool hovered = IsHovered(K_MENU_H, 2, i);
            if (hovered) {
                FillSolidRect(hDC, RC(workspaceX + 4, rowY, 192, 40), TP_PrimaryBg);
            }
            label(workspaceX + 20, rowY, 146, 40, popupItems[i], font::Body14,
                (active || hovered) ? theme::Primary : theme::TextRegular);
            if (i == 3) {
                label(workspaceX + 164, rowY, 16, 40, L"\uE76C", font::ThIcon12,
                    hovered ? theme::Primary : theme::TextSecondary,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            AddHotspot(RC(workspaceX, rowY, 200, 40), K_MENU_H, 2, i);
        }
        if (m_menuHSubOpen) {
            const int subX = workspaceX + 200;
            const int subY = 5557 + 3 * 40;
            RECT sub = RC(subX, subY, 200, 128);
            shadow(sub, P(4));
            FillRoundRectBorder(hDC, sub, P(4), theme::White, theme::BorderLight, 1);
            for (int i = 0; i < 3; ++i) {
                const int rowY = subY + 4 + i * 40;
                const bool active = m_menuHSub == i;
                const bool hovered = IsHovered(K_MENU_H, 3, i);
                if (hovered) {
                    FillSolidRect(hDC, RC(subX + 4, rowY, 192, 40), TP_PrimaryBg);
                }
                label(subX + 20, rowY, 150, 40, popupItems[i], font::Body14,
                    (active || hovered) ? theme::Primary : theme::TextRegular);
                AddHotspot(RC(subX, rowY, 200, 40), K_MENU_H, 3, i);
            }
        }
    }

    return true;
}

}
