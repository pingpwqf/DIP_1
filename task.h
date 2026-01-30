#ifndef TASK_H
#define TASK_H

#include <QRunnable>
#include <QObject>
#include <QAction>

#include "ImgPcAlg.h"

class Task : public QRunnable
{
public:
    Task();
    void run() override;
private:
    std::unique_ptr<AlgInterface> interface;
    cv::UMat img_1;
    cv::UMat img_2;
    QAction* choice;
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
