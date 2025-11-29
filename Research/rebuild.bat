@echo off
setlocal enabledelayedexpansion

REM Look for Visual Studio 2022
for /D %%D in ("C:\Program Files\Microsoft Visual Studio\2022\*") do (
    if exist "%%D\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD=%%D\MSBuild\Current\Bin\MSBuild.exe"
        goto :found
    )
)

REM Look for Visual Studio 2019
for /D %%D in ("C:\Program Files (x86)\Microsoft Visual Studio\2019\*") do (
    if exist "%%D\MSBuild\Current\Bin\MSBuild.exe" (
        set "MSBUILD=%%D\MSBuild\Current\Bin\MSBuild.exe"
        goto :found
    )
)

:found
if not defined MSBUILD (
    echo MSBuild not found
    exit /b 1
)

echo Found MSBuild at: !MSBUILD!
"!MSBUILD!" "d:\Research\EnlistedESPOverlay.vcxproj" /p:Configuration=Release /p:Platform=x64 /m

if errorlevel 1 (
    echo Build failed
    exit /b 1
) else (
    echo Build succeeded
    if exist "d:\Research\x64\Release\EnlistedESPOverlay.exe" (
        echo Running executable...
        "d:\Research\x64\Release\EnlistedESPOverlay.exe" --test
    )
)
