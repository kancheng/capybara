# OpenCV Auto-Install Script
# This script automatically downloads and configures pre-built OpenCV

param(
    [string]$Version = "4.8.0",
    [string]$InstallPath = "$env:USERPROFILE\opencv",
    [switch]$Force = $false
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "OpenCV Auto-Install Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if already installed
$opencvPath = "$InstallPath\opencv"
if (Test-Path "$opencvPath\build\include\opencv2\opencv.hpp" -ErrorAction SilentlyContinue) {
    if (-not $Force) {
        Write-Host "OpenCV already installed at: $opencvPath" -ForegroundColor Green
        Write-Host "Use -Force to reinstall" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Setting environment variable..." -ForegroundColor Yellow
        [System.Environment]::SetEnvironmentVariable("OpenCV_DIR", "$opencvPath\build", "User")
        Write-Host "Environment variable set: OpenCV_DIR = $opencvPath\build" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "Force reinstall..." -ForegroundColor Yellow
        Remove-Item -Path $opencvPath -Recurse -Force -ErrorAction SilentlyContinue
    }
}

# Create install directory
Write-Host "Creating install directory: $InstallPath" -ForegroundColor Yellow
New-Item -ItemType Directory -Force -Path $InstallPath | Out-Null

# Download URL
$baseUrl = "https://github.com/opencv/opencv/releases/download/$Version"
$filename = "opencv-$Version-windows.exe"
$downloadUrl = "$baseUrl/$filename"
$downloadPath = "$InstallPath\$filename"

Write-Host ""
Write-Host "Downloading OpenCV $Version..." -ForegroundColor Yellow
Write-Host "URL: $downloadUrl" -ForegroundColor Gray
Write-Host "Target: $downloadPath" -ForegroundColor Gray
Write-Host ""

# Check if already downloaded
if (Test-Path $downloadPath -ErrorAction SilentlyContinue) {
    Write-Host "File exists, skipping download" -ForegroundColor Green
} else {
    try {
        # Download file
        $ProgressPreference = 'SilentlyContinue'
        Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath -UseBasicParsing
        Write-Host "Download completed" -ForegroundColor Green
    } catch {
        Write-Host "Download failed: $_" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please manually download and extract to: $InstallPath" -ForegroundColor Yellow
        Write-Host "Download URL: $downloadUrl" -ForegroundColor Yellow
        exit 1
    }
}

# Extract file
Write-Host ""
Write-Host "Extracting OpenCV..." -ForegroundColor Yellow

$extractPath = "$InstallPath\opencv"
if (Test-Path $extractPath -ErrorAction SilentlyContinue) {
    Remove-Item -Path $extractPath -Recurse -Force
}

# Use 7-Zip or built-in extraction
$7zipPath = "C:\Program Files\7-Zip\7z.exe"
if (Test-Path $7zipPath) {
    Write-Host "Using 7-Zip to extract..." -ForegroundColor Gray
    & $7zipPath x "$downloadPath" -o"$InstallPath" -y | Out-Null
} else {
    # Try built-in extraction
    Write-Host "Trying built-in extraction..." -ForegroundColor Gray
    try {
        # For .exe files, may need to run installer
        Write-Host "Note: This is a self-extracting archive, please run manually and extract to: $extractPath" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Or use 7-Zip to extract this file" -ForegroundColor Yellow
        Write-Host "7-Zip download: https://www.7-zip.org/" -ForegroundColor Cyan
        
        # Try to run installer directly
        $process = Start-Process -FilePath $downloadPath -ArgumentList "-o$InstallPath -y" -Wait -PassThru -NoNewWindow
        if ($process.ExitCode -ne 0) {
            throw "Extraction failed"
        }
    } catch {
        Write-Host "Auto-extraction failed, please extract manually" -ForegroundColor Red
        Write-Host "Extract to: $extractPath" -ForegroundColor Yellow
        exit 1
    }
}

# Check extraction result
$expectedPath = "$extractPath\build\include\opencv2\opencv.hpp"
if (-not (Test-Path $expectedPath -ErrorAction SilentlyContinue)) {
    # Try to find extracted file structure
    $possiblePaths = @(
        "$InstallPath\opencv-$Version\build",
        "$InstallPath\opencv\build",
        "$InstallPath\build"
    )
    
    $found = $false
    foreach ($path in $possiblePaths) {
        if (Test-Path "$path\include\opencv2\opencv.hpp" -ErrorAction SilentlyContinue) {
            $extractPath = $path
            $found = $true
            break
        }
    }
    
    if (-not $found) {
        Write-Host "Error: Cannot find OpenCV files" -ForegroundColor Red
        Write-Host "Please check extraction directory: $InstallPath" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "OpenCV extraction completed" -ForegroundColor Green

# Set environment variable
Write-Host ""
Write-Host "Setting environment variable..." -ForegroundColor Yellow
$buildPath = if (Test-Path "$extractPath\build") { "$extractPath\build" } else { $extractPath }
[System.Environment]::SetEnvironmentVariable("OpenCV_DIR", $buildPath, "User")
Write-Host "Environment variable set: OpenCV_DIR = $buildPath" -ForegroundColor Green

# Clean up download file (optional)
Write-Host ""
Write-Host "Delete download file? (Y/N): " -NoNewline -ForegroundColor Yellow
$response = Read-Host
if ($response -eq "Y" -or $response -eq "y") {
    Remove-Item -Path $downloadPath -Force -ErrorAction SilentlyContinue
    Write-Host "Download file deleted" -ForegroundColor Green
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Installation completed!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "OpenCV install path: $buildPath" -ForegroundColor Green
Write-Host ""
Write-Host "Please restart your terminal or IDE, then rerun CMake configuration" -ForegroundColor Yellow
Write-Host ""
