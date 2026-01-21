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

    Ui::MainWindow *ui;
    QList<PythonEnvironment> pythonEnvironments;
    QList<VirtualEnvironment> virtualEnvironments;
    QString currentPythonPath;
};
#endif // MAINWINDOW_H / Header guard end
