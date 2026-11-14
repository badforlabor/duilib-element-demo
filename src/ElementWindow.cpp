#include "ElementWindow.h"
#include "ElementControls.h"
#include "IconCatalog.h"
#include "Resource.h"
#include "UiScale.h"

#include <algorithm>
#include <commctrl.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

namespace el {
using namespace DuiLib;

namespace {

enum NativeCommand : int
{
    kCmdExit = 40001,
    kCmdModeComponent = 40011,
    kCmdModeTheme = 40012,
    kCmdModeIcon = 40013,
    kCmdAbout = 40021,
    kCmdCrash = 40031,
};

const int kCmdFirst = 40000;
const int kCmdLast = 40099;
const int kStatusBarId = 100;

std::wstring ToW(int value)
{
    wchar_t buffer[24] = {};
    ::_snwprintf_s(buffer, _TRUNCATE, L"%d", value);
    return buffer;
}

std::wstring PageXml(const std::wstring& id);

const wchar_t* kOverviewRows =
    L"基础|Layout 布局|部分|HorizontalLayout/VerticalLayout/TileLayout；无 24 栅格\n"
    L"基础|Container 容器|已有|Container 与三种 Layout；inset/padding\n"
    L"基础|Color 色彩|已有|bkcolor/bordercolor/textcolor 样式体系\n"
    L"基础|Typography 字体|已有|Label/Text + Font 资源\n"
    L"基础|Icon 图标|部分|Label 符号字体或图片；无内置图标集\n"
    L"基础|Button 按钮|已有|Button + borderround + 状态色\n"
    L"基础|Link 文字链接|已有|ELink 子类：主色/悬停下划线/手型/禁用\n"
    L"表单|Radio 单选框|已有|Option + group 分组互斥\n"
    L"表单|Checkbox 多选框|已有|CheckBox（继承 Option）\n"
    L"表单|Input 输入框|已有|Edit 原生子窗口；placeholder 需扩展\n"
    L"表单|InputNumber 计数器|已有|EInputNumber：数字输入 + 步进区 + min/max/step\n"
    L"表单|Select 选择器|已有|ESelect：单选/多选标签/可清空/可搜索/分组/尺寸\n"
    L"表单|Cascader 级联选择器|已有|ECascader：浮层多列联动 + 禁用/清空/搜索/多选/懒加载\n"
    L"表单|Switch 开关|已有|ESwitch：自绘轨道/滑块 + 点击切换\n"
    L"表单|Slider 滑块|已有|Slider 拖动内置；轨道需自绘\n"
    L"表单|TimePicker 时间选择器|部分|DateTime 选时间；面板为系统样式\n"
    L"表单|DatePicker 日期选择器|部分|DateTime 系统日历面板\n"
    L"表单|DateTimePicker 日期时间选择器|部分|DateTime 日期+时间\n"
    L"表单|Upload 上传|需开发|按钮 + 文件对话框 + 列表\n"
    L"表单|Rate 评分|已有|ERate：五角星自绘 + 悬停预览/半星\n"
    L"表单|ColorPicker 颜色选择器|需开发|需取色面板/色板控件\n"
    L"表单|Transfer 穿梭框|需开发|双列表 + 移动逻辑\n"
    L"表单|Form 表单|需开发|校验/联动/错误提示需自建\n"
    L"数据展示|Table 表格|已有|List + Header + ListTextElement\n"
    L"数据展示|Tag 标签|部分|Label + borderround；动态增删需扩展\n"
    L"数据展示|Progress 进度条|已有|Progress 值/范围内置；圆角需定制\n"
    L"数据展示|Tree 树形控件|已有|ETree：自绘树，勾选联动/懒加载/过滤/手风琴/拖拽\n"
    L"数据展示|Pagination 分页|已有|EPagination：页码组 + 省略号 + 上/下一页\n"
    L"数据展示|Badge 标记|已有|EBadge：角标压住内容右上角（水平内缩 10px、垂直居中于上边缘），容器按内容+角标 AABB 布局\n"
    L"数据展示|Avatar 头像|已有|EAvatar：GDI+ 圆形/方形裁剪，图片/文字/图标\n"
    L"通知|Alert 警告|需开发|Container + Label 可近似\n"
    L"通知|Loading 加载|已有|ELoading：抗锯齿旋转弧线，定时器驱动\n"
    L"通知|Message 消息提示|需开发|需顶部浮层队列\n"
    L"通知|MessageBox 弹框|需开发|需独立窗口或浮层面板\n"
    L"通知|Notification 通知|需开发|需右上角浮层队列\n"
    L"导航|NavMenu 导航菜单|需开发|静态菜单可拼，折叠联动需自建\n"
    L"导航|Tabs 标签页|已有|TabLayout 切换 + Button 标签头\n"
    L"导航|Breadcrumb 面包屑|已有|EBreadcrumb：路径 + 分隔符，点击切换层级\n"
    L"导航|PageHeader 页头|需开发|标题/返回/操作区组合\n"
    L"导航|Dropdown 下拉菜单|需开发|Combo 仅做值选择\n"
    L"导航|Steps 步骤条|已有|ESteps：节点圆环/勾选 + 连接线\n"
    L"其他|Dialog 对话框|需开发|CWindowWnd 或浮层自建\n"
    L"其他|Tooltip 文字提示|部分|SetToolTip 原生系统 tooltip\n"
    L"其他|Popover 弹出框|已有|EPopover：浮层气泡 + 箭头，点击外部关闭\n"
    L"其他|Popconfirm 气泡确认框|需开发|Popover + 按钮区\n"
    L"其他|Card 卡片|部分|Container + borderround 静态卡片\n"
    L"其他|Carousel 走马灯|需开发|分页指示与切换动画\n"
    L"其他|Collapse 折叠面板|已有|ECollapse：标题栏 + 展开/收起动画布局\n"
    L"其他|Timeline 时间线|需开发|节点/轴线绘制\n"
    L"其他|Divider 分割线|已有|Control 1px + bkcolor\n"
    L"其他|Calendar 日历|部分|DateTime 系统日历弹窗\n"
    L"其他|Image 图片|已有|Control bkimage / GifAnim\n"
    L"其他|Backtop 回到顶部|需开发|滚动监听 + 悬浮按钮\n"
    L"其他|InfiniteScroll 无限滚动|已有|EInfiniteList：滚动到底发送 loadmore\n"
    L"其他|Drawer 抽屉|需开发|侧滑面板 + 遮罩";

struct IconItem
{
    wchar_t glyph;
    const wchar_t* name;
};

const IconItem kIcons[] = {
    {0xE700, L"GlobalNavButton"},
    {0xE70F, L"Edit"},
    {0xE713, L"Settings"},
    {0xE721, L"Search"},
    {0xE72C, L"Refresh"},
    {0xE73E, L"CheckMark"},
    {0xE74D, L"Delete"},
    {0xE74E, L"Save"},
    {0xE768, L"Play"},
    {0xE769, L"Pause"},
    {0xE71E, L"ZoomIn"},
    {0xE8B7, L"Folder"},
    {0xE8A5, L"Document"},
    {0xE8C8, L"Copy"},
    {0xE7C3, L"Page"},
    {0xE72D, L"Share"},
};

struct NavEntry
{
    const wchar_t* group;
    const wchar_t* id;
    const wchar_t* label;
};

const NavEntry kNavEntries[] = {
    {L"总览", L"overview", L"组件对照表"},
    {L"基础", L"color", L"Color 色彩"},
    {L"基础", L"typography", L"Typography 字体"},
    {L"基础", L"icon", L"Icon 图标"},
    {L"基础", L"button", L"Button 按钮"},
    {L"基础", L"link", L"Link 文字链接"},
    {L"基础", L"container", L"Container 容器"},
    {L"基础", L"layout", L"Layout 布局"},
    {L"基础", L"divider", L"Divider 分割线"},
    {L"基础", L"tag", L"Tag 标签"},
    {L"基础", L"image", L"Image 图片"},
    {L"表单", L"radio", L"Radio 单选框"},
    {L"表单", L"checkbox", L"Checkbox 多选框"},
    {L"表单", L"input", L"Input 输入框"},
    {L"表单", L"inputnumber", L"InputNumber 计数器"},
    {L"表单", L"select", L"Select 选择器"},
    {L"表单", L"cascader", L"Cascader 级联选择器"},
    {L"表单", L"switch", L"Switch 开关"},
    {L"表单", L"slider", L"Slider 滑块"},
    {L"表单", L"rate", L"Rate 评分"},
    {L"表单", L"datetime", L"DateTime 日期时间"},
    {L"数据", L"table", L"Table 表格"},
    {L"数据", L"progress", L"Progress 进度条"},
    {L"数据", L"tree", L"Tree 树形控件"},
    {L"数据", L"pagination", L"Pagination 分页"},
    {L"数据", L"badge", L"Badge 标记"},
    {L"数据", L"avatar", L"Avatar 头像"},
    {L"导航", L"tabs", L"Tabs 标签页"},
    {L"导航", L"breadcrumb", L"Breadcrumb 面包屑"},
    {L"导航", L"steps", L"Steps 步骤条"},
    {L"反馈", L"tooltip", L"Tooltip 文字提示"},
    {L"反馈", L"loading", L"Loading 加载"},
    {L"其他", L"popover", L"Popover 弹出框"},
    {L"其他", L"collapse", L"Collapse 折叠面板"},
    {L"其他", L"infinite", L"InfiniteScroll 无限滚动"},
};

std::vector<std::wstring> Split(const std::wstring& text, wchar_t separator)
{
    std::vector<std::wstring> parts;
    size_t begin = 0;
    while (begin <= text.size()) {
        const size_t end = text.find(separator, begin);
        if (end == std::wstring::npos) {
            parts.push_back(text.substr(begin));
            break;
        }
        parts.push_back(text.substr(begin, end - begin));
        begin = end + 1;
    }
    return parts;
}

} // namespace

CMainWindow::CMainWindow()
    : m_nativeMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_statusBar(nullptr)
    , m_navHost(nullptr)
    , m_pageHost(nullptr)
    , m_componentHost(nullptr)
    , m_themeHost(nullptr)
    , m_iconHost(nullptr)
    , m_editMenuButton(nullptr)
    , m_editMenu(nullptr)
    , m_demoMode(DemoMode::Component)
    , m_popoverVisible(false)
    , m_infiniteCount(0)
    , m_activeSelect(nullptr)
    , m_selectPopup(nullptr)
    , m_activeCascader(nullptr)
    , m_cascaderPopup(nullptr)
{
}

LPCTSTR CMainWindow::GetWindowClassName() const
{
    return _T("DuiLibElementWindow");
}

void CMainWindow::OnFinalMessage(HWND hWnd)
{
    (void)hWnd;
    delete this;
}

void CMainWindow::CreateNativeMenu()
{
    m_nativeMenu = ::CreateMenu();
    if (m_nativeMenu == nullptr) {
        return;
    }

    HMENU fileMenu = ::CreatePopupMenu();
    ::AppendMenuW(fileMenu, MF_STRING, kCmdExit, L"退出(&X)\tAlt+F4");
    ::AppendMenuW(m_nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"文件(&F)");

    m_viewMenu = ::CreatePopupMenu();
    ::AppendMenuW(m_viewMenu, MF_STRING, kCmdModeComponent, L"组件演示(&C)");
    ::AppendMenuW(m_viewMenu, MF_STRING, kCmdModeTheme, L"主题演示(&T)");
    ::AppendMenuW(m_viewMenu, MF_STRING, kCmdModeIcon, L"图标演示(&I)");
    ::AppendMenuW(m_nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(m_viewMenu), L"视图(&V)");

    HMENU debugMenu = ::CreatePopupMenu();
    ::AppendMenuW(debugMenu, MF_STRING, kCmdCrash, L"触发崩溃（生成 dump）(&C)");
    ::AppendMenuW(m_nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(debugMenu), L"调试(&D)");

    HMENU helpMenu = ::CreatePopupMenu();
    ::AppendMenuW(helpMenu, MF_STRING, kCmdAbout, L"关于 DuiLib Element(&A)");
    ::AppendMenuW(m_nativeMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(helpMenu), L"帮助(&H)");

    UpdateNativeMenuState();
}

void CMainWindow::CreateStatusBar()
{
    m_statusBar = ::CreateWindowExW(0, STATUSCLASSNAMEW, nullptr,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | SBARS_SIZEGRIP,
        0, 0, 0, 0, m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStatusBarId)),
        CPaintManagerUI::GetInstance(), nullptr);
    if (m_statusBar == nullptr) {
        return;
    }
    ::SendMessage(m_statusBar, WM_SIZE, 0, 0);
}

void CMainWindow::UpdateNativeMenuState()
{
    if (m_viewMenu == nullptr) {
        return;
    }
    UINT checkId = kCmdModeComponent;
    if (m_demoMode == DemoMode::Theme) {
        checkId = kCmdModeTheme;
    }
    else if (m_demoMode == DemoMode::Icon) {
        checkId = kCmdModeIcon;
    }
    ::CheckMenuRadioItem(m_viewMenu, kCmdModeComponent, kCmdModeIcon, checkId, MF_BYCOMMAND);
}

void CMainWindow::UpdateStatusBar()
{
    if (m_statusBar == nullptr) {
        return;
    }

    RECT client = {};
    ::GetClientRect(m_hWnd, &client);
    int parts[3] = { client.right * 55 / 100, client.right * 76 / 100, -1 };
    ::SendMessage(m_statusBar, SB_SETPARTS, 3, reinterpret_cast<LPARAM>(parts));

    std::wstring current = L"组件演示";
    if (m_demoMode == DemoMode::Theme) {
        current = L"主题演示";
    }
    else if (m_demoMode == DemoMode::Icon) {
        current = L"图标演示";
    }
    else {
        for (const NavEntry& entry : kNavEntries) {
            if (m_currentPage == entry.id) {
                current += L" / ";
                current += entry.label;
                break;
            }
        }
    }

    ::SendMessage(m_statusBar, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(L"就绪"));
    ::SendMessage(m_statusBar, SB_SETTEXTW, 1, reinterpret_cast<LPARAM>(current.c_str()));
    ::SendMessage(m_statusBar, SB_SETTEXTW, 2,
        reinterpret_cast<LPARAM>(L"Windows 默认菜单与状态栏"));
}

void CMainWindow::OnNativeCommand(int id)
{
    switch (id) {
    case kCmdExit:
        ::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        break;
    case kCmdModeComponent:
        ShowDemoMode(DemoMode::Component);
        break;
    case kCmdModeTheme:
        ShowDemoMode(DemoMode::Theme);
        break;
    case kCmdModeIcon:
        ShowDemoMode(DemoMode::Icon);
        break;
    case kCmdAbout:
        ::MessageBoxW(m_hWnd,
            L"DuiLib Element 组件演示\n\n"
            L"基于 DuiLib 的 Element 风格控件演示。\n"
            L"菜单栏和状态栏为 Win32 原生控件（HMENU + msctls_statusbar32）。\n"
            L"调试 → 触发崩溃 可在 exe 同级的 Saved 目录生成 minidump。",
            L"关于 DuiLib Element", MB_OK | MB_ICONINFORMATION);
        break;
    case kCmdCrash: {
        volatile int* invalid = nullptr;
        *invalid = 1;
        break;
    }
    default:
        break;
    }
}

void CMainWindow::Show()
{
    RECT work = {};
    ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    const int width = (std::min)(UiScale::Px(1200), static_cast<int>(work.right - work.left) - UiScale::Px(40));
    const int height = (std::min)(UiScale::Px(820), static_cast<int>(work.bottom - work.top) - UiScale::Px(40));
    CreateNativeMenu();
    Create(nullptr, _T("DuiLib Element 组件演示"),
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_APPWINDOW,
        0, 0, width, height, m_nativeMenu);
    if (m_hWnd == nullptr) {
        return;
    }

    SetIcon(IDI_APP);
    CenterWindow();
    ShowWindow(true);
}

