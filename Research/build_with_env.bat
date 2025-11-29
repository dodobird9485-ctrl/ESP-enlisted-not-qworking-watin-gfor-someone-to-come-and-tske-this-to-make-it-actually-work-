@echo off
setlocal enabledelayedexpansion

REM Try to find and run VsDevCmd.bat
set "VS_PATHS="
set "VS_PATHS=!VS_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
set "VS_PATHS=!VS_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
set "VS_PATHS=!VS_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
set "VS_PATHS=!VS_PATHS! C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

set "DEVENV="
for %%p in (!VS_PATHS!) do (
    if exist "%%p" (
        set "DEVENV=%%p"
        goto :found
    )
)

:found
if "!DEVENV!"=="" (
    echo Visual Studio not found
    exit /b 1
)

echo Found VS at: !DEVENV!
call "!DEVENV!"

echo Building...
msbuild d:\Research\EnlistedESPOverlay.vcxproj /p:Configuration=Release /p:Platform=x64

if errorlevel 1 (
    echo Build FAILED
    exit /b 1
) else (
    echo Build succeeded
    exit /b 0
)
