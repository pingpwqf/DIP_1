#include "task.h"
#include <QThreadPool>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>

// 辅助函数：处理 OpenCV 在 Windows 下的中文路径读取问题
cv::Mat imread_safe(const QString& path)
{
#ifdef Q_OS_WIN
    // Windows 下使用特殊处理确保中文路径可用
    return cv::imread(path.toLocal8Bit().constData(), cv::IMREAD_GRAYSCALE);
#else
    return cv::imread(path.toUtf8().data(), cv::IMREAD_GRAYSCALE);
#endif
}

void ProcessingTask::run()
{
    if (m_sessionHandle && m_sessionHandle->IsCancelled()) {
        emit resultsSkipped(m_algNames.size());
        emit finished();
        return;
    }

    try {
        cv::Mat fullImg = imread_safe(m_path);
        if (fullImg.empty()) return;

        // --- 核心改动：应用 ROI ---
        cv::Mat img;
        if (m_roi.width > 0 && m_roi.height > 0) {
            img = fullImg(m_roi).clone(); // 只处理 ROI 区域
        } else {
            img = fullImg;
        }
        if (img.empty()) {
            emit resultsSkipped(m_algNames.size());
            emit finished();
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
            if (m_sessionHandle && m_sessionHandle->IsCancelled()) break;

            if(m_sessionHandle->IsCancelled()) return;
            double val = 0;
            if (algName == CORRNAME && sharedGlcm) {
                val = sharedGlcm->getCorrelation();
            }
            else if (algName == HOMONAME && sharedGlcm) {
                val = sharedGlcm->getHomogeneity();
            }
            else {
                auto alg = AlgRegistry<QString>::instance().get(algName, m_refImg);
                if (alg) val = alg->process(img);
            }
            emit resultReady(algName, fileName, val);
        }
    }
    catch (const std::exception& e) {
        QMessageBox::warning(nullptr, "TaskError",
                             tr(e.what()));
        emit resultsSkipped(m_algNames.size());
    }
    catch (...) {
        emit resultsSkipped(m_algNames.size());
    }

    emit finished();
}

ProcessingSession* TaskManager::createSession()
{
    return new ProcessingSession(m_collector);
}

void ProcessingSession::start(const cv::Mat& refImg, const QStringList& files, const QDir& dir, const QVector<QString>& algs)
{
    m_totalTasks = files.size();
    m_activeTasks = m_totalTasks;

    if (m_totalTasks == 0) {
        emit sessionFinished();
        return;
    }

    m_collector->resetExpectedCount(m_totalTasks * algs.size());

    for (const QString& fileName : files) {
        if(m_isCancelled.load()) {
            // 补偿未提交任务的计数，确保 activeTasks 最终能归零
            m_activeTasks--;
            m_collector->decrementExpectedCount(algs.size());
            continue;
        }

        ProcessingTask* task = new ProcessingTask(dir.absoluteFilePath(fileName), algs, refImg);
        task->setSession(this);
        task->setROI(roi4Task);
        connect(task, &ProcessingTask::resultReady, m_collector, &ResultCollector::handleResult);
        // 如果任务内部失败，也要同步计数
        connect(task, &ProcessingTask::resultsSkipped, m_collector, &ResultCollector::decrementExpectedCount);
        connect(task, &ProcessingTask::finished, this, &ProcessingSession::onTaskFinished);
        QThreadPool::globalInstance()->start(task);
    }
}

void ProcessingSession::onTaskFinished()
{
    m_activeTasks--;
    emit progressUpdated(m_totalTasks - m_activeTasks, m_totalTasks);

    if (m_activeTasks <= 0) emit sessionFinished();
}

void ProcessingSession::cancel()
{
    m_isCancelled = true;
    if (m_collector) {
        m_collector->abort(); // 立即强行释放文件句柄
    }
}

/*****************
 *ResultCollector*
 *****************/
void ResultCollector::setOutputDir(QString path)
{
    QMutexLocker locker(&m_mutex);
    m_outputDir = path;
}

void ResultCollector::prepare()
{
    closeAll();
    if (!m_outputDir.isEmpty()) {
        QDir dir;
        if (!dir.exists(m_outputDir)) {
            // if (dir.mkpath(m_outputDir)) {
            //     qDebug() << "Created output directory:" << m_outputDir;
            // } else {
            //     qDebug() << "Critical: Could not create output directory!";
            // }
        }
    }
}

void ResultCollector::closeAll()
{
    QMutexLocker locker(&m_mutex);

    m_streams.clear();

    for (auto f : m_files) {
        if (f && f->isOpen()) {
            f->flush(); // 显式刷盘
            f->close();
        }
    }
    m_files.clear();
    // QCoreApplication::processEvents();
}

void ResultCollector::abort()
{
    QMutexLocker locker(&m_mutex);
    m_isAborted = true;
    m_expectedResults = 0; // 清空预期，防止后续 handleResult 继续工作
    closeAll();            // 立即关闭所有文件句柄
}

void ResultCollector::resetExpectedCount(int count)
{
    QMutexLocker locker(&m_mutex);
    m_expectedResults = count;
}

void ResultCollector::decrementExpectedCount(int count)
{
    QMutexLocker locker(&m_mutex);
    m_expectedResults -= count;
    if (m_expectedResults <= 0) {
        // 虽然在子线程，但因为是 DirectConnection 或简单计数，我们最好通过信号去通知
        QMetaObject::invokeMethod(this, "allResultsSaved", Qt::QueuedConnection);
    }
}

void ResultCollector::handleResult(QString algName, QString fileName, double value)
{
    QMutexLocker locker(&m_mutex);

    if (m_isAborted) {
        m_expectedResults--;
        return;
    }

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
            m_expectedResults--;
            auto info = "Failed to open output file:" + fullPath;
            QMessageBox::warning(nullptr, "fail",
                                 tr(info.toLatin1()));
            return;
        }
    }

    if (m_streams.contains(algName)) {
        *m_streams[algName] << fileName << "," << QString::number(value, 'f', 6) << "\n";
        // m_streams[algName]->flush(); // 强制刷盘，防止崩溃丢失数据
    }

    m_expectedResults--;
    if (m_expectedResults <= 0) {
        emit allResultsSaved();
    }
}
