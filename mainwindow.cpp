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
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &MainWindow::onRefreshButtonClicked);
    
    // 掃描 Python 環境 / Scan Python environments
    scanPythonEnvironments();
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
    } else {
        ui->pythonPathLabel->setText("路徑：未選擇");
        currentPythonPath.clear();
        virtualEnvironments.clear();
        ui->venvComboBox->clear();
        ui->venvComboBox->setEnabled(false);
        ui->venvPathLabel->setText("虛擬環境路徑：未選擇");
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
    // 檢查 Windows 下的 Scripts/python.exe / Check Scripts/python.exe on Windows
    QString scriptsPython = QDir(venvPath).absoluteFilePath("Scripts/python.exe");
    if (QFileInfo::exists(scriptsPython)) {
        return scriptsPython;
    }
    
    // 檢查 Unix 風格下的 bin/python / Check bin/python in Unix style
    QString binPython = QDir(venvPath).absoluteFilePath("bin/python");
    if (QFileInfo::exists(binPython)) {
        return binPython;
    }
    
    // 檢查 bin/python.exe (某些 Windows 虛擬環境) / Check bin/python.exe (some Windows virtual environments)
    QString binPythonExe = QDir(venvPath).absoluteFilePath("bin/python.exe");
    if (QFileInfo::exists(binPythonExe)) {
        return binPythonExe;
    }
    
    return QString();
}

void MainWindow::updateVenvComboBox()
{
    ui->venvComboBox->clear();
    
    if (virtualEnvironments.isEmpty()) {
        ui->venvComboBox->addItem("未找到虛擬環境");
        ui->venvComboBox->setEnabled(false);
        ui->venvPathLabel->setText("虛擬環境路徑：未找到");
    } else {
        ui->venvComboBox->setEnabled(true);
        for (const VirtualEnvironment &venv : virtualEnvironments) {
            ui->venvComboBox->addItem(venv.displayName);
        }
        ui->venvPathLabel->setText("虛擬環境路徑：" + virtualEnvironments[0].path);
    }
}

void MainWindow::onVenvComboBoxChanged(int index)
{
    if (index >= 0 && index < virtualEnvironments.size()) {
        const VirtualEnvironment &venv = virtualEnvironments[index];
        ui->venvPathLabel->setText("虛擬環境路徑：" + venv.path);
    } else {
        ui->venvPathLabel->setText("虛擬環境路徑：未選擇");
    }
}
