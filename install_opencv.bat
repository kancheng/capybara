@echo off
REM OpenCV 自動安裝批處理文件 / OpenCV Auto-Install Batch File
REM 此文件會調用 PowerShell 腳本進行安裝
REM This file calls the PowerShell script for installation

echo ========================================
echo OpenCV 自動安裝 / OpenCV Auto-Install
echo ========================================
echo.

REM 檢查 PowerShell 是否可用 / Check if PowerShell is available
powershell -Command "Get-Host" >nul 2>&1
if errorlevel 1 (
    echo 錯誤: 無法運行 PowerShell / Error: Cannot run PowerShell
    echo 請確保已安裝 PowerShell / Please ensure PowerShell is installed
    pause
    exit /b 1
)

REM 運行 PowerShell 腳本 / Run PowerShell script
echo 正在運行安裝腳本... / Running install script...
echo.

powershell -ExecutionPolicy Bypass -File "%~dp0install_opencv.ps1" %*

if errorlevel 1 (
    echo.
    echo 安裝失敗 / Installation failed
    pause
    exit /b 1
) else (
    echo.
    echo 安裝完成！請重新啟動終端或 IDE / Installation completed! Please restart terminal or IDE
    pause
)
