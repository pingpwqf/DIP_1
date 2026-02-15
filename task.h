#ifndef TASK_H
#define TASK_H

#include <QRunnable>
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QSharedPointer>
#include "ImgPcAlg.h"

// 结果收集器：负责将不同线程产生的数据分类写入文件
class ResultCollector : public QObject {
    Q_OBJECT
public:
    explicit ResultCollector(QObject* parent = nullptr) : QObject(parent) {}
    ~ResultCollector() { closeAll(); }

    void setOutputDir(QString path) { m_outputDir = path; }
    void prepare(); // 准备工作：检查并创建目录
    void closeAll();

public slots:
    // 增加 fileName 参数，让结果知道对应哪张图
    void handleResult(QString algName, QString fileName, double value);

private:
    QString m_outputDir;
    QMap<QString, QSharedPointer<QFile>> m_files;
    QMap<QString, QSharedPointer<QTextStream>> m_streams;
};

// 具体的处理任务
class ProcessingTask : public QObject, public QRunnable {
    Q_OBJECT
public:
    // 传递算法名称和参考图，而不是直接传递算法实例，以保证线程安全
    ProcessingTask(QString imgPath, QString algName, cv::Mat refImg)
        : m_path(imgPath), m_algName(algName), m_refImg(refImg) {
        setAutoDelete(true);
    }

    void run() override;

private:
    QString m_path;
    QString m_algName;
    cv::Mat m_refImg;

signals:
    void resultReady(QString algName, QString fileName, double value);
    void errorOccurred(QString msg);
};

// 任务管理器
class TaskManager : public QObject
{
    Q_OBJECT
public:
    TaskManager(ResultCollector* rc) : m_collector(rc) {}
    void ExecuteSelected(const QString& refPath, const QString& dirPath, QVector<QString> selectedAlgs);

private:
    ResultCollector* m_collector;
};

#endif // TASK_H
