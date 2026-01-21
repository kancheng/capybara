#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QRegularExpression>
#include <QApplication>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // 連接信號槽 / Connect signals and slots
    connect(ui->pythonComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPythonComboBoxChanged);
    connect(ui->venvComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onVenvComboBoxChanged);
    connect(ui->selectEnvButton, &QPushButton::clicked,
            this, &MainWindow::onSelectEnvButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &MainWindow::onRefreshButtonClicked);
    
    // 掃描 Python 環境 / Scan Python environments
    scanPythonEnvironments();
    
    // 檢測系統信息 / Detect system information
    detectSystemInfo();
    
    // 延遲載入保存的設定，確保 UI 完全初始化 / Delay loading saved settings to ensure UI is fully initialized
    QTimer::singleShot(100, this, [this]() {
        if (hasSavedSettings()) {
            loadSettings();
            applySavedSettings();
        } else {
            // 如果沒有保存的設定，提醒用戶 / If no saved settings, remind user
            QMessageBox::information(this, 
                                     tr("歡迎使用 Capybara / Welcome to Capybara"), 
                                     tr("請選擇一個 Python 環境並點擊「指定此環境」按鈕來保存設定。\n"
                                        "Please select a Python environment and click \"指定此環境\" button to save settings."));
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::scanPythonEnvironments()
{
    pythonEnvironments.clear();
    ui->pythonComboBox->clear();
    
    detectPythonEnvironments();
    updateComboBox();
    
    if (pythonEnvironments.isEmpty()) {
        ui->pythonComboBox->addItem("未找到 Python 環境");
        ui->pythonPathLabel->setText("路徑：未找到");
        currentPythonPath.clear();
        virtualEnvironments.clear();
        ui->venvComboBox->clear();
        ui->venvComboBox->setEnabled(false);
        ui->venvPathLabel->setText("虛擬環境路徑：未找到");
    } else {
        ui->pythonPathLabel->setText("路徑：" + pythonEnvironments[0].path);
        currentPythonPath = pythonEnvironments[0].path;
        // 掃描第一個 Python 環境的虛擬環境 / Scan virtual environments for first Python environment
        scanVirtualEnvironments(pythonEnvironments[0].path);
    }
}

void MainWindow::detectPythonEnvironments()
{
    QStringList pythonPaths;
    
    // 方法 1: 從 PATH 環境變數中查找 / Method 1: Find from PATH environment variable
    pythonPaths.append(findPythonInPath());
    
    // 方法 2: 使用 Python Launcher (py.exe) 查找 / Method 2: Find using Python Launcher (py.exe)
    pythonPaths.append(findPythonViaLauncher());
    
    // 方法 3: 檢查常見安裝位置 / Method 3: Check common installation locations
    pythonPaths.append(findPythonInCommonLocations());
    
    // 去重並驗證每個 Python 路徑 / Remove duplicates and validate each Python path
    QStringList uniquePaths;
    for (const QString &path : pythonPaths) {
        if (!uniquePaths.contains(path) && QFileInfo::exists(path)) {
            QString version = getPythonVersion(path);
            if (!version.isEmpty()) {
                PythonEnvironment env;
                env.path = path;
                env.version = version;
                env.displayName = QString("Python %1 - %2").arg(version, path);
                pythonEnvironments.append(env);
                uniquePaths.append(path);
            }
        }
    }
}

QStringList MainWindow::findPythonInPath()
{
    QStringList results;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString pathEnv = env.value("PATH");
    
    QStringList paths = pathEnv.split(QDir::listSeparator());
    for (const QString &dir : paths) {
        QString pythonPath = QDir(dir).absoluteFilePath("python.exe");
        if (QFileInfo::exists(pythonPath)) {
            results.append(pythonPath);
        }
    }
    
    return results;
}

QStringList MainWindow::findPythonViaLauncher()
{
    QStringList results;
    
    // 使用 py.exe -0 列出所有已安裝的 Python / Use py.exe -0 to list all installed Python versions
    QProcess process;
    process.start("py.exe", QStringList() << "-0");
    process.waitForFinished(3000);
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        for (const QString &line : lines) {
            // 解析 py.exe -0 的輸出格式 / Parse py.exe -0 output format
            // 例如: " -3.11-64 *        C:\Python311\python.exe" / Example: " -3.11-64 *        C:\Python311\python.exe"
            QString trimmed = line.trimmed();
            if (trimmed.contains("python.exe")) {
                // 提取路徑 / Extract path
                int exeIndex = trimmed.indexOf("python.exe");
                if (exeIndex > 0) {
                    QString path = trimmed.mid(exeIndex - 1).trimmed();
                    if (QFileInfo::exists(path)) {
                        results.append(path);
                    }
                }
            }
        }
    }
    
    return results;
}

QStringList MainWindow::findPythonInCommonLocations()
{
    QStringList results;
    
    // Windows 常見 Python 安裝位置 / Windows common Python installation locations
    QStringList commonPaths = {
        "C:\\Python*\\python.exe",
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/AppData/Local/Programs/Python/*/python.exe",
        "C:\\Program Files\\Python*\\python.exe",
        "C:\\Program Files (x86)\\Python*\\python.exe",
    };
    
    // 檢查特定版本目錄 / Check version-specific directories
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QDir pythonDir(homeDir + "/AppData/Local/Programs/Python");
    if (pythonDir.exists()) {
        QStringList entries = pythonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            QString pythonPath = pythonDir.absoluteFilePath(entry + "/python.exe");
            if (QFileInfo::exists(pythonPath)) {
                results.append(pythonPath);
            }
        }
    }
    
    // 檢查系統 Program Files / Check system Program Files
    QDir programFiles("C:/Program Files");
    if (programFiles.exists()) {
        QStringList entries = programFiles.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.startsWith("Python")) {
                QString pythonPath = programFiles.absoluteFilePath(entry + "/python.exe");
                if (QFileInfo::exists(pythonPath)) {
                    results.append(pythonPath);
                }
            }
        }
    }
    
    // 檢查 Program Files (x86) / Check Program Files (x86)
    QDir programFilesX86("C:/Program Files (x86)");
    if (programFilesX86.exists()) {
        QStringList entries = programFilesX86.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.startsWith("Python")) {
                QString pythonPath = programFilesX86.absoluteFilePath(entry + "/python.exe");
                if (QFileInfo::exists(pythonPath)) {
                    results.append(pythonPath);
                }
            }
        }
    }
    
    // 檢查虛擬環境（常見位置） / Check virtual environments (common locations)
    QStringList venvPaths = {
        homeDir + "/.virtualenvs",
        homeDir + "/venv",
        homeDir + "/env",
    };
    
    for (const QString &venvBase : venvPaths) {
        QDir venvDir(venvBase);
        if (venvDir.exists()) {
            // 查找直接子目錄中的虛擬環境 / Find virtual environments in direct subdirectories
            QStringList entries = venvDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                QString pythonPath = venvDir.absoluteFilePath(entry + "/Scripts/python.exe");
                if (QFileInfo::exists(pythonPath)) {
                    results.append(pythonPath);
                }
            }
        }
    }
    
    // 檢查當前目錄下的虛擬環境 / Check virtual environments in current directory
    QDir currentDir(QDir::current());
    QStringList currentVenvDirs = {"venv", "env", ".venv"};
    for (const QString &venvDir : currentVenvDirs) {
        QString pythonPath = currentDir.absoluteFilePath(venvDir + "/Scripts/python.exe");
        if (QFileInfo::exists(pythonPath)) {
            results.append(pythonPath);
        }
    }
    
    return results;
}

QString MainWindow::getPythonVersion(const QString &pythonPath)
{
    QProcess process;
    process.start(pythonPath, QStringList() << "--version");
    process.waitForFinished(3000);
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        // 輸出格式通常是 "Python 3.11.0" 或 "Python 3.11.0\n" / Output format is usually "Python 3.11.0" or "Python 3.11.0\n"
        output = output.trimmed();
        if (output.startsWith("Python ")) {
            return output.mid(7).trimmed(); // 移除 "Python " 前綴 / Remove "Python " prefix
        }
    }
    
    return QString();
}

void MainWindow::updateComboBox()
{
    ui->pythonComboBox->clear();
    
    for (const PythonEnvironment &env : pythonEnvironments) {
        ui->pythonComboBox->addItem(env.displayName);
    }
}

void MainWindow::onPythonComboBoxChanged(int index)
{
    if (index >= 0 && index < pythonEnvironments.size()) {
        const PythonEnvironment &env = pythonEnvironments[index];
        ui->pythonPathLabel->setText("路徑：" + env.path);
        currentPythonPath = env.path;
        
        // 掃描該 Python 環境的虛擬環境 / Scan virtual environments for this Python
        scanVirtualEnvironments(env.path);
        
        // 清除之前的選擇和檢測結果 / Clear previous selection and detection results
        selectedEnvName.clear();
        selectedEnvPath.clear();
        selectedPythonPath.clear();
        ui->selectedEnvLabel->setText("已指定環境：無");
        ui->selectEnvButton->setEnabled(false);
        ui->pythonPackagesLabel->setText("Python 套件：請選擇虛擬環境並點擊「指定此環境」");
        ui->pytorchInfoLabel->setText("PyTorch：未檢測");
        ui->ultralyticsInfoLabel->setText("Ultralytics：未檢測");
    } else {
        ui->pythonPathLabel->setText("路徑：未選擇");
        currentPythonPath.clear();
        virtualEnvironments.clear();
        ui->venvComboBox->clear();
        ui->venvComboBox->setEnabled(false);
        ui->venvPathLabel->setText("虛擬環境路徑：未選擇");
        
        // 清除 Python 套件信息 / Clear Python package information
        ui->pythonPackagesLabel->setText("Python 套件：未選擇環境");
        ui->pytorchInfoLabel->setText("PyTorch：未檢測");
        ui->ultralyticsInfoLabel->setText("Ultralytics：未檢測");
    }
}

void MainWindow::onRefreshButtonClicked()
{
    scanPythonEnvironments();
    
    // 如果已選擇 Python 環境，重新掃描虛擬環境 / If Python environment is selected, rescan virtual environments
    if (!currentPythonPath.isEmpty()) {
        scanVirtualEnvironments(currentPythonPath);
    }
    
    QMessageBox::information(this, "掃描完成", 
                            QString("找到 %1 個 Python 環境，%2 個虛擬環境")
                            .arg(pythonEnvironments.size())
                            .arg(virtualEnvironments.size()));
}

void MainWindow::scanVirtualEnvironments(const QString &pythonPath)
{
    virtualEnvironments.clear();
    ui->venvComboBox->clear();
    
    QStringList venvPaths = findVirtualEnvironments(pythonPath);
    
    // 添加 conda 環境 / Add conda environments
    QMap<QString, QString> condaEnvs = findCondaEnvironments(); // 名稱到路徑的映射 / Name to path mapping
    for (const QString &condaEnvPath : condaEnvs.values()) {
        if (!venvPaths.contains(condaEnvPath)) {
            venvPaths.append(condaEnvPath);
        }
    }
    
    // 驗證每個虛擬環境是否屬於選定的 Python 或是 conda 環境 / Validate each virtual environment belongs to selected Python or is a conda environment
    for (const QString &venvPath : venvPaths) {
        bool isValid = false;
        QString envName;
        
        // 檢查是否為 conda 環境 / Check if it's a conda environment
        if (isCondaEnvironment(venvPath)) {
            isValid = true;
            // 從映射中獲取 conda 環境名稱 / Get conda environment name from map
            QString condaName = condaEnvs.key(venvPath, "");
            if (!condaName.isEmpty()) {
                envName = condaName; // 使用 conda 的實際環境名稱 / Use actual conda environment name
            } else {
                // 如果找不到名稱，使用目錄名 / If name not found, use directory name
                QFileInfo venvInfo(venvPath);
                envName = venvInfo.dir().dirName();
            }
        } else if (isVirtualEnvironmentForPython(venvPath, pythonPath)) {
            isValid = true;
            // 獲取虛擬環境名稱（目錄名） / Get virtual environment name (directory name)
            QFileInfo venvInfo(venvPath);
            envName = venvInfo.dir().dirName();
        }
        
        if (isValid) {
            VirtualEnvironment venv;
            venv.path = venvPath;
            venv.pythonPath = pythonPath;
            
            // 設置環境名稱 / Set environment name
            if (isCondaEnvironment(venvPath)) {
                venv.name = "[Conda] " + envName; // 使用 conda 的實際環境名稱 / Use actual conda environment name
            } else {
                venv.name = envName;
            }
            
            // 獲取虛擬環境中的 Python 路徑 / Get Python path in virtual environment
            QString venvPython = getVenvPythonPath(venvPath);
            if (!venvPython.isEmpty()) {
                venv.displayName = QString("%1 (%2)").arg(venv.name, venvPython);
            } else {
                venv.displayName = venv.name;
            }
            
            virtualEnvironments.append(venv);
        }
    }
    
    updateVenvComboBox();
}

QStringList MainWindow::findVirtualEnvironments(const QString &pythonPath)
{
    QStringList results;
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    
    // 常見虛擬環境位置 / Common virtual environment locations
    QStringList searchPaths = {
        homeDir + "/.virtualenvs",
        homeDir + "/venv",
        homeDir + "/env",
        homeDir + "/Envs",
        QDir::current().absolutePath() + "/venv",
        QDir::current().absolutePath() + "/env",
        QDir::current().absolutePath() + "/.venv",
    };
    
    // 檢查 Python 安裝目錄的父目錄 / Check parent directory of Python installation
    QFileInfo pythonInfo(pythonPath);
    QDir pythonParentDir = pythonInfo.dir();
    pythonParentDir.cdUp();
    searchPaths.append(pythonParentDir.absolutePath() + "/venv");
    searchPaths.append(pythonParentDir.absolutePath() + "/env");
    
    // 遞歸查找所有包含 pyvenv.cfg 的目錄 / Recursively find all directories containing pyvenv.cfg
    for (const QString &basePath : searchPaths) {
        QDir baseDir(basePath);
        if (baseDir.exists()) {
            // 檢查直接子目錄 / Check direct subdirectories
            QStringList entries = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                QString venvPath = baseDir.absoluteFilePath(entry);
                QString pyvenvCfg = QDir(venvPath).absoluteFilePath("pyvenv.cfg");
                if (QFileInfo::exists(pyvenvCfg)) {
                    results.append(venvPath);
                }
            }
            
            // 也檢查當前目錄本身是否為虛擬環境 / Also check if current directory itself is a virtual environment
            QString pyvenvCfg = baseDir.absoluteFilePath("pyvenv.cfg");
            if (QFileInfo::exists(pyvenvCfg)) {
                results.append(basePath);
            }
        }
    }
    
    // 使用 Python 的 site-packages 位置來查找可能的虛擬環境 / Use Python's site-packages location to find possible virtual environments
    QProcess process;
    process.start(pythonPath, QStringList() << "-c" << "import sys; print(sys.prefix)");
    process.waitForFinished(3000);
    if (process.exitCode() == 0) {
        QString pythonPrefix = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        QDir pythonPrefixDir(pythonPrefix);
        if (pythonPrefixDir.exists()) {
            // 檢查父目錄中的虛擬環境 / Check virtual environments in parent directory
            pythonPrefixDir.cdUp();
            QStringList entries = pythonPrefixDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                QString venvPath = pythonPrefixDir.absoluteFilePath(entry);
                QString pyvenvCfg = QDir(venvPath).absoluteFilePath("pyvenv.cfg");
                if (QFileInfo::exists(pyvenvCfg)) {
                    results.append(venvPath);
                }
            }
        }
    }
    
    return results;
}

QMap<QString, QString> MainWindow::findCondaEnvironments()
{
    QMap<QString, QString> results; // 環境名稱 -> 路徑映射 / Environment name -> path mapping
    
    // 嘗試使用 conda info -e 命令 / Try using conda info -e command
    QProcess process;
    
    // 首先嘗試直接執行 conda / First try executing conda directly
    process.start("conda", QStringList() << "info" << "-e");
    process.waitForFinished(5000);
    
    // 如果失敗，嘗試使用 conda.exe / If failed, try conda.exe
    if (process.exitCode() != 0) {
        process.start("conda.exe", QStringList() << "info" << "-e");
        process.waitForFinished(5000);
    }
    
    // 如果還是失敗，嘗試 conda env list / If still failed, try conda env list
    if (process.exitCode() != 0) {
        process.start("conda", QStringList() << "env" << "list");
        process.waitForFinished(5000);
    }
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        
        // 解析 conda 輸出 / Parse conda output
        // 格式通常是: # conda environments: 或環境列表 / Format is usually: # conda environments: or environment list
        // 例如: base                  C:\Users\...\anaconda3 / Example: base                  C:\Users\...\anaconda3
        // 或者: base    *  C:\Users\...\anaconda3 / Or: base    *  C:\Users\...\anaconda3
        bool inEnvironmentList = false;
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            
            // 跳過空行 / Skip empty lines
            if (trimmed.isEmpty()) {
                continue;
            }
            
            // 檢查是否為標題行 / Check if it's a header line
            if (trimmed.startsWith("#")) {
                if (trimmed.contains("conda environments", Qt::CaseInsensitive) ||
                    trimmed.contains("environment", Qt::CaseInsensitive)) {
                    inEnvironmentList = true;
                }
                continue;
            }
            
            // 如果已經進入環境列表部分，解析環境路徑 / If in environment list section, parse environment paths
            if (inEnvironmentList || (!trimmed.startsWith("#") && trimmed.contains(QRegularExpression("^[a-zA-Z0-9_\\-]+")))) {
                // 分割環境名稱和路徑 / Split environment name and path
                // 格式: 環境名稱 + 多個空格 + 可選的 * + 路徑 / Format: env name + multiple spaces + optional * + path
                // 例如: base                   C:\Users\...\anaconda3 / Example: base                   C:\Users\...\anaconda3
                // 或者: base    *  C:\Users\...\anaconda3 / Or: base    *  C:\Users\...\anaconda3
                
                QString envName;
                QString envPath;
                
                // 使用正則表達式匹配：環境名 + 一個或多個空格 + 可選的 * + 一個或多個空格 + 路徑 / Use regex to match: env name + one or more spaces + optional * + one or more spaces + path
                QRegularExpression re("^([a-zA-Z0-9_\\-]+)\\s+\\*?\\s*(.+)$");
                QRegularExpressionMatch match = re.match(trimmed);
                
                if (match.hasMatch()) {
                    // 匹配成功，提取名稱和路徑 / Match successful, extract name and path
                    envName = match.captured(1).trimmed();
                    envPath = match.captured(2).trimmed();
                } else {
                    // 如果正則匹配失敗，使用空格分割 / If regex match fails, split by spaces
                    QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    if (parts.size() >= 2) {
                        // 第一個部分是環境名稱，最後一個部分是路徑 / First part is environment name, last part is path
                        envName = parts.first();
                        envPath = parts.last();
                    } else if (parts.size() == 1) {
                        // 只有一個部分，可能是路徑 / Only one part, might be a path
                        envPath = parts[0];
                        // 從路徑提取名稱 / Extract name from path
                        QFileInfo pathInfo(envPath);
                        QString dirName = pathInfo.dir().dirName();
                        // 如果是 envs 目錄，使用父目錄名 / If it's in envs directory, use parent directory name
                        if (dirName == "envs") {
                            envName = pathInfo.baseName();
                        } else {
                            envName = dirName;
                        }
                    }
                }
                
                // 驗證路徑 / Validate path
                if (!envPath.isEmpty()) {
                    QDir envDir(envPath);
                    if (envDir.exists()) {
                        // 檢查是否為 conda 環境（包含 conda-meta 目錄） / Check if it's a conda environment (contains conda-meta directory)
                        QString condaMeta = envDir.absoluteFilePath("conda-meta");
                        if (QFileInfo::exists(condaMeta)) {
                            // 如果沒有名稱，使用目錄名 / If no name, use directory name
                            if (envName.isEmpty()) {
                                QFileInfo pathInfo(envPath);
                                envName = pathInfo.dir().dirName();
                            }
                            results.insert(envName, envPath); // 存儲名稱和路徑的映射 / Store name and path mapping
                        }
                    }
                }
            }
        }
    }
    
    // 如果命令執行失敗，嘗試查找常見的 conda 安裝位置 / If command execution failed, try finding common conda installation locations
    if (results.isEmpty()) {
        QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QStringList condaPaths = {
            homeDir + "/anaconda3",
            homeDir + "/miniconda3",
            homeDir + "/anaconda",
            homeDir + "/miniconda",
            "C:/ProgramData/Anaconda3",
            "C:/ProgramData/Miniconda3",
            "C:/Users/Public/anaconda3",
            "C:/Users/Public/miniconda3",
        };
        
        // 檢查 conda 環境目錄 / Check conda environments directory
        for (const QString &condaBase : condaPaths) {
            QDir condaDir(condaBase);
            if (condaDir.exists()) {
                // 檢查 envs 目錄 / Check envs directory
                QDir envsDir(condaDir.absoluteFilePath("envs"));
                if (envsDir.exists()) {
                    QStringList envDirs = envsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    for (const QString &envName : envDirs) {
                        QString envPath = envsDir.absoluteFilePath(envName);
                        QString condaMeta = QDir(envPath).absoluteFilePath("conda-meta");
                        if (QFileInfo::exists(condaMeta)) {
                            results.insert(envName, envPath); // 使用目錄名作為環境名稱 / Use directory name as environment name
                        }
                    }
                }
                
                // base 環境本身 / Base environment itself
                QString condaMeta = condaDir.absoluteFilePath("conda-meta");
                if (QFileInfo::exists(condaMeta)) {
                    // base 環境名稱固定為 "base" / Base environment name is always "base"
                    results.insert("base", condaBase);
                }
            }
        }
    }
    
    return results;
}

bool MainWindow::isCondaEnvironment(const QString &venvPath)
{
    // 檢查是否包含 conda-meta 目錄 / Check if it contains conda-meta directory
    QDir venvDir(venvPath);
    QString condaMeta = venvDir.absoluteFilePath("conda-meta");
    return QFileInfo::exists(condaMeta);
}

bool MainWindow::isVirtualEnvironmentForPython(const QString &venvPath, const QString &pythonPath)
{
    // 讀取 pyvenv.cfg 文件 / Read pyvenv.cfg file
    QString pyvenvCfgPath = QDir(venvPath).absoluteFilePath("pyvenv.cfg");
    QFile pyvenvCfg(pyvenvCfgPath);
    
    if (!pyvenvCfg.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&pyvenvCfg);
    QString basePython;
    
    // 解析配置文件，查找 base-python 或 home / Parse config file, find base-python or home
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("base-python") || line.startsWith("home")) {
            QStringList parts = line.split("=");
            if (parts.size() >= 2) {
                basePython = parts[1].trimmed();
                break;
            }
        }
    }
    
    pyvenvCfg.close();
    
    // 標準化路徑進行比較 / Normalize paths for comparison
    QFileInfo pythonInfo(pythonPath);
    QString normalizedPythonPath = QDir::toNativeSeparators(pythonInfo.absoluteFilePath());
    
    if (!basePython.isEmpty()) {
        QString normalizedBasePython = QDir::toNativeSeparators(QFileInfo(basePython).absoluteFilePath());
        // 比較路徑（不區分大小寫，因為 Windows） / Compare paths (case-insensitive for Windows)
        return normalizedBasePython.compare(normalizedPythonPath, Qt::CaseInsensitive) == 0;
    }
    
    return false;
}

QString MainWindow::getVenvPythonPath(const QString &venvPath)
{
    qDebug() << "\n=== 查找 Python 路徑 ===";
    qDebug() << "虛擬環境路徑:" << venvPath;
    
    // 檢查是否為 Conda 環境 / Check if it's a Conda environment
    bool isConda = isCondaEnvironment(venvPath);
    qDebug() << "是否為 Conda 環境:" << isConda;
    
    // Conda 環境的 Python 路徑檢查順序 / Conda environment Python path check order
    QStringList condaPaths = {
        QDir(venvPath).absoluteFilePath("python.exe"),  // Windows 根目錄 / Windows root
        QDir(venvPath).absoluteFilePath("bin/python.exe"), // Windows bin 目錄 / Windows bin directory
        QDir(venvPath).absoluteFilePath("bin/python"),    // Unix bin 目錄 / Unix bin directory
    };
    
    // 標準虛擬環境的 Python 路徑檢查順序 / Standard virtual environment Python path check order
    QStringList standardPaths = {
        QDir(venvPath).absoluteFilePath("Scripts/python.exe"), // Windows Scripts / Windows Scripts
        QDir(venvPath).absoluteFilePath("bin/python.exe"),     // Windows bin / Windows bin
        QDir(venvPath).absoluteFilePath("bin/python"),         // Unix bin / Unix bin
    };
    
    // 優先檢查 Conda 環境路徑 / Check Conda paths first if it's a Conda environment
    QStringList pathsToCheck = isConda ? condaPaths : standardPaths;
    
    for (const QString &path : pathsToCheck) {
        qDebug() << "檢查路徑:" << path;
        if (QFileInfo::exists(path)) {
            qDebug() << "找到 Python:" << path;
            return path;
        }
    }
    
    // 如果都沒找到，也檢查標準路徑 / If not found, also check standard paths
    if (isConda) {
        for (const QString &path : standardPaths) {
            qDebug() << "檢查標準路徑:" << path;
            if (QFileInfo::exists(path)) {
                qDebug() << "找到 Python (標準路徑):" << path;
                return path;
            }
        }
    }
    
    qDebug() << "未找到 Python 可執行文件";
    return QString();
}

void MainWindow::updateVenvComboBox()
{
    ui->venvComboBox->clear();
    
    if (virtualEnvironments.isEmpty()) {
        ui->venvComboBox->addItem("未找到虛擬環境");
        ui->venvComboBox->setEnabled(false);
        ui->venvPathLabel->setText("虛擬環境路徑：未找到");
        ui->selectEnvButton->setEnabled(false);
    } else {
        ui->venvComboBox->setEnabled(true);
        for (const VirtualEnvironment &venv : virtualEnvironments) {
            ui->venvComboBox->addItem(venv.displayName);
        }
        ui->venvPathLabel->setText("虛擬環境路徑：" + virtualEnvironments[0].path);
        ui->selectEnvButton->setEnabled(true);
    }
}

void MainWindow::onVenvComboBoxChanged(int index)
{
    if (index >= 0 && index < virtualEnvironments.size()) {
        const VirtualEnvironment &venv = virtualEnvironments[index];
        ui->venvPathLabel->setText("虛擬環境路徑：" + venv.path);
        
        // 啟用"指定此環境"按鈕 / Enable "Select Environment" button
        ui->selectEnvButton->setEnabled(true);
        
        // 清除之前的檢測結果 / Clear previous detection results
        ui->pythonPackagesLabel->setText("Python 套件：請點擊「指定此環境」按鈕開始檢測");
        ui->pytorchInfoLabel->setText("PyTorch：未檢測");
        ui->ultralyticsInfoLabel->setText("Ultralytics：未檢測");
    } else {
        ui->venvPathLabel->setText("虛擬環境路徑：未選擇");
        ui->selectEnvButton->setEnabled(false);
        ui->pythonPackagesLabel->setText("Python 套件：請選擇環境");
        ui->pytorchInfoLabel->setText("PyTorch：未檢測");
        ui->ultralyticsInfoLabel->setText("Ultralytics：未檢測");
    }
}

void MainWindow::onSelectEnvButtonClicked()
{
    int index = ui->venvComboBox->currentIndex();
    if (index >= 0 && index < virtualEnvironments.size()) {
        const VirtualEnvironment &venv = virtualEnvironments[index];
        
        // 保存選擇的環境信息 / Save selected environment information
        selectedEnvName = venv.name;
        selectedEnvPath = venv.path;
        
        // 移除 [Conda] 前綴用於顯示 / Remove [Conda] prefix for display
        QString displayName = selectedEnvName;
        if (displayName.startsWith("[Conda] ")) {
            displayName = displayName.mid(8);
        }
        
        // 更新已指定環境標籤 / Update selected environment label
        ui->selectedEnvLabel->setText(QString("已指定環境：%1").arg(displayName));
        
        // 獲取虛擬環境中的 Python 路徑 / Get Python path in virtual environment
        QString venvPython = getVenvPythonPath(venv.path);
        selectedPythonPath = venvPython;
        
        // 保存設定 / Save settings
        saveSettings();
        
        // 調試：顯示 Python 路徑 / Debug: display Python path
        qDebug() << "=== 開始檢測環境 ===";
        qDebug() << "環境名稱:" << displayName;
        qDebug() << "環境路徑:" << venv.path;
        qDebug() << "獲取的 Python 路徑:" << venvPython;
        qDebug() << "Python 路徑是否存在:" << QFileInfo::exists(venvPython);
        
        // 如果獲取的路徑為空，嘗試其他方法 / If path is empty, try other methods
        if (venvPython.isEmpty()) {
            // 嘗試直接使用環境路徑下的 python.exe / Try python.exe directly in environment path
            QString directPython = QDir(venv.path).absoluteFilePath("python.exe");
            if (QFileInfo::exists(directPython)) {
                venvPython = directPython;
                qDebug() << "使用直接路徑:" << venvPython;
            }
        }
        
        if (!venvPython.isEmpty() && QFileInfo::exists(venvPython)) {
            // 測試 Python 是否可用 / Test if Python is usable
            QProcess testProcess;
            
            // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
            QProcessEnvironment testEnv = QProcessEnvironment::systemEnvironment();
            testEnv.remove("PYTHONHOME"); // 清除 PYTHONHOME / Clear PYTHONHOME
            testEnv.remove("PYTHONPATH"); // 清除 PYTHONPATH / Clear PYTHONPATH
            testProcess.setProcessEnvironment(testEnv);
            
            testProcess.start(venvPython, QStringList() << "--version");
            testProcess.waitForFinished(3000);
            if (testProcess.exitCode() == 0) {
                QString pythonVersion = QString::fromUtf8(testProcess.readAllStandardOutput()).trimmed();
                qDebug() << "Python 版本測試成功:" << pythonVersion;
            } else {
                QString errorOutput = QString::fromUtf8(testProcess.readAllStandardError());
                qDebug() << "Python 版本測試失敗，退出碼:" << testProcess.exitCode();
                qDebug() << "錯誤輸出:" << errorOutput;
            }
            // 再次判斷是否為 Conda 環境 / Check again if it's a Conda environment
            bool isConda = isCondaEnvironment(venv.path);
            bool isPythonConda = isPythonFromConda(venvPython);
            
            // 顯示選擇的環境名稱和開始檢測 / Display selected environment name and start detection
            if (isConda || isPythonConda) {
                ui->pythonPackagesLabel->setText(QString("Python 套件：%1 (Conda 環境) - 檢測中...").arg(displayName));
            } else {
                ui->pythonPackagesLabel->setText(QString("Python 套件：%1 - 檢測中...").arg(displayName));
            }
            
            // 立即更新 UI，然後開始檢測 / Update UI immediately, then start detection
            QApplication::processEvents();
            
            // 開始檢測 Python 套件 / Start detecting Python packages
            detectPythonPackages(venvPython);
        } else {
            QString errorMsg = venvPython.isEmpty() ? "無法找到 Python 可執行文件" : QString("Python 路徑不存在：%1").arg(venvPython);
            ui->pythonPackagesLabel->setText(QString("Python 套件：%1 - %2").arg(displayName, errorMsg));
            ui->pytorchInfoLabel->setText("PyTorch：未檢測");
            ui->ultralyticsInfoLabel->setText("Ultralytics：未檢測");
        }
    }
}

// 系統檢測方法實現 / System detection method implementations
void MainWindow::detectSystemInfo()
{
    QString arch = detectSystemArchitecture();
    QString cuda = detectCUDA();
    QString gpu = detectGPU();
    
    ui->systemInfoLabel->setText(QString("系統架構：%1").arg(arch));
    ui->cudaInfoLabel->setText(QString("CUDA：%1").arg(cuda));
    ui->gpuInfoLabel->setText(QString("GPU：%1").arg(gpu));
}

QString MainWindow::detectSystemArchitecture()
{
    // 檢測系統架構 / Detect system architecture
    #ifdef Q_PROCESSOR_X86_64
        return "x64 (64-bit)";
    #elif defined(Q_PROCESSOR_X86_32)
        return "x86 (32-bit)";
    #elif defined(Q_PROCESSOR_ARM_64)
        return "ARM64";
    #elif defined(Q_PROCESSOR_ARM)
        return "ARM";
    #else
        return "未知 / Unknown";
    #endif
}

QString MainWindow::detectCUDA()
{
    QStringList cudaPaths = {
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA",
        "C:/Program Files (x86)/NVIDIA GPU Computing Toolkit/CUDA",
    };
    
    // 查找 CUDA 安裝目錄 / Find CUDA installation directory
    QStringList cudaVersions;
    for (const QString &basePath : cudaPaths) {
        QDir cudaDir(basePath);
        if (cudaDir.exists()) {
            QStringList entries = cudaDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                // 檢查是否為版本目錄（例如 v11.8, v12.0） / Check if it's a version directory
                QRegularExpression versionRe("^v?\\d+\\.\\d+");
                if (versionRe.match(entry).hasMatch()) {
                    QString version = entry;
                    if (version.startsWith("v")) {
                        version = version.mid(1);
                    }
                    cudaVersions.append(version);
                }
            }
        }
    }
    
    // 使用 nvidia-smi 檢測 CUDA 版本 / Use nvidia-smi to detect CUDA version
    QProcess process;
    process.start("nvidia-smi", QStringList() << "--query-gpu=driver_version,cuda_version" << "--format=csv,noheader");
    process.waitForFinished(3000);
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            QStringList parts = output.split(',');
            if (parts.size() >= 2) {
                QString cudaVersion = parts[1].trimmed();
                if (!cudaVersion.isEmpty() && cudaVersion != "Not Supported") {
                    return QString("已安裝 (驅動版本: %1, CUDA: %2)").arg(parts[0].trimmed(), cudaVersion);
                }
            }
        }
    }
    
    // 如果找到 CUDA 目錄但 nvidia-smi 失敗 / If CUDA directory found but nvidia-smi failed
    if (!cudaVersions.isEmpty()) {
        cudaVersions.sort();
        return QString("已安裝 (版本: %1)").arg(cudaVersions.last());
    }
    
    return "未安裝 / Not installed";
}

QString MainWindow::detectGPU()
{
    // 使用 nvidia-smi 檢測 GPU / Use nvidia-smi to detect GPU
    QProcess process;
    process.start("nvidia-smi", QStringList() << "--query-gpu=name" << "--format=csv,noheader");
    process.waitForFinished(3000);
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        if (!output.isEmpty()) {
            QStringList gpus = output.split('\n', Qt::SkipEmptyParts);
            if (!gpus.isEmpty()) {
                QString gpuList = gpus.join(", ");
                return QString("可用 (%1)").arg(gpuList);
            }
        }
    }
    
    // 嘗試使用 wmic 檢測 GPU (Windows) / Try using wmic to detect GPU (Windows)
    process.start("wmic", QStringList() << "path" << "win32_VideoController" << "get" << "name");
    process.waitForFinished(3000);
    
    if (process.exitCode() == 0) {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
        QStringList gpus;
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty() && trimmed != "Name" && !trimmed.contains("VideoController")) {
                gpus.append(trimmed);
            }
        }
        if (!gpus.isEmpty()) {
            QString gpuList = gpus.join(", ");
            // 檢查是否為 NVIDIA GPU / Check if it's NVIDIA GPU
            bool hasNvidia = false;
            for (const QString &gpu : gpus) {
                if (gpu.contains("NVIDIA", Qt::CaseInsensitive)) {
                    hasNvidia = true;
                    break;
                }
            }
            if (hasNvidia) {
                return QString("可用 (%1)").arg(gpuList);
            } else {
                return QString("僅 CPU (%1)").arg(gpuList);
            }
        }
    }
    
    return "僅 CPU / CPU only";
}

// Python 套件檢測方法實現 / Python package detection method implementations
void MainWindow::detectPythonPackages(const QString &pythonPath)
{
    // 更新狀態標籤 / Update status labels
    ui->pytorchInfoLabel->setText("PyTorch：檢測中...");
    ui->ultralyticsInfoLabel->setText("Ultralytics：檢測中...");
    
    // 處理 UI 事件，確保標籤更新 / Process UI events to ensure labels are updated
    QApplication::processEvents();
    
    // 檢測 PyTorch / Detect PyTorch
    QString pytorchInfo = checkPyTorch(pythonPath);
    ui->pytorchInfoLabel->setText(QString("PyTorch：%1").arg(pytorchInfo));
    QApplication::processEvents();
    
    // 檢測 Ultralytics / Detect Ultralytics
    QString ultralyticsInfo = checkUltralytics(pythonPath);
    ui->ultralyticsInfoLabel->setText(QString("Ultralytics：%1").arg(ultralyticsInfo));
    
    // 更新 Python 套件標籤，移除"檢測中"狀態 / Update Python packages label, remove "detecting" status
    QString currentLabel = ui->pythonPackagesLabel->text();
    if (currentLabel.contains("檢測中")) {
        currentLabel.replace(" - 檢測中...", "");
        ui->pythonPackagesLabel->setText(currentLabel);
    }
    
    QApplication::processEvents();
}

QString MainWindow::checkPyTorch(const QString &pythonPath)
{
    qDebug() << "\n=== PyTorch 檢測開始 ===";
    qDebug() << "Python 路徑:" << pythonPath;
    
    // 首先驗證 Python 路徑是否存在 / First verify Python path exists
    if (!QFileInfo::exists(pythonPath)) {
        qDebug() << "錯誤：Python 路徑不存在:" << pythonPath;
        return QString("未安裝（Python 路徑不存在：%1）").arg(pythonPath);
    }
    
    // 測試 Python 是否可執行 / Test if Python is executable
    QProcess testProcess;
    testProcess.start(pythonPath, QStringList() << "--version");
    if (!testProcess.waitForFinished(3000)) {
        qDebug() << "錯誤：Python 無法執行";
        return "未安裝（Python 無法執行）";
    }
    QString pythonVersion = QString::fromUtf8(testProcess.readAllStandardOutput()).trimmed();
    qDebug() << "Python 版本:" << pythonVersion;
    
    // 使用 pip show 檢查 PyTorch / Use pip show to check PyTorch
    qDebug() << "執行: pip show torch";
    QProcess process;
    QFileInfo pythonInfo(pythonPath);
    QString workDir = pythonInfo.absolutePath();
    process.setWorkingDirectory(workDir);
    qDebug() << "工作目錄:" << workDir;
    
    // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("PYTHONHOME"); // 移除可能干擾的 PYTHONHOME / Remove interfering PYTHONHOME
    env.remove("PYTHONPATH"); // 移除可能干擾的 PYTHONPATH / Remove interfering PYTHONPATH
    process.setProcessEnvironment(env);
    
    QStringList args;
    args << "-m" << "pip" << "show" << "torch";
    qDebug() << "命令參數:" << args;
    process.start(pythonPath, args);
    
    // 等待完成，增加超時時間 / Wait for completion, increase timeout
    if (!process.waitForFinished(15000)) {
        process.kill();
        process.waitForFinished(1000);
        qDebug() << "PyTorch check: pip show timeout";
        // 繼續嘗試 pip list / Continue to try pip list
    } else {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        QString errorOutput = QString::fromUtf8(process.readAllStandardError());
        
        qDebug() << "PyTorch check: pip show exit code:" << process.exitCode();
        qDebug() << "PyTorch check: pip show output:" << output.left(200);
        if (!errorOutput.isEmpty()) {
            qDebug() << "PyTorch check: pip show error:" << errorOutput;
        }
        
        // 檢查退出碼和輸出 / Check exit code and output
        if (process.exitCode() == 0 && !output.isEmpty()) {
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            
            QString version;
            QString location;
            
            for (const QString &line : lines) {
                if (line.startsWith("Version:")) {
                    version = line.mid(8).trimmed();
                } else if (line.startsWith("Location:")) {
                    location = line.mid(10).trimmed();
                }
            }
            
            if (!version.isEmpty()) {
                // 檢查是否為 CUDA 版本 / Check if it's CUDA version
                QString cudaInfo = "";
                QProcess torchProcess;
                
                // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
                QProcessEnvironment torchEnv = QProcessEnvironment::systemEnvironment();
                torchEnv.remove("PYTHONHOME");
                torchEnv.remove("PYTHONPATH");
                torchProcess.setProcessEnvironment(torchEnv);
                
                torchProcess.start(pythonPath, QStringList() << "-c" << "import torch; print(torch.__version__); print(torch.cuda.is_available()); print(torch.version.cuda if torch.cuda.is_available() else 'N/A')");
                torchProcess.waitForFinished(5000);
                
                if (torchProcess.exitCode() == 0) {
                    QString torchOutput = QString::fromUtf8(torchProcess.readAllStandardOutput()).trimmed();
                    QStringList torchLines = torchOutput.split('\n', Qt::SkipEmptyParts);
                    if (torchLines.size() >= 2) {
                        bool cudaAvailable = torchLines[1].trimmed() == "True";
                        if (cudaAvailable && torchLines.size() >= 3) {
                            QString cudaVersion = torchLines[2].trimmed();
                            cudaInfo = QString(" (CUDA: %1)").arg(cudaVersion);
                        } else {
                            cudaInfo = " (CPU only)";
                        }
                    }
                }
                
                return QString("已安裝 %1%2").arg(version, cudaInfo);
            }
        }
    }
    
    // 如果 pip show 失敗，嘗試使用 pip list / If pip show fails, try pip list
    qDebug() << "\n嘗試使用 pip list 作為備用方法 / Trying pip list as fallback";
    QProcess listProcess;
    listProcess.setWorkingDirectory(workDir);
    
    // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
    QProcessEnvironment listEnv = QProcessEnvironment::systemEnvironment();
    listEnv.remove("PYTHONHOME"); // 移除可能干擾的 PYTHONHOME / Remove interfering PYTHONHOME
    listEnv.remove("PYTHONPATH"); // 移除可能干擾的 PYTHONPATH / Remove interfering PYTHONPATH
    listProcess.setProcessEnvironment(listEnv);
    
    QStringList listArgs;
    listArgs << "-m" << "pip" << "list";
    qDebug() << "執行命令:" << pythonPath << listArgs;
    listProcess.start(pythonPath, listArgs);
    
    bool listFinished = listProcess.waitForFinished(20000); // 增加超時時間 / Increase timeout
    qDebug() << "pip list 完成:" << listFinished;
    qDebug() << "pip list 退出碼:" << listProcess.exitCode();
    
    if (listFinished) {
        QString listOutput = QString::fromUtf8(listProcess.readAllStandardOutput());
        QString listError = QString::fromUtf8(listProcess.readAllStandardError());
        
        qDebug() << "pip list 輸出長度:" << listOutput.length();
        qDebug() << "pip list 錯誤輸出:" << listError;
        
        if (listProcess.exitCode() == 0 && !listOutput.isEmpty()) {
            // 保存完整輸出到文件以便調試 / Save full output to file for debugging
            qDebug() << "pip list 輸出前1000字符:" << listOutput.left(1000);
            
            QStringList lines = listOutput.split('\n', Qt::SkipEmptyParts);
            qDebug() << "總行數:" << lines.size();
            
            // 查找 torch 行 / Find torch line
            bool foundTorch = false;
            for (int i = 0; i < lines.size(); ++i) {
                const QString &line = lines[i];
                QString trimmed = line.trimmed();
                
                // 跳過標題行 / Skip header lines
                if (trimmed.startsWith("Package") || trimmed.startsWith("---") || trimmed.isEmpty()) {
                    continue;
                }
                
                // 檢查是否為 torch 行（不是 torchaudio 或 torchvision）/ Check if it's torch line (not torchaudio or torchvision)
                if (trimmed.startsWith("torch", Qt::CaseInsensitive) && 
                    !trimmed.startsWith("torchaudio", Qt::CaseInsensitive) && 
                    !trimmed.startsWith("torchvision", Qt::CaseInsensitive)) {
                    qDebug() << "找到 torch 行 (第" << i << "行):" << trimmed;
                    foundTorch = true;
                    
                    // 解析版本 / Parse version - 使用多種方法
                    QString version;
                    
                    // 方法1: 按空格分割 / Method 1: Split by spaces
                    QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    qDebug() << "分割結果，部分數:" << parts.size();
                    for (int j = 0; j < parts.size(); ++j) {
                        qDebug() << "  部分" << j << ":" << parts[j];
                    }
                    
                    if (parts.size() >= 2) {
                        version = parts[1];
                        qDebug() << "提取的版本:" << version;
                        
                        // 檢查 CUDA 版本 / Check CUDA version
                        QString cudaInfo = "";
                        if (version.contains("cu")) {
                            QRegularExpression cuRe("cu(\\d+)");
                            QRegularExpressionMatch match = cuRe.match(version);
                            if (match.hasMatch()) {
                                cudaInfo = QString(" (CUDA: %1)").arg(match.captured(1));
                            }
                        }
                        QString result = QString("已安裝 %1%2").arg(version, cudaInfo);
                        qDebug() << "=== PyTorch 檢測結果:" << result << "===";
                        return result;
                    }
                    
                    // 如果分割失敗，嘗試正則表達式 / If split fails, try regex
                    QRegularExpression versionRe("torch\\s+(\\S+)");
                    QRegularExpressionMatch match = versionRe.match(trimmed);
                    if (match.hasMatch()) {
                        version = match.captured(1);
                        qDebug() << "正則提取的版本:" << version;
                        QString result = QString("已安裝 %1").arg(version);
                        qDebug() << "=== PyTorch 檢測結果:" << result << "===";
                        return result;
                    }
                    
                    qDebug() << "無法解析版本，但找到 torch 行";
                    return "已安裝（版本未知）";
                }
            }
            
            if (!foundTorch) {
                qDebug() << "在 pip list 中未找到 torch";
                // 輸出前20行以便調試 / Output first 20 lines for debugging
                qDebug() << "前20行內容:";
                for (int i = 0; i < qMin(20, lines.size()); ++i) {
                    qDebug() << "  行" << i << ":" << lines[i];
                }
            }
        } else {
            qDebug() << "pip list 執行失敗，退出碼:" << listProcess.exitCode();
            qDebug() << "錯誤輸出:" << listError;
        }
    } else {
        qDebug() << "pip list 超時";
    }
    
    qDebug() << "=== PyTorch 檢測結果: 未安裝 ===";
    return "未安裝 / Not installed";
}

QString MainWindow::checkUltralytics(const QString &pythonPath)
{
    qDebug() << "\n=== Ultralytics 檢測開始 ===";
    qDebug() << "Python 路徑:" << pythonPath;
    
    // 首先驗證 Python 路徑是否存在 / First verify Python path exists
    if (!QFileInfo::exists(pythonPath)) {
        qDebug() << "錯誤：Python 路徑不存在:" << pythonPath;
        return QString("未安裝（Python 路徑不存在：%1）").arg(pythonPath);
    }
    
    // 使用 pip show 檢查 ultralytics / Use pip show to check ultralytics
    qDebug() << "執行: pip show ultralytics";
    QProcess process;
    QFileInfo pythonInfo(pythonPath);
    QString workDir = pythonInfo.absolutePath();
    process.setWorkingDirectory(workDir);
    qDebug() << "工作目錄:" << workDir;
    
    // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("PYTHONHOME"); // 移除可能干擾的 PYTHONHOME / Remove interfering PYTHONHOME
    env.remove("PYTHONPATH"); // 移除可能干擾的 PYTHONPATH / Remove interfering PYTHONPATH
    process.setProcessEnvironment(env);
    
    QStringList args;
    args << "-m" << "pip" << "show" << "ultralytics";
    qDebug() << "命令參數:" << args;
    process.start(pythonPath, args);
    
    // 等待完成，增加超時時間 / Wait for completion, increase timeout
    if (!process.waitForFinished(15000)) {
        process.kill();
        process.waitForFinished(1000);
        qDebug() << "Ultralytics check: pip show timeout";
        // 繼續嘗試 pip list / Continue to try pip list
    } else {
        QString output = QString::fromUtf8(process.readAllStandardOutput());
        QString errorOutput = QString::fromUtf8(process.readAllStandardError());
        
        qDebug() << "Ultralytics check: pip show exit code:" << process.exitCode();
        qDebug() << "Ultralytics check: pip show output:" << output.left(200);
        if (!errorOutput.isEmpty()) {
            qDebug() << "Ultralytics check: pip show error:" << errorOutput;
        }
        
        // 檢查退出碼和輸出 / Check exit code and output
        if (process.exitCode() == 0 && !output.isEmpty()) {
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);
            
            QString version;
            
            for (const QString &line : lines) {
                if (line.startsWith("Version:")) {
                    version = line.mid(8).trimmed();
                    break;
                }
            }
            
            if (!version.isEmpty()) {
                // 檢查 ultralytics-thop / Check ultralytics-thop
                QString thopVersion = "";
                QProcess thopProcess;
                thopProcess.setWorkingDirectory(QFileInfo(pythonPath).absolutePath());
                
                // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
                QProcessEnvironment thopEnv = QProcessEnvironment::systemEnvironment();
                thopEnv.remove("PYTHONHOME");
                thopEnv.remove("PYTHONPATH");
                thopProcess.setProcessEnvironment(thopEnv);
                
                thopProcess.start(pythonPath, QStringList() << "-m" << "pip" << "show" << "ultralytics-thop");
                thopProcess.waitForFinished(5000);
                
                if (thopProcess.exitCode() == 0) {
                    QString thopOutput = QString::fromUtf8(thopProcess.readAllStandardOutput());
                    QStringList thopLines = thopOutput.split('\n', Qt::SkipEmptyParts);
                    for (const QString &line : thopLines) {
                        if (line.startsWith("Version:")) {
                            thopVersion = line.mid(8).trimmed();
                            break;
                        }
                    }
                }
                
                if (!thopVersion.isEmpty()) {
                    return QString("已安裝 %1 (thop: %2)").arg(version, thopVersion);
                } else {
                    return QString("已安裝 %1").arg(version);
                }
            }
        }
    }
    
    // 如果 pip show 失敗，嘗試使用 pip list / If pip show fails, try pip list
    qDebug() << "\n嘗試使用 pip list 作為備用方法 / Trying pip list as fallback";
    QProcess listProcess;
    listProcess.setWorkingDirectory(workDir);
    
    // 設置環境變數，清除可能干擾的 PYTHONHOME / Set environment variables, clear interfering PYTHONHOME
    QProcessEnvironment listEnv = QProcessEnvironment::systemEnvironment();
    listEnv.remove("PYTHONHOME"); // 移除可能干擾的 PYTHONHOME / Remove interfering PYTHONHOME
    listEnv.remove("PYTHONPATH"); // 移除可能干擾的 PYTHONPATH / Remove interfering PYTHONPATH
    listProcess.setProcessEnvironment(listEnv);
    
    QStringList listArgs;
    listArgs << "-m" << "pip" << "list";
    qDebug() << "執行命令:" << pythonPath << listArgs;
    listProcess.start(pythonPath, listArgs);
    
    bool listFinished = listProcess.waitForFinished(20000); // 增加超時時間 / Increase timeout
    qDebug() << "pip list 完成:" << listFinished;
    qDebug() << "pip list 退出碼:" << listProcess.exitCode();
    
    if (listFinished) {
        QString listOutput = QString::fromUtf8(listProcess.readAllStandardOutput());
        QString listError = QString::fromUtf8(listProcess.readAllStandardError());
        
        qDebug() << "pip list 輸出長度:" << listOutput.length();
        qDebug() << "pip list 錯誤輸出:" << listError;
        
        if (listProcess.exitCode() == 0 && !listOutput.isEmpty()) {
            qDebug() << "pip list 輸出前1000字符:" << listOutput.left(1000);
            
            QStringList lines = listOutput.split('\n', Qt::SkipEmptyParts);
            qDebug() << "總行數:" << lines.size();
            
            QString ultralyticsVersion;
            QString thopVersion;
            
            // 查找 ultralytics 和 ultralytics-thop 行 / Find ultralytics and ultralytics-thop lines
            for (int i = 0; i < lines.size(); ++i) {
                const QString &line = lines[i];
                QString trimmed = line.trimmed();
                
                // 跳過標題行 / Skip header lines
                if (trimmed.startsWith("Package") || trimmed.startsWith("---") || trimmed.isEmpty()) {
                    continue;
                }
                
                if (trimmed.startsWith("ultralytics", Qt::CaseInsensitive) && !trimmed.startsWith("ultralytics-thop", Qt::CaseInsensitive)) {
                    qDebug() << "找到 ultralytics 行 (第" << i << "行):" << trimmed;
                    QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    qDebug() << "分割結果，部分數:" << parts.size();
                    for (int j = 0; j < parts.size(); ++j) {
                        qDebug() << "  部分" << j << ":" << parts[j];
                    }
                    if (parts.size() >= 2) {
                        ultralyticsVersion = parts[1];
                        qDebug() << "提取的 ultralytics 版本:" << ultralyticsVersion;
                    }
                } else if (trimmed.startsWith("ultralytics-thop", Qt::CaseInsensitive)) {
                    qDebug() << "找到 ultralytics-thop 行 (第" << i << "行):" << trimmed;
                    QStringList parts = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    qDebug() << "thop 分割結果，部分數:" << parts.size();
                    for (int j = 0; j < parts.size(); ++j) {
                        qDebug() << "  部分" << j << ":" << parts[j];
                    }
                    if (parts.size() >= 2) {
                        thopVersion = parts[1];
                        qDebug() << "提取的 thop 版本:" << thopVersion;
                    }
                }
            }
            
            if (!ultralyticsVersion.isEmpty()) {
                QString result;
                if (!thopVersion.isEmpty()) {
                    result = QString("已安裝 %1 (thop: %2)").arg(ultralyticsVersion, thopVersion);
                } else {
                    result = QString("已安裝 %1").arg(ultralyticsVersion);
                }
                qDebug() << "=== Ultralytics 檢測結果:" << result << "===";
                return result;
            } else {
                qDebug() << "在 pip list 中未找到 ultralytics";
                // 輸出前20行以便調試 / Output first 20 lines for debugging
                qDebug() << "前20行內容:";
                for (int i = 0; i < qMin(20, lines.size()); ++i) {
                    qDebug() << "  行" << i << ":" << lines[i];
                }
            }
        } else {
            qDebug() << "pip list 執行失敗，退出碼:" << listProcess.exitCode();
            qDebug() << "錯誤輸出:" << listError;
        }
    } else {
        qDebug() << "pip list 超時";
    }
    
    qDebug() << "=== Ultralytics 檢測結果: 未安裝 ===";
    return "未安裝 / Not installed";
}

bool MainWindow::isPythonFromConda(const QString &pythonPath)
{
    // 檢查 Python 路徑是否在 Conda 環境中 / Check if Python path is in Conda environment
    QFileInfo pythonInfo(pythonPath);
    QString pythonDir = pythonInfo.absolutePath();
    
    // 檢查路徑中是否包含 conda 相關關鍵字 / Check if path contains conda-related keywords
    QString normalizedPath = QDir::toNativeSeparators(pythonDir).toLower();
    if (normalizedPath.contains("anaconda") || 
        normalizedPath.contains("miniconda") ||
        normalizedPath.contains("conda")) {
        
        // 檢查是否在 envs 目錄下（conda 環境） / Check if it's in envs directory (conda environment)
        if (normalizedPath.contains("envs")) {
            // 向上查找 conda-meta 目錄 / Look up for conda-meta directory
            QDir dir(pythonDir);
            while (dir.cdUp()) {
                QString condaMeta = dir.absoluteFilePath("conda-meta");
                if (QFileInfo::exists(condaMeta)) {
                    return true;
                }
                // 如果到達 envs 目錄，停止向上查找 / Stop if reached envs directory
                if (dir.dirName().toLower() == "envs") {
                    break;
                }
            }
        } else {
            // 檢查是否為 base 環境（直接在 anaconda3/miniconda3 目錄下） / Check if it's base environment
            QDir dir(pythonDir);
            // 向上查找 conda-meta 目錄 / Look up for conda-meta directory
            while (dir.cdUp()) {
                QString condaMeta = dir.absoluteFilePath("conda-meta");
                if (QFileInfo::exists(condaMeta)) {
                    return true;
                }
                // 如果到達根目錄或 Program Files，停止 / Stop if reached root or Program Files
                if (dir.isRoot() || dir.dirName().isEmpty()) {
                    break;
                }
            }
        }
    }
    
    // 檢查 Python 路徑的父目錄是否包含 conda-meta / Check if parent directory contains conda-meta
    QDir pythonParentDir(pythonDir);
    pythonParentDir.cdUp();
    QString condaMeta = pythonParentDir.absoluteFilePath("conda-meta");
    if (QFileInfo::exists(condaMeta)) {
        return true;
    }
    
    return false;
}

// 設定保存和載入方法實現 / Settings save and load method implementations
void MainWindow::saveSettings()
{
    // 使用 QSettings 保存設定 / Use QSettings to save settings
    QSettings settings("Capybara", "PythonEnvironment");
    
    // 保存當前選擇的 Python 環境路徑 / Save current selected Python environment path
    settings.setValue("pythonPath", currentPythonPath);
    
    // 保存選擇的虛擬環境信息 / Save selected virtual environment information
    settings.setValue("venvName", selectedEnvName);
    settings.setValue("venvPath", selectedEnvPath);
    settings.setValue("venvPythonPath", selectedPythonPath);
    
    qDebug() << "設定已保存 / Settings saved:";
    qDebug() << "  Python 路徑:" << currentPythonPath;
    qDebug() << "  虛擬環境名稱:" << selectedEnvName;
    qDebug() << "  虛擬環境路徑:" << selectedEnvPath;
    qDebug() << "  Python 可執行文件路徑:" << selectedPythonPath;
}

void MainWindow::loadSettings()
{
    // 使用 QSettings 載入設定 / Use QSettings to load settings
    QSettings settings("Capybara", "PythonEnvironment");
    
    // 載入保存的 Python 環境路徑 / Load saved Python environment path
    currentPythonPath = settings.value("pythonPath").toString();
    
    // 載入保存的虛擬環境信息 / Load saved virtual environment information
    selectedEnvName = settings.value("venvName").toString();
    selectedEnvPath = settings.value("venvPath").toString();
    selectedPythonPath = settings.value("venvPythonPath").toString();
    
    qDebug() << "設定已載入 / Settings loaded:";
    qDebug() << "  Python 路徑:" << currentPythonPath;
    qDebug() << "  虛擬環境名稱:" << selectedEnvName;
    qDebug() << "  虛擬環境路徑:" << selectedEnvPath;
    qDebug() << "  Python 可執行文件路徑:" << selectedPythonPath;
}

bool MainWindow::hasSavedSettings()
{
    // 檢查是否有保存的設定 / Check if saved settings exist
    QSettings settings("Capybara", "PythonEnvironment");
    
    // 檢查關鍵設定是否存在 / Check if key settings exist
    bool hasPythonPath = settings.contains("pythonPath") && !settings.value("pythonPath").toString().isEmpty();
    bool hasVenvPath = settings.contains("venvPath") && !settings.value("venvPath").toString().isEmpty();
    bool hasVenvPythonPath = settings.contains("venvPythonPath") && !settings.value("venvPythonPath").toString().isEmpty();
    
    return hasPythonPath && hasVenvPath && hasVenvPythonPath;
}

void MainWindow::applySavedSettings()
{
    // 應用保存的設定 / Apply saved settings
    if (currentPythonPath.isEmpty() || selectedEnvPath.isEmpty() || selectedPythonPath.isEmpty()) {
        qDebug() << "無法應用設定：設定不完整 / Cannot apply settings: settings incomplete";
        return;
    }
    
    // 驗證保存的路徑是否仍然存在 / Verify if saved paths still exist
    if (!QFileInfo::exists(currentPythonPath)) {
        qDebug() << "保存的 Python 路徑不存在:" << currentPythonPath;
        QMessageBox::warning(this, 
                            tr("設定錯誤 / Settings Error"), 
                            tr("保存的 Python 環境路徑不存在，請重新選擇環境。\n"
                               "Saved Python environment path does not exist, please reselect environment."));
        return;
    }
    
    if (!QFileInfo::exists(selectedPythonPath)) {
        qDebug() << "保存的 Python 可執行文件路徑不存在:" << selectedPythonPath;
        QMessageBox::warning(this, 
                            tr("設定錯誤 / Settings Error"), 
                            tr("保存的 Python 可執行文件路徑不存在，請重新選擇環境。\n"
                               "Saved Python executable path does not exist, please reselect environment."));
        return;
    }
    
    // 在 Python 環境列表中查找匹配的環境 / Find matching environment in Python environments list
    int pythonIndex = -1;
    for (int i = 0; i < pythonEnvironments.size(); ++i) {
        if (pythonEnvironments[i].path == currentPythonPath) {
            pythonIndex = i;
            break;
        }
    }
    
    if (pythonIndex >= 0) {
        // 暫時阻止信號發射，避免遞歸調用 / Block signals temporarily to avoid recursive calls
        ui->pythonComboBox->blockSignals(true);
        ui->venvComboBox->blockSignals(true);
        
        // 設置 Python 環境選擇 / Set Python environment selection
        ui->pythonComboBox->setCurrentIndex(pythonIndex);
        ui->pythonPathLabel->setText("路徑：" + currentPythonPath);
        
        // 掃描該 Python 環境的虛擬環境 / Scan virtual environments for this Python environment
        scanVirtualEnvironments(currentPythonPath);
        
        // 在虛擬環境列表中查找匹配的環境 / Find matching environment in virtual environments list
        int venvIndex = -1;
        for (int i = 0; i < virtualEnvironments.size(); ++i) {
            if (virtualEnvironments[i].path == selectedEnvPath) {
                venvIndex = i;
                break;
            }
        }
        
        if (venvIndex >= 0) {
            // 設置虛擬環境選擇 / Set virtual environment selection
            ui->venvComboBox->setCurrentIndex(venvIndex);
            
            // 恢復信號發射 / Restore signal emission
            ui->pythonComboBox->blockSignals(false);
            ui->venvComboBox->blockSignals(false);
            
            // 更新顯示 / Update display
            QString displayName = selectedEnvName;
            if (displayName.startsWith("[Conda] ")) {
                displayName = displayName.mid(8);
            }
            ui->selectedEnvLabel->setText(QString("已指定環境：%1").arg(displayName));
            
            // 自動執行檢測 / Automatically run detection
            QApplication::processEvents();
            detectPythonPackages(selectedPythonPath);
            
            qDebug() << "設定已成功應用 / Settings successfully applied";
        } else {
            // 恢復信號發射 / Restore signal emission
            ui->pythonComboBox->blockSignals(false);
            ui->venvComboBox->blockSignals(false);
            
            qDebug() << "無法找到保存的虛擬環境:" << selectedEnvPath;
            QMessageBox::warning(this, 
                                tr("設定警告 / Settings Warning"), 
                                tr("無法找到保存的虛擬環境，請重新選擇。\n"
                                   "Cannot find saved virtual environment, please reselect."));
        }
    } else {
        qDebug() << "無法找到保存的 Python 環境:" << currentPythonPath;
        QMessageBox::warning(this, 
                            tr("設定警告 / Settings Warning"), 
                            tr("無法找到保存的 Python 環境，請重新選擇。\n"
                               "Cannot find saved Python environment, please reselect."));
    }
}
