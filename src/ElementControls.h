#pragma once

#include "UIlib.h"

#include <string>
#include <vector>

struct IStream;

namespace el {

namespace theme {
constexpr DWORD Primary = 0xFF409EFF;
constexpr DWORD PrimaryLight = 0xFF66B1FF;
constexpr DWORD PrimaryDark = 0xFF3A8EE6;
constexpr DWORD PrimaryBg = 0xFFECF5FF;
constexpr DWORD PrimaryBorder = 0xFFB3D8FF;
constexpr DWORD PrimaryDisabled = 0xFFA0CFFF;

constexpr DWORD Success = 0xFF67C23A;
constexpr DWORD SuccessLight = 0xFF85CE61;
constexpr DWORD SuccessDark = 0xFF5DAF34;
constexpr DWORD SuccessBg = 0xFFF0F9EB;
constexpr DWORD SuccessBorder = 0xFFC2E7B0;

constexpr DWORD Warning = 0xFFE6A23C;
constexpr DWORD WarningLight = 0xFFEBB563;
constexpr DWORD WarningDark = 0xFFCF9236;
constexpr DWORD WarningBg = 0xFFFDF6EC;
constexpr DWORD WarningBorder = 0xFFF5DAB1;

constexpr DWORD Danger = 0xFFF56C6C;
constexpr DWORD DangerLight = 0xFFF78989;
constexpr DWORD DangerDark = 0xFFDD6161;
constexpr DWORD DangerBg = 0xFFFEF0F0;
constexpr DWORD DangerBorder = 0xFFFBC4C4;

constexpr DWORD Info = 0xFF909399;
constexpr DWORD InfoLight = 0xFFA6A9AD;
constexpr DWORD InfoDark = 0xFF82848A;
constexpr DWORD InfoBg = 0xFFF4F4F5;
constexpr DWORD InfoBorder = 0xFFD3D4D6;

constexpr DWORD TextPrimary = 0xFF303133;
constexpr DWORD TextRegular = 0xFF606266;
constexpr DWORD TextSecondary = 0xFF909399;
constexpr DWORD TextPlaceholder = 0xFFC0C4CC;
constexpr DWORD BorderBase = 0xFFDCDFE6;
constexpr DWORD BorderLight = 0xFFE4E7ED;
constexpr DWORD BorderLighter = 0xFFEBEEF5;
constexpr DWORD FillLight = 0xFFF5F7FA;
constexpr DWORD FillLighter = 0xFFFAFAFA;
constexpr DWORD White = 0xFFFFFFFF;

constexpr DWORD RateGold = 0xFFF7BA2A;
constexpr DWORD RateVoid = 0xFFC6D1DE;
}

namespace font {
enum {
    Body = 0,
    Small = 1,
    BodyBold = 2,
    PageTitle = 3,
    Section = 4,
    Body14 = 5,
    Icon = 6,
    Group = 7,
    IconSmall = 8,
    H1 = 9,
    H2 = 10,
    Body14Bold = 11,
    Th20 = 12,
    Th18 = 13,
    Th16 = 14,
    Th15 = 15,
    Th12Bold = 16,
    Th11 = 17,
    ThIcon18 = 18,
    ThIcon12 = 19,
    Icon24 = 20,
};
}

class EButtonUI : public DuiLib::CButtonUI
{
public:
    enum class Variant { Default, Primary, Success, Warning, Danger, Info, Text };
    enum class Size { Small, Normal, Large };

    EButtonUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetVariant(Variant variant);
    void SetPlain(bool plain);
    void SetButtonSize(Size size);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
    void PaintBorder(HDC hDC) override;

private:
    void ApplyStyle();
    DWORD BackgroundColor() const;
    DWORD BorderColor() const;
    DWORD TextColor() const;

    Variant m_variant;
    bool m_plain;
    Size m_size;
};

class ELinkUI : public DuiLib::CButtonUI
{
public:
    enum class Type { Default, Primary, Success, Warning, Danger, Info };

    ELinkUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetType(Type type);
    void SetUnderline(bool underline);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
    void PaintBorder(HDC hDC) override;

private:
    DWORD TextColor() const;

    Type m_type;
    bool m_underline;
};

class EInputNumberUI : public DuiLib::CHorizontalLayoutUI
{
public:
    EInputNumberUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void SetEnabled(bool bEnable = true) override;