LRESULT CMainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    BOOL handled = FALSE;
    switch (uMsg) {
    case WM_CREATE:
        result = OnCreate(uMsg, wParam, lParam, handled);
        handled = TRUE;
        break;
    case WM_DESTROY:
        result = OnDestroy(uMsg, wParam, lParam, handled);
        handled = TRUE;
        break;
    case WM_GETMINMAXINFO:
        result = OnGetMinMaxInfo(uMsg, wParam, lParam, handled);
        handled = TRUE;
        break;
    case WM_SIZE:
        if (m_statusBar != nullptr) {
            ::SendMessage(m_statusBar, WM_SIZE, 0, 0);
        }
        UpdateStatusBar();
        break;
    case WM_COMMAND:
        if (lParam == 0) {
            const int id = LOWORD(wParam);
            if (id >= kCmdFirst && id <= kCmdLast) {
                OnNativeCommand(id);
                handled = TRUE;
            }
        }
        break;
    case WM_LBUTTONDOWN: {
        POINT pt = { static_cast<LONG>(static_cast<short>(LOWORD(lParam))),
            static_cast<LONG>(static_cast<short>(HIWORD(lParam))) };
        CloseEditMenuIfOutside(pt);
        ClosePopoverIfOutside(pt);
        CloseSelectIfOutside(pt);
        CloseCascaderIfOutside(pt);
        break;
    }
    default:
        break;
    }

    if (handled) {
        return result;
    }
    if (m_pm.MessageHandler(uMsg, wParam, lParam, result)) {
        return result;
    }
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMainWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    (void)uMsg;
    (void)wParam;
    (void)lParam;
    (void)bHandled;

    m_pm.Init(m_hWnd);
    CreateStatusBar();

    EControlFactory factory;
    CDialogBuilder builder;
    CControlUI* root = builder.Create(MAKEINTRESOURCEW(IDR_SKIN_XML), L"XML", &factory, &m_pm);
    if (root == nullptr) {
        TCHAR message[512] = {};
        builder.GetLastErrorMessage(message, 512);
        ::MessageBoxW(m_hWnd, message[0] != L'\0' ? message : L"界面资源加载失败。",
            L"DuiLib Element", MB_OK | MB_ICONERROR);
        return -1;
    }

    SetupFonts();
    ScaleTree(root);
    m_pm.AttachDialog(root);
    m_pm.AddNotifier(this);

    m_navHost = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("navHost")));
    m_pageHost = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("pageHost")));
    m_componentHost = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("componentHost")));
    m_themeHost = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("themeHost")));
    m_iconHost = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("iconHost")));
    m_editMenuButton = m_pm.FindControl(_T("editMenu"));
    if (m_navHost == nullptr || m_pageHost == nullptr || m_componentHost == nullptr
        || m_themeHost == nullptr || m_iconHost == nullptr || m_editMenuButton == nullptr) {
        ::MessageBoxW(m_hWnd, L"界面控件初始化失败。", L"DuiLib Element", MB_OK | MB_ICONERROR);
        return -1;
    }
    m_pageHost->SetDelayedDestroy(false);

    if (CControlUI* spacer = m_pm.FindControl(_T("statusSpacer"))) {
        int statusHeight = 0;
        if (m_statusBar != nullptr) {
            RECT statusRect = {};
            if (::GetWindowRect(m_statusBar, &statusRect)) {
                statusHeight = statusRect.bottom - statusRect.top;
            }
        }
        spacer->SetFixedHeight(statusHeight);
    }

    BuildNav();
    BuildEditMenu();
    BuildThemeDemo();
    BuildIconPreview();
    ShowPage(L"overview");
    ShowDemoMode(DemoMode::Component);
    return 0;
}

void CMainWindow::BuildThemeDemo()
{
    if (m_themeHost == nullptr) {
        return;
    }
    EThemePreviewUI* preview = new EThemePreviewUI;
    preview->SetName(_T("themePreview"));
    preview->SetFixedHeight(EThemePreviewUI::PreferredHeight());
    m_themeHost->Add(preview);
}

void CMainWindow::BuildIconPreview()
{
    if (m_iconHost == nullptr) {
        return;
    }

    struct IconSpec
    {
        int size;
        int resource;
    };
    static const IconSpec specs[] = {
        {16, IDR_ICON_16},
        {20, IDR_ICON_20},
        {24, IDR_ICON_24},
        {32, IDR_ICON_32},
        {40, IDR_ICON_40},
        {48, IDR_ICON_48},
    };

    CLabelUI* title = MakeLabelH(L"Icon 图标", font::PageTitle, theme::TextPrimary, 34);
    m_iconHost->Add(title);

    CLabelUI* subtitle = MakeLabelH(
        L"building-lock.svg 转为 6 张不同尺寸的 PNG，作为 RCDATA 内嵌进 exe，无需外部文件",
        font::Small, theme::TextSecondary, 22);
    m_iconHost->Add(subtitle);

    CVerticalLayoutUI* card = new CVerticalLayoutUI;
    card->SetBkColor(theme::White);
    card->SetBorderColor(theme::BorderLighter);
    card->SetBorderSize({ UiScale::Px(1), UiScale::Px(1), UiScale::Px(1), UiScale::Px(1) });
    card->SetBorderRound({ UiScale::Px(4), UiScale::Px(4) });
    card->SetInset({ UiScale::Px(20), UiScale::Px(20), UiScale::Px(20), UiScale::Px(20) });
    card->SetChildPadding(UiScale::Px(16));
    m_iconHost->Add(card);

    card->Add(MakeLabelH(L"不同尺寸", font::BodyBold, theme::TextPrimary, 24));

    const int cellHeight = 160;
    const int slotHeight = 64;
    const int cardHeight = 20 + 20 + 24 + 16 + cellHeight + 16 + 20;
    card->SetFixedHeight(UiScale::Px(cardHeight));
    CHorizontalLayoutUI* row = new CHorizontalLayoutUI;
    row->SetFixedHeight(UiScale::Px(cellHeight));
    row->SetChildPadding(UiScale::Px(12));
    card->Add(row);

    auto addSpacer = [](CVerticalLayoutUI* cell, int height) {
        if (height <= 0) {
            return;
        }
        CLabelUI* spacer = new CLabelUI;
        spacer->SetFixedHeight(height);
        cell->Add(spacer);
    };

    for (const IconSpec& spec : specs) {
        CVerticalLayoutUI* cell = new CVerticalLayoutUI;
        cell->SetFixedWidth(UiScale::Px(128));
        cell->SetFixedHeight(UiScale::Px(cellHeight));
        cell->SetChildAlign(DT_CENTER);
        cell->SetChildPadding(UiScale::Px(8));
        cell->SetInset({ 0, UiScale::Px(12), 0, UiScale::Px(12) });
        cell->SetBkColor(theme::FillLight);
        cell->SetBorderColor(theme::BorderLighter);
        cell->SetBorderSize({ UiScale::Px(1), UiScale::Px(1), UiScale::Px(1), UiScale::Px(1) });
        cell->SetBorderRound({ UiScale::Px(6), UiScale::Px(6) });

        CLabelUI* sizeLabel = MakeLabelH(ToW(spec.size) + L" px", font::Group, theme::TextRegular, 20);
        sizeLabel->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        cell->Add(sizeLabel);

        const int iconSize = UiScale::Px(spec.size);
        const int slotSize = UiScale::Px(slotHeight);
        const int topGap = (slotSize - iconSize) / 2;
        addSpacer(cell, topGap);

        EIconImageUI* icon = new EIconImageUI;
        icon->SetResourceId(spec.resource);
        icon->SetTintColor(theme::Primary);
        icon->SetFixedWidth(iconSize);
        icon->SetFixedHeight(iconSize);
        cell->Add(icon);

        addSpacer(cell, slotSize - iconSize - topGap);

        CLabelUI* name = MakeLabelH(L"building-lock", font::Small, theme::TextSecondary, 20);
        name->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        cell->Add(name);

        row->Add(cell);
    }

    card->Add(MakeLabelH(
        L"PNG 统一为白色，绘制时用颜色矩阵着色；按显示尺寸的 4 倍预渲染，保证小尺寸边缘清晰。",
        font::Small, theme::TextPlaceholder, 20));

    m_iconHost->Add(MakeLabelH(
        L"icons 目录下 " + ToW(kCatalogIconCount)
            + L" 个 SVG 已全部转为 24 px PNG，按蓝 / 红 / 绿 / 橘红四色循环着色，并内嵌进 exe。",
        font::Small, theme::TextSecondary, 22));

    CVerticalLayoutUI* catalogCard = new CVerticalLayoutUI;
    catalogCard->SetBkColor(theme::White);
    catalogCard->SetBorderColor(theme::BorderLighter);
    catalogCard->SetBorderSize({ UiScale::Px(1), UiScale::Px(1), UiScale::Px(1), UiScale::Px(1) });
    catalogCard->SetBorderRound({ UiScale::Px(4), UiScale::Px(4) });
    catalogCard->SetInset({ UiScale::Px(20), UiScale::Px(20), UiScale::Px(20), UiScale::Px(20) });
    catalogCard->SetChildPadding(UiScale::Px(12));
    m_iconHost->Add(catalogCard);

    catalogCard->Add(MakeLabelH(L"全部图标（24 px）", font::BodyBold, theme::TextPrimary, 24));

    static const DWORD kTints[] = { theme::Primary, theme::Danger, theme::Success, theme::Warning };
    const int catalogColumns = 8;
    const int catalogCellWidth = 90;
    const int catalogCellHeight = 84;
    const int catalogCellPadding = 10;
    const int catalogRowHeight = 84;
    const int catalogRows = (kCatalogIconCount + catalogColumns - 1) / catalogColumns;
    const int gridHeight = catalogRows * catalogRowHeight + (catalogRows - 1) * UiScale::Px(8);
    const int catalogHeight = 20 + 24 + 12 + 20 + gridHeight;
    catalogCard->SetFixedHeight(UiScale::Px(catalogHeight));

    CVerticalLayoutUI* grid = new CVerticalLayoutUI;
    grid->SetChildPadding(UiScale::Px(8));
    grid->SetFixedHeight(UiScale::Px(gridHeight));
    catalogCard->Add(grid);

    for (int rowIndex = 0; rowIndex < catalogRows; ++rowIndex) {
        CHorizontalLayoutUI* row = new CHorizontalLayoutUI;
        row->SetFixedHeight(UiScale::Px(catalogRowHeight));
        row->SetChildPadding(UiScale::Px(catalogCellPadding));
        grid->Add(row);

        for (int column = 0; column < catalogColumns; ++column) {
            const int index = rowIndex * catalogColumns + column;
            if (index >= kCatalogIconCount) {
                break;
            }
            const CatalogIcon& catalogIcon = kCatalogIcons[index];

            CVerticalLayoutUI* cell = new CVerticalLayoutUI;
            cell->SetFixedWidth(UiScale::Px(catalogCellWidth));
            cell->SetFixedHeight(UiScale::Px(catalogCellHeight));
            cell->SetChildAlign(DT_CENTER);
            cell->SetChildPadding(UiScale::Px(4));
            cell->SetInset({ 0, UiScale::Px(19), 0, UiScale::Px(19) });
            row->Add(cell);

            EIconImageUI* icon = new EIconImageUI;
            icon->SetResourceId(catalogIcon.resourceId);
            icon->SetTintColor(kTints[index % 4]);
            icon->SetFixedWidth(UiScale::Px(24));
            icon->SetFixedHeight(UiScale::Px(24));
            cell->Add(icon);

            CLabelUI* name = MakeLabelH(catalogIcon.name, font::Small, theme::TextSecondary, 18);
            name->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            cell->Add(name);
        }
    }
}

LRESULT CMainWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    (void)uMsg;
    (void)wParam;
    (void)lParam;
    bHandled = FALSE;
    ::PostQuitMessage(0);
    return 0;
}

LRESULT CMainWindow::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    (void)uMsg;
    (void)wParam;
    (void)bHandled;
    auto* info = reinterpret_cast<MINMAXINFO*>(lParam);
    info->ptMinTrackSize.x = UiScale::Px(960);
    info->ptMinTrackSize.y = UiScale::Px(600);
    return 0;
}

void CMainWindow::SetupFonts()
{
    struct FontSpec
    {
        int id;
        int size;
        bool bold;
        const wchar_t* name;
    };

    static const FontSpec fonts[] = {
        {0, 13, false, L"Microsoft YaHei UI"},
        {1, 12, false, L"Microsoft YaHei UI"},
        {2, 13, true, L"Microsoft YaHei UI"},
        {3, 20, true, L"Microsoft YaHei UI"},
        {4, 16, true, L"Microsoft YaHei UI"},
        {5, 14, false, L"Microsoft YaHei UI"},
        {6, 20, false, L"Segoe MDL2 Assets"},
        {7, 12, true, L"Microsoft YaHei UI"},
        {8, 14, false, L"Segoe MDL2 Assets"},
        {9, 28, true, L"Microsoft YaHei UI"},
        {10, 22, true, L"Microsoft YaHei UI"},
        {11, 14, true, L"Microsoft YaHei UI"},
        {12, 20, false, L"Microsoft YaHei UI"},
        {13, 18, false, L"Microsoft YaHei UI"},
        {14, 16, false, L"Microsoft YaHei UI"},
        {15, 15, false, L"Microsoft YaHei UI"},
        {16, 12, true, L"Microsoft YaHei UI"},
        {17, 11, false, L"Microsoft YaHei UI"},
        {18, 18, false, L"Segoe MDL2 Assets"},
        {19, 12, false, L"Segoe MDL2 Assets"},
        {20, 24, false, L"Segoe MDL2 Assets"},
    };

    for (const FontSpec& font : fonts) {
        m_pm.AddFont(font.id, font.name, UiScale::Px(font.size), font.bold, false, false);
    }
}

void CMainWindow::BuildNav()
{
    if (m_navHost == nullptr) {
        return;
    }

    const wchar_t* currentGroup = nullptr;
    for (const NavEntry& entry : kNavEntries) {
        if (currentGroup == nullptr || std::wstring(currentGroup) != entry.group) {
            currentGroup = entry.group;
            CLabelUI* group = MakeLabelH(entry.group, font::Group, theme::TextSecondary, 32);
            group->SetTextPadding({UiScale::Px(16), UiScale::Px(10), 0, 0});
            m_navHost->Add(group);
        }

        ENavItemUI* item = new ENavItemUI;
        item->SetName((std::wstring(L"nav:") + entry.id).c_str());
        item->SetText(entry.label);
        m_navHost->Add(item);
        m_navItems[entry.id] = item;
    }
}

