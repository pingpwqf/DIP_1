#ifndef TASK_H
#define TASK_H

#include <QRunnable>
#include <QObject>
#include <QAction>

#include "ImgPcAlg.h"

class ProcessingTask : public QRunnable {
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
};

class TaskManager : public QObject
{
    Q_OBJECT
public:
    void ExcuteSelected(cv::InputArray img_1, cv::InputArray img_2, QStringList files);
    QStringList CreateFiles(cv::InputArray img);

private:
    AlgRegistry<QAction*> reg = AlgRegistry<QAction*>::instance();
};

#endif // TASK_H
