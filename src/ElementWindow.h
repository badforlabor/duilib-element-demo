#pragma once

#include "UIlib.h"

#include <map>
#include <string>
#include <vector>

namespace el {

class ENavItemUI;
class ESelectUI;
class ESelectPopupUI;
class ECascaderUI;
class ECascaderPopupUI;

class CMainWindow : public DuiLib::CWindowWnd, public DuiLib::INotifyUI
{
public:
    CMainWindow();

    enum class DemoMode { Component, Theme, Icon };

    void Show();
    void Notify(DuiLib::TNotifyUI& msg) override;
    LPCTSTR GetWindowClassName() const override;
    void OnFinalMessage(HWND hWnd) override;

protected:
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    void SetupFonts();
    void CreateNativeMenu();
    void CreateStatusBar();
    void UpdateNativeMenuState();
    void UpdateStatusBar();
    void OnNativeCommand(int id);
    void BuildNav();
    void BuildEditMenu();
    void BuildThemeDemo();
    void BuildIconPreview();
    void ShowDemoMode(DemoMode mode);
    void UpdateEditMenuState();
    void ToggleEditMenu();
    void CloseEditMenu();
    void CloseEditMenuIfOutside(POINT pt);
    void ShowPage(const std::wstring& id);
    void SelectNav(const std::wstring& id);
    DuiLib::CContainerUI* CreatePage(const std::wstring& id);

    void FillOverview();
    void FillIconPage();
    void FillImagePage();
    void FillTablePage();
    void FillTreePage();
    void FitTreeLayout(DuiLib::CControlUI* tree);
    void FillCascaderPage();
    void FillAvatarPage();
    void FillBreadcrumbPage();
    void FillCollapsePage();
    void FillInfiniteScrollPage();
    void FillSelectPage();
    void UpdateTabSelection();
    void AppendInfiniteRows(int count);

    void OpenPopover();
    void ClosePopover();
    void ClosePopoverIfOutside(POINT pt);

    void ToggleSelect(ESelectUI* select);
    void OpenSelect(ESelectUI* select);
    void CloseSelect();
    void CloseSelectIfOutside(POINT pt);
    void ShowSelectSummary(ESelectUI* select);

    void ToggleCascader(ECascaderUI* cascader);
    void OpenCascader(ECascaderUI* cascader);
    void CloseCascader();
    void CloseCascaderIfOutside(POINT pt);
    void ShowCascaderSummary(ECascaderUI* cascader);

    DuiLib::CListUI* CreateTable(const std::vector<std::pair<std::wstring, int>>& columns, int height);
    void AddTextRow(DuiLib::CListUI* list, const std::vector<std::wstring>& cells);
    void AddOverviewRow(DuiLib::CListUI* list, const std::wstring& category,
        const std::wstring& name, const std::wstring& status, const std::wstring& note);

    DuiLib::CPaintManagerUI m_pm;
    HMENU m_nativeMenu;
    HMENU m_viewMenu;
    HWND m_statusBar;
    DuiLib::CContainerUI* m_navHost;
    DuiLib::CContainerUI* m_pageHost;
    DuiLib::CContainerUI* m_componentHost;
    DuiLib::CContainerUI* m_themeHost;
    DuiLib::CContainerUI* m_iconHost;
    DuiLib::CControlUI* m_editMenuButton;
    DuiLib::CContainerUI* m_editMenu;
    DemoMode m_demoMode;
    std::map<std::wstring, ENavItemUI*> m_navItems;
    std::wstring m_currentPage;
    bool m_popoverVisible;
    int m_infiniteCount;
    ESelectUI* m_activeSelect;
    ESelectPopupUI* m_selectPopup;
    ECascaderUI* m_activeCascader;
    ECascaderPopupUI* m_cascaderPopup;
};

}
