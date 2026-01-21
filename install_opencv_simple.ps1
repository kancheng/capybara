# Simplified OpenCV Install Script (using vcpkg)
# Use this script if vcpkg is already installed

param(
    [string]$VcpkgPath = ""
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Install OpenCV using vcpkg" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Find vcpkg
if ([string]::IsNullOrEmpty($VcpkgPath)) {
    $possiblePaths = @(
        "$env:USERPROFILE\vcpkg",
        "$env:ProgramFiles\vcpkg",
        "C:\vcpkg",
        "$env:LOCALAPPDATA\vcpkg"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path "$path\vcpkg.exe" -ErrorAction SilentlyContinue) {
            $VcpkgPath = $path
            break
        }
    }
}

if ([string]::IsNullOrEmpty($VcpkgPath) -or -not (Test-Path "$VcpkgPath\vcpkg.exe" -ErrorAction SilentlyContinue)) {
    Write-Host "vcpkg not found!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install vcpkg first:" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "  git clone https://github.com/Microsoft/vcpkg.git" -ForegroundColor Cyan
    Write-Host "  cd vcpkg" -ForegroundColor Cyan
    Write-Host "  .\bootstrap-vcpkg.bat" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Or use full install script: .\install_opencv.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "Found vcpkg: $VcpkgPath" -ForegroundColor Green
Write-Host ""

# Install OpenCV
Write-Host "Installing OpenCV (this may take a while)..." -ForegroundColor Yellow
Write-Host ""

$vcpkgExe = "$VcpkgPath\vcpkg.exe"
& $vcpkgExe install opencv:x64-windows

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "Installation completed!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Please set during CMake configuration:" -ForegroundColor Yellow
    Write-Host "  -DCMAKE_TOOLCHAIN_FILE=$VcpkgPath\scripts\buildsystems\vcpkg.cmake" -ForegroundColor Cyan
    Write-Host ""
} else {
    Write-Host ""
    Write-Host "Installation failed" -ForegroundColor Red
    exit 1
}