void CMainWindow::BuildEditMenu()
{
    auto* root = dynamic_cast<CContainerUI*>(m_pm.GetRoot());
    if (root == nullptr) {
        return;
    }

    const int inset = UiScale::Px(6);
    const int itemHeight = UiScale::Px(34);

    m_editMenu = new CVerticalLayoutUI;
    m_editMenu->SetName(_T("editMenuPopup"));
    m_editMenu->SetFloat(true);
    m_editMenu->SetVisible(false);
    m_editMenu->SetBkColor(theme::White);
    m_editMenu->SetBorderColor(theme::BorderLight);
    m_editMenu->SetBorderSize({ UiScale::Px(1), UiScale::Px(1), UiScale::Px(1), UiScale::Px(1) });
    m_editMenu->SetBorderRound({ UiScale::Px(6), UiScale::Px(6) });
    m_editMenu->SetInset({ inset, inset, inset, inset });
    m_editMenu->SetChildPadding(UiScale::Px(2));
    m_editMenu->SetFixedWidth(UiScale::Px(168));
    m_editMenu->SetFixedHeight(itemHeight * 3 + UiScale::Px(2) * 2 + inset * 2);
    root->Add(m_editMenu);

    const wchar_t* ids[] = { L"menu:component", L"menu:theme", L"menu:icon" };
    const wchar_t* texts[] = { L"展示组件demo", L"展示主题demo", L"展示icon" };
    for (int i = 0; i < 3; ++i) {
        CButtonUI* item = new CButtonUI;
        item->SetName(ids[i]);
        item->SetText(texts[i]);
        item->SetFont(font::Body);
        item->SetTextColor(theme::TextRegular);
        item->SetHotTextColor(theme::Primary);
        item->SetHotBkColor(theme::FillLight);
        item->SetBkColor(0);
        item->SetBorderSize({ 0, 0, 0, 0 });
        item->SetBorderRound({ UiScale::Px(4), UiScale::Px(4) });
        item->SetTextPadding({ UiScale::Px(10), 0, UiScale::Px(10), 0 });
        item->SetTextStyle(DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        item->SetFixedHeight(itemHeight);
        m_editMenu->Add(item);
    }
    UpdateEditMenuState();
}

void CMainWindow::ShowDemoMode(DemoMode mode)
{
    m_demoMode = mode;
    if (m_componentHost != nullptr) {
        m_componentHost->SetVisible(mode == DemoMode::Component);
    }
    if (m_themeHost != nullptr) {
        m_themeHost->SetVisible(mode == DemoMode::Theme);
        if (mode == DemoMode::Theme) {
            m_themeHost->SetScrollPos(SIZE{ 0, 0 });
        }
    }
    if (m_iconHost != nullptr) {
        m_iconHost->SetVisible(mode == DemoMode::Icon);
        if (mode == DemoMode::Icon) {
            m_iconHost->SetScrollPos(SIZE{ 0, 0 });
        }
    }
    UpdateEditMenuState();
    UpdateNativeMenuState();
    UpdateStatusBar();
    if (CControlUI* root = m_pm.GetRoot()) {
        root->NeedParentUpdate();
    }
}

void CMainWindow::UpdateEditMenuState()
{
    if (m_editMenu == nullptr) {
        return;
    }
    const wchar_t* activeName = _T("menu:component");
    if (m_demoMode == DemoMode::Theme) {
        activeName = _T("menu:theme");
    }
    else if (m_demoMode == DemoMode::Icon) {
        activeName = _T("menu:icon");
    }
    for (int i = 0; i < m_editMenu->GetCount(); ++i) {
        auto* item = dynamic_cast<CButtonUI*>(m_editMenu->GetItemAt(i));
        if (item == nullptr) {
            continue;
        }
        const bool active = item->GetName() == activeName;
        item->SetTextColor(active ? theme::Primary : theme::TextRegular);
    }
}

void CMainWindow::ToggleEditMenu()
{
    if (m_editMenu == nullptr || m_editMenuButton == nullptr) {
        return;
    }
    if (m_editMenu->IsVisible()) {
        CloseEditMenu();
        return;
    }
    const RECT button = m_editMenuButton->GetPos();
    const RECT root = m_pm.GetRoot() != nullptr ? m_pm.GetRoot()->GetPos() : RECT{ 0, 0, 0, 0 };
    SIZE xy = { button.left - root.left, button.bottom - root.top + UiScale::Px(4) };
    m_editMenu->SetFixedXY(xy);
    m_editMenu->SetVisible(true);
    m_editMenu->NeedParentUpdate();
}

void CMainWindow::CloseEditMenu()
{
    if (m_editMenu == nullptr || !m_editMenu->IsVisible()) {
        return;
    }
    m_editMenu->SetVisible(false);
    m_editMenu->NeedParentUpdate();
}

void CMainWindow::CloseEditMenuIfOutside(POINT pt)
{
    if (m_editMenu == nullptr || !m_editMenu->IsVisible()) {
        return;
    }
    RECT popup = m_editMenu->GetPos();
    if (::PtInRect(&popup, pt)) {
        return;
    }
    if (m_editMenuButton != nullptr) {
        RECT button = m_editMenuButton->GetPos();
        if (::PtInRect(&button, pt)) {
            return;
        }
    }
    CloseEditMenu();
}

void CMainWindow::SelectNav(const std::wstring& id)
{
    for (auto& pair : m_navItems) {
        pair.second->SetSelected(pair.first == id);
    }
}

void CMainWindow::ShowPage(const std::wstring& id)
{
    if (m_pageHost == nullptr) {
        return;
    }

    m_pageHost->RemoveAll();
    m_currentPage = id;
    m_popoverVisible = false;
    m_activeSelect = nullptr;
    m_selectPopup = nullptr;
    m_activeCascader = nullptr;
    m_cascaderPopup = nullptr;

    CContainerUI* page = CreatePage(id);
    if (page == nullptr) {
        return;
    }
    m_pageHost->Add(page);

    SIZE zero = {0, 0};
    m_pageHost->SetScrollPos(zero);
    SelectNav(id);
    UpdateStatusBar();

    if (id == L"overview") {
        FillOverview();
    }
    else if (id == L"icon") {
        FillIconPage();
    }
    else if (id == L"image") {
        FillImagePage();
    }
    else if (id == L"table") {
        FillTablePage();
    }
    else if (id == L"tree") {
        FillTreePage();
    }
    else if (id == L"tabs") {
        UpdateTabSelection();
    }
    else if (id == L"cascader") {
        FillCascaderPage();
    }
    else if (id == L"avatar") {
        FillAvatarPage();
    }
    else if (id == L"breadcrumb") {
        FillBreadcrumbPage();
    }
    else if (id == L"collapse") {
        FillCollapsePage();
    }
    else if (id == L"infinite") {
        FillInfiniteScrollPage();
    }
    else if (id == L"select") {
        FillSelectPage();
    }
}

DuiLib::CContainerUI* CMainWindow::CreatePage(const std::wstring& id)
{
    const std::wstring xml = PageXml(id);
    if (xml.empty()) {
        return nullptr;
    }

    EControlFactory factory;
    CDialogBuilder builder;
    const std::wstring document = L"<Window>" + xml + L"</Window>";
    CControlUI* root = builder.Create((LPCTSTR)document.c_str(), nullptr, &factory, &m_pm);
    if (root == nullptr) {
        TCHAR message[512] = {};
        builder.GetLastErrorMessage(message, 512);
        ::MessageBoxW(m_hWnd, message[0] != L'\0' ? message : L"页面加载失败。",
            L"DuiLib Element", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    auto* page = dynamic_cast<CContainerUI*>(root);
    if (page == nullptr) {
        root->Delete();
        return nullptr;
    }

    ScaleTree(page);
    return page;
}

CListUI* CMainWindow::CreateTable(const std::vector<std::pair<std::wstring, int>>& columns, int height)
{
    CListUI* list = new CListUI;
    list->SetFixedHeight(UiScale::Px(height));
    list->SetBkColor(theme::White);
    list->SetBorderColor(theme::BorderLighter);
    list->SetBorderSize(UiScale::Px(1));
    list->EnableScrollBar(true, false);

    CListHeaderUI* header = new CListHeaderUI;
    header->SetFixedHeight(UiScale::Px(40));
    header->SetBkColor(theme::FillLight);
    for (const auto& column : columns) {
        CListHeaderItemUI* item = new CListHeaderItemUI;
        item->SetText(column.first.c_str());
        item->SetFont(font::Body);
        item->SetTextColor(theme::TextSecondary);
        item->SetTextPadding({UiScale::Px(12), 0, 0, 0});
        item->SetSepWidth(UiScale::Px(1));
        item->SetSepColor(theme::BorderLighter);
        if (column.second > 0) {
            item->SetFixedWidth(UiScale::Px(column.second));
        }
        header->Add(item);
    }
    list->Add(header);

    list->SetItemFont(font::Body);
    list->SetItemTextColor(theme::TextRegular);
    list->SetItemBkColor(theme::White);
    list->SetHotItemBkColor(theme::FillLight);
    list->SetSelectedItemBkColor(theme::PrimaryBg);
    list->SetSelectedItemTextColor(theme::TextRegular);
    list->SetItemHLineSize(UiScale::Px(1));
    list->SetItemHLineColor(theme::BorderLighter);
    list->SetItemTextPadding({UiScale::Px(12), 0, 0, 0});
    list->SetItemFixedHeight(UiScale::Px(36));
    return list;
}

void CMainWindow::AddOverviewRow(CListUI* list, const std::wstring& category,
    const std::wstring& name, const std::wstring& status, const std::wstring& note)
{
    CListHBoxElementUI* element = new CListHBoxElementUI;
    element->SetFixedHeight(UiScale::Px(36));

    DWORD statusColor = theme::TextSecondary;
    if (status == L"已有") {
        statusColor = theme::Success;
    }
    else if (status == L"部分") {
        statusColor = theme::Warning;
    }
    else if (status == L"需开发") {
        statusColor = theme::Danger;
    }

    struct Cell
    {
        std::wstring text;
        int fontId;
        DWORD color;
    };
    const Cell cells[] = {
        {category, font::Small, theme::TextSecondary},
        {name, font::Body, theme::TextPrimary},
        {status, font::Small, statusColor},
        {note, font::Small, theme::TextSecondary},
    };

    for (const Cell& cell : cells) {
        CLabelUI* label = MakeLabel(cell.text, cell.fontId, cell.color);
        label->SetPadding({UiScale::Px(12), 0, 0, 0});
        element->Add(label);
    }

    list->Add(element);
}

void CMainWindow::AddTextRow(CListUI* list, const std::vector<std::wstring>& cells)
{
    CListTextElementUI* row = new CListTextElementUI;
    list->Add(row);
    for (size_t index = 0; index < cells.size(); ++index) {
        row->SetText(static_cast<int>(index), cells[index].c_str());
    }
}

void CMainWindow::FillOverview()
{
    auto* slot = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("overviewSlot")));
    if (slot == nullptr) {
        return;
    }

    CListUI* list = CreateTable({{L"分类", 90}, {L"组件", 180}, {L"支持度", 100}, {L"说明", 0}}, 460);
    slot->Add(list);

    std::wstringstream stream(kOverviewRows);
    std::wstring line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const std::vector<std::wstring> cells = Split(line, L'|');
        if (cells.size() >= 4) {
            AddOverviewRow(list, cells[0], cells[1], cells[2], cells[3]);
        }
    }
}

void CMainWindow::FillIconPage()
{
    auto* slot = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("iconSlot")));
    if (slot == nullptr) {
        return;
    }

    const int count = static_cast<int>(sizeof(kIcons) / sizeof(kIcons[0]));
    const int perRow = 8;
    for (int start = 0; start < count; start += perRow) {
        CHorizontalLayoutUI* row = new CHorizontalLayoutUI;
        row->SetFixedHeight(UiScale::Px(76));
        row->SetChildPadding(UiScale::Px(12));

        for (int index = start; index < start + perRow && index < count; ++index) {
            CVerticalLayoutUI* cell = new CVerticalLayoutUI;
            cell->SetChildPadding(UiScale::Px(2));

            CLabelUI* glyph = MakeLabelH(std::wstring(1, kIcons[index].glyph),
                font::Icon, theme::TextRegular, 36);
            glyph->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            CLabelUI* name = MakeLabelH(kIcons[index].name, font::Small, theme::TextSecondary, 20);
            name->SetTextStyle(DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

            cell->Add(glyph);
            cell->Add(name);
            row->Add(cell);
        }
        slot->Add(row);
    }
}

void CMainWindow::FillImagePage()
{
    auto* image = dynamic_cast<CControlUI*>(m_pm.FindControl(_T("demoImage")));
    if (image == nullptr) {
        return;
    }

    const int width = UiScale::Px(600);
    const int height = UiScale::Px(300);
    image->SetFixedWidth(width);
    image->SetFixedHeight(height);
    const std::wstring imageInfo = L"file='demo-image.png' dest='0,0," + ToW(width) + L"," + ToW(height) + L"'";
    image->SetBkImage(imageInfo.c_str());
}

void CMainWindow::FillTablePage()
{
    auto* slot = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("tableSlot")));
    if (slot == nullptr) {
        return;
    }

    CListUI* list = CreateTable({{L"日期", 150}, {L"姓名", 110}, {L"地址", 0}, {L"状态", 100}}, 290);
    list->SetItemFixedHeight(UiScale::Px(40));
    slot->Add(list);

    static const wchar_t* rows[] = {
        L"2019-05-02|王小虎|上海市普陀区金沙江路 1518 弄|已启用",
        L"2019-05-04|张小刚|上海市普陀区金沙江路 1517 弄|已启用",
        L"2019-05-01|李小明|上海市普陀区金沙江路 1519 弄|已停用",
        L"2019-05-03|刘小红|上海市普陀区金沙江路 1516 弄|已启用",
        L"2019-05-07|陈小亮|上海市普陀区金沙江路 1515 弄|已停用",
    };
    for (const wchar_t* row : rows) {
        AddTextRow(list, Split(row, L'|'));
    }
}

void CMainWindow::FitTreeLayout(CControlUI* tree)
{
    auto* card = dynamic_cast<CContainerUI*>(tree != nullptr ? tree->GetParent() : nullptr);
    if (card == nullptr) {
        return;
    }
    auto measure = [](CContainerUI* box) {
        int height = box->GetInset().top + box->GetInset().bottom;
        const int count = box->GetCount();
        for (int i = 0; i < count; ++i) {
            height += box->GetItemAt(i)->EstimateSize({0, 0}).cy;
            if (i + 1 < count) {
                height += box->GetChildPadding();
            }
        }
        return height;
    };
    card->SetFixedHeight(measure(card));
    if (auto* page = dynamic_cast<CContainerUI*>(card->GetParent())) {
        page->SetFixedHeight(measure(page));
    }
}

