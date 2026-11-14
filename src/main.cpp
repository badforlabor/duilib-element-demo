#include "UIlib.h"
#include "ElementWindow.h"
#include "Resource.h"
#include "UiScale.h"

#include <windows.h>
#include <commctrl.h>
#include <dbghelp.h>

#include <string>

namespace {

std::wstring ExeDirectory()
{
    wchar_t path[MAX_PATH] = {};
    const DWORD length = ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return std::wstring();
    }
    std::wstring full(path, length);
    const size_t slash = full.find_last_of(L"\\/");
    return slash == std::wstring::npos ? full : full.substr(0, slash);
}

LONG WINAPI WriteCrashDump(EXCEPTION_POINTERS* info)
{
    const std::wstring directory = ExeDirectory() + L"\\Saved";
    ::CreateDirectoryW(directory.c_str(), nullptr);

    SYSTEMTIME now = {};
    ::GetLocalTime(&now);
    wchar_t file[MAX_PATH] = {};
    ::_snwprintf_s(file, _TRUNCATE,
        L"%s\\DuiLibElement_%04u%02u%02u_%02u%02u%02u_%03u.dmp", directory.c_str(),
        now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);

    HANDLE handle = ::CreateFileW(file, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exception = {};
        exception.ThreadId = ::GetCurrentThreadId();
        exception.ExceptionPointers = info;
        exception.ClientPointers = FALSE;
        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), handle,
            static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithHandleData
                | MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules),
            &exception, nullptr, nullptr);
        ::CloseHandle(handle);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void InstallCrashDumpHandler()
{
    ::SetUnhandledExceptionFilter(WriteCrashDump);
}

typedef UINT(WINAPI* GetDpiForSystemFn)();

int DetectSystemDpi()
{
    HMODULE user32 = ::GetModuleHandleW(L"user32.dll");
    if (user32 != nullptr) {
        auto getDpiForSystem = reinterpret_cast<GetDpiForSystemFn>(
            ::GetProcAddress(user32, "GetDpiForSystem"));
        if (getDpiForSystem != nullptr) {
            const UINT dpi = getDpiForSystem();
            if (dpi >= 72 && dpi <= 480) {
                return static_cast<int>(dpi);
            }
        }
    }

    HDC screenDc = ::GetDC(nullptr);
    int dpi = 96;
    if (screenDc != nullptr) {
        dpi = ::GetDeviceCaps(screenDc, LOGPIXELSX);
        ::ReleaseDC(nullptr, screenDc);
    }
    return dpi > 0 ? dpi : 96;
}

std::wstring ExtractDemoImage(HINSTANCE instance)
{
    wchar_t tempPath[MAX_PATH] = {};
    const DWORD length = ::GetTempPathW(MAX_PATH, tempPath);
    if (length == 0 || length >= MAX_PATH) {
        return std::wstring();
    }

    std::wstring directory = tempPath;
    directory += L"DuiLibElement";
    ::CreateDirectoryW(directory.c_str(), nullptr);

    HRSRC resource = ::FindResourceW(instance, MAKEINTRESOURCEW(IDR_DEMO_IMAGE), RT_RCDATA);
    if (resource == nullptr) {
        return std::wstring();
    }
    const DWORD size = ::SizeofResource(instance, resource);
    HGLOBAL memory = ::LoadResource(instance, resource);
    const void* data = memory != nullptr ? ::LockResource(memory) : nullptr;
    if (data == nullptr || size == 0) {
        return std::wstring();
    }

    const std::wstring file = directory + L"\\demo-image.png";
    HANDLE handle = ::CreateFileW(file.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return std::wstring();
    }
    DWORD written = 0;
    const BOOL ok = ::WriteFile(handle, data, size, &written, nullptr);
    ::CloseHandle(handle);
    ::FreeResource(resource);
    if (!ok || written != size) {
        return std::wstring();
    }
    return directory;
}

} // namespace

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    INITCOMMONCONTROLSEX controls = {};
    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
    ::InitCommonControlsEx(&controls);

    InstallCrashDumpHandler();

    UiScale::SetDpi(DetectSystemDpi());

    DuiLib::CPaintManagerUI::SetInstance(hInstance);

    const std::wstring resourcePath = ExtractDemoImage(hInstance);
    if (!resourcePath.empty()) {
        DuiLib::CPaintManagerUI::SetResourcePath(resourcePath.c_str());
    }

    el::CMainWindow* window = new el::CMainWindow();
    window->Show();

    DuiLib::CPaintManagerUI::MessageLoop();
    DuiLib::CPaintManagerUI::Term();
    return 0;
}
