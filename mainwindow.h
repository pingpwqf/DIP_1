#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "task.h"

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

private:
    Ui::MainWindow *ui;
    QString filePath, dirPath, dirOutPath;
    QStringList inFileList, outFileList;

    QVector<QString> selectedChoices;
    ResultCollector collector;
    // std::unique_ptr<AlgInterface> basePtr;
    std::unique_ptr<TaskManager> taskEngine;
    // AlgRegistry<QString> reg = AlgRegistry<QString>::instance();

private slots:
    void showFile();
    void showDir();
    void showOutDir();
    // void on_pushButton_clicked(bool checked);
    // void registerGLCMhomo();

    // void check();
    void MainExecute();
};



#endif // MAINWINDOW_H
