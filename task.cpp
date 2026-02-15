#include "task.h"
#include <QThreadPool>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

// 辅助函数：处理 OpenCV 在 Windows 下的中文路径读取问题
cv::Mat imread_safe(const QString& path) {
#ifdef Q_OS_WIN
    // Windows 下使用特殊处理确保中文路径可用
    return cv::imread(path.toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);
#else
    return cv::imread(path.toLatin1().data(), cv::IMREAD_GRAYSCALE);
#endif
}

void ProcessingTask::run() {
    cv::Mat img = imread_safe(m_path);
    if (img.empty()) {
        qDebug() << "Failed to read image:" << m_path;
        return;
    }else qDebug() << "get image!";

    // 在子线程中获取属于自己的算法实例
    auto alg = AlgRegistry<QString>::instance().get(m_algName, m_refImg);
    if (!alg) return;
    else qDebug() << "get alg!";

    double val = 0;
    // try {
    if (alg->expectInput()) {
        val = alg->process(img);
    } else {
        // 对于 GLCM 等不直接接受输入的算法，需要针对当前图创建实例
        auto currentAlg = AlgRegistry<QString>::instance().get(m_algName, img);
        if (currentAlg) val = currentAlg->process();
    }
    qDebug() << "getval:";
    emit resultReady(m_algName, QFileInfo(m_path).fileName(), val);
    // } catch (const std::exception& e) {
    //     qDebug() << "Algorithm error:" << e.what();
    // }
}

void TaskManager::ExecuteSelected(const QString& refPath, const QString& dirPath, QVector<QString> selectedAlgs) {
    if (refPath.isEmpty() || dirPath.isEmpty()) {
        qDebug() << "Source or reference path is empty!";
        return;
    }

    if (selectedAlgs.empty()){
        qDebug() << "empty choice!";
        return;
    }

    QDir dir(dirPath);
    QStringList files = dir.entryList({"*.bmp", "*.png", "*.jpg"}, QDir::Files);
    if (files.isEmpty()) {
        qDebug() << "No valid images found in" << dirPath;
        return;
    }

    cv::Mat refImg = imread_safe(refPath);
    if (refImg.empty()) {
        qDebug() << "Reference image load failed:" << refPath;
        return;
    }

    // QVector<QString> selectedAlgs = AlgRegistry<QString>::instance().names();

    for (QString algName : selectedAlgs) {
        qDebug() << algName;
        for (QString fileName : files) {
            QString fullPath = dir.absoluteFilePath(fileName);
            ProcessingTask* task = new ProcessingTask(fullPath, algName, refImg);

            connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);
            // qDebug() << "ready to allocate a thread";
            QThreadPool::globalInstance()->start(task);
            // qDebug() << "run succeed";
        }
    }
}

void ResultCollector::prepare() {
    closeAll();
    if (!m_outputDir.isEmpty()) {
        QDir dir;
        if (!dir.exists(m_outputDir)) {
            if (dir.mkpath(m_outputDir)) {
                qDebug() << "Created output directory:" << m_outputDir;
            } else {
                qDebug() << "Critical: Could not create output directory!";
            }
        }
    }
}

void ResultCollector::handleResult(QString algName, QString fileName, double value) {
    if (!m_streams.contains(algName)) {
        QString fullPath = m_outputDir + "/" + algName + ".csv"; // 建议用 csv 方便表格打开
        auto file = QSharedPointer<QFile>::create(fullPath);

        // 使用 Append 模式，并在文件开头写入表头
        bool isNew = !file->exists();
        if (file->open(QIODevice::Append | QIODevice::Text)) {
            m_files[algName] = file;
            auto stream = QSharedPointer<QTextStream>::create(file.data());
            if (isNew) *stream << "FileName,Value\n";
            m_streams[algName] = stream;
        } else {
            qDebug() << "Failed to open output file:" << fullPath;
            return;
        }
    }

    if (m_streams.contains(algName)) {
        *m_streams[algName] << fileName << "," << QString::number(value, 'f', 6) << "\n";
        m_streams[algName]->flush(); // 强制刷盘，防止崩溃丢失数据
    }
}

void ResultCollector::closeAll() {
    m_streams.clear();
    for (auto f : m_files) {
        if (f->isOpen()) f->close();
    }
    m_files.clear();
}
