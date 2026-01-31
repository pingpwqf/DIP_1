#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ImgPcAlg.h"

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

    std::unique_ptr<AlgInterface> basePtr;
    AlgRegistry<QAction*> reg = AlgRegistry<QAction*>::instance();

private slots:
    void showFile();
    void showDir();
    void showOutDir();
    // void on_pushButton_clicked(bool checked);
    // void registerGLCMhomo();

    void resultCollector();

    void check();
};



#endif // MAINWINDOW_H