void CMainWindow::FillTreePage()
{
    auto makeTree = [this](const wchar_t* name) -> ETreeUI* {
        ETreeUI* tree = dynamic_cast<ETreeUI*>(m_pm.FindControl(name));
        if (tree != nullptr) {
            tree->SetRowHeight(UiScale::Px(26));
        }
        return tree;
    };
    auto addCityTree = [](ETreeUI* tree, const wchar_t* prefix) {
        const std::wstring p = prefix;
        tree->AddNode(p + L"1", L"", L"一级节点 1");
        tree->AddNode(p + L"1-1", p + L"1", L"二级节点 1-1");
        tree->AddNode(p + L"1-1-1", p + L"1-1", L"三级节点 1-1-1");
        tree->AddNode(p + L"1-2", p + L"1", L"二级节点 1-2");
        tree->AddNode(p + L"2", L"", L"一级节点 2");
        tree->AddNode(p + L"2-1", p + L"2", L"二级节点 2-1");
    };

    if (ETreeUI* tree = makeTree(L"treeBasic")) {
        addCityTree(tree, L"b");
        tree->ExpandKey(L"b1");
        tree->ExpandKey(L"b2");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeCheck")) {
        tree->SetShowCheckbox(true);
        addCityTree(tree, L"c");
        tree->ExpandKey(L"c1");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeLazy")) {
        tree->SetLazy(true);
        tree->AddNode(L"l1", L"", L"一级节点 1");
        tree->AddNode(L"l1-1", L"l1", L"二级节点 1-1");
        tree->AddNode(L"l1-2", L"l1", L"二级节点 1-2");
        tree->AddNode(L"l2", L"", L"一级节点 2");
        tree->AddNode(L"l2-1", L"l2", L"二级节点 2-1");
        tree->AddNode(L"l2-2", L"l2", L"二级节点 2-2");
        tree->AddNode(L"l3", L"", L"一级节点 3（叶子）", false, true);
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeDefault")) {
        tree->SetShowCheckbox(true);
        tree->AddNode(L"d1", L"", L"一级节点 1");
        tree->AddNode(L"d1-1", L"d1", L"二级节点 1-1");
        tree->AddNode(L"d1-1-1", L"d1-1", L"三级节点 1-1-1");
        tree->AddNode(L"d1-1-2", L"d1-1", L"三级节点 1-1-2");
        tree->AddNode(L"d1-2", L"d1", L"二级节点 1-2");
        tree->AddNode(L"d2", L"", L"一级节点 2");
        tree->AddNode(L"d2-1", L"d2", L"二级节点 2-1");
        tree->ExpandKey(L"d1");
        tree->ExpandKey(L"d1-1");
        tree->CheckKey(L"d1-1-1");
        tree->CheckKey(L"d1-1-2");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeDisabled")) {
        tree->SetShowCheckbox(true);
        tree->AddNode(L"e1", L"", L"一级节点 1");
        tree->AddNode(L"e1-1", L"e1", L"二级节点 1-1（禁用）", true);
        tree->AddNode(L"e1-2", L"e1", L"二级节点 1-2");
        tree->AddNode(L"e2", L"", L"一级节点 2");
        tree->AddNode(L"e2-1", L"e2", L"二级节点 2-1（禁用）", true);
        tree->AddNode(L"e2-2", L"e2", L"二级节点 2-2");
        tree->ExpandKey(L"e1");
        tree->ExpandKey(L"e2");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeSelect")) {
        addCityTree(tree, L"s");
        tree->ExpandKey(L"s1");
        tree->SelectKey(L"s1-2");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeCustom")) {
        tree->SetShowPlus(true);
        tree->AddNode(L"u1", L"", L"一级节点 1");
        tree->AddNode(L"u1-1", L"u1", L"二级节点 1-1");
        tree->AddNode(L"u2", L"", L"一级节点 2");
        tree->AddNode(L"u2-1", L"u2", L"二级节点 2-1");
        tree->ExpandKey(L"u1");
        tree->ExpandKey(L"u2");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeFilter")) {
        tree->AddNode(L"f1", L"", L"一级节点 1");
        tree->AddNode(L"f1-1", L"f1", L"二级节点 1-1");
        tree->AddNode(L"f1-1-1", L"f1-1", L"三级节点 1-1-1");
        tree->AddNode(L"f1-2", L"f1", L"二级节点 1-2");
        tree->AddNode(L"f2", L"", L"一级节点 2");
        tree->AddNode(L"f2-1", L"f2", L"二级节点 2-1");
        tree->AddNode(L"f2-2", L"f2", L"二级节点 2-2");
        tree->AddNode(L"f2-2-1", L"f2-2", L"三级节点 2-2-1");
        tree->AddNode(L"f3", L"", L"一级节点 3");
        tree->ExpandKey(L"f1");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeAccordion")) {
        tree->SetAccordion(true);
        tree->AddNode(L"a1", L"", L"一级节点 1");
        tree->AddNode(L"a1-1", L"a1", L"二级节点 1-1");
        tree->AddNode(L"a1-2", L"a1", L"二级节点 1-2");
        tree->AddNode(L"a2", L"", L"一级节点 2");
        tree->AddNode(L"a2-1", L"a2", L"二级节点 2-1");
        tree->AddNode(L"a2-2", L"a2", L"二级节点 2-2");
        tree->AddNode(L"a3", L"", L"一级节点 3");
        tree->ExpandKey(L"a1");
        tree->UpdateHeight();
    }

    if (ETreeUI* tree = makeTree(L"treeDrag")) {
        tree->SetDraggable(true);
        tree->AddNode(L"g1", L"", L"一级节点 1");
        tree->AddNode(L"g1-1", L"g1", L"二级节点 1-1");
        tree->AddNode(L"g2", L"", L"一级节点 2");
        tree->AddNode(L"g2-1", L"g2", L"二级节点 2-1");
        tree->AddNode(L"g3", L"", L"一级节点 3");
        tree->ExpandKey(L"g1");
        tree->ExpandKey(L"g2");
        tree->UpdateHeight();
    }
}

void CMainWindow::FillCascaderPage()
{
    const std::vector<CascaderNode> cityTree = {
        { L"浙江", { { L"杭州", { { L"西湖区" }, { L"滨江区" }, { L"余杭区" } } },
                     { L"宁波", { { L"海曙区" }, { L"鄞州区" } } } } },
        { L"江苏", { { L"南京", { { L"玄武区" }, { L"鼓楼区" } } },
                     { L"苏州", { { L"姑苏区" }, { L"工业园区" } } } } },
        { L"广东", { { L"广州", { { L"天河区" }, { L"越秀区" } } },
                     { L"深圳", { { L"南山区" }, { L"福田区" } } } } },
    };

    std::vector<CascaderNode> disabledTree = cityTree;
    disabledTree[0].children[1].disabled = true;
    disabledTree[1].children[0].children[1].disabled = true;

    std::vector<CascaderNode> customTree = cityTree;
    customTree[0].children[0].desc = L"省会";
    customTree[0].children[1].desc = L"港口城市";
    customTree[1].children[0].desc = L"古都";
    customTree[1].children[1].desc = L"园林城市";
    customTree[2].children[0].desc = L"花城";
    customTree[2].children[1].desc = L"科技之城";

    std::vector<CascaderNode> lazyTree;
    for (const wchar_t* name : { L"浙江", L"江苏", L"广东" }) {
        CascaderNode node;
        node.name = name;
        node.lazy = true;
        lazyTree.push_back(node);
    }

    auto find = [this](LPCTSTR name) {
        return dynamic_cast<ECascaderUI*>(m_pm.FindControl(name));
    };

    if (auto* basic = find(_T("cascaderBasic"))) {
        basic->SetOptions(cityTree);
    }
    if (auto* disabled = find(_T("cascaderDisabled"))) {
        disabled->SetOptions(disabledTree);
    }
    if (auto* clearable = find(_T("cascaderClearable"))) {
        clearable->SetOptions(cityTree);
        clearable->ApplySelection({ { 0, 0, 1 } }, { { L"浙江", L"杭州", L"滨江区" } });
    }
    if (auto* showAll = find(_T("cascaderShowAll"))) {
        showAll->SetOptions(cityTree);
        showAll->ApplySelection({ { 0, 0, 0 } }, { { L"浙江", L"杭州", L"西湖区" } });
    }
    if (auto* multiple = find(_T("cascaderMultiple"))) {
        multiple->SetOptions(cityTree);
        multiple->ApplySelection({ { 0, 0, 0 }, { 2, 0, 0 } },
            { { L"浙江", L"杭州", L"西湖区" }, { L"广东", L"广州", L"天河区" } });
    }
    if (auto* strictly = find(_T("cascaderStrict"))) {
        strictly->SetOptions(cityTree);
    }
    if (auto* lazy = find(_T("cascaderLazy"))) {
        lazy->SetOptions(lazyTree);
    }
    if (auto* filter = find(_T("cascaderFilter"))) {
        filter->SetOptions(cityTree);
    }
    if (auto* custom = find(_T("cascaderCustom"))) {
        custom->SetOptions(customTree);
    }

    if (m_pageHost != nullptr && m_pageHost->GetCount() > 0) {
        auto* pageRoot = dynamic_cast<CContainerUI*>(m_pageHost->GetItemAt(0));
        if (pageRoot != nullptr) {
            m_cascaderPopup = new ECascaderPopupUI;
            m_cascaderPopup->SetName(_T("demoCascaderPopup"));
            m_cascaderPopup->SetFloat(true);
            pageRoot->Add(m_cascaderPopup);
            m_cascaderPopup->SetVisible(false);
        }
    }
}

void CMainWindow::FillAvatarPage()
{
    if (auto* avatar = dynamic_cast<EAvatarUI*>(m_pm.FindControl(_T("avatarIcon")))) {
        avatar->SetText(std::wstring(1, static_cast<wchar_t>(0xE77B)).c_str());
        avatar->Invalidate();
    }
}

void CMainWindow::FillBreadcrumbPage()
{
    auto* crumb = dynamic_cast<EBreadcrumbUI*>(m_pm.FindControl(_T("demoBreadcrumb")));
    if (crumb == nullptr) {
        return;
    }
    crumb->SetItems({ L"首页", L"活动管理", L"活动列表", L"活动详情" });
    if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("breadcrumbResult")))) {
        result->SetText(L"当前节点：活动详情");
    }
}

void CMainWindow::FillCollapsePage()
{
    auto* host = dynamic_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("collapseHost")));
    if (host == nullptr) {
        return;
    }

    struct PanelSpec
    {
        const wchar_t* title;
        bool expanded;
        const wchar_t* lines[3];
    };
    const PanelSpec panels[] = {
        { L"一致性 Consistency",
          true,
          { L"与现实生活一致：与现实生活的流程、逻辑保持一致。",
            L"在界面中一致：所有元素、结构需保持一致。",
            L"反馈一致：操作结果与用户预期保持一致。" } },
        { L"反馈 Feedback",
          false,
          { L"控制反馈：通过界面样式与交互反馈给用户操作结果。",
            L"页面反馈：操作后通过页面元素变化展示当前状态。",
            L"提示反馈：使用消息提示、对话框等方式反馈。" } },
        { L"效率 Efficiency",
          true,
          { L"简化流程：设计简洁直观的操作流程。",
            L"清晰明确：语言表达清晰且表意明确。",
            L"帮助用户识别：界面简单直白，减少记忆负担。" } },
    };

    for (const PanelSpec& spec : panels) {
        ECollapseUI* panel = new ECollapseUI;
        panel->SetTitle(spec.title);
        for (const wchar_t* line : spec.lines) {
            panel->Body()->Add(MakeLabelH(line, font::Body, theme::TextSecondary, 22));
        }
        panel->SetExpanded(spec.expanded);
        host->Add(panel);
    }
}

void CMainWindow::AppendInfiniteRows(int count)
{
    auto* list = dynamic_cast<EInfiniteListUI*>(m_pm.FindControl(_T("demoInfinite")));
    if (list == nullptr) {
        return;
    }

    const int limit = 30;
    for (int index = 0; index < count && m_infiniteCount < limit; ++index) {
        ++m_infiniteCount;
        const std::wstring text = L"第 " + ToW(m_infiniteCount) + L" 条 · 无限滚动加载示例";
        CListLabelElementUI* row = new CListLabelElementUI;
        row->SetText(text.c_str());
        list->Add(row);
    }

    if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("infiniteResult")))) {
        const std::wstring text = m_infiniteCount >= limit
            ? L"已加载全部 " + ToW(limit) + L" 条"
            : L"已加载 " + ToW(m_infiniteCount) + L" 条，继续向下滚动加载更多";
        result->SetText(text.c_str());
    }
}

void CMainWindow::FillInfiniteScrollPage()
{
    auto* list = dynamic_cast<EInfiniteListUI*>(m_pm.FindControl(_T("demoInfinite")));
    if (list == nullptr) {
        return;
    }

    list->SetBkColor(theme::White);
    list->SetBorderColor(theme::BorderLighter);
    list->SetBorderSize(UiScale::Px(1));
    list->SetItemFont(font::Body);
    list->SetItemTextColor(theme::TextRegular);
    list->SetItemBkColor(theme::White);
    list->SetHotItemBkColor(theme::FillLight);
    list->SetSelectedItemBkColor(theme::PrimaryBg);
    list->SetSelectedItemTextColor(theme::TextRegular);
    list->SetItemHLineSize(UiScale::Px(1));
    list->SetItemHLineColor(theme::BorderLighter);
    list->SetItemTextPadding({UiScale::Px(14), 0, 0, 0});
    list->SetItemFixedHeight(UiScale::Px(36));

    m_infiniteCount = 0;
    AppendInfiniteRows(15);
}

void CMainWindow::FillSelectPage()
{
    auto find = [this](const wchar_t* name) -> ESelectUI* {
        return dynamic_cast<ESelectUI*>(m_pm.FindControl(name));
    };

    if (auto* select = find(L"selectBasic")) {
        static const wchar_t* labels[] = { L"选项一", L"选项二", L"选项三", L"选项四", L"选项五" };
        for (const wchar_t* label : labels) {
            select->AddOption(label);
        }
    }
    if (auto* select = find(L"selectDisabledOption")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二", L"", true);
        select->AddOption(L"选项三");
    }
    if (auto* select = find(L"selectDisabled")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二");
    }
    if (auto* select = find(L"selectClearable")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二");
        select->AddOption(L"选项三");
    }
    if (auto* select = find(L"selectMultiple")) {
        static const wchar_t* labels[] = { L"金针菇", L"双孢菇", L"香菇", L"平菇", L"茶树菇", L"草菇" };
        for (const wchar_t* label : labels) {
            select->AddOption(label);
        }
        select->ApplySelection({ 0, 1 });
    }
    if (auto* select = find(L"selectTemplate")) {
        select->AddOption(L"北京", L"首都");
        select->AddOption(L"上海", L"直辖市");
        select->AddOption(L"广州", L"省会");
        select->AddOption(L"深圳", L"经济特区");
        select->AddOption(L"成都", L"省会");
        select->ApplySelection({ 0 });
    }
    if (auto* select = find(L"selectGroup")) {
        select->AddGroup(L"热门城市");
        select->AddOption(L"北京");
        select->AddOption(L"上海");
        select->AddOption(L"广州");
        select->AddOption(L"深圳");
        select->AddGroup(L"城市");
        select->AddOption(L"成都");
        select->AddOption(L"杭州");
        select->AddOption(L"南京");
    }
    if (auto* select = find(L"selectFilter")) {
        static const wchar_t* labels[] = { L"北京", L"上海", L"广州", L"深圳", L"杭州", L"南京" };
        for (const wchar_t* label : labels) {
            select->AddOption(label);
        }
    }
    if (auto* select = find(L"selectRemote")) {
        static const wchar_t* labels[] = { L"北京", L"上海", L"广州", L"深圳", L"杭州",
            L"南京", L"成都", L"武汉", L"西安", L"重庆" };
        for (const wchar_t* label : labels) {
            select->AddOption(label);
        }
    }
    if (auto* select = find(L"selectCreate")) {
        static const wchar_t* labels[] = { L"苹果", L"香蕉", L"橙子", L"葡萄" };
        for (const wchar_t* label : labels) {
            select->AddOption(label);
        }
        select->ApplySelection({ 0 });
    }
    if (auto* select = find(L"selectSizeLarge")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二");
        select->AddOption(L"选项三");
    }
    if (auto* select = find(L"selectSizeDefault")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二");
        select->AddOption(L"选项三");
    }
    if (auto* select = find(L"selectSizeSmall")) {
        select->AddOption(L"选项一");
        select->AddOption(L"选项二");
        select->AddOption(L"选项三");
    }

    if (m_pageHost != nullptr && m_pageHost->GetCount() > 0) {
        auto* pageRoot = dynamic_cast<CContainerUI*>(m_pageHost->GetItemAt(0));
        if (pageRoot != nullptr) {
            m_selectPopup = new ESelectPopupUI;
            m_selectPopup->SetName(_T("demoSelectPopup"));
            m_selectPopup->SetFloat(true);
            pageRoot->Add(m_selectPopup);
            m_selectPopup->SetVisible(false);
        }
    }
}

