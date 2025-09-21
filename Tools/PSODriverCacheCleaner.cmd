@echo off
setlocal EnableDelayedExpansion

:: Clear PSO Driver Cache Script
:: Cleans DirectX shader cache for NVIDIA, AMD and Intel GPUs

echo ========================================
echo PSO Driver Cache Cleaner
echo ========================================
echo.

:: Check LOCALAPPDATA environment variable
if "%LOCALAPPDATA%"=="" (
    echo [ERROR] LOCALAPPDATA environment variable is not set
    echo Please ensure LOCALAPPDATA points to C:\Users\^<username^>\AppData\Local
    pause
    exit /b 1
)

echo User data path: %LOCALAPPDATA%
echo.

:: Set LocalLow path
set "LOCALLOW=%LOCALAPPDATA%\..\LocalLow"

:: Initialize counter
set /a totalDeleted=0

:: NVIDIA cache cleanup
echo [NVIDIA] Clearing cache...

:: NVIDIA global cache
set "nvPath1=%LOCALAPPDATA%\NVIDIA\DXCache"
if exist "%nvPath1%" (
    echo   Clearing: %nvPath1%
    del /f /s /q "%nvPath1%\*" >nul 2>&1
    for /d %%x in ("%nvPath1%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %nvPath1%
)

:: NVIDIA per-driver cache
set "nvPath2=%LOCALLOW%\NVIDIA\PerDriverVersion\DXCache"
if exist "%nvPath2%" (
    echo   Clearing: %nvPath2%
    del /f /s /q "%nvPath2%\*" >nul 2>&1
    for /d %%x in ("%nvPath2%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %nvPath2%
)

echo.

:: AMD cache cleanup
echo [AMD] Clearing cache...

:: AMD LocalAppData paths
set "amdPath1=%LOCALAPPDATA%\AMD\DxCache"
if exist "%amdPath1%" (
    echo   Clearing: %amdPath1%
    del /f /s /q "%amdPath1%\*" >nul 2>&1
    for /d %%x in ("%amdPath1%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %amdPath1%
)

set "amdPath2=%LOCALAPPDATA%\AMD\DxcCache"
if exist "%amdPath2%" (
    echo   Clearing: %amdPath2%
    del /f /s /q "%amdPath2%\*" >nul 2>&1
    for /d %%x in ("%amdPath2%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %amdPath2%
)

:: AMD LocalLow paths
set "amdPath3=%LOCALLOW%\AMD\DxCache"
if exist "%amdPath3%" (
    echo   Clearing: %amdPath3%
    del /f /s /q "%amdPath3%\*" >nul 2>&1
    for /d %%x in ("%amdPath3%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %amdPath3%
)

set "amdPath4=%LOCALLOW%\AMD\DxcCache"
if exist "%amdPath4%" (
    echo   Clearing: %amdPath4%
    del /f /s /q "%amdPath4%\*" >nul 2>&1
    for /d %%x in ("%amdPath4%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %amdPath4%
)

echo.

:: Intel cache cleanup
echo [Intel] Clearing cache...

set "intelPath=%LOCALLOW%\Intel\ShaderCache"
if exist "%intelPath%" (
    echo   Clearing: %intelPath%
    del /f /s /q "%intelPath%\*" >nul 2>&1
    for /d %%x in ("%intelPath%\*") do @rd /s /q "%%x" >nul 2>&1
    set /a totalDeleted+=1
) else (
    echo   Path not found: %intelPath%
)

echo.
echo ========================================
echo Cleanup complete! Processed !totalDeleted! cache directories
echo ========================================
echo.

pause