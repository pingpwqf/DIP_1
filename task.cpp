#include "task.h"

#include <QThreadPool>
#include <QFile>
#include <QFileInfo>
#include <QDir>

void ProcessingTask::run()
{
    cv::Mat img = cv::imread(m_path.toStdString(), cv::IMREAD_GRAYSCALE);
    if(img.empty()) return;

    double val = 0;
    if(m_alg->expectInput()) {
        val = m_alg->process(img); // 非 GLCM 模式
    } else {
        m_alg = AlgRegistry<QString>::instance().get(m_alg->Name(), img);
        val = m_alg->process();
    }

    // 发射信号给结果收集器，而不是直接写文件
    // QString fileName = QFileInfo(m_path).fileName();
    emit resultReady(m_alg->Name(), val);
}

// task.cpp
void TaskManager::ExcuteSelected(const QString& refPath, const QString& dirPath)
{
    QDir dir(dirPath);
    QStringList files = dir.entryList({"*.bmp", "*.png"}, QDir::Files);
    QVector<QString> selectedAlgs = reg.names();

    // 提前读入参考图（一次即可）
    cv::Mat refImg = cv::imread(refPath.toStdString(), cv::IMREAD_GRAYSCALE);

    for(const QString& algName : selectedAlgs) {
        // 获取一个探测对象，确定模式
        auto probe = reg.get(algName, refImg);

        for(const QString& fileName : files) {
            QString fullPath = dir.absoluteFilePath(fileName);
            // 将读取和创建逻辑封装进 Task，主线程只管发任务
            ProcessingTask* task = new ProcessingTask(fullPath, algName, (probe->expectInput() ? probe : nullptr));

            // 确保 m_collector 已经由 MainWindow 初始化并传入
            connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);

            QThreadPool::globalInstance()->setMaxThreadCount(5);
            QThreadPool::globalInstance()->start(task);
        }
    }
}

void ResultCollector::handleResult(QString algName, double value)
{
    QString fullPath = m_outputDir + "/" + algName + ".txt";
    QFile file(fullPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << algName << " : " << value << "\n";
    }
}
