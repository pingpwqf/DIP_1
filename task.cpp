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
    QString fileName = QFileInfo(m_path).fileName();
    emit resultReady(m_alg->Name(),fileName, val);
}

// task.cpp 核心逻辑
void TaskManager::ExcuteSelected(const QString& refPath, const QString& dirPath, const QString& outPath)
{
    QDir dir(dirPath);
    QStringList files = dir.entryList({"*.bmp", "*.png"}, QDir::Files);
    cv::Mat refImg = cv::imread(refPath.toStdString(), cv::IMREAD_GRAYSCALE);

    const QVector<QString>& choices = reg.names();

    for(const auto& choice : choices) {
        if(choice.isEmpty()) continue;

        // 1. 判定算法模式
        // 先用参考图预创建一个算法实例来探测类型
        auto probeAlg = reg.get(choice, refImg);

        for(const QString& fileName : files) {
            QString fullPath = dir.absoluteFilePath(fileName);

            ProcessingTask* task = nullptr;

            if(probeAlg->expectInput()) {
                // 【参考图模式】：所有图片共用同一个 probeAlg (内含参考图的 UMat)
                task = new ProcessingTask(fullPath, probeAlg, outPath);
            } else {
                // 【GLCM模式】：每张图需要独立的算法实例
                // 此时直接读取当前图并创建算法
                cv::Mat currentImg = cv::imread(fullPath.toStdString(), cv::IMREAD_GRAYSCALE);
                auto dynamicAlg = reg.get(choice, currentImg);
                task = new ProcessingTask(fullPath, dynamicAlg, outPath);
            }

            // 2. 连接信号到收集器
            connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);

            // 3. 丢进线程池
            QThreadPool::globalInstance()->start(task);
        }
    }
}

void ResultCollector::handleResult(QString algName, QString fileName, double value)
{
    QString fullPath = m_outputDir + "/" + algName + ".txt";
    QFile file(fullPath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << fileName << " : " << value << "\n";
    }
}