    void DoEvent(DuiLib::TEventUI& event) override;
    void PaintBkColor(HDC hDC) override;
    void PaintBorder(HDC hDC) override;

    int GetValue() const;
    void SetValue(int value);
    void CommitEdit();
    void Step(int delta);

private:
    int Clamp(int value) const;
    void UpdateEditText();

    DuiLib::CEditUI* m_edit;
    int m_value;
    int m_min;
    int m_max;
    int m_step;
};

class EInputNumberEditUI : public DuiLib::CEditUI
{
public:
    explicit EInputNumberEditUI(EInputNumberUI* owner);

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void Commit();

private:
    EInputNumberUI* m_owner;
};

struct CascaderNode
{
    std::wstring name;
    std::vector<CascaderNode> children;
    std::wstring desc;
    bool disabled = false;
    bool lazy = false;
};

class ECascaderUI : public DuiLib::CControlUI
{
public:
    ECascaderUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void SetEnabled(bool bEnable = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void PaintBkColor(HDC hDC) override;
    void PaintBorder(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

    void SetOptions(const std::vector<CascaderNode>& options);
    const std::vector<CascaderNode>& GetOptions() const;
    std::vector<std::vector<int>> GetSelection() const;
    std::vector<std::vector<std::wstring>> GetSelectionLabels() const;
    void ApplySelection(const std::vector<std::vector<int>>& paths,
        const std::vector<std::vector<std::wstring>>& labels);
    void ClearSelection();

    bool IsMultiple() const;
    bool IsClearable() const;
    bool IsCheckStrictly() const;
    bool IsFilterable() const;
    bool IsLazy() const;
    bool IsShowAllLevels() const;
    bool IsCollapseTags() const;
    void SetOpen(bool open);
    std::wstring GetDisplayText() const;
    std::wstring GetDemoTitle() const;

private:
    bool HasSelection() const;
    RECT ClearRect() const;
    void BuildTagLayout(std::vector<RECT>& tags, std::vector<RECT>& closes,
        std::vector<int>& indexes, int& plusWidth) const;
    int TextWidth(const std::wstring& text, int fontId) const;
    void NotifyChanged();

    std::vector<CascaderNode> m_options;
    std::vector<std::vector<int>> m_paths;
    std::vector<std::vector<std::wstring>> m_labels;
    bool m_multiple;
    bool m_clearable;
    bool m_checkStrictly;
    bool m_filterable;
    bool m_lazy;
    bool m_showAllLevels;
    bool m_collapseTags;
    bool m_open;
    bool m_hover;
    std::wstring m_placeholder;
    std::wstring m_demo;
};

class ECascaderPopupUI : public DuiLib::CContainerUI
{
public:
    ECascaderPopupUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetPos(RECT rc, bool bNeedInvalidate = true) override;
    void SetVisible(bool bVisible = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void PaintBkColor(HDC hDC) override;
    void PaintBorder(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

    void Open(const std::vector<CascaderNode>& options,
        const std::vector<std::vector<int>>& paths,
        const std::vector<std::vector<std::wstring>>& labels,
        bool multiple, bool checkStrictly, bool filterable, bool lazy);
    void SetColumnWidth(int width);
    int ContentWidth() const;
    int ContentHeight() const;
    void SetQuery(const std::wstring& query);
    void FocusSearch();
    DuiLib::CEditUI* SearchEdit() const;
    std::vector<std::vector<int>> GetSelection() const;
    std::vector<std::vector<std::wstring>> GetSelectionLabels() const;
    bool IsMultiple() const;

private:
    struct HitRow
    {
        int column = -1;
        int row = -1;
        int nodeIndex = -1;
        std::vector<int> path;
    };

    int ColumnsTop() const;
    int VisibleColumns() const;
    int ColumnX(int column) const;
    RECT ColumnRect(int column) const;
    int RowHeight() const;
    int ViewRows() const;
    int MaxRows() const;
    int ScrollAt(int column) const;
    void SetScroll(int column, int value);
    const std::vector<CascaderNode>* NodesAt(const std::vector<int>& path) const;
    std::vector<CascaderNode>* MutableNodesAt(const std::vector<int>& path);
    const std::vector<CascaderNode>* ColumnNodes(int column) const;
    std::vector<CascaderNode>* MutableColumnNodes(int column);
    bool HasChildren(const CascaderNode& node) const;
    void BrowseTo(const std::vector<int>& path);
    void TogglePath(const std::vector<int>& path, const std::vector<std::wstring>& labels);
    bool IsPathSelected(const std::vector<int>& path) const;
    void CollectResults(const std::vector<CascaderNode>& nodes, std::vector<int> prefix,
        std::vector<std::wstring> names);
    void BuildSearchResults();
    void StartLazy(const std::vector<int>& path);
    void FinishLazy();
    HitRow HitTest(const POINT& pt) const;
    int SearchAreaHeight() const;
    void NotifyPick();
    void LayoutSearchEdit();
    void UpdatePanelSize();

    std::vector<CascaderNode> m_options;
    std::vector<int> m_browse;
    std::vector<std::vector<int>> m_paths;
    std::vector<std::vector<std::wstring>> m_labels;
    std::vector<std::vector<int>> m_results;
    std::vector<std::vector<std::wstring>> m_resultLabels;
    std::vector<int> m_scrolls;
    std::vector<int> m_loadingPath;
    std::wstring m_query;
    bool m_multiple;
    bool m_checkStrictly;
    bool m_filterable;
    bool m_lazy;
    bool m_loading;
    int m_hoverColumn;
    int m_hoverRow;
    int m_rowHeight;
    int m_colWidth;
    int m_pad;
    int m_searchHeight;
    DuiLib::CEditUI* m_edit;
};

class ESwitchUI : public DuiLib::CButtonUI
{
public:
    ESwitchUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void SetChecked(bool checked);
    bool IsChecked() const;

    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

private:
    bool m_checked;
};

class ERateUI : public DuiLib::CControlUI
{
public:
    ERateUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void DoEvent(DuiLib::TEventUI& event) override;
    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;

    void SetMax(int max);
    void SetValue(double value);
    void SetAllowHalf(bool allowHalf);
    double GetValue() const;

private:
    double ValueAt(int x) const;
    int StarSize() const;
    int StarGap() const;
    void DrawStar(HDC hDC, const RECT& star, double fill) const;

    int m_max;
    double m_value;
    double m_hover;
    bool m_allowHalf;
};

class EPaginationUI : public DuiLib::CControlUI
{
public:
    EPaginationUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void DoEvent(DuiLib::TEventUI& event) override;
    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

    int GetCurrentPage() const;
    void SetCurrentPage(int page);
    void SetTotal(int total);
    int GetPageCount() const;

private:
    struct Cell
    {
        RECT rc;
        int page;
        bool nav;
        bool next;
        bool hover;
    };

    int PageCount() const;
    void BuildCells(std::vector<Cell>& cells) const;
    int HitTest(const POINT& pt) const;
    void ChangePage(int page);

    int m_total;
    int m_pageSize;
    int m_current;
    int m_hover;
};

class EBadgeUI : public DuiLib::CContainerUI
{
public:
    EBadgeUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetManager(DuiLib::CPaintManagerUI* pManager, DuiLib::CControlUI* pParent, bool bInit = true) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    SIZE EstimateSize(SIZE szAvailable) override;
    void SetPos(RECT rc, bool bNeedInvalidate = true) override;
    bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;
    void ScaleForDpi();

private:
    DWORD BadgeColor() const;
    std::wstring BadgeText() const;
    bool HasBadge() const;
    int BadgeWidth() const;
    int BadgeHeight() const;
    int BadgeOverlap() const;
    int BadgeOverhead() const;
    void UpdateOuterSize();

    int m_value;
    std::wstring m_valueText;
    bool m_numeric;
    int m_max;
    bool m_dot;
    int m_type;
    int m_contentWidth;
    int m_contentHeight;
    RECT m_rcContent;
};

class EAvatarUI : public DuiLib::CControlUI
{
public:
    EAvatarUI();
    ~EAvatarUI() override;

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void PaintStatusImage(HDC hDC) override;

private:
    DWORD BackgroundColor() const;
    Gdiplus::Bitmap* BitmapFor(int size);
    void FillShape(HDC hDC, Gdiplus::Graphics& graphics, const Gdiplus::GraphicsPath& path, int size);

    int m_type;
    bool m_square;
    bool m_icon;
    std::wstring m_src;
    Gdiplus::Bitmap* m_bitmap;
    int m_bitmapSize;
};

class EIconImageUI : public DuiLib::CControlUI
{
public:
    EIconImageUI();
    ~EIconImageUI() override;

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void PaintStatusImage(HDC hDC) override;

    void SetResourceId(int resourceId);
    void SetTintColor(DWORD argb);

private:
    Gdiplus::Bitmap* EnsureBitmap();
    Gdiplus::Bitmap* DecodeFromStream();

    int m_resourceId;
    DWORD m_tint;
    bool m_loaded;
    Gdiplus::Bitmap* m_bitmap;
    IStream* m_stream;
};

class ELoadingUI : public DuiLib::CControlUI
{
public:
    ELoadingUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetManager(DuiLib::CPaintManagerUI* pManager, DuiLib::CControlUI* pParent, bool bInit = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;
    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

private:
    int m_angle;
    bool m_timerStarted;
};

class EBreadcrumbUI : public DuiLib::CHorizontalLayoutUI
{
public:
    EBreadcrumbUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetItems(const std::vector<std::wstring>& items);
    void SetCurrent(int index);
    int GetCurrent() const;

private:
    void Rebuild();

    std::vector<std::wstring> m_items;
    std::vector<DuiLib::CControlUI*> m_itemControls;
    int m_current;
};

class EStepsUI : public DuiLib::CControlUI
{
public:
    EStepsUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void PaintStatusImage(HDC hDC) override;

    void SetActive(int active);
    int GetActive() const;

private:
    std::vector<std::wstring> m_steps;
    int m_active;
};

class EPopoverUI : public DuiLib::CContainerUI
{
public:
    EPopoverUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;

private:
    int m_arrow;
};

class ECollapseUI : public DuiLib::CVerticalLayoutUI
{
public:
    ECollapseUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetTitle(const std::wstring& title);
    void SetExpanded(bool expanded);
    bool IsExpanded() const;
    void Toggle();
    DuiLib::CVerticalLayoutUI* Body() const;
    void SetHeaderHeight(int height);

private:
    void UpdateHeight();

    DuiLib::CButtonUI* m_header;
    DuiLib::CVerticalLayoutUI* m_body;
    bool m_expanded;
    std::wstring m_title;
};

class EInfiniteListUI : public DuiLib::CListUI
{
public:
    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetManager(DuiLib::CPaintManagerUI* pManager, DuiLib::CControlUI* pParent, bool bInit = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;

private:
    void CheckNearBottom();
};

struct SelectOption
{
    std::wstring label;
    std::wstring desc;
    bool disabled = false;
    bool group = false;
};

class ESelectUI : public DuiLib::CControlUI
{
public:
    ESelectUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
    void SetEnabled(bool bEnable = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void PaintBkColor(HDC hDC) override;
    void PaintBorder(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

    void AddOption(const std::wstring& label, const std::wstring& desc = L"", bool disabled = false);
    void AddGroup(const std::wstring& label);
    const std::vector<SelectOption>& GetOptions() const;
    std::vector<int> GetSelection() const;
    void ApplySelection(const std::vector<int>& selection);
    void ClearSelection();
    void AddCreated(const std::wstring& label);

    bool IsMultiple() const;
    bool IsClearable() const;
    bool IsSearchable() const;
    bool IsRemote() const;
    bool IsAllowCreate() const;
    void SetOpen(bool open);
    std::wstring GetDisplayText() const;
    std::wstring GetDemoTitle() const;

private:
    int BoxHeight() const;
    int FontForBox() const;
    int TextWidth(const std::wstring& text, int fontId) const;
    const SelectOption* OptionAt(int index) const;
    bool HasSelection() const;
    RECT ClearRect() const;
    void BuildTagLayout(std::vector<RECT>& tags, std::vector<RECT>& closes,
        std::vector<int>& indexes, int& plusWidth) const;
    void NotifyChanged();

    std::vector<SelectOption> m_options;
    std::vector<int> m_selected;
    bool m_multiple;
    bool m_clearable;
    bool m_filterable;
    bool m_remote;
    bool m_allowCreate;
    bool m_open;
    bool m_hover;
    int m_size;
    std::wstring m_placeholder;
    std::wstring m_demo;
};

class ESelectPopupUI : public DuiLib::CContainerUI
{
public:
    ESelectPopupUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void SetPos(RECT rc, bool bNeedInvalidate = true) override;
    void SetVisible(bool bVisible = true) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    void PaintBkColor(HDC hDC) override;
    void PaintBorder(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;

    void Open(const std::vector<SelectOption>& options, const std::vector<int>& selected,
        bool multiple, bool searchable, bool remote);
    int ContentHeight() const;
    void SetQuery(const std::wstring& query);
    void FinishRemote();
    void FocusSearch();
    DuiLib::CEditUI* SearchEdit() const;
    std::vector<int> GetSelection() const;
    bool IsMultiple() const;

private:
    void RebuildRows();
    void UpdatePanelSize();
    void LayoutSearchEdit();
    bool IsLoading() const;
    bool Matches(const std::wstring& label) const;
    int RowsHeight() const;
    int RowsTop() const;
    int RowsViewHeight() const;
    int RowAt(const POINT& pt) const;
    void NotifyPick(int optionIndex);

    std::vector<SelectOption> m_options;
    std::vector<int> m_selected;
    std::vector<int> m_rows;
    std::wstring m_query;
    std::wstring m_pendingQuery;
    bool m_multiple;
    bool m_searchable;
    bool m_remote;
    bool m_loading;
    int m_scroll;
    int m_hover;
    int m_rowHeight;
    int m_groupHeight;
    int m_searchHeight;
    DuiLib::CEditUI* m_edit;
};

class ENavItemUI : public DuiLib::CButtonUI
{
public:
    ENavItemUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetSelected(bool selected);
    bool IsSelected() const;

    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
    void PaintBorder(HDC hDC) override;

private:
    bool m_selected;
};

class ETabButtonUI : public DuiLib::CButtonUI
{
public:
    ETabButtonUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void SetSelected(bool selected);
    bool IsSelected() const;

    void PaintBkColor(HDC hDC) override;
    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
    void PaintBorder(HDC hDC) override;

private:
    bool m_selected;
};

class EOptionUI : public DuiLib::COptionUI
{
public:
    EOptionUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
};

class ECheckBoxUI : public DuiLib::CCheckBoxUI
{
public:
    ECheckBoxUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void PaintStatusImage(HDC hDC) override;
    void PaintText(HDC hDC) override;
};

class EProgressUI : public DuiLib::CProgressUI
{
public:
    EProgressUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void PaintStatusImage(HDC hDC) override;
};

class ESliderUI : public DuiLib::CSliderUI
{
public:
    ESliderUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;

    void PaintStatusImage(HDC hDC) override;
};

class EDateTimeUI : public DuiLib::CDateTimeUI
{
public:
    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    void DoEvent(DuiLib::TEventUI& event) override;
};

class ETreeUI : public DuiLib::CControlUI
{
public:
    ETreeUI();
    ~ETreeUI() override;

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    UINT GetControlFlags() const override;
    void DoEvent(DuiLib::TEventUI& event) override;
    void PaintStatusImage(HDC hDC) override;

    void AddNode(const std::wstring& id, const std::wstring& parentId, const std::wstring& label,
        bool disabled = false, bool isLeaf = false);
    void ExpandKey(const std::wstring& id);
    void CheckKey(const std::wstring& id);
    void SelectKey(const std::wstring& id);
    void SetFilterText(const std::wstring& text);
    void SetShowCheckbox(bool show);
    void SetAccordion(bool accordion);
    void SetDraggable(bool draggable);
    void SetLazy(bool lazy);
    void SetShowPlus(bool show);
    void SetRowHeight(int height);
    void UpdateHeight();

private:
    struct TreeNode
    {
        std::wstring id;
        std::wstring label;
        bool disabled = false;
        bool isLeaf = false;
        bool expanded = false;
        bool checked = false;
        bool indeterminate = false;
        bool loading = false;
        bool loaded = true;
        TreeNode* parent = nullptr;
        std::vector<TreeNode*> children;
        int depth = 0;
    };

    struct TreeRow
    {
        TreeNode* node;
        bool loading;
    };

    TreeNode* FindNode(TreeNode* node, const std::wstring& id) const;
    bool SubtreeMatches(TreeNode* node) const;
    bool LabelMatches(TreeNode* node) const;
    void CollectRows(TreeNode* node, std::vector<TreeRow>& rows) const;
    void RebuildRows();
    int FindRowAt(const POINT& pt) const;
    RECT RowRect(int index) const;
    void PaintRow(HDC hDC, const TreeRow& row, int index);
    void ToggleExpand(TreeNode* node);
    void SetSubtreeChecked(TreeNode* node, bool checked);
    void UpdateAncestors(TreeNode* node);
    void AppendChild(TreeNode* parent);
    void FinishLazyLoad();
    void MoveNode(TreeNode* node, TreeNode* target);
    void UpdateDepths(TreeNode* node, int depth);
    bool IsAncestor(TreeNode* ancestor, TreeNode* node) const;
    TreeNode* Detach(TreeNode* node);

    TreeNode* m_root;
    std::vector<TreeRow> m_rows;
    std::wstring m_filter;
    TreeNode* m_selected;
    TreeNode* m_hover;
    TreeNode* m_drag;
    TreeNode* m_drop;
    bool m_dragging;
    bool m_dragMoved;
    POINT m_dragStart;
    bool m_showCheckbox;
    bool m_accordion;
    bool m_draggable;
    bool m_lazy;
    bool m_showPlus;
    bool m_loadingActive;
    int m_rowHeight;
    int m_appended;
};

class EThemePreviewUI : public DuiLib::CControlUI
{
public:
    EThemePreviewUI();

    LPCTSTR GetClass() const override;
    LPVOID GetInterface(LPCTSTR pstrName) override;
    SIZE EstimateSize(SIZE szAvailable) override;
    bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;
    void DoEvent(DuiLib::TEventUI& event) override;

    static int PreferredHeight();

private:
    struct Hotspot
    {
        RECT rc;
        int kind;
        int group;
        int index;
    };

    void AddHotspot(const RECT& rc, int kind, int group, int index);
    const Hotspot* HitTestPoint(const POINT& pt) const;
    bool IsHovered(int kind, int group, int index) const;
    bool IsPressed(int kind, int group, int index) const;
    void UpdateHover(const POINT& pt);
    void HandleClick(const Hotspot& hot);
    void ClosePopups();
    void SyncHeight();

    std::vector<Hotspot> m_hotspots;
    int m_hoverKind;
    int m_hoverGroup;
    int m_hoverIndex;
    int m_pressKind;
    int m_pressGroup;
    int m_pressIndex;
    bool m_pressed;
    bool m_dragSlider;

    int m_radioBasic;
    int m_radioCity;
    int m_radioBordered;
    bool m_checkBasic;
    int m_checkCityMask;
    bool m_checkBordered;
    bool m_switchYear;
    bool m_switchMonth;
    int m_sliderValue;
    std::wstring m_inputText;
    bool m_inputFocused;
    int m_number;
    bool m_selectOpen;
    int m_selectIndex;
    bool m_cascaderOpen;
    int m_cascaderProvince;
    int m_cascaderCity;
    bool m_dateOpen;
    int m_calYear;
    int m_calMonth;
    std::wstring m_dateText;
    int m_rateValue;
    int m_rateHoverRow;
    int m_rateHoverValue;
    int m_tabTop;
    int m_tabCard;
    int m_menuH;
    int m_menuV;
    int m_page;
    int m_collapseMask;
    int m_carousel;
    bool m_tagVisible[5];
    int m_extraHeight;
    int m_pageExtra;
    int m_treeOpenMask;
    int m_treeSelNode;
    int m_treeExtra;
    int m_menuExtra;
    bool m_menuHOpen;
    bool m_menuHSubOpen;
    int m_menuHPopup;
    int m_menuHSub;
    int m_menuVOpenMask;
};

class EControlFactory : public DuiLib::IDialogBuilderCallback
{
public:
    DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass) override;
};

void ScaleTree(DuiLib::CControlUI* control);
DuiLib::CLabelUI* MakeLabel(const std::wstring& text, int fontId, DWORD color);
DuiLib::CLabelUI* MakeLabelW(const std::wstring& text, int fontId, DWORD color, int width);
DuiLib::CLabelUI* MakeLabelH(const std::wstring& text, int fontId, DWORD color, int height);
DuiLib::CLabelUI* MakeLabelWH(const std::wstring& text, int fontId, DWORD color, int width, int height);

}
