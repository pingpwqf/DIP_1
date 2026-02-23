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
    return cv::imread(path.toUtf8().data(), cv::IMREAD_GRAYSCALE);
#endif
}

void ProcessingTask::run() {
    try {
        cv::Mat img = imread_safe(m_path);
        if (img.empty()) {
            qDebug() << "Image is empty:" << m_path;
            return;
        }

        QString fileName = QFileInfo(m_path).fileName();

        // GLCM 缓存逻辑
        std::shared_ptr<GLCM::GLCmat> sharedGlcm = nullptr;
        bool needsGlcm = m_algNames.contains(CORRNAME) || m_algNames.contains(HOMONAME);

        if (needsGlcm) {
            sharedGlcm = GLCM::getPSGLCM(img, 32, 1, 0, ScaleStrategy::ToPowerOfTwo);
        }

        for (const QString& algName : m_algNames) {
            double val = 0;
            if (algName == CORRNAME && sharedGlcm) {
                val = sharedGlcm->getCorrelation();
            }
            else if (algName == HOMONAME && sharedGlcm) {
                val = sharedGlcm->getHomogeneity();
            }
            else {
                auto alg = AlgRegistry<QString>::instance().get(algName, m_refImg);
                if (alg) {
                    val = alg->process(img);
                }
            }
            emit resultReady(algName, fileName, val);
        }
    }
    catch (const cv::Exception& e) {
        qDebug() << "OpenCV Exception in thread:" << e.what() << "File:" << m_path;
        emit errorOccurred(QString("OpenCV Error: %1").arg(e.what()));
    }
    catch (const std::exception& e) {
        qDebug() << "Standard Exception in thread:" << e.what() << "File:" << m_path;
        emit errorOccurred(QString("Error: %1").arg(e.what()));
    }
    catch (...) {
        qDebug() << "Unknown Exception in thread. File:" << m_path;
        emit errorOccurred("Unknown Error occurred during processing.");
    }
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

    for (const QString& fileName : files) {
        QString fullPath = dir.absoluteFilePath(fileName);

        // 为每一张图创建一个任务，包含所有选中的算法
        ProcessingTask* task = new ProcessingTask(fullPath, selectedAlgs, refImg);
        connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);

        QThreadPool::globalInstance()->start(task);
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
