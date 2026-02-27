/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNIPC;
    QAction *actionZNCC;
    QAction *actionCorrelation;
    QAction *actionHomogeneity;
    QAction *actionMSV;
    QAction *actionROI;
    QWidget *centralwidget;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_4;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pushButton_3;
    QLineEdit *dirLineEdit;
    QGraphicsView *graphicsView;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QLineEdit *fileLineEdit;
    QPushButton *pushButton_5;
    QLineEdit *dirLineEdit_2;
    QLabel *label_4;
    QMenuBar *menubar;
    QMenu *menuselect;
    QMenu *menuGLCM;
    QMenu *menupre;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(518, 275);
        actionNIPC = new QAction(MainWindow);
        actionNIPC->setObjectName("actionNIPC");
        actionNIPC->setCheckable(true);
        actionZNCC = new QAction(MainWindow);
        actionZNCC->setObjectName("actionZNCC");
        actionZNCC->setCheckable(true);
        actionCorrelation = new QAction(MainWindow);
        actionCorrelation->setObjectName("actionCorrelation");
        actionCorrelation->setCheckable(true);
        actionHomogeneity = new QAction(MainWindow);
        actionHomogeneity->setObjectName("actionHomogeneity");
        actionHomogeneity->setCheckable(true);
        actionMSV = new QAction(MainWindow);
        actionMSV->setObjectName("actionMSV");
        actionMSV->setCheckable(true);
        actionROI = new QAction(MainWindow);
        actionROI->setObjectName("actionROI");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        horizontalLayoutWidget_2 = new QWidget(centralwidget);
        horizontalLayoutWidget_2->setObjectName("horizontalLayoutWidget_2");
        horizontalLayoutWidget_2->setGeometry(QRect(30, 200, 461, 22));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        pushButton_4 = new QPushButton(horizontalLayoutWidget_2);
        pushButton_4->setObjectName("pushButton_4");

        horizontalLayout_2->addWidget(pushButton_4);

        horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        pushButton_3 = new QPushButton(horizontalLayoutWidget_2);
        pushButton_3->setObjectName("pushButton_3");

        horizontalLayout_2->addWidget(pushButton_3);

        dirLineEdit = new QLineEdit(centralwidget);
        dirLineEdit->setObjectName("dirLineEdit");
        dirLineEdit->setGeometry(QRect(210, 90, 251, 19));
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName("graphicsView");
        graphicsView->setGeometry(QRect(40, 30, 141, 141));
        graphicsView->setInteractive(false);
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(210, 10, 81, 16));
        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(210, 70, 71, 16));
        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(40, 10, 71, 16));
        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(470, 30, 31, 21));
        QFont font;
        font.setPointSize(14);
        pushButton->setFont(font);
        pushButton_2 = new QPushButton(centralwidget);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(470, 90, 31, 21));
        pushButton_2->setFont(font);
        fileLineEdit = new QLineEdit(centralwidget);
        fileLineEdit->setObjectName("fileLineEdit");
        fileLineEdit->setGeometry(QRect(210, 30, 251, 19));
        pushButton_5 = new QPushButton(centralwidget);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(470, 150, 31, 21));
        pushButton_5->setFont(font);
        dirLineEdit_2 = new QLineEdit(centralwidget);
        dirLineEdit_2->setObjectName("dirLineEdit_2");
        dirLineEdit_2->setGeometry(QRect(210, 150, 251, 19));
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(210, 130, 71, 16));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 518, 18));
        menuselect = new QMenu(menubar);
        menuselect->setObjectName("menuselect");
        menuGLCM = new QMenu(menuselect);
        menuGLCM->setObjectName("menuGLCM");
        menupre = new QMenu(menubar);
        menupre->setObjectName("menupre");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menupre->menuAction());
        menubar->addAction(menuselect->menuAction());
        menuselect->addAction(actionNIPC);
        menuselect->addAction(actionZNCC);
        menuselect->addSeparator();
        menuselect->addAction(menuGLCM->menuAction());
        menuselect->addSeparator();
        menuselect->addAction(actionMSV);
        menuGLCM->addAction(actionCorrelation);
        menuGLCM->addAction(actionHomogeneity);
        menupre->addAction(actionROI);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "\346\225\243\346\226\221\345\233\276\345\203\217\345\244\204\347\220\206\350\275\257\344\273\266", nullptr));
        actionNIPC->setText(QCoreApplication::translate("MainWindow", "NIPC", nullptr));
        actionZNCC->setText(QCoreApplication::translate("MainWindow", "ZNCC", nullptr));
        actionCorrelation->setText(QCoreApplication::translate("MainWindow", "correlation", nullptr));
        actionHomogeneity->setText(QCoreApplication::translate("MainWindow", "homogeneity", nullptr));
        actionMSV->setText(QCoreApplication::translate("MainWindow", "MSV", nullptr));
        actionROI->setText(QCoreApplication::translate("MainWindow", "ROI", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "\347\241\256\350\256\244", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "\345\217\226\346\266\210", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\217\202\350\200\203\345\233\276\345\203\217", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\233\276\345\203\217\346\226\207\344\273\266\345\244\271", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "ROI\350\214\203\345\233\264\346\230\276\347\244\272\357\274\232", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\350\276\223\345\207\272\346\226\207\344\273\266\345\244\271", nullptr));
        menuselect->setTitle(QCoreApplication::translate("MainWindow", "\347\256\227\346\263\225\351\200\211\346\213\251", nullptr));
        menuGLCM->setTitle(QCoreApplication::translate("MainWindow", "GLCM", nullptr));
        menupre->setTitle(QCoreApplication::translate("MainWindow", "\351\242\204\345\244\204\347\220\206", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
