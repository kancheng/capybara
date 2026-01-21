# 環境設定詳細說明 / Environment Setup Details

本文檔詳細說明 Capybara 環境設定模塊的實現和使用方法。

This document provides detailed information about the implementation and usage of Capybara's environment setup module.

## 目錄 / Table of Contents

- [功能實現 / Implementation](#功能實現--implementation)
- [Python 環境檢測 / Python Environment Detection](#python-環境檢測--python-environment-detection)
- [虛擬環境管理 / Virtual Environment Management](#虛擬環境管理--virtual-environment-management)
- [Conda 環境支持 / Conda Environment Support](#conda-環境支持--conda-environment-support)
- [依賴項檢測 / Dependency Detection](#依賴項檢測--dependency-detection)
- [系統信息檢測 / System Information Detection](#系統信息檢測--system-information-detection)
- [設定保存機制 / Settings Persistence](#設定保存機制--settings-persistence)
- [API 參考 / API Reference](#api-參考--api-reference)

## 功能實現 / Implementation

### 架構概述 / Architecture Overview

環境設定模塊採用 Qt 框架實現，主要包含以下組件：

The environment setup module is implemented using the Qt framework and mainly consists of the following components:

- **MainWindow**：主視窗類，負責 UI 和邏輯協調
- **Python Environment Scanner**：Python 環境掃描器
- **Virtual Environment Manager**：虛擬環境管理器
- **Dependency Detector**：依賴項檢測器
- **Settings Manager**：設定管理器

### 核心類和方法 / Core Classes and Methods

#### MainWindow 類

主要方法：

Main methods:

- `scanPythonEnvironments()`：掃描所有 Python 環境
- `scanVirtualEnvironments()`：掃描虛擬環境
- `detectPythonPackages()`：檢測 Python 套件
- `detectSystemInfo()`：檢測系統信息
- `saveSettings()`：保存設定
- `loadSettings()`：載入設定

## Python 環境檢測 / Python Environment Detection

### 檢測方法 / Detection Methods

應用程式使用三種方法檢測 Python 環境：

The application uses three methods to detect Python environments:

1. **PATH 環境變數掃描** / PATH Environment Variable Scan
   - 掃描系統 PATH 中的所有 Python 可執行文件
   - Scans all Python executables in the system PATH

2. **Python Launcher (py.exe)** / Python Launcher
   - 使用 Windows Python Launcher 檢測已安裝的 Python 版本
   - Uses Windows Python Launcher to detect installed Python versions

3. **常見安裝位置** / Common Installation Locations
   - 檢查常見的 Python 安裝目錄
   - Checks common Python installation directories

### 檢測流程 / Detection Process

```
1. 掃描 PATH 環境變數
   ↓
2. 使用 py.exe -0 列出所有 Python 版本
   ↓
3. 檢查常見安裝位置（Program Files, AppData 等）
   ↓
4. 驗證每個 Python 路徑的有效性
   ↓
5. 獲取 Python 版本信息
   ↓
6. 去重並排序
```

## 虛擬環境管理 / Virtual Environment Management

### 支持的虛擬環境類型 / Supported Virtual Environment Types

1. **標準 venv** / Standard venv
   - Python 內建的虛擬環境
   - Built-in Python virtual environments

2. **Conda 環境** / Conda Environments
   - Anaconda/Miniconda 環境
   - Anaconda/Miniconda environments

3. **虛擬環境目錄** / Virtual Environment Directories
   - 自定義位置的虛擬環境
   - Custom-located virtual environments

### 虛擬環境檢測 / Virtual Environment Detection

#### Conda 環境檢測

應用程式通過以下方式檢測 Conda 環境：

The application detects Conda environments through:

1. 執行 `conda info -e` 或 `conda env list` 命令
2. 解析輸出獲取環境名稱和路徑
3. 驗證環境路徑是否存在
4. 檢查 `conda-meta` 目錄確認 Conda 環境

#### 標準虛擬環境檢測

檢測標準虛擬環境的方法：

Methods for detecting standard virtual environments:

1. 掃描常見虛擬環境目錄（.venv, venv, env 等）
2. 檢查 `pyvenv.cfg` 文件
3. 驗證 Python 可執行文件存在

### 虛擬環境選擇邏輯 / Virtual Environment Selection Logic

應用程式會優先選擇：

The application will prioritize:

1. **保存的虛擬環境** / Saved Virtual Environment
   - 如果之前保存過設定，優先選擇保存的環境
   - If settings were previously saved, prioritize the saved environment

2. **第一個可用環境** / First Available Environment
   - 如果沒有保存的設定，選擇列表中的第一個環境
   - If no saved settings, select the first environment in the list

## Conda 環境支持 / Conda Environment Support

### Conda 環境識別 / Conda Environment Identification

應用程式通過以下特徵識別 Conda 環境：

The application identifies Conda environments by:

- 路徑中包含 "anaconda" 或 "miniconda"
- 存在 `conda-meta` 目錄
- 環境名稱與 `conda info -e` 輸出匹配

### Conda 環境顯示 / Conda Environment Display

- Conda 環境在列表中顯示為 `[Conda] 環境名稱`
- Conda environments are displayed as `[Conda] Environment Name` in the list
- 顯示完整的環境路徑
- Displays the complete environment path

## 依賴項檢測 / Dependency Detection

### PyTorch 檢測 / PyTorch Detection

檢測流程：

Detection process:

1. 執行 `pip show torch` 獲取詳細信息
2. 如果失敗，使用 `pip list` 作為備用方法
3. 解析版本信息
4. 執行 Python 代碼檢測 CUDA 支持：
   ```python
   import torch
   print(torch.__version__)
   print(torch.cuda.is_available())
   print(torch.version.cuda if torch.cuda.is_available() else 'N/A')
   ```

### Ultralytics 檢測 / Ultralytics Detection

檢測以下套件：

Detects the following packages:

- `ultralytics`：主要 YOLO 套件
- `ultralytics-thop`：模型複雜度分析工具

### 環境變數處理 / Environment Variable Handling

為確保正確檢測，應用程式會：

To ensure correct detection, the application:

- 清除可能干擾的 `PYTHONHOME` 環境變數
- Clears potentially interfering `PYTHONHOME` environment variable
- 清除可能干擾的 `PYTHONPATH` 環境變數
- Clears potentially interfering `PYTHONPATH` environment variable
- 使用選定環境的 Python 可執行文件
- Uses the selected environment's Python executable

## 系統信息檢測 / System Information Detection

### 系統架構檢測 / System Architecture Detection

使用 `QSysInfo::currentCpuArchitecture()` 檢測：
- x64（64 位）
- x86（32 位）

### CUDA 檢測 / CUDA Detection

檢測方法：

Detection methods:

1. 執行 `nvidia-smi` 獲取 CUDA 版本和驅動信息
2. 掃描常見 CUDA 安裝路徑
3. 顯示 CUDA 版本和驅動版本

### GPU 檢測 / GPU Detection

使用以下命令檢測 GPU：

Uses the following commands to detect GPUs:

- `nvidia-smi`：NVIDIA GPU 信息
- `wmic path win32_VideoController get name`：Windows GPU 信息

## 設定保存機制 / Settings Persistence

### 設定存儲 / Settings Storage

使用 Qt 的 `QSettings` 類保存設定：

Uses Qt's `QSettings` class to save settings:

- **組織名稱** / Organization Name：Capybara
- **應用名稱** / Application Name：PythonEnvironment
- **存儲位置** / Storage Location：Windows 註冊表或配置文件

### 保存的設定項 / Saved Settings

應用程式保存以下設定：

The application saves the following settings:

- `pythonPath`：當前選擇的 Python 環境路徑
- `venvName`：虛擬環境名稱
- `venvPath`：虛擬環境路徑
- `venvPythonPath`：虛擬環境中的 Python 可執行文件路徑

### 設定載入流程 / Settings Load Process

1. 應用程式啟動時檢查是否有保存的設定
2. 如果有，載入設定值
3. 驗證保存的路徑是否仍然存在
4. 自動選擇對應的 Python 環境和虛擬環境
5. 自動執行套件檢測

### 設定應用 / Settings Application

設定應用時會：

When applying settings:

- 暫時阻止 UI 信號發射，避免遞歸調用
- Temporarily blocks UI signal emission to avoid recursive calls
- 設置 ComboBox 的選擇
- Sets ComboBox selections
- 更新所有相關標籤
- Updates all related labels
- 自動執行套件檢測
- Automatically runs package detection

## API 參考 / API Reference

### 主要方法 / Main Methods

#### `void scanPythonEnvironments()`

掃描系統中的所有 Python 環境。

Scans all Python environments on the system.

#### `void scanVirtualEnvironments(const QString &pythonPath)`

掃描指定 Python 環境下的虛擬環境。

Scans virtual environments under the specified Python environment.

**參數** / Parameters:
- `pythonPath`：Python 可執行文件路徑

#### `void detectPythonPackages(const QString &pythonPath)`

檢測指定 Python 環境中安裝的套件。

Detects packages installed in the specified Python environment.

**參數** / Parameters:
- `pythonPath`：Python 可執行文件路徑

#### `void saveSettings()`

保存當前環境選擇到設定文件。

Saves current environment selection to settings file.

#### `void loadSettings()`

從設定文件載入環境選擇。

Loads environment selection from settings file.

#### `bool hasSavedSettings()`

檢查是否有保存的設定。

Checks if saved settings exist.

**返回** / Returns:
- `true`：如果有保存的設定
- `false`：如果沒有保存的設定

#### `void applySavedSettings()`

應用保存的設定到 UI。

Applies saved settings to UI.

## 故障排除 / Troubleshooting

### 常見問題 / Common Issues

#### 問題：Python 環境未檢測到

**可能原因** / Possible Causes:
- Python 未添加到系統 PATH
- Python 安裝不完整

**解決方法** / Solutions:
- 重新安裝 Python 並確保添加到 PATH
- 手動添加 Python 到系統 PATH

#### 問題：Conda 環境未顯示

**可能原因** / Possible Causes:
- Conda 未正確安裝
- Conda 未添加到 PATH

**解決方法** / Solutions:
- 確保 Conda 已正確安裝
- 在命令行運行 `conda info -e` 確認環境列表
- 重新初始化 Conda

#### 問題：PyTorch/Ultralytics 顯示未安裝

**可能原因** / Possible Causes:
- 選擇了錯誤的虛擬環境
- 套件確實未安裝
- 環境變數干擾

**解決方法** / Solutions:
- 確認已選擇正確的虛擬環境
- 在選定的環境中運行 `pip list` 確認
- 重新安裝套件

## 相關文檔 / Related Documentation

- [環境設定快速指南](environment-setup.md)
- [數據處理](data-processing.md)
- [返回主目錄](README.md)

## 開發者信息 / Developer Information

### 技術棧 / Technology Stack

- **框架** / Framework：Qt 6
- **語言** / Language：C++17
- **構建系統** / Build System：CMake
- **編譯器** / Compiler：MinGW-w64

### 代碼結構 / Code Structure

```
mainwindow.h          - 類定義和聲明
mainwindow.cpp        - 類實現
mainwindow.ui         - UI 設計文件
```

### 擴展指南 / Extension Guide

要添加新的檢測功能：

To add new detection features:

1. 在 `mainwindow.h` 中添加方法聲明
2. 在 `mainwindow.cpp` 中實現方法
3. 在適當的位置調用新方法
4. 更新 UI 以顯示結果
