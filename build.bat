@echo off
setlocal enabledelayedexpansion

set "ROOT=%~dp0"
set "PF86=%ProgramFiles(x86)%"
set "BUILD_DIR=%ROOT%build"
set "DIST_DIR=%ROOT%dist"
set "ARTIFACT=DuiLibElement.exe"

set "VSWHERE=%PF86%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSDIR="
set "VSMAJOR="
if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
    for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) do set "VSMAJOR=%%i"
)
if defined VSMAJOR for /f "tokens=1 delims=." %%v in ("!VSMAJOR!") do set "VSMAJOR=%%v"

set "GENERATOR=Visual Studio 17 2022"
if "%VSMAJOR%"=="16" set "GENERATOR=Visual Studio 16 2019"
if "%VSMAJOR%"=="15" set "GENERATOR=Visual Studio 15 2017"

set "CMAKE_EXE="
where cmake >nul 2>nul && set "CMAKE_EXE=cmake"
if not defined CMAKE_EXE if defined CMAKE if exist "%CMAKE%" set "CMAKE_EXE=%CMAKE%"
if not defined CMAKE_EXE if defined VSDIR if exist "%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" set "CMAKE_EXE=%VSDIR%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
rem Standalone CMake installs (may live on any drive, e.g. D:\Program Files\CMake)
if not defined CMAKE_EXE for %%r in (C D E F) do if not defined CMAKE_EXE if exist "%%r:\Program Files\CMake\bin\cmake.exe" set "CMAKE_EXE=%%r:\Program Files\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "%ProgramFiles%\CMake\bin\cmake.exe" set "CMAKE_EXE=%ProgramFiles%\CMake\bin\cmake.exe"
if not defined CMAKE_EXE if exist "%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe" set "CMAKE_EXE=%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe"

if not defined CMAKE_EXE (
    echo [ERROR] cmake not found. Install Visual Studio C++ tools or put cmake in PATH.
    exit /b 1
)

echo [INFO] cmake: %CMAKE_EXE%
echo [INFO] generator: %GENERATOR%

rem duilib's UIFlash/UIWebBrowser controls need ATL headers (atlcomcli.h).
rem Some VS installs only ship ATL for the newest VC toolset, while MSBuild may
rem default to an older toolset without ATL. Pick the newest toolset that has ATL
rem and pass it to MSBuild through the VCToolsVersion environment variable.
if not defined VCToolsVersion if defined VSDIR (
    for /f "delims=" %%v in ('dir /b /ad /o-n "%VSDIR%\VC\Tools\MSVC" 2^>nul') do (
        if not defined VCToolsVersion if exist "%VSDIR%\VC\Tools\MSVC\%%v\atlmfc\include\atlcomcli.h" set "VCToolsVersion=%%v"
    )
)
if defined VCToolsVersion (
    echo [INFO] toolset: %VCToolsVersion% - ATL found
) else (
    echo [WARN] no VC toolset with ATL found; install "C++ ATL" for the VS build tools
)

"%CMAKE_EXE%" -S "%ROOT%." -B "%BUILD_DIR%" -G "%GENERATOR%" -A x64
if errorlevel 1 exit /b 1

"%CMAKE_EXE%" --build "%BUILD_DIR%" --config Release --parallel
if errorlevel 1 exit /b 1

if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"
copy /y "%ROOT%bin\Release\DuiLibElement.exe" "%DIST_DIR%\%ARTIFACT%" >nul
if errorlevel 1 exit /b 1
copy /y "%ROOT%bin\Release\DuiLibElement.pdb" "%DIST_DIR%\DuiLibElement.pdb" >nul
if errorlevel 1 exit /b 1

echo [OK] %DIST_DIR%\%ARTIFACT%
echo [OK] %DIST_DIR%\DuiLibElement.pdb
endlocal
