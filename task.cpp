#include "task.h"

#include <QThreadPool>
#include <QFile>

void ProcessingTask::run()
{
    cv::Mat img = cv::imread(m_path.toStdString(), cv::IMREAD_GRAYSCALE);
    if(img.empty()) return;

    double val = 0;
    if(m_alg->expectInput()) {
        val = m_alg->process(img); // 非 GLCM 模式
    } else {
        // GLCM 模式：这里可能需要动态创建，或者 m_alg 内部处理了重新计算逻辑
        val = m_alg->process();
    }

    // 发射信号给结果收集器，而不是直接写文件
    emit resultReady(m_alg->Name(), val);
}

void TaskManager::ExcuteSelected(cv::InputArray img_1, cv::InputArray img_2, QStringList files)
{
    const QVector<QAction*>& choices = reg.names();
    double result;

    for(const auto& choice : choices){
        std::shared_ptr<AlgInterface> task = nullptr;

        if(choice->isChecked()){
            task = reg.get(choice, img_1);
        }else continue;

        if(task){
            task->setAutoDelete(true);

            if(task->expectInput()) result = task->process(img_2);
            else {
                task = reg.get(choice, img_2);
                result = task->process();
            }

            QThreadPool::globalInstance()->start(task.get());
        }
    }
}

QStringList TaskManager::CreateFiles(cv::InputArray img)
{
    const QVector<QAction*>& choices = reg.names();
    QStringList flst;

    for(const auto& choice : choices){
        if(choice->isChecked()){
            std::shared_ptr<AlgInterface> task = reg.get(choice, img);
            QString fileName = task->Name() + ".txt";
            flst.append(fileName);
        }else continue;
    }

    return flst;
}