void CMainWindow::OpenPopover()
{
    auto* host = dynamic_cast<CContainerUI*>(m_pm.FindControl(_T("popoverHost")));
    auto* popover = dynamic_cast<EPopoverUI*>(m_pm.FindControl(_T("demoPopover")));
    auto* button = dynamic_cast<CControlUI*>(m_pm.FindControl(_T("popoverBtn")));
    if (host == nullptr || popover == nullptr || button == nullptr) {
        return;
    }

    const RECT btn = button->GetPos();
    const RECT hostRect = host->GetPos();
    SIZE xy = { (btn.left + btn.right) / 2 - UiScale::Px(24) - hostRect.left,
        btn.bottom + UiScale::Px(4) - hostRect.top };
    popover->SetFixedXY(xy);
    popover->SetVisible(true);
    popover->NeedParentUpdate();
    m_popoverVisible = true;
}

void CMainWindow::ClosePopover()
{
    if (auto* popover = dynamic_cast<EPopoverUI*>(m_pm.FindControl(_T("demoPopover")))) {
        popover->SetVisible(false);
        popover->NeedParentUpdate();
    }
    m_popoverVisible = false;
}

void CMainWindow::ClosePopoverIfOutside(POINT pt)
{
    if (!m_popoverVisible) {
        return;
    }
    auto* popover = dynamic_cast<EPopoverUI*>(m_pm.FindControl(_T("demoPopover")));
    auto* button = dynamic_cast<CControlUI*>(m_pm.FindControl(_T("popoverBtn")));
    if (popover == nullptr || button == nullptr) {
        m_popoverVisible = false;
        return;
    }
    RECT popoverRect = popover->GetPos();
    RECT buttonRect = button->GetPos();
    if (::PtInRect(&popoverRect, pt) || ::PtInRect(&buttonRect, pt)) {
        return;
    }
    ClosePopover();
}

void CMainWindow::ToggleSelect(ESelectUI* select)
{
    if (select == nullptr) {
        return;
    }
    if (select == m_activeSelect && m_selectPopup != nullptr && m_selectPopup->IsVisible()) {
        CloseSelect();
        return;
    }
    OpenSelect(select);
}

void CMainWindow::OpenSelect(ESelectUI* select)
{
    if (select == nullptr || m_selectPopup == nullptr || m_pageHost == nullptr) {
        return;
    }
    auto* pageRoot = m_pageHost->GetCount() > 0
        ? dynamic_cast<CContainerUI*>(m_pageHost->GetItemAt(0)) : nullptr;
    if (pageRoot == nullptr) {
        return;
    }

    if (m_activeCascader != nullptr) {
        CloseCascader();
    }

    m_selectPopup->Open(select->GetOptions(), select->GetSelection(), select->IsMultiple(),
        select->IsSearchable(), select->IsRemote());

    const RECT root = pageRoot->GetPos();
    const RECT box = select->GetPos();
    const RECT host = m_pageHost->GetPos();
    const int width = (std::max)(static_cast<int>(box.right - box.left), UiScale::Px(220));
    const int height = m_selectPopup->ContentHeight();
    const int x = box.left;
    int y = box.bottom + UiScale::Px(4);
    if (y + height > host.bottom - UiScale::Px(8)) {
        const int above = box.top - UiScale::Px(6) - height;
        if (above >= host.top) {
            y = above;
        }
    }

    m_selectPopup->SetFixedWidth(width);
    m_selectPopup->SetFixedHeight(height);
    SIZE xy = { x - root.left, y - root.top };
    m_selectPopup->SetFixedXY(xy);
    m_selectPopup->SetVisible(true);
    m_selectPopup->NeedParentUpdate();
    m_activeSelect = select;
    select->SetOpen(true);
    m_selectPopup->FocusSearch();
}

void CMainWindow::CloseSelect()
{
    if (m_selectPopup != nullptr) {
        m_selectPopup->SetVisible(false);
        m_selectPopup->NeedParentUpdate();
    }
    if (m_activeSelect != nullptr) {
        m_activeSelect->SetOpen(false);
    }
    m_activeSelect = nullptr;
}

void CMainWindow::CloseSelectIfOutside(POINT pt)
{
    if (m_selectPopup == nullptr || !m_selectPopup->IsVisible()) {
        return;
    }
    RECT popupRect = m_selectPopup->GetPos();
    if (::PtInRect(&popupRect, pt)) {
        return;
    }
    if (m_activeSelect != nullptr) {
        RECT box = m_activeSelect->GetPos();
        if (::PtInRect(&box, pt)) {
            return;
        }
    }
    CloseSelect();
}

void CMainWindow::ShowSelectSummary(ESelectUI* select)
{
    if (select == nullptr) {
        return;
    }
    if (auto* label = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("selectResult")))) {
        const std::wstring text = select->GetDemoTitle() + L"：" + select->GetDisplayText();
        label->SetText(text.c_str());
    }
}

void CMainWindow::ToggleCascader(ECascaderUI* cascader)
{
    if (cascader == nullptr) {
        return;
    }
    if (cascader == m_activeCascader && m_cascaderPopup != nullptr && m_cascaderPopup->IsVisible()) {
        CloseCascader();
        return;
    }
    OpenCascader(cascader);
}

void CMainWindow::OpenCascader(ECascaderUI* cascader)
{
    if (cascader == nullptr || m_cascaderPopup == nullptr || m_pageHost == nullptr) {
        return;
    }
    auto* pageRoot = m_pageHost->GetCount() > 0
        ? dynamic_cast<CContainerUI*>(m_pageHost->GetItemAt(0)) : nullptr;
    if (pageRoot == nullptr) {
        return;
    }

    if (m_activeSelect != nullptr) {
        CloseSelect();
    }
    if (m_activeCascader != nullptr && m_activeCascader != cascader) {
        m_activeCascader->SetOpen(false);
    }

    const RECT box = cascader->GetPos();
    const int boxWidth = static_cast<int>(box.right - box.left);
    m_cascaderPopup->SetColumnWidth(
        (std::max)(UiScale::Px(180), (std::min)(boxWidth, UiScale::Px(220))));
    m_cascaderPopup->Open(cascader->GetOptions(), cascader->GetSelection(),
        cascader->GetSelectionLabels(), cascader->IsMultiple(), cascader->IsCheckStrictly(),
        cascader->IsFilterable(), cascader->IsLazy());

    const RECT root = pageRoot->GetPos();
    const RECT host = m_pageHost->GetPos();
    const int width = m_cascaderPopup->ContentWidth();
    const int height = m_cascaderPopup->ContentHeight();
    int x = box.left;
    if (x + width > host.right - UiScale::Px(8)) {
        x = host.right - UiScale::Px(8) - width;
    }
    if (x < host.left + UiScale::Px(8)) {
        x = host.left + UiScale::Px(8);
    }
    int y = box.bottom + UiScale::Px(4);
    if (y + height > host.bottom - UiScale::Px(8)) {
        const int above = box.top - UiScale::Px(6) - height;
        if (above >= host.top) {
            y = above;
        }
    }

    m_cascaderPopup->SetFixedWidth(width);
    m_cascaderPopup->SetFixedHeight(height);
    SIZE xy = { x - root.left, y - root.top };
    m_cascaderPopup->SetFixedXY(xy);
    m_cascaderPopup->SetVisible(true);
    m_cascaderPopup->NeedParentUpdate();
    m_activeCascader = cascader;
    cascader->SetOpen(true);
    m_cascaderPopup->FocusSearch();
}

void CMainWindow::CloseCascader()
{
    if (m_cascaderPopup != nullptr) {
        m_cascaderPopup->SetVisible(false);
        m_cascaderPopup->NeedParentUpdate();
    }
    if (m_activeCascader != nullptr) {
        m_activeCascader->SetOpen(false);
    }
    m_activeCascader = nullptr;
}

void CMainWindow::CloseCascaderIfOutside(POINT pt)
{
    if (m_cascaderPopup == nullptr || !m_cascaderPopup->IsVisible()) {
        return;
    }
    RECT popupRect = m_cascaderPopup->GetPos();
    if (::PtInRect(&popupRect, pt)) {
        return;
    }
    if (m_activeCascader != nullptr) {
        RECT box = m_activeCascader->GetPos();
        if (::PtInRect(&box, pt)) {
            return;
        }
    }
    CloseCascader();
}

void CMainWindow::ShowCascaderSummary(ECascaderUI* cascader)
{
    if (cascader == nullptr) {
        return;
    }
    if (auto* label = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("cascaderResult")))) {
        const std::wstring text = cascader->GetDemoTitle() + L"：" + cascader->GetDisplayText();
        label->SetText(text.c_str());
    }
}

void CMainWindow::UpdateTabSelection()
{
    auto* tabs = dynamic_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tabHost")));
    if (tabs == nullptr) {
        return;
    }

    const int current = tabs->GetCurSel();
    for (int index = 0; index < 3; ++index) {
        const std::wstring name = L"tab:" + ToW(index);
        if (auto* button = dynamic_cast<ETabButtonUI*>(m_pm.FindControl(name.c_str()))) {
            button->SetSelected(index == current);
        }
    }
}

void CMainWindow::Notify(TNotifyUI& msg)
{
    if (msg.pSender == nullptr) {
        return;
    }

    const std::wstring name = msg.pSender->GetName().GetData();
    if (msg.sType == _T("selecttoggle")) {
        ToggleSelect(dynamic_cast<ESelectUI*>(msg.pSender));
        return;
    }
    if (msg.sType == _T("selectpick")) {
        if (m_activeSelect != nullptr && m_selectPopup != nullptr) {
            ESelectUI* select = m_activeSelect;
            const int index = static_cast<int>(msg.wParam);
            if (m_selectPopup->IsMultiple()) {
                select->ApplySelection(m_selectPopup->GetSelection());
            }
            else {
                select->ApplySelection({ index });
                CloseSelect();
            }
            ShowSelectSummary(select);
        }
        return;
    }
    if (msg.sType == _T("selectclear")) {
        auto* select = dynamic_cast<ESelectUI*>(msg.pSender);
        if (select != nullptr && select == m_activeSelect) {
            if (select->IsMultiple() && m_selectPopup != nullptr && m_selectPopup->IsVisible()) {
                OpenSelect(select);
            }
            else {
                CloseSelect();
            }
        }
        return;
    }
    if (m_selectPopup != nullptr && msg.pSender == m_selectPopup->SearchEdit()) {
        auto* edit = dynamic_cast<CEditUI*>(msg.pSender);
        if (msg.sType == _T("textchanged") && edit != nullptr) {
            m_selectPopup->SetQuery(edit->GetText().GetData());
            return;
        }
        if (msg.sType == _T("return") && edit != nullptr && m_activeSelect != nullptr
            && m_activeSelect->IsAllowCreate()) {
            std::wstring text = edit->GetText().GetData();
            const size_t first = text.find_first_not_of(L" \t");
            const size_t last = text.find_last_not_of(L" \t");
            text = first == std::wstring::npos ? std::wstring() : text.substr(first, last - first + 1);
            if (!text.empty()) {
                ESelectUI* select = m_activeSelect;
                select->AddCreated(text);
                ShowSelectSummary(select);
                if (select->IsMultiple()) {
                    OpenSelect(select);
                }
                else {
                    CloseSelect();
                }
            }
            return;
        }
    }
    if (msg.sType == _T("cascadertoggle")) {
        ToggleCascader(dynamic_cast<ECascaderUI*>(msg.pSender));
        return;
    }
    if (msg.sType == _T("cascaderpick")) {
        if (m_activeCascader != nullptr && m_cascaderPopup != nullptr) {
            ECascaderUI* cascader = m_activeCascader;
            cascader->ApplySelection(m_cascaderPopup->GetSelection(),
                m_cascaderPopup->GetSelectionLabels());
            ShowCascaderSummary(cascader);
            if (!cascader->IsMultiple()) {
                CloseCascader();
            }
        }
        return;
    }
    if (msg.sType == _T("cascaderclear")) {
        auto* cascader = dynamic_cast<ECascaderUI*>(msg.pSender);
        if (cascader != nullptr) {
            if (cascader == m_activeCascader) {
                CloseCascader();
            }
            ShowCascaderSummary(cascader);
        }
        return;
    }
    if (m_cascaderPopup != nullptr && msg.pSender == m_cascaderPopup->SearchEdit()) {
        auto* edit = dynamic_cast<CEditUI*>(msg.pSender);
        if (msg.sType == _T("textchanged") && edit != nullptr) {
            m_cascaderPopup->SetQuery(edit->GetText().GetData());
            return;
        }
    }
    if (msg.sType == _T("textchanged") && name == L"treeFilterEdit") {
        auto* edit = dynamic_cast<CEditUI*>(msg.pSender);
        auto* tree = dynamic_cast<ETreeUI*>(m_pm.FindControl(_T("treeFilter")));
        if (edit != nullptr && tree != nullptr) {
            tree->SetFilterText(edit->GetText().GetData());
        }
        return;
    }
    if (msg.sType == _T("treesize")) {
        FitTreeLayout(msg.pSender);
        return;
    }
    if (msg.sType == _T("click")) {
        if (name == L"editMenu") {
            ToggleEditMenu();
            return;
        }
        if (name == L"menu:component") {
            ShowDemoMode(DemoMode::Component);
            CloseEditMenu();
            return;
        }
        if (name == L"menu:theme") {
            ShowDemoMode(DemoMode::Theme);
            CloseEditMenu();
            return;
        }
        if (name == L"menu:icon") {
            ShowDemoMode(DemoMode::Icon);
            CloseEditMenu();
            return;
        }
        if (name.rfind(L"nav:", 0) == 0) {
            ShowPage(name.substr(4));
            return;
        }
        if (name.rfind(L"tab:", 0) == 0) {
            const int index = _wtoi(name.c_str() + 4);
            if (auto* tabs = dynamic_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tabHost")))) {
                tabs->SelectItem(index);
            }
            UpdateTabSelection();
            return;
        }
        if (name.rfind(L"btnDemo", 0) == 0) {
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("btnResult")))) {
                const std::wstring text = L"事件回调：click · " + std::wstring(msg.pSender->GetText().GetData());
                result->SetText(text.c_str());
            }
            return;
        }
        if (name.rfind(L"linkDemo", 0) == 0) {
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("linkResult")))) {
                const std::wstring text = L"事件回调：click · " + std::wstring(msg.pSender->GetText().GetData());
                result->SetText(text.c_str());
            }
            return;
        }
        if (name.rfind(L"crumb:", 0) == 0) {
            auto* crumb = dynamic_cast<EBreadcrumbUI*>(m_pm.FindControl(_T("demoBreadcrumb")));
            if (crumb == nullptr) {
                return;
            }
            crumb->SetCurrent(_wtoi(name.c_str() + 6));
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("breadcrumbResult")))) {
                result->SetText((L"当前节点：" + std::wstring(msg.pSender->GetText().GetData())).c_str());
            }
            return;
        }
        if (name == L"stepsNext") {
            auto* steps = dynamic_cast<EStepsUI*>(m_pm.FindControl(_T("demoSteps")));
            if (steps == nullptr) {
                return;
            }
            int active = steps->GetActive() + 1;
            if (active > 4) {
                active = 0;
            }
            steps->SetActive(active);
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("stepsResult")))) {
                const std::wstring text = active >= 4
                    ? std::wstring(L"当前进度：全部完成")
                    : L"当前进度：" + ToW(active) + L" / 4";
                result->SetText(text.c_str());
            }
            return;
        }
        if (name == L"popoverBtn") {
            if (m_popoverVisible) {
                ClosePopover();
            }
            else {
                OpenPopover();
            }
            return;
        }
        if (name == L"popoverClose") {
            ClosePopover();
            return;
        }
    }
    else if (msg.sType == _T("return")) {
        if (auto* edit = dynamic_cast<EInputNumberEditUI*>(msg.pSender)) {
            edit->Commit();
            return;
        }
    }
    else if (msg.sType == _T("valuechanged")) {
        if (auto* select = dynamic_cast<ESelectUI*>(msg.pSender)) {
            ShowSelectSummary(select);
            return;
        }
        if (name == L"demoSlider") {
            auto* slider = dynamic_cast<CSliderUI*>(msg.pSender);
            if (slider == nullptr) {
                return;
            }
            if (auto* value = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("sliderValue")))) {
                value->SetText(std::to_wstring(slider->GetValue()).c_str());
            }
        }
        else if (name == L"progressSlider") {
            auto* slider = dynamic_cast<CSliderUI*>(msg.pSender);
            if (slider == nullptr) {
                return;
            }
            if (auto* bar = dynamic_cast<CProgressUI*>(m_pm.FindControl(_T("progressDemo")))) {
                bar->SetValue(slider->GetValue());
            }
            if (auto* value = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("progressValue")))) {
                value->SetText((std::to_wstring(slider->GetValue()) + L"%").c_str());
            }
        }
        else if (name == L"demoInputNumber" || name == L"demoInputNumberStep") {
            auto* number = dynamic_cast<EInputNumberUI*>(msg.pSender);
            if (number == nullptr) {
                return;
            }
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("inputNumberResult")))) {
                const std::wstring text = L"当前值：" + std::to_wstring(number->GetValue());
                result->SetText(text.c_str());
            }
        }
        else if (name.rfind(L"demoSwitch", 0) == 0) {
            auto* toggle = dynamic_cast<ESwitchUI*>(msg.pSender);
            if (toggle == nullptr) {
                return;
            }
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("switchResult")))) {
                const std::wstring text = std::wstring(L"开关状态：") + (toggle->IsChecked() ? L"开" : L"关");
                result->SetText(text.c_str());
            }
        }
        else if (name.rfind(L"demoRate", 0) == 0) {
            auto* rate = dynamic_cast<ERateUI*>(msg.pSender);
            if (rate == nullptr) {
                return;
            }
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("rateResult")))) {
                std::wstringstream stream;
                stream << L"评分：" << rate->GetValue();
                const std::wstring text = stream.str();
                result->SetText(text.c_str());
            }
        }
        else if (name == L"demoPagination") {
            auto* pagination = dynamic_cast<EPaginationUI*>(msg.pSender);
            if (pagination == nullptr) {
                return;
            }
            if (auto* result = dynamic_cast<CLabelUI*>(m_pm.FindControl(_T("paginationResult")))) {
                const std::wstring text = L"当前页：" + ToW(pagination->GetCurrentPage())
                    + L" / 共 " + ToW(pagination->GetPageCount()) + L" 页";
                result->SetText(text.c_str());
            }
        }
    }
    else if (msg.sType == _T("loadmore")) {
        if (name == L"demoInfinite") {
            AppendInfiniteRows(5);
        }
    }
}

