#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QStringList>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onPythonComboBoxChanged(int index);
    void onVenvComboBoxChanged(int index);
    void onSelectEnvButtonClicked();
    void onRefreshButtonClicked();
    void scanPythonEnvironments();

private:
    struct PythonEnvironment {
        QString version;
        QString path;
        QString displayName;
    };

    struct VirtualEnvironment {
        QString name;
        QString path;
        QString pythonPath;
        QString displayName;
    };

    void detectPythonEnvironments();
    QStringList findPythonInPath();
    QStringList findPythonViaLauncher();
    QStringList findPythonInCommonLocations();
    QString getPythonVersion(const QString &pythonPath);
    void updateComboBox();
    
    void scanVirtualEnvironments(const QString &pythonPath);
    QStringList findVirtualEnvironments(const QString &pythonPath);
    QMap<QString, QString> findCondaEnvironments(); // 返回環境名稱到路徑的映射 / Returns map of environment name to path
    bool isVirtualEnvironmentForPython(const QString &venvPath, const QString &pythonPath);
    bool isCondaEnvironment(const QString &venvPath);
    QString getVenvPythonPath(const QString &venvPath);
    void updateVenvComboBox();
    
    // 系統檢測方法 / System detection methods
    void detectSystemInfo();
    QString detectSystemArchitecture();
    QString detectCUDA();
    QString detectGPU();
    
    // Python 包檢測方法 / Python package detection methods
    void detectPythonPackages(const QString &pythonPath);
    QString checkPyTorch(const QString &pythonPath);
    QString checkUltralytics(const QString &pythonPath);
    bool isPythonFromConda(const QString &pythonPath); // 檢查 Python 是否來自 Conda 環境 / Check if Python is from Conda environment
    
    // 設定保存和載入方法 / Settings save and load methods
    void saveSettings(); // 保存設定 / Save settings
    void loadSettings(); // 載入設定 / Load settings
    bool hasSavedSettings(); // 檢查是否有保存的設定 / Check if saved settings exist
    void applySavedSettings(); // 應用保存的設定 / Apply saved settings

    Ui::MainWindow *ui;
    QList<PythonEnvironment> pythonEnvironments;
    QList<VirtualEnvironment> virtualEnvironments;
    QString currentPythonPath;
    QString selectedEnvName; // 已指定的環境名稱 / Selected environment name
    QString selectedEnvPath; // 已指定的環境路徑 / Selected environment path
    QString selectedPythonPath; // 已指定的 Python 路徑 / Selected Python path
};
#endif // MAINWINDOW_H / Header guard end
