<div align="center">

# Capybara

Under development | In Entwicklung | En développement | 開発中 | 開發中


![Capybara Logo](icon/icon.png)

**A unified GUI for vision model training, ONNX export, and cross-platform deployment.**

**統一的視覺模型訓練、ONNX 匯出和跨平台部署 GUI 工具**

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.10.1-green.svg)](https://www.qt.io/)
[![Python](https://img.shields.io/badge/Python-3.8+-blue.svg)](https://www.python.org/)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

</div>

---

## 📖 目錄 / Table of Contents

- [簡介 / Introduction](#簡介--introduction)
- [功能特點 / Features](#功能特點--features)
- [系統要求 / System Requirements](#系統要求--system-requirements)
- [安裝指南 / Installation](#安裝指南--installation)
- [快速開始 / Quick Start](#快速開始--quick-start)
- [使用手冊 / Documentation](#使用手冊--documentation)
- [項目結構 / Project Structure](#項目結構--project-structure)
- [開發 / Development](#開發--development)
- [貢獻 / Contributing](#貢獻--contributing)
- [許可證 / License](#許可證--license)

## 🎯 簡介 / Introduction

Capybara 是一個專為機器學習工作流程設計的綜合性 GUI 工具，從環境配置到模型部署，提供一站式的解決方案。它簡化了視覺模型訓練、數據處理和模型匯出的複雜流程，讓開發者能夠更專注於模型本身。

Capybara is a comprehensive GUI tool designed for machine learning workflows, providing a one-stop solution from environment configuration to model deployment. It simplifies the complex processes of vision model training, data processing, and model export, allowing developers to focus more on the models themselves.

### 核心價值 / Core Value

- 🚀 **簡化工作流程** / Simplified Workflow：從數據到部署的完整流程
- 🎨 **直觀的用戶界面** / Intuitive UI：易於使用的圖形界面
- 🔧 **自動化環境管理** / Automated Environment Management：智能檢測和管理 Python 環境
- 📦 **多格式支持** / Multi-format Support：支持多種數據格式和模型格式
- 🌐 **跨平台部署** / Cross-platform Deployment：支持多種部署目標

## ✨ 功能特點 / Features

### ✅ 已實現 / Implemented

- **環境設定模塊** / Environment Setup Module
  - ✅ 自動檢測 Python 環境（PATH、Python Launcher、常見位置）
  - ✅ Conda 環境支持（自動檢測和顯示）
  - ✅ 虛擬環境管理
  - ✅ PyTorch 和 Ultralytics 依賴檢測
  - ✅ CUDA 和 GPU 檢測
  - ✅ 設定保存和自動載入

### 🚧 規劃中 / In Planning

- **數據處理模塊** / Data Processing Module
  - 數據導入和預處理
  - 數據增強
  - 數據統計分析

- **數據標註模塊** / Data Annotation Module
  - 圖像標註工具（邊界框、多邊形、關鍵點）
  - 多格式支持（YOLO、COCO、Pascal VOC）

- **數據格式轉換模塊** / Data Format Conversion Module
  - 標註格式轉換
  - 批量轉換功能

- **模型訓練模塊** / Model Training Module
  - PyTorch 和 YOLO 訓練支持
  - 訓練監控和可視化
  - 超參數配置

- **模型匯出模塊** / Model Export Module
  - ONNX 匯出
  - TensorRT、CoreML、TensorFlow Lite 支持
  - 模型優化和量化

## 💻 系統要求 / System Requirements

### 最低要求 / Minimum Requirements

- **操作系統** / Operating System：Windows 10/11 或更高版本
- **Python**：3.8 或更高版本
- **內存** / RAM：至少 4GB
- **硬盤空間** / Disk Space：至少 500MB

### 推薦配置 / Recommended Configuration

- **操作系統** / Operating System：Windows 11
- **Python**：3.10 或更高版本
- **內存** / RAM：8GB 或更多
- **GPU**：NVIDIA GPU（支持 CUDA，用於 GPU 加速）
- **硬盤空間** / Disk Space：至少 2GB（包含模型和數據集）

### 依賴項 / Dependencies

- **Qt 6.10.1** 或更高版本
- **PyTorch**（可選，用於模型訓練）
- **Ultralytics**（可選，用於 YOLO 模型）
- **CUDA**（可選，用於 GPU 加速）

## 📦 安裝指南 / Installation

### 從源代碼構建 / Build from Source

1. **克隆倉庫** / Clone Repository
   ```bash
   git clone https://github.com/kancheng/capybara.git
   cd capybara
   ```

2. **安裝 Qt** / Install Qt
   - 下載並安裝 [Qt 6.10.1](https://www.qt.io/download) 或更高版本
   - 確保 Qt Creator 或 CMake 已安裝

3. **配置 CMake** / Configure CMake
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

4. **編譯** / Build
   ```bash
   cmake --build .
   ```

5. **運行** / Run
   ```bash
   ./capybara.exe
   ```

### 使用 Qt Creator

1. 打開 Qt Creator
2. 選擇「打開項目」/ "Open Project"
3. 選擇 `CMakeLists.txt`
4. 配置構建設置
5. 點擊「構建」/ "Build" 和「運行」/ "Run"

## 🚀 快速開始 / Quick Start

### 1. 啟動應用程式 / Launch Application

啟動 Capybara 後，應用程式會自動掃描系統中的所有 Python 環境。

After launching Capybara, the application will automatically scan all Python environments on your system.

### 2. 配置環境 / Configure Environment

1. **選擇 Python 環境** / Select Python Environment
   - 從「Python 環境」下拉選單中選擇您要使用的 Python 版本
   - Select the Python version you want to use from the "Python Environment" dropdown

2. **選擇虛擬環境** / Select Virtual Environment
   - 從「虛擬環境」下拉選單中選擇您的 Conda 環境或虛擬環境
   - Select your Conda environment or virtual environment from the "Virtual Environment" dropdown

3. **指定環境** / Specify Environment
   - 點擊「指定此環境」按鈕保存您的選擇
   - Click the "指定此環境" button to save your selection
   - 應用程式會自動檢測 PyTorch 和 Ultralytics 的安裝狀態
   - The application will automatically detect PyTorch and Ultralytics installation status

### 3. 查看系統信息 / View System Information

應用程式會自動顯示：
- 系統架構（x64/x86）
- CUDA 版本（如果已安裝）
- GPU 信息（如果可用）

The application automatically displays:
- System architecture (x64/x86)
- CUDA version (if installed)
- GPU information (if available)

## 📚 使用手冊 / Documentation

詳細的使用手冊和 API 文檔請參閱 [document 目錄](document/README.md)。

For detailed user manual and API documentation, please refer to the [document directory](document/README.md).

### 模塊文檔 / Module Documentation

- 📘 [環境設定](document/environment-setup.md) - 環境配置和依賴檢測
- 📗 [環境設定詳細說明](document/environment-setup-details.md) - 技術實現細節和 API 參考
- 📙 [數據處理](document/data-processing.md) - 數據預處理和增強（規劃中）
- 📕 [數據標註](document/data-annotation.md) - 圖像標註工具（規劃中）
- 📔 [數據格式轉換](document/data-conversion.md) - 標註格式轉換（規劃中）
- 📓 [模型訓練](document/model-training.md) - 模型訓練流程（規劃中）
- 📒 [模型匯出](document/model-export.md) - 模型導出和部署（規劃中）

## 📁 項目結構 / Project Structure

```
capybara/
├── document/                 # 文檔目錄 / Documentation directory
│   ├── README.md            # 使用手冊主頁 / User manual homepage
│   ├── environment-setup.md # 環境設定指南 / Environment setup guide
│   ├── environment-setup-details.md # 環境設定詳細說明 / Detailed documentation
│   ├── data-processing.md   # 數據處理文檔 / Data processing docs
│   ├── data-annotation.md   # 數據標註文檔 / Data annotation docs
│   ├── data-conversion.md   # 格式轉換文檔 / Format conversion docs
│   ├── model-training.md    # 模型訓練文檔 / Model training docs
│   └── model-export.md      # 模型匯出文檔 / Model export docs
├── icon/                    # 圖標資源 / Icon resources
│   ├── icon.png            # 項目 LOGO / Project logo
│   ├── raw.png             # 原始圖標 / Raw icon
│   └── raw.ico             # Windows 圖標 / Windows icon
├── build/                   # 構建目錄 / Build directory
├── main.cpp                 # 主程序入口 / Main entry point
├── mainwindow.h             # 主視窗頭文件 / Main window header
├── mainwindow.cpp           # 主視窗實現 / Main window implementation
├── mainwindow.ui            # UI 設計文件 / UI design file
├── resources.qrc            # Qt 資源文件 / Qt resource file
├── capybara.rc              # Windows 資源文件 / Windows resource file
├── CMakeLists.txt           # CMake 構建配置 / CMake build configuration
├── LICENSE                  # 許可證文件 / License file
└── README.md               # 本文件 / This file
```

## 🛠️ 開發 / Development

### 技術棧 / Technology Stack

- **框架** / Framework：[Qt 6.10.1](https://www.qt.io/)
- **語言** / Language：C++17
- **構建系統** / Build System：CMake 3.16+
- **編譯器** / Compiler：MinGW-w64 (GCC)

### 開發環境設置 / Development Environment Setup

1. **安裝開發工具** / Install Development Tools
   - Qt Creator 或 Visual Studio
   - CMake
   - Git

2. **配置 Qt** / Configure Qt
   - 確保 Qt 6.10.1 或更高版本已安裝
   - 配置環境變數

3. **克隆並構建** / Clone and Build
   ```bash
   git clone https://github.com/yourusername/capybara.git
   cd capybara
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

### 代碼風格 / Code Style

- 使用 Qt 編碼規範
- 雙語註釋（中文/英文）
- 遵循 C++17 標準

### 構建配置 / Build Configuration

項目使用 CMake 進行構建管理。主要配置選項：

The project uses CMake for build management. Main configuration options:

- `CMAKE_AUTOUIC ON`：自動處理 UI 文件
- `CMAKE_AUTOMOC ON`：自動處理 Qt MOC
- `CMAKE_AUTORCC ON`：自動處理資源文件

## 🤝 貢獻 / Contributing

我們歡迎所有形式的貢獻！請遵循以下步驟：

We welcome all forms of contributions! Please follow these steps:

1. Fork 本倉庫
2. 創建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 開啟一個 Pull Request

### 貢獻指南 / Contribution Guidelines

- 確保代碼遵循項目的編碼風格
- 添加適當的註釋和文檔
- 編寫清晰的提交信息
- 測試您的更改

## 📄 許可證 / License

本項目採用 MIT 許可證。詳見 [LICENSE](LICENSE) 文件。

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙏 致謝 / Acknowledgments

- [Qt](https://www.qt.io/) - 跨平台應用框架
- [PyTorch](https://pytorch.org/) - 深度學習框架
- [Ultralytics](https://ultralytics.com/) - YOLO 實現

## 📞 聯繫 / Contact

如有問題或建議，請通過以下方式聯繫：

For questions or suggestions, please contact:

- 提交 Issue：[GitHub Issues](https://github.com/yourusername/capybara/issues)
- 郵箱：your-email@example.com

---

<div align="center">

**Made with ❤️ by the Capybara Team**

[返回頂部](#-capybara) | [文檔](document/README.md) | [許可證](LICENSE)

</div>
