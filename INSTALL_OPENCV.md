# OpenCV 自動安裝指南 / OpenCV Auto-Installation Guide

本專案提供了自動安裝 OpenCV 的腳本，讓您無需手動配置即可使用圖像處理功能。

This project provides auto-installation scripts for OpenCV, allowing you to use image processing features without manual configuration.

## 快速開始 / Quick Start

### 方法 1: 自動安裝腳本（推薦）/ Method 1: Auto-Install Script (Recommended)

在 PowerShell 中運行：

Run in PowerShell:

```powershell
.\install_opencv.ps1
```

此腳本會：
- 自動下載 OpenCV 預編譯版本
- 解壓到用戶目錄 (`%USERPROFILE%\opencv`)
- 自動設置環境變量
- 配置完成後即可使用

This script will:
- Automatically download pre-built OpenCV
- Extract to user directory (`%USERPROFILE%\opencv`)
- Automatically set environment variables
- Ready to use after configuration

### 方法 2: 使用 vcpkg / Method 2: Using vcpkg

如果您已經安裝了 vcpkg：

If you already have vcpkg installed:

```powershell
.\install_opencv_simple.ps1
```

或者手動運行：

Or run manually:

```powershell
vcpkg install opencv:x64-windows
```

然後在 CMake 配置時添加：

Then add during CMake configuration:

```
-DCMAKE_TOOLCHAIN_FILE=<vcpkg_path>/scripts/buildsystems/vcpkg.cmake
```

## 安裝後 / After Installation

1. **重新啟動終端或 IDE** / Restart terminal or IDE
   - 環境變量需要重新載入
   - Environment variables need to be reloaded

2. **重新運行 CMake 配置** / Rerun CMake configuration
   - CMake 會自動檢測到 OpenCV
   - CMake will automatically detect OpenCV

3. **編譯專案** / Build project
   - OpenCV 功能將自動啟用
   - OpenCV features will be automatically enabled

## 驗證安裝 / Verify Installation

運行 CMake 配置後，您應該看到：

After running CMake configuration, you should see:

```
-- OpenCV found: 4.x.x
-- OpenCV include dirs: ...
-- OpenCV libraries: ...
-- OpenCV enabled - HAVE_OPENCV defined
```

## 手動安裝選項 / Manual Installation Options

如果自動安裝失敗，您可以手動安裝：

If auto-installation fails, you can install manually:

### 選項 1: 下載預編譯版本 / Option 1: Download Pre-built

1. 訪問 [OpenCV Releases](https://github.com/opencv/opencv/releases)
2. 下載 Windows 版本（例如 `opencv-4.8.0-windows.exe`）
3. 運行安裝程序，解壓到 `C:\opencv` 或自定義路徑
4. 設置環境變量 `OpenCV_DIR` 指向 `opencv\build` 目錄

### 選項 2: 使用包管理器 / Option 2: Use Package Manager

**vcpkg:**
```powershell
vcpkg install opencv:x64-windows
```

**Conan:**
```bash
conan install opencv/4.8.0@
```

**Chocolatey:**
```powershell
choco install opencv
```

### 選項 3: 從源碼編譯 / Option 3: Build from Source

參考 [OpenCV 官方文檔](https://docs.opencv.org/master/d3/d52/tutorial_windows_install.html)

## 故障排除 / Troubleshooting

### 問題: CMake 找不到 OpenCV

**解決方案:**
1. 確認環境變量已設置：`echo $env:OpenCV_DIR`
2. 重新啟動終端/IDE
3. 手動設置 CMake 變量：`-DOpenCV_DIR=<path_to_opencv_build>`

### 問題: 編譯錯誤，找不到 OpenCV 頭文件

**解決方案:**
1. 檢查 `OpenCV_DIR` 環境變量是否正確
2. 確認 `opencv\build\include\opencv2\opencv.hpp` 文件存在
3. 清理 CMake 緩存並重新配置

### 問題: 鏈接錯誤，找不到 OpenCV 庫

**解決方案:**
1. 確認庫文件存在於 `opencv\build\x64\<compiler>\lib` 目錄
2. 檢查編譯器版本是否匹配（VC15/VC16/Mingw）
3. 確認庫文件路徑在 `OpenCV_LIBS` 變量中

## 腳本參數 / Script Parameters

### install_opencv.ps1

```powershell
.\install_opencv.ps1 [-Version <version>] [-InstallPath <path>] [-Force]
```

- `-Version`: OpenCV 版本（默認: 4.8.0）
- `-InstallPath`: 安裝路徑（默認: `%USERPROFILE%\opencv`）
- `-Force`: 強制重新安裝

### install_opencv_simple.ps1

```powershell
.\install_opencv_simple.ps1 [-VcpkgPath <path>]
```

- `-VcpkgPath`: vcpkg 安裝路徑（自動檢測）

## 注意事項 / Notes

1. **網絡連接**: 自動安裝需要穩定的網絡連接下載 OpenCV
2. **磁盤空間**: OpenCV 預編譯版本約需要 500MB-1GB 空間
3. **編譯器兼容性**: 確保 OpenCV 版本與您的編譯器兼容
4. **權限**: 某些操作可能需要管理員權限

## 更多信息 / More Information

- [OpenCV 官方網站](https://opencv.org/)
- [OpenCV GitHub](https://github.com/opencv/opencv)
- [vcpkg 文檔](https://github.com/Microsoft/vcpkg)
