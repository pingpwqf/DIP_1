#include "task.h"

#include <QThreadPool>
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
void TaskManager::ExecuteSelected(const QString& refPath, const QString& dirPath)
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
            ProcessingTask* task = new ProcessingTask(fullPath, probe);

            // 确保 m_collector 已经由 MainWindow 初始化并传入
            connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);
            QThreadPool::globalInstance()->start(task);
        }
    }
}

void ResultCollector::handleResult(QString algName, double value)
{
    if (!m_streams.contains(algName)) {
        QString fullPath = m_outputDir + "/" + algName + ".txt";
        auto file = QSharedPointer<QFile>::create(fullPath);
        // 使用 WriteOnly | Text 模式，如果需要追加改为 Append
        if (file->open(QIODevice::Append | QIODevice::Text)) {
            m_files[algName] = file;
            m_streams[algName] = QSharedPointer<QTextStream>::create(file.data());
        }
    }

    if (m_streams.contains(algName)) {
        *m_streams[algName] << algName << "," << value << "\n";
        // 适当的时候 flush，或者依赖析构时自动处理
    }
}

void ResultCollector::closeAll() {
    m_streams.clear(); // 顺序很重要：先销毁流
    for(auto f : m_files) f->close();
    m_files.clear();
}
