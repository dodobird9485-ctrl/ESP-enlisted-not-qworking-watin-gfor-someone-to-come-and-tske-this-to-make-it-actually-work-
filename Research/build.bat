@echo off
setlocal enabledelayedexpansion

REM Try common Visual Studio 2022 paths
set "MSBUILD_PATHS="
set "MSBUILD_PATHS=!MSBUILD_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
set "MSBUILD_PATHS=!MSBUILD_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
set "MSBUILD_PATHS=!MSBUILD_PATHS! C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
set "MSBUILD_PATHS=!MSBUILD_PATHS! C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
set "MSBUILD_PATHS=!MSBUILD_PATHS! C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"

set "MSBUILD="
for %%p in (!MSBUILD_PATHS!) do (
    if exist "%%p" (
        set "MSBUILD=%%p"
        goto :found
    )
)

:found
if "!MSBUILD!"=="" (
    echo MSBuild not found in expected locations
    exit /b 1
)

echo Found MSBuild at: !MSBUILD!
call "!MSBUILD!" d:\Research\EnlistedESPOverlay.vcxproj /p:Configuration=Release /p:Platform=x64

if errorlevel 1 (
    echo Build FAILED
    exit /b 1
) else (
    echo Build succeeded
    exit /b 0
)
