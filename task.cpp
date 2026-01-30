#include "task.h"

#include <QThreadPool>
#include <QFile>

Task::Task() {}

void Task::run()
{
    QString fileName = "1";
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) return;

    QTextStream out(&file);
    double result;
    // 调用process
    if(interface->expectInput()) result = interface->process(img_2);
    else {
        interface = AlgRegistry<QAction*>::instance().get(choice, img_2);
        result = interface->process();
    }
    // 写入数据
    out << result << "\n";
    out.flush();
}

void TaskManager::ExcuteSelected(cv::InputArray img_1, cv::InputArray img_2, QStringList files)
{
    const QVector<QAction*>& choices = reg.names();
    double result;

    for(const auto& choice : choices){
        std::unique_ptr<AlgInterface> task = nullptr;

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
            std::unique_ptr<AlgInterface> task = reg.get(choice, img);
            QString fileName = task->Name() + ".txt";
            flst.append(fileName);
        }else continue;
    }

    return flst;
}

