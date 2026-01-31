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
    void handleResult(QString algName, QString fileName, double value);

private:
    QString m_outputDir;
};

Q_DECLARE_METATYPE(ResultCollector);

class ProcessingTask : public QRunnable, public QObject {
    Q_OBJECT
public:
    ProcessingTask(QString imgPath, std::shared_ptr<AlgInterface> alg, QString outPath)
        : m_path(imgPath), m_alg(alg), m_out(outPath) {
        setAutoDelete(true);
    }

    void run() override;

private:
    QString m_path;
    std::shared_ptr<AlgInterface> m_alg;
    QString m_out;

signals:
    void resultReady(QString algName, QString fileName, double value);
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    void ExcuteSelected(const QString& refPath, const QString& dirPath, const QString& outPath);
    QStringList CreateFiles(cv::InputArray img);

private:
    AlgRegistry<QString> reg = AlgRegistry<QString>::instance();
    ResultCollector* m_collector;
};

#endif // TASK_H