namespace {

std::wstring PageXml(const std::wstring& id)
{
    if (id == L"overview") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Element 组件总览" font="3" textcolor="#FF303133" />
<Label text="Element 2.x 的 55 个视觉组件与 duilib 原生控件的对应关系" font="1" textcolor="#FF909399" />
<HorizontalLayout height="108" childpadding="18">
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,16,20,16" childpadding="2">
<Label text="29" font="9" textcolor="#FF67C23A" />
<Label text="duilib 已有原生控件" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,16,20,16" childpadding="2">
<Label text="9" font="9" textcolor="#FFE6A23C" />
<Label text="部分支持 / 可近似" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,16,20,16" childpadding="2">
<Label text="17" font="9" textcolor="#FFF56C6C" />
<Label text="需二次开发" font="1" textcolor="#FF909399" />
</VerticalLayout>
</HorizontalLayout>
<Label text="完整对照见 docs/Element组件对照表.md；状态色、圆角与状态交互由 duilib 皮肤属性与轻量控件子类实现。" font="1" textcolor="#FF909399" />
<VerticalLayout name="overviewSlot" />
</VerticalLayout>)";
    }

    if (id == L"color") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Color 色彩" font="3" textcolor="#FF303133" />
<Label text="duilib 通过 bkcolor / bordercolor / textcolor 属性表达 Element 色板" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<HorizontalLayout childpadding="16">
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF409EFF" borderround="4,4" />
<Label text="Primary" font="2" textcolor="#FF303133" />
<Label text="#409EFF" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF66B1FF" borderround="4,4" />
<Label text="Primary Light" font="2" textcolor="#FF303133" />
<Label text="#66B1FF" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF3A8EE6" borderround="4,4" />
<Label text="Primary Dark" font="2" textcolor="#FF303133" />
<Label text="#3A8EE6" font="1" textcolor="#FF909399" />
</VerticalLayout>
</HorizontalLayout>
<HorizontalLayout childpadding="16">
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF67C23A" borderround="4,4" />
<Label text="Success" font="2" textcolor="#FF303133" />
<Label text="#67C23A" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF85CE61" borderround="4,4" />
<Label text="Success Light" font="2" textcolor="#FF303133" />
<Label text="#85CE61" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF5DAF34" borderround="4,4" />
<Label text="Success Dark" font="2" textcolor="#FF303133" />
<Label text="#5DAF34" font="1" textcolor="#FF909399" />
</VerticalLayout>
</HorizontalLayout>
<HorizontalLayout childpadding="16">
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FFE6A23C" borderround="4,4" />
<Label text="Warning" font="2" textcolor="#FF303133" />
<Label text="#E6A23C" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FFF56C6C" borderround="4,4" />
<Label text="Danger" font="2" textcolor="#FF303133" />
<Label text="#F56C6C" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF909399" borderround="4,4" />
<Label text="Info" font="2" textcolor="#FF303133" />
<Label text="#909399" font="1" textcolor="#FF909399" />
</VerticalLayout>
</HorizontalLayout>
<HorizontalLayout childpadding="16">
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF303133" borderround="4,4" />
<Label text="Text Primary" font="2" textcolor="#FF303133" />
<Label text="#303133" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FF606266" borderround="4,4" />
<Label text="Text Regular" font="2" textcolor="#FF303133" />
<Label text="#606266" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout width="160" childpadding="6">
<Control width="160" height="56" bkcolor="#FFDCDFE6" borderround="4,4" />
<Label text="Border Base" font="2" textcolor="#FF303133" />
<Label text="#DCDFE6" font="1" textcolor="#FF909399" />
</VerticalLayout>
</HorizontalLayout>
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"typography") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Typography 字体" font="3" textcolor="#FF303133" />
<Label text="duilib 字体资源（Font 节点）与 Label / Text 组合" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="0">
<HorizontalLayout height="48" childvalign="vcenter" childpadding="16">
<Label text="Element DuiLib" font="9" textcolor="#FF303133" width="330" />
<Label text="H1 · 28px 粗体" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="46" childvalign="vcenter" childpadding="16">
<Label text="组件演示" font="10" textcolor="#FF303133" width="330" />
<Label text="H2 · 22px 粗体" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="42" childvalign="vcenter" childpadding="16">
<Label text="数据展示" font="3" textcolor="#FF303133" width="330" />
<Label text="页面标题 · 20px 粗体" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="38" childvalign="vcenter" childpadding="16">
<Label text="基础组件" font="4" textcolor="#FF303133" width="330" />
<Label text="小节标题 · 16px 粗体" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="34" childvalign="vcenter" childpadding="16">
<Label text="正文示例文本 Body" font="5" textcolor="#FF303133" width="330" />
<Label text="正文 · 14px" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="32" childvalign="vcenter" childpadding="16">
<Label text="默认文本 Default" font="0" textcolor="#FF303133" width="330" />
<Label text="默认 · 13px" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="30" childvalign="vcenter" childpadding="16">
<Label text="次要说明文字" font="1" textcolor="#FF909399" width="330" />
<Label text="辅助文字 · 12px" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<HorizontalLayout height="30" childvalign="vcenter" childpadding="16">
<Label text="加粗强调" font="2" textcolor="#FF303133" width="330" />
<Label text="正文加粗 · 13px" font="1" textcolor="#FF909399" />
</HorizontalLayout>
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"icon") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Icon 图标" font="3" textcolor="#FF303133" />
<Label text="Label + 符号字体；duilib 没有内置图标集" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label text="Segoe MDL2 Assets（Windows 10/11 自带符号字体）" font="2" textcolor="#FF303133" />
<VerticalLayout name="iconSlot" childpadding="10" />
<Label text="图标来自系统符号字体，跨平台或自定义图标需改用图片 / 图标字体资源。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"button") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Button 按钮" font="3" textcolor="#FF303133" />
<Label text="Button + borderround 原生圆角，悬停 / 按下 / 禁用状态由皮肤子类绘制" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<HorizontalLayout height="40" childpadding="12" childvalign="vcenter">
<EButton name="btnDemoPrimary" text="主要按钮" variant="primary" width="96" />
<EButton name="btnDemoSuccess" text="成功按钮" variant="success" width="96" />
<EButton name="btnDemoWarning" text="警告按钮" variant="warning" width="96" />
<EButton name="btnDemoDanger" text="危险按钮" variant="danger" width="96" />
<EButton name="btnDemoInfo" text="信息按钮" variant="info" width="96" />
<EButton name="btnDemoDefault" text="默认按钮" width="96" />
</HorizontalLayout>
<HorizontalLayout height="40" childpadding="12" childvalign="vcenter">
<EButton name="btnDemoPlainPrimary" text="朴素按钮" variant="primary" plain="true" width="96" />
<EButton name="btnDemoPlainSuccess" text="成功按钮" variant="success" plain="true" width="96" />
<EButton name="btnDemoPlainWarning" text="警告按钮" variant="warning" plain="true" width="96" />
<EButton name="btnDemoPlainDanger" text="危险按钮" variant="danger" plain="true" width="96" />
<EButton name="btnDemoPlainInfo" text="信息按钮" variant="info" plain="true" width="96" />
<EButton name="btnDemoText" text="文字按钮" variant="text" width="96" />
</HorizontalLayout>
<HorizontalLayout height="44" childpadding="12" childvalign="vcenter">
<EButton name="btnDemoLarge" text="大型按钮" variant="primary" size="large" width="110" />
<EButton name="btnDemoNormal" text="默认尺寸" variant="primary" width="96" />
<EButton name="btnDemoSmall" text="小型按钮" variant="primary" size="small" width="80" />
<EButton name="btnDemoDisabled" text="禁用按钮" variant="primary" enabled="false" width="96" />
<EButton name="btnDemoDisabledPlain" text="禁用朴素" variant="danger" plain="true" enabled="false" width="96" />
</HorizontalLayout>
<Label name="btnResult" text="点击任意按钮查看事件回调" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"link") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Link 文字链接" font="3" textcolor="#FF303133" />
<Label text="ELink 子类：主色文字、悬停变色与下划线、手型光标、禁用置灰" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childpadding="16" childvalign="vcenter">
<ELink name="linkDemoBasic" text="默认链接" width="90" />
<ELink name="linkDemoPrimary" text="主要链接" type="primary" width="90" />
<ELink name="linkDemoSuccess" text="成功链接" type="success" width="90" />
<ELink name="linkDemoWarning" text="警告链接" type="warning" width="90" />
<ELink name="linkDemoDanger" text="危险链接" type="danger" width="90" />
<ELink name="linkDemoInfo" text="信息链接" type="info" width="90" />
</HorizontalLayout>
<Label text="下划线与禁用" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childpadding="16" childvalign="vcenter">
<ELink name="linkDemoUnderline" text="悬停下划线" underline="true" width="110" />
<ELink name="linkDemoDisabled" text="禁用链接" enabled="false" width="110" />
</HorizontalLayout>
<Label name="linkResult" text="点击任意链接查看事件回调" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"inputnumber") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="InputNumber 计数器" font="3" textcolor="#FF303133" />
<Label text="EInputNumber：Edit 数字输入 + 两侧步进区，支持 min / max / step / 禁用" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="34" childpadding="14" childvalign="vcenter">
<EInputNumber name="demoInputNumber" width="180" value="1" />
<Label text="输入数字或点击两侧 - / +" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="步长与范围" font="2" textcolor="#FF303133" />
<HorizontalLayout height="34" childpadding="14" childvalign="vcenter">
<EInputNumber name="demoInputNumberStep" width="180" value="10" min="0" max="20" step="5" />
<Label text="min=0 · max=20 · step=5" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="34" childpadding="14" childvalign="vcenter">
<EInputNumber width="180" value="3" enabled="false" />
<Label text="enabled=false" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label name="inputNumberResult" text="当前值：1" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"cascader") {
        return LR"(<VerticalLayout childpadding="18" height="1546">
<Label height="34" text="Cascader 级联选择器" font="3" textcolor="#FF303133" />
<Label height="22" text="ECascader：自绘选择框 + 浮层多列联动面板，覆盖 Element 文档中的全部用法" font="1" textcolor="#FF909399" />
<HorizontalLayout height="24" childpadding="8" childvalign="vcenter">
<Label width="70" text="当前选择：" font="1" textcolor="#FF909399" />
<Label name="cascaderResult" width="420" text="（未选择）" font="2" textcolor="#FF409EFF" />
</HorizontalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderBasic" demo="基础用法" width="260" placeholder="请选择地区" />
</HorizontalLayout>
<Label height="20" text="逐列点击展开下级，选中最后一级后回填完整路径。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="禁用选项" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderDisabled" demo="禁用选项" width="260" placeholder="请选择地区" />
</HorizontalLayout>
<Label height="20" text="被置灰的节点不可选中，也不会展开下一级。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="可清空" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderClearable" demo="可清空" width="260" placeholder="请选择地区" clearable="true" />
</HorizontalLayout>
<Label height="20" text="悬停有值时右侧显示清空圆钮，点击清除当前选择。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="仅显示最后一级" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderShowAll" demo="仅显示最后一级" width="260" placeholder="请选择地区" showall="false" />
</HorizontalLayout>
<Label height="20" text="show-all-levels=false 时输入框只显示最后一级名称。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="多选" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderMultiple" demo="多选" width="340" placeholder="请选择地区" multiple="true" collapsetags="true" clearable="true" />
</HorizontalLayout>
<Label height="20" text="叶子节点显示复选框，选中项以标签折叠展示，面板保持展开。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="选择任意一级选项" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderStrict" demo="选择任意一级" width="260" placeholder="请选择地区" checkstrictly="true" />
</HorizontalLayout>
<Label height="20" text="checkStrictly=true 时每一级都可直接选中，节点右侧显示单选圈。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="动态加载" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderLazy" demo="动态加载" width="260" placeholder="请选择地区" lazy="true" />
</HorizontalLayout>
<Label height="20" text="点击节点后模拟 500ms 懒加载子级，期间显示加载中。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="可搜索" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderFilter" demo="可搜索" width="260" placeholder="请选择地区" filterable="true" />
</HorizontalLayout>
<Label height="20" text="面板顶部输入关键字，平铺显示匹配的完整路径。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="自定义节点内容" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ECascader name="cascaderCustom" demo="自定义节点内容" width="260" placeholder="请选择地区" />
</HorizontalLayout>
<Label height="20" text="选项右侧附加说明文本，模拟 scoped slot 自定义节点内容。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"switch") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Switch 开关" font="3" textcolor="#FF303133" />
<Label text="ESwitch：轨道与滑块自绘，点击切换并发送 valuechanged" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childpadding="14" childvalign="vcenter">
<ESwitch name="demoSwitch" checked="true" />
<Label text="默认开启" font="1" textcolor="#FF909399" />
<ESwitch name="demoSwitchOff" />
<Label text="默认关闭" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childpadding="14" childvalign="vcenter">
<ESwitch checked="true" enabled="false" />
<Label text="禁用开启" font="1" textcolor="#FF909399" />
<ESwitch enabled="false" />
<Label text="禁用关闭" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label name="switchResult" text="开关状态：开" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"rate") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Rate 评分" font="3" textcolor="#FF303133" />
<Label text="ERate：五角星自绘，支持悬停预览、点击评分与半星" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础评分" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="14" childvalign="vcenter">
<ERate name="demoRate" value="3" />
<Label text="默认 3 星，悬停预览，点击评分" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="半星" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="14" childvalign="vcenter">
<ERate name="demoRateHalf" value="2.5" allowhalf="true" />
<Label text="allowhalf=true · value=2.5" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="14" childvalign="vcenter">
<ERate value="4" enabled="false" />
<Label text="enabled=false" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label name="rateResult" text="评分：3" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"container") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Container 容器" font="3" textcolor="#FF303133" />
<Label text="Container 与三种 Layout 的边距、圆角组合" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="14">
<Label text="嵌套容器与内容边距" font="2" textcolor="#FF303133" />
<HorizontalLayout height="140" childpadding="12">
<VerticalLayout width="180" bkcolor="#FFECF5FF" borderround="4,4" inset="14,14,14,14" childpadding="6">
<Label text="Container A" font="2" textcolor="#FF409EFF" />
<Label text="固定宽度 180" font="1" textcolor="#FF606266" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFF0F9EB" borderround="4,4" inset="14,14,14,14" childpadding="6">
<Label text="Container B" font="2" textcolor="#FF67C23A" />
<Label text="宽度自动填充剩余空间" font="1" textcolor="#FF606266" />
</VerticalLayout>
</HorizontalLayout>
<Label text="inset 控制内容边距，childpadding 控制子控件间距，borderround 提供原生圆角裁剪。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"layout") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Layout 布局" font="3" textcolor="#FF303133" />
<Label text="HorizontalLayout / VerticalLayout / TileLayout 的排布方式" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="14">
<Label text="水平布局：固定宽 + 自动填充" font="2" textcolor="#FF303133" />
<HorizontalLayout height="44" childpadding="8">
<VerticalLayout width="120" bkcolor="#FFECF5FF" borderround="4,4" childvalign="vcenter">
<Label text="120" font="1" textcolor="#FF409EFF" align="center" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFF0F9EB" borderround="4,4" childvalign="vcenter">
<Label text="自动填充" font="1" textcolor="#FF67C23A" align="center" />
</VerticalLayout>
<VerticalLayout width="160" bkcolor="#FFFDF6EC" borderround="4,4" childvalign="vcenter">
<Label text="160" font="1" textcolor="#FFE6A23C" align="center" />
</VerticalLayout>
</HorizontalLayout>
<Label text="垂直布局：VerticalLayout 顺序排列" font="2" textcolor="#FF303133" />
<VerticalLayout childpadding="6">
<Control height="36" bkcolor="#FFECF5FF" borderround="4,4" />
<Control height="36" bkcolor="#FFF0F9EB" borderround="4,4" />
<Control height="36" bkcolor="#FFFDF6EC" borderround="4,4" />
</VerticalLayout>
<Label text="TileLayout：等宽瓦片（columns=3）" font="2" textcolor="#FF303133" />
<TileLayout columns="3" itemsize="160,56" childvpadding="8" height="128">
<Label text="瓦片 1" font="1" textcolor="#FF409EFF" align="center" bkcolor="#FFECF5FF" borderround="4,4" />
<Label text="瓦片 2" font="1" textcolor="#FF409EFF" align="center" bkcolor="#FFECF5FF" borderround="4,4" />
<Label text="瓦片 3" font="1" textcolor="#FF409EFF" align="center" bkcolor="#FFECF5FF" borderround="4,4" />
<Label text="瓦片 4" font="1" textcolor="#FF67C23A" align="center" bkcolor="#FFF0F9EB" borderround="4,4" />
<Label text="瓦片 5" font="1" textcolor="#FF67C23A" align="center" bkcolor="#FFF0F9EB" borderround="4,4" />
<Label text="瓦片 6" font="1" textcolor="#FF67C23A" align="center" bkcolor="#FFF0F9EB" borderround="4,4" />
</TileLayout>
<Label text="duilib 没有 24 栅格与响应式断点，通常用固定宽 + 自动填充组合模拟。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"divider") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Divider 分割线" font="3" textcolor="#FF303133" />
<Label text="Control 1px 线条 + bkcolor 实现分割线" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础分割线" font="1" textcolor="#FF909399" />
<Control height="1" bkcolor="#FFDCDFE6" />
<HorizontalLayout height="20" childpadding="12" childvalign="vcenter">
<Control height="1" bkcolor="#FFDCDFE6" />
<Label text="文本分割线" font="1" textcolor="#FF909399" width="90" align="center" />
<Control height="1" bkcolor="#FFDCDFE6" />
</HorizontalLayout>
<HorizontalLayout height="40" childpadding="14" childvalign="vcenter">
<Label text="左侧内容" font="1" textcolor="#FF606266" />
<Control width="2" height="20" bkcolor="#FFDCDFE6" />
<Label text="右侧内容" font="1" textcolor="#FF606266" />
</HorizontalLayout>
<Label text="横向分割线使用高度 1px 的 Control，纵向分割线使用宽度 1px 的 Control。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"tag") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Tag 标签" font="3" textcolor="#FF303133" />
<Label text="Label + borderround 做静态标签；动态关闭需扩展" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<Label text="类型标签" font="2" textcolor="#FF303133" />
<HorizontalLayout height="34" childpadding="10" childvalign="vcenter">
<Label text="主要" width="64" font="1" textcolor="#FF409EFF" bkcolor="#FFECF5FF" bordercolor="#FFB3D8FF" bordersize="1" borderround="4,4" textpadding="10,4,10,4" align="center" />
<Label text="成功" width="64" font="1" textcolor="#FF67C23A" bkcolor="#FFF0F9EB" bordercolor="#FFC2E7B0" bordersize="1" borderround="4,4" textpadding="10,4,10,4" align="center" />
<Label text="警告" width="64" font="1" textcolor="#FFE6A23C" bkcolor="#FFFDF6EC" bordercolor="#FFF5DAB1" bordersize="1" borderround="4,4" textpadding="10,4,10,4" align="center" />
<Label text="危险" width="64" font="1" textcolor="#FFF56C6C" bkcolor="#FFFEF0F0" bordercolor="#FFFBC4C4" bordersize="1" borderround="4,4" textpadding="10,4,10,4" align="center" />
<Label text="信息" width="64" font="1" textcolor="#FF909399" bkcolor="#FFF4F4F5" bordercolor="#FFD3D4D6" bordersize="1" borderround="4,4" textpadding="10,4,10,4" align="center" />
</HorizontalLayout>
<Label text="带关闭样式的标签（静态展示）" font="2" textcolor="#FF303133" />
<HorizontalLayout height="34" childpadding="10" childvalign="vcenter">
<Label text="标签一 ×" width="92" font="1" textcolor="#FF409EFF" bkcolor="#FFECF5FF" bordercolor="#FFB3D8FF" bordersize="1" borderround="12,12" textpadding="12,4,12,4" align="center" />
<Label text="标签二 ×" width="92" font="1" textcolor="#FF67C23A" bkcolor="#FFF0F9EB" bordercolor="#FFC2E7B0" bordersize="1" borderround="12,12" textpadding="12,4,12,4" align="center" />
</HorizontalLayout>
<Label text="关闭按钮与动态增删需要写成自定义控件，本页仅演示静态外观。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"image") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Image 图片" font="3" textcolor="#FF303133" />
<Label text="Control 的 bkimage 属性显示图片资源" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Control name="demoImage" borderround="4,4" />
<Label text="图片以 RCDATA 内嵌在 exe 中，启动时释放到临时目录，再通过 SetResourcePath 加载。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"radio") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Radio 单选框" font="3" textcolor="#FF303133" />
<Label text="Option + group 实现互斥选择" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="14">
<Label text="单选框组（group 互斥）" font="2" textcolor="#FF303133" />
<HorizontalLayout height="30" childpadding="24">
<ERadio text="选项 A" group="rg1" selected="true" width="100" />
<ERadio text="选项 B" group="rg1" width="100" />
<ERadio text="选项 C" group="rg1" width="100" />
</HorizontalLayout>
<Label text="第二个分组 + 禁用项" font="2" textcolor="#FF303133" />
<HorizontalLayout height="30" childpadding="24">
<ERadio text="选项 D" group="rg2" width="100" />
<ERadio text="选项 E" group="rg2" width="100" />
<ERadio text="禁用选项" group="rg2" enabled="false" width="120" />
</HorizontalLayout>
<Label text="圆点与选中态由 EOptionUI 自绘；分组与互斥逻辑沿用 duilib Option。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"checkbox") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Checkbox 多选框" font="3" textcolor="#FF303133" />
<Label text="CheckBox 继承 Option，支持选中与禁用态" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="14">
<Label text="多选框" font="2" textcolor="#FF303133" />
<HorizontalLayout height="30" childpadding="24">
<ECheckBox text="复选框 A" selected="true" width="120" />
<ECheckBox text="复选框 B" width="120" />
<ECheckBox text="复选框 C" width="120" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="30" childpadding="24">
<ECheckBox text="已选禁用" selected="true" enabled="false" width="120" />
<ECheckBox text="未选禁用" enabled="false" width="120" />
</HorizontalLayout>
<Label text="勾选标记与圆角方块由 ECheckBoxUI 自绘，点击逻辑沿用 duilib CheckBox。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"input") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Input 输入框" font="3" textcolor="#FF303133" />
<Label text="Edit 内嵌原生 EDIT 子窗口，支持密码 / 只读 / 数字 / 禁用" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<HorizontalLayout height="92" childpadding="20">
<VerticalLayout width="190" childpadding="6">
<Label text="普通输入" font="1" textcolor="#FF606266" />
<Edit width="190" height="32" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFFFFFFF" text="duilib" />
<Label text="Edit 内的原生 EDIT 子窗口" font="1" textcolor="#FFC0C4CC" />
</VerticalLayout>
<VerticalLayout width="190" childpadding="6">
<Label text="密码输入" font="1" textcolor="#FF606266" />
<Edit width="190" height="32" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFFFFFFF" password="true" text="123456" />
<Label text="password=true" font="1" textcolor="#FFC0C4CC" />
</VerticalLayout>
<VerticalLayout width="190" childpadding="6">
<Label text="数字输入" font="1" textcolor="#FF606266" />
<Edit width="190" height="32" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFFFFFFF" numberonly="true" text="1024" />
<Label text="numberonly=true" font="1" textcolor="#FFC0C4CC" />
</VerticalLayout>
</HorizontalLayout>
<HorizontalLayout height="92" childpadding="20">
<VerticalLayout width="190" childpadding="6">
<Label text="只读输入" font="1" textcolor="#FF606266" />
<Edit width="190" height="32" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFF5F7FA" readonly="true" text="只读内容" />
<Label text="readonly=true" font="1" textcolor="#FFC0C4CC" />
</VerticalLayout>
<VerticalLayout width="190" childpadding="6">
<Label text="禁用输入" font="1" textcolor="#FF606266" />
<Edit width="190" height="32" font="0" textcolor="#FFC0C4CC" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFE4E7ED" borderround="4,4" nativebkcolor="#FFF5F7FA" enabled="false" text="禁用内容" />
<Label text="enabled=false" font="1" textcolor="#FFC0C4CC" />
</VerticalLayout>
</HorizontalLayout>
<Label text="placeholder、前后缀与一键清空等 Element 交互需要扩展 Edit；原生输入框外观仍由系统绘制。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"select") {
        return LR"(<VerticalLayout childpadding="18" height="2040">
