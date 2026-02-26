#include "mainwindow.h"
#include "ImgPcAlg.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QThreadPool>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    taskEngine = std::make_unique<TaskManager>(&collector);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::showFile);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::showDir);
    connect(ui->pushButton_5, &QPushButton::clicked, this, &MainWindow::showOutDir);
    connect(ui->actionROI, &QAction::changed, this, &MainWindow::selectROI);
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this](){
        ui->pushButton_4->setEnabled(true);
        if (taskEngine) {
            auto canc = QMessageBox::warning(this, "cancel", "stop processing！");
        } else {
            // 若未运行，则执行重置逻辑：清空路径
            ui->fileLineEdit->clear();
            ui->dirLineEdit->clear();
        }
    });

    // connect(ui->actionMSV, &QAction::toggled,
    //         this, &MainWindow::registerMSV);
    AlgRegistry<QString>::instance().Register(MSVNAME, [](cv::InputArray img){
        return std::make_unique<MSVAlg>(img);
    });
    AlgRegistry<QString>::instance().Register(NIPCNAME, [](cv::InputArray img){
        return std::make_unique<NIPCAlg>(img);
    });
    AlgRegistry<QString>::instance().Register(ZNCCNAME, [](cv::InputArray img){
        return std::make_unique<ZNCCAlg>(img);
    });
    AlgRegistry<QString>::instance().Register(CORRNAME, [](cv::InputArray img){
        return std::make_unique<GLCM::GLCMcorrAlg>(img);
    });
    AlgRegistry<QString>::instance().Register(HOMONAME, [](cv::InputArray img){
        return std::make_unique<GLCM::GLCMhomoAlg>(img);
    });

    connect(ui->pushButton_4, &QPushButton::clicked,
            this, &MainWindow::MainExecute);
}

void MainWindow::showFile()
{
    filePath = QFileDialog::getOpenFileName(this, tr("Open Image"),"/",
                                             tr("Image Files (*.bmp *.png)"));
    ui->fileLineEdit->setText(filePath);
}

void MainWindow::showDir()
{
    dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),filePath+"/..",
                                                  QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    ui->dirLineEdit->setText(dirPath);
}

void MainWindow::showOutDir()
{
    dirOutPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"/",
                                                QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    ui->dirLineEdit_2->setText(dirOutPath);
}

void MainWindow::selectROI()
{
    if(ui->actionROI->isChecked()){

    }
}

void MainWindow::MainExecute()
{
    if(filePath.isEmpty()) QMessageBox::warning(this, "noRef",
                             tr("haven't select reference image!"));
    else if(dirPath.isEmpty()) QMessageBox::warning(this, "noSource",
                             tr("haven't select Source images!"));
    else if(dirOutPath.isEmpty()) QMessageBox::warning(this, "noOutPath",
                             tr("haven't select output path!"));
    else{
        ui->pushButton_4->setEnabled(false); // 冻结按钮
        collector.setOutputDir(dirOutPath);
        collector.prepare();

        ProcessingSession* session = taskEngine->createSession();
        connect(ui->pushButton_3, &QPushButton::clicked, session, &ProcessingSession::cancel);

        connect(session, &ProcessingSession::sessionFinished, this, [this, session](){
            ui->pushButton_4->setEnabled(true); // 解冻
            collector.closeAll();               // 关闭文件
            ui->statusbar->showMessage(tr("批处理完成！"), 5000);

            session->deleteLater(); // 销毁 Session 对象
        });

        QDir dir(dirPath);
        QStringList files = dir.entryList({"*.bmp", "*.png", "*.jpg"}, QDir::Files);
        cv::Mat refImg = imread_safe(filePath);

        if(ui->actionMSV->isChecked())selectedChoices.emplaceBack(MSVNAME);
        if(ui->actionNIPC->isChecked())selectedChoices.emplaceBack(NIPCNAME);
        if(ui->actionZNCC->isChecked())selectedChoices.emplaceBack(ZNCCNAME);
        if(ui->actionCorrelation->isChecked())selectedChoices.emplaceBack(CORRNAME);
        if(ui->actionHomogeneity->isChecked())selectedChoices.emplaceBack(HOMONAME);
        // taskEngine->ExecuteSelected(filePath, dirPath, selectedChoices);
        if(selectedChoices.isEmpty()) {
            QMessageBox::warning(this, "noChoice",
                                 tr("haven't choose any processing method!"));
            ui->pushButton_4->setEnabled(true);
        }else session->start(refImg, files, dir, selectedChoices);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

