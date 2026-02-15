#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QThreadPool>

cv::Mat testImage = cv::Mat::ones(3, 3, CV_8UC1);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    taskEngine = std::make_unique<TaskManager>(&collector);
    QThreadPool::globalInstance()->setMaxThreadCount(5);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(showFile()));
    connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(showDir()));
    connect(ui->pushButton_5, SIGNAL(clicked(bool)), this, SLOT(showOutDir()));

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

    // connect(ui->pushButton_4, &QPushButton::clicked,
    //         this, &MainWindow::check);
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

// void MainWindow::registerGLCMhomo()
// {
//     reg.Register(HOMONAME, [](cv::InputArray img){
//         return std::make_unique<GLCM::GLCMhomoAlg>(img, 32, 1, 0);
//     });
//     QString filename = dirOutPath + "/" + HOMONAME + ".txt";
//     QFile HOMOfile(filename);
//     if(HOMOfile.exists()) outFileList.append(filename);
// }

// void MainWindow::check()
// {
//     basePtr->checkAlg();
//     QFile file1(outFileList[0]);
//     file1.open(QIODevice::WriteOnly | QIODevice::Text);
//     QTextStream outMSV(&file1);
//     outMSV.setAutoDetectUnicode(true);
//     outMSV << "output\n";
//     Qt::flush(outMSV);
// }

void MainWindow::MainExecute()
{
    if(dirOutPath.isEmpty()) qDebug()<< "haven't select output path";
    collector.setOutputDir(dirOutPath);
    collector.prepare();
    if(ui->actionMSV->isChecked()){
        selectedChoices.emplaceBack(MSVNAME);
        qDebug() << "MSV";
    }
    if(ui->actionNIPC->isChecked())selectedChoices.emplaceBack(NIPCNAME);
    if(ui->actionZNCC->isChecked())selectedChoices.emplaceBack(ZNCCNAME);
    if(ui->actionCorrelation->isChecked())selectedChoices.emplaceBack(CORRNAME);
    if(ui->actionHomogeneity->isChecked())selectedChoices.emplaceBack(HOMONAME);
    taskEngine->ExecuteSelected(filePath, dirPath, selectedChoices);
    ui->statusbar->showMessage(tr("完成"), 2000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