<Label height="34" text="Select 选择器" font="3" textcolor="#FF303133" />
<Label height="22" text="ESelect：自绘选择框 + 浮层选项面板，覆盖 Element 文档中的全部用法" font="1" textcolor="#FF909399" />
<HorizontalLayout height="24" childpadding="8" childvalign="vcenter">
<Label width="70" text="当前选择：" font="1" textcolor="#FF909399" />
<Label name="selectResult" width="420" text="（未选择）" font="2" textcolor="#FF409EFF" />
</HorizontalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectBasic" demo="基础用法" width="240" placeholder="请选择" />
</HorizontalLayout>
<Label height="20" text="点击选择框展开选项面板，选中项以主色高亮。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="有禁用选项" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectDisabledOption" demo="禁用选项" width="240" placeholder="请选择" />
</HorizontalLayout>
<Label height="20" text="单个选项可置灰并禁止点选。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectDisabled" demo="禁用状态" width="240" placeholder="请选择" enabled="false" />
</HorizontalLayout>
<Label height="20" text="整体禁用时呈灰底、灰字与灰箭头，且不响应点击。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="可清空单选" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectClearable" demo="可清空" width="240" placeholder="请选择" clearable="true" />
</HorizontalLayout>
<Label height="20" text="悬停有值时右侧显示清空圆钮，点击清除当前选择。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="基础多选" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectMultiple" demo="基础多选" width="340" placeholder="请选择" multiple="true" clearable="true" />
</HorizontalLayout>
<Label height="20" text="已选项以标签展示，点击标签上的 × 可单独移除；选择后面板保持展开。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="自定义模板" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectTemplate" demo="自定义模板" width="260" placeholder="请选择" />
</HorizontalLayout>
<Label height="20" text="选项右侧附加说明文本，模拟自定义选项模板。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="分组" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectGroup" demo="分组" width="240" placeholder="请选择" />
</HorizontalLayout>
<Label height="20" text="不支持选择的选项组标题以次要字号显示，组内选项缩进排列。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="可搜索" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectFilter" demo="可搜索" width="240" placeholder="请选择" filterable="true" />
</HorizontalLayout>
<Label height="20" text="面板顶部提供搜索框，输入关键字即时过滤选项。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="远程搜索" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectRemote" demo="远程搜索" width="240" placeholder="请输入关键字" filterable="true" remote="true" />
</HorizontalLayout>
<Label height="20" text="输入后延迟 400ms 模拟远程请求，期间显示加载中。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="140" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="创建条目" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childvalign="vcenter">
<ESelect name="selectCreate" demo="创建条目" width="340" placeholder="请选择或输入" multiple="true" clearable="true" filterable="true" allowcreate="true" />
</HorizontalLayout>
<Label height="20" text="在搜索框输入不存在的内容后按回车，即可创建并选中新条目。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout height="156" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label height="24" text="尺寸" font="2" textcolor="#FF303133" />
<HorizontalLayout height="48" childpadding="16" childvalign="vcenter">
<ESelect name="selectSizeLarge" demo="尺寸-大" size="large" width="180" placeholder="请选择" />
<ESelect name="selectSizeDefault" demo="尺寸-默认" width="180" placeholder="请选择" />
<ESelect name="selectSizeSmall" demo="尺寸-小" size="small" width="180" placeholder="请选择" />
</HorizontalLayout>
<Label height="20" text="large / default / small 三档高度与字号。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"slider") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Slider 滑块" font="3" textcolor="#FF303133" />
<Label text="Slider 内置拖动逻辑，轨道与滑块自绘" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<Label text="拖动滑块或使用鼠标滚轮改变值" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="12" childvalign="vcenter">
<Label text="值" font="1" textcolor="#FF909399" width="40" />
<ESlider name="demoSlider" width="360" value="40" />
<Label name="sliderValue" text="40" font="2" textcolor="#FF409EFF" width="50" />
</HorizontalLayout>
<HorizontalLayout height="24" childpadding="12" childvalign="vcenter">
<Label text="禁用" font="1" textcolor="#FFC0C4CC" width="40" />
<ESlider width="360" value="65" enabled="false" />
<Label text="65" font="2" textcolor="#FFC0C4CC" width="50" />
</HorizontalLayout>
<Label text="VALUECHANGED 通知在松开鼠标或滚轮时触发；界面通过通知刷新数值标签。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"datetime") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="DateTime 日期时间" font="3" textcolor="#FF303133" />
<Label text="DateTime 使用系统日期选择器，面板样式随操作系统" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<Label text="日期选择" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childpadding="16" childvalign="vcenter">
<Label text="请选择日期" font="1" textcolor="#FF606266" width="100" />
<EDateTime name="dateDemo" width="220" height="32" textpadding="15,0,15,0" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childpadding="16" childvalign="vcenter">
<Label text="不可选择" font="1" textcolor="#FFC0C4CC" width="100" />
<EDateTime width="220" height="32" textpadding="15,0,15,0" bordercolor="#FFE4E7ED" bordersize="1" borderround="4,4" enabled="false" />
</HorizontalLayout>
<Label text="DateTime 是原生 SysDateTimePick32，日历弹窗为系统样式；时间格式需要改原生控件，属部分支持。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"table") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Table 表格" font="3" textcolor="#FF303133" />
<Label text="List + ListHeaderItem + ListTextElement，支持悬停与选中" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<VerticalLayout name="tableSlot" />
<Label text="表头宽度由 ListHeaderItem 固定宽度决定，最后一个不设宽度即自适应；行线、悬停色、选中色由 item* 属性控制。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"progress") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Progress 进度条" font="3" textcolor="#FF303133" />
<Label text="Progress 内置值 / 范围，圆角与颜色自绘" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<Label text="基础进度" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="12" childvalign="vcenter">
<EProgress width="360" value="30" />
<Label text="30%" font="1" textcolor="#FF606266" width="50" />
</HorizontalLayout>
<HorizontalLayout height="24" childpadding="12" childvalign="vcenter">
<EProgress width="360" value="70" />
<Label text="70%" font="1" textcolor="#FF606266" width="50" />
</HorizontalLayout>
<Label text="拖动滑块联动进度" font="2" textcolor="#FF303133" />
<HorizontalLayout height="24" childpadding="12" childvalign="vcenter">
<ESlider name="progressSlider" width="360" value="45" />
<Label name="progressValue" text="45%" font="2" textcolor="#FF409EFF" width="60" />
</HorizontalLayout>
<EProgress name="progressDemo" value="45" />
<Label text="原生 Progress 只绘制前景图片；本页的圆角轨道与主色填充由 EProgressUI 自绘。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"tree") {
        return LR"(<VerticalLayout childpadding="16">
