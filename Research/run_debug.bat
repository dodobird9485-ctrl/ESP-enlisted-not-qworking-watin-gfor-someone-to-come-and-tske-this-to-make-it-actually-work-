@echo off
cd /d "D:\Research\x64\Release"
EnlistedESPOverlay.exe > debug.log 2>&1
echo Exit code: %ERRORLEVEL%
pause
type debug.log
