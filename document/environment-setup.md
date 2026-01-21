# 環境設定 / Environment Setup

環境設定是使用 Capybara 的第一步。本模塊負責檢測和管理 Python 環境，確保所有依賴項正確安裝。

Environment setup is the first step in using Capybara. This module is responsible for detecting and managing Python environments, ensuring all dependencies are correctly installed.

## 功能概述 / Feature Overview

環境設定模塊提供以下功能：

The environment setup module provides the following features:

- **自動檢測 Python 環境** / Automatic Python Environment Detection
- **Conda 環境支持** / Conda Environment Support
- **虛擬環境管理** / Virtual Environment Management
- **依賴項檢測** / Dependency Detection
- **CUDA 和 GPU 檢測** / CUDA and GPU Detection
- **設定保存和載入** / Settings Save and Load

## 詳細說明 / Detailed Documentation

有關環境設定的詳細說明，請參閱：[環境設定詳細說明](environment-setup-details.md)

For detailed documentation on environment setup, please refer to: [Environment Setup Details](environment-setup-details.md)

## 快速指南 / Quick Guide

### 1. 啟動應用程式 / Launch Application

啟動 Capybara 後，應用程式會自動掃描系統中的所有 Python 環境。

After launching Capybara, the application will automatically scan all Python environments on your system.

### 2. 選擇 Python 環境 / Select Python Environment

1. 在「Python 環境」下拉選單中選擇您要使用的 Python 版本
2. 應用程式會自動掃描該環境下的虛擬環境

1. Select the Python version you want to use from the "Python Environment" dropdown
2. The application will automatically scan for virtual environments under that environment

### 3. 選擇虛擬環境 / Select Virtual Environment

1. 在「虛擬環境」下拉選單中選擇您的 Conda 環境或虛擬環境
2. 應用程式會自動檢測該環境中安裝的套件

1. Select your Conda environment or virtual environment from the "Virtual Environment" dropdown
2. The application will automatically detect packages installed in that environment

### 4. 指定環境 / Specify Environment

點擊「指定此環境」按鈕來保存您的選擇。應用程式會：
- 保存您的環境選擇
- 檢測 PyTorch 和 Ultralytics 的安裝狀態
- 顯示 CUDA 支持信息

Click the "指定此環境" button to save your selection. The application will:
- Save your environment selection
- Detect PyTorch and Ultralytics installation status
- Display CUDA support information

## 系統信息檢測 / System Information Detection

應用程式會自動檢測以下系統信息：

The application automatically detects the following system information:

- **系統架構** / System Architecture：x64 或 x86
- **CUDA 版本** / CUDA Version：如果已安裝 NVIDIA GPU 驅動
- **GPU 信息** / GPU Information：可用的 GPU 設備

## 依賴項檢測 / Dependency Detection

應用程式會檢測以下 Python 套件：

The application detects the following Python packages:

- **PyTorch**：版本和 CUDA 支持狀態
- **Ultralytics**：YOLO 相關工具套件
- **ultralytics-thop**：模型複雜度分析工具

## 設定保存 / Settings Persistence

您的環境選擇會自動保存，下次啟動應用程式時會自動載入。

Your environment selection is automatically saved and will be loaded automatically the next time you launch the application.

## 相關文檔 / Related Documentation

- [環境設定詳細說明](environment-setup-details.md)
- [數據處理](data-processing.md) - 下一步
- [返回主目錄](README.md)

## 故障排除 / Troubleshooting

### Python 環境未檢測到 / Python Environment Not Detected

- 確保 Python 已正確安裝並添加到系統 PATH
- 檢查 Python 安裝是否完整

### Conda 環境未顯示 / Conda Environments Not Displayed

- 確保 Conda 已正確安裝
- 嘗試在命令行運行 `conda info -e` 確認環境列表

### PyTorch/Ultralytics 顯示未安裝 / PyTorch/Ultralytics Shows Not Installed

- 確認已選擇正確的虛擬環境
- 在選定的環境中運行 `pip list` 確認套件是否安裝
- 檢查 Python 路徑是否正確

## 下一步 / Next Steps

完成環境設定後，您可以：

After completing environment setup, you can:

1. 開始[數據處理](data-processing.md)
2. 進行[數據標註](data-annotation.md)
3. 準備[模型訓練](model-training.md)