<Label text="Tree 树形控件" font="3" textcolor="#FF303133" />
<Label text="ETree：自绘展开箭头 / 复选框 / 高亮 / 过滤 / 懒加载 / 手风琴 / 拖拽" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<ETree name="treeBasic" />
<Label text="默认展开一级节点，点击箭头展开 / 收起，点击节点行高亮" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="可选择" font="2" textcolor="#FF303133" />
<ETree name="treeCheck" />
<Label text="show-checkbox：勾选父节点联动子节点，子节点部分勾选时父节点显示半选" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="懒加载" font="2" textcolor="#FF303133" />
<ETree name="treeLazy" />
<Label text="lazy：点击箭头后模拟 600ms 加载二级节点；“一级节点 3（叶子）” 标记 isLeaf，无展开箭头" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="默认展开和默认勾选" font="2" textcolor="#FF303133" />
<ETree name="treeDefault" />
<Label text="default-expanded-keys / default-checked-keys：默认展开两级并勾选两个三级节点" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<ETree name="treeDisabled" />
<Label text="node.disabled：复选框置灰，节点不可勾选、不可选中" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="树节点选择" font="2" textcolor="#FF303133" />
<ETree name="treeSelect" />
<Label text="点击节点行设置当前节点并高亮，初始选中“二级节点 1-2”" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="自定义节点内容" font="2" textcolor="#FF303133" />
<ETree name="treeCustom" />
<Label text="render-content：行尾 “+” 按钮向当前节点追加子节点" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="节点过滤" font="2" textcolor="#FF303133" />
<Edit name="treeFilterEdit" width="240" height="30" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFFFFFFF" />
<ETree name="treeFilter" />
<Label text="filter-node-method：输入关键字即时过滤，匹配路径自动展开（试试“三级”或“2-2”）" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="手风琴模式" font="2" textcolor="#FF303133" />
<ETree name="treeAccordion" />
<Label text="accordion：同级节点同时只展开一个，展开其它一级节点会自动收起当前节点" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<Label text="可拖拽节点" font="2" textcolor="#FF303133" />
<ETree name="treeDrag" />
<Label text="draggable：按住节点拖到目标行上松开，节点会移动为目标节点的子节点" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"tabs") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Tabs 标签页" font="3" textcolor="#FF303133" />
<Label text="TabLayout 切换页面 + Button 做标签头" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="0">
<HorizontalLayout height="38" childpadding="0">
<ETabButton name="tab:0" text="用户管理" width="110" />
<ETabButton name="tab:1" text="配置管理" width="110" />
<ETabButton name="tab:2" text="角色管理" width="110" />
<Control />
</HorizontalLayout>
<Control height="1" bkcolor="#FFE4E7ED" />
<TabLayout name="tabHost" height="150">
<VerticalLayout inset="16,16,16,16" childpadding="8">
<Label text="用户管理" font="4" textcolor="#FF303133" />
<Label text="TabLayout 只显示当前选中的子容器，切换由 Button 点击驱动。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout inset="16,16,16,16" childpadding="8">
<Label text="配置管理" font="4" textcolor="#FF303133" />
<Label text="第二个子容器，由 TabLayout 按索引切换显示。" font="1" textcolor="#FF909399" />
</VerticalLayout>
<VerticalLayout inset="16,16,16,16" childpadding="8">
<Label text="角色管理" font="4" textcolor="#FF303133" />
<Label text="第三个子容器，选中标签头后显示。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</TabLayout>
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"tooltip") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Tooltip 文字提示" font="3" textcolor="#FF303133" />
<Label text="duilib 内置 tooltip 属性，使用系统 tooltip 窗口" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="16">
<Label text="将鼠标悬停在控件上查看提示" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childpadding="20" childvalign="vcenter">
<Label text="悬停查看 Label 提示" tooltip="这是 duilib 原生的 tooltip 提示" width="220" />
<EButton text="按钮提示" tooltip="按钮上的 tooltip" width="110" />
</HorizontalLayout>
<HorizontalLayout height="32" childpadding="20" childvalign="vcenter">
<Edit width="190" height="32" font="0" textcolor="#FF606266" textpadding="10,0,10,0" bordersize="1" bordercolor="#FFDCDFE6" borderround="4,4" nativebkcolor="#FFFFFFFF" tooltip="输入框上的 tooltip" text="悬停输入框" />
</HorizontalLayout>
<Label text="原生 tooltip 的延迟、字体与配色由系统决定，无 Element 深色气泡样式；自定义气泡需浮层控件。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"pagination") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Pagination 分页" font="3" textcolor="#FF303133" />
<Label text="EPagination：页码按钮组 + 首尾省略号 + 上一页 / 下一页" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childvalign="vcenter">
<EPagination name="demoPagination" total="100" pagesize="10" page="1" />
</HorizontalLayout>
<Label text="每页 20 条（共 10 页，当前第 5 页）" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childvalign="vcenter">
<EPagination total="200" pagesize="20" page="5" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="28" childvalign="vcenter">
<EPagination total="100" pagesize="10" page="2" enabled="false" />
</HorizontalLayout>
<Label name="paginationResult" text="当前页：1 / 共 10 页" font="1" textcolor="#FF909399" />
<Label text="页码与省略号由控件自绘，点击发送 valuechanged 通知；省略号向前 / 后跳转 5 页。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"badge") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Badge 标记" font="3" textcolor="#FF303133" />
<Label text="EBadge：角标水平内缩 10px 压住内容右上角，容器按“内容+角标”的 AABB 参与布局，支持数值上限、圆点与五种类型色" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="20">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<HorizontalLayout height="43" childpadding="30" childvalign="vcenter">
<EBadge value="9" width="96" height="34"><Label text="消息" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge value="120" max="99" width="96" height="34"><Label text="评论" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge dot="true" width="96" height="34"><Label text="通知" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
</HorizontalLayout>
<Label text="自定义内容" font="2" textcolor="#FF303133" />
<HorizontalLayout height="43" childpadding="30" childvalign="vcenter">
<EBadge value="new" width="96" height="34"><Label text="评论" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge value="hot" width="96" height="34"><Label text="回复" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
</HorizontalLayout>
<Label text="类型色" font="2" textcolor="#FF303133" />
<HorizontalLayout height="43" childpadding="30" childvalign="vcenter">
<EBadge value="12" type="primary" width="96" height="34"><Label text="主要" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge value="7" type="success" width="96" height="34"><Label text="成功" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge value="3" type="warning" width="96" height="34"><Label text="警告" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
<EBadge value="1" type="info" width="96" height="34"><Label text="信息" font="0" textcolor="#FF606266" align="center" bkcolor="#FFF4F4F5" bordercolor="#FFDCDFE6" bordersize="1" borderround="4,4" height="34" /></EBadge>
</HorizontalLayout>
<Label text="角标内缩 10px 压住内容右上角、垂直居中于上边缘；value 支持文本（new/hot）、数值上限 99+ 与 dot 圆点。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"avatar") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Avatar 头像" font="3" textcolor="#FF303133" />
<Label text="EAvatar：GDI+ 抗锯齿圆形 / 方形裁剪，支持图片、文字与图标" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="20">
<Label text="文字头像与类型色" font="2" textcolor="#FF303133" />
<HorizontalLayout height="72" childpadding="18" childvalign="vcenter">
<EAvatar text="虎" />
<EAvatar text="李" type="primary" />
<EAvatar text="张" type="success" />
<EAvatar text="王" type="danger" />
<Label text="默认 40，文字居中兜底" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="尺寸与形状" font="2" textcolor="#FF303133" />
<HorizontalLayout height="86" childpadding="18" childvalign="vcenter">
<EAvatar width="64" height="64" text="大" type="warning" />
<EAvatar width="24" height="24" text="小" type="info" />
<EAvatar shape="square" width="64" height="64" text="方" type="danger" />
<EAvatar shape="square" text="方" type="primary" />
<EAvatar name="avatarIcon" icons="true" type="primary" />
<Label text="圆形 64 / 圆形 24 / 方形 64 / 方形 40 / 图标" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="图片头像" font="2" textcolor="#FF303133" />
<HorizontalLayout height="72" childpadding="18" childvalign="vcenter">
<EAvatar src="demo-image.png" width="64" height="64" />
<EAvatar src="demo-image.png" shape="square" width="64" height="64" />
<Label text="src 指向 SetResourcePath 目录下的图片文件" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="圆形与方形均由 GDI+ 路径裁剪并抗锯齿，图片按控件尺寸居中裁剪缓存。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"loading") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Loading 加载" font="3" textcolor="#FF303133" />
<Label text="ELoading：GDI+ 抗锯齿旋转弧线，由定时器驱动" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="20">
<Label text="尺寸" font="2" textcolor="#FF303133" />
<HorizontalLayout height="44" childpadding="28" childvalign="vcenter">
<ELoading />
<ELoading width="32" height="32" />
<ELoading width="14" height="14" />
<Label text="默认 20 / 32 / 14" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="与文字组合" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childpadding="10" childvalign="vcenter">
<ELoading width="16" height="16" />
<Label text="正在加载数据…" font="0" textcolor="#FF606266" />
</HorizontalLayout>
<Label text="禁用状态" font="2" textcolor="#FF303133" />
<HorizontalLayout height="32" childpadding="28" childvalign="vcenter">
<ELoading enabled="false" />
<Label text="enabled=false" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="每 50ms 旋转 24°，控件移除或窗口销毁时定时器随之结束。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"breadcrumb") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Breadcrumb 面包屑" font="3" textcolor="#FF303133" />
<Label text="EBreadcrumb：横向路径 + 分隔符，点击节点切换当前层级" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="20">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<EBreadcrumb name="demoBreadcrumb" width="520" />
<Label name="breadcrumbResult" text="当前节点：活动详情" font="1" textcolor="#FF909399" />
<Label text="节点宽度按文字测量，悬停变主色，当前节点为深色文本。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"steps") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Steps 步骤条" font="3" textcolor="#FF303133" />
<Label text="ESteps：节点圆环 / 勾选 + 连接线，active 控制进度" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<ESteps name="demoSteps" steps="填写信息|确认订单|等待付款|完成" active="1" />
<HorizontalLayout height="34" childpadding="12" childvalign="vcenter">
<EButton name="stepsNext" text="下一步" variant="primary" width="96" />
<Label name="stepsResult" text="当前进度：1 / 4" font="1" textcolor="#FF909399" />
</HorizontalLayout>
<Label text="全部完成" font="2" textcolor="#FF303133" />
<ESteps steps="已完成|进行中|待开始" active="3" />
<Label text="已完成节点填充主色并绘制勾，当前节点白底主色描边，未开始节点为灰色描边。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"popover") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Popover 弹出框" font="3" textcolor="#FF303133" />
<Label text="EPopover：浮层气泡 + 顶部箭头，点击按钮弹出，点击外部关闭" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="18">
<Label text="基础用法" font="2" textcolor="#FF303133" />
<VerticalLayout name="popoverHost" height="180">
<EButton name="popoverBtn" text="点我弹出" variant="primary" width="110" />
<EPopover name="demoPopover" float="true" width="230" height="108" arrow="24" visible="false">
<VerticalLayout inset="14,10,14,12" childpadding="6">
<Label text="这是一段内容，可以放任意控件。" font="0" textcolor="#FF606266" />
<Label text="点击外部区域或下方按钮关闭。" font="1" textcolor="#FF909399" />
<HorizontalLayout height="26" childpadding="8">
<EButton name="popoverClose" text="关闭" variant="primary" plain="true" width="64" />
</HorizontalLayout>
</VerticalLayout>
</EPopover>
</VerticalLayout>
<Label text="气泡背景、圆角与箭头由 EPopoverUI 自绘；浮层坐标按按钮位置计算。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"collapse") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="Collapse 折叠面板" font="3" textcolor="#FF303133" />
<Label text="ECollapse：标题栏点击展开 / 收起，箭头与边框自绘" font="1" textcolor="#FF909399" />
<VerticalLayout name="collapseHost" bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="12">
<Label text="基础用法" font="2" textcolor="#FF303133" />
</VerticalLayout>
</VerticalLayout>)";
    }

    if (id == L"infinite") {
        return LR"(<VerticalLayout childpadding="18">
<Label text="InfiniteScroll 无限滚动" font="3" textcolor="#FF303133" />
<Label text="EInfiniteList：滚动到底部时发送 loadmore 通知并追加数据" font="1" textcolor="#FF909399" />
<VerticalLayout bkcolor="#FFFFFFFF" bordercolor="#FFEBEEF5" bordersize="1" borderround="4,4" inset="20,20,20,20" childpadding="10">
<EInfiniteList name="demoInfinite" height="300" />
<Label name="infiniteResult" text="已加载 15 条，继续向下滚动加载更多" font="1" textcolor="#FF909399" />
<Label text="在列表中向下滚动到底部触发加载，每次追加 5 条，共 30 条。" font="1" textcolor="#FF909399" />
</VerticalLayout>
</VerticalLayout>)";
    }

    return std::wstring();
}

} // namespace

}
