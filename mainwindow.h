#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QtConsole;
class QtPlotter;
namespace logo {
class Engine;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString cmd_buf;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void launchEditor();
    void About();
    void showHelp();
    void savePic();
    void openFile();
    void loadFile();
    bool saveFile(QString path);
    void onSaveasFile();
    void bufferExec();
    void apply(QString &str, bool echo = false);
    void runScript(const QString &name, const QString &text);

    void setListenerText(QString str);

    // 阶段 0：无头脚本执行 / 回归基线用的只读接口
    QString transcript() const;
    void clearTranscript();
    QString stateSnapshot() const;

    void initPArea();
    void initListener();

private:
    // 注意声明顺序：engine 先销毁，console/plotter 后销毁。
    std::unique_ptr<QtPlotter> plotter_;
    std::unique_ptr<QtConsole> console_;
    std::unique_ptr<logo::Engine> engine_;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
