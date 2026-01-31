#ifndef TASK_H
#define TASK_H

#include <QRunnable>
#include <QObject>
#include <QAction>

#include "ImgPcAlg.h"

class ResultCollector : public QObject {
    Q_OBJECT
public:
    void setOutputDir(QString path) { m_outputDir = path; }

public slots:
    void handleResult(QString algName, double value);

private:
    QString m_outputDir;
};

class ProcessingTask : public QObject,  public QRunnable {
    Q_OBJECT
public:
    ProcessingTask(QString imgPath, std::shared_ptr<AlgInterface> alg)
        : m_path(imgPath), m_alg(alg) {
        setAutoDelete(true);
    }

    void run() override;

private:
    QString m_path;
    std::shared_ptr<AlgInterface> m_alg;
    // QString m_out;

signals:
    void resultReady(QString algName, double value);
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    TaskManager(ResultCollector* rc) : m_collector(rc){};
    void ExcuteSelected(const QString& refPath, const QString& dirPath);
    QStringList CreateFiles(cv::InputArray img);

private:
    AlgRegistry<QString> reg = AlgRegistry<QString>::instance();
    ResultCollector* m_collector;
};

#endif // TASK_H
