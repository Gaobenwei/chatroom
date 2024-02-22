/********************************************************************************
** Form generated from reading UI file 'chat.ui'
**
** Created by: Qt User Interface Compiler version 6.4.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Chat
{
public:
    QWidget *widget;
    QPushButton *refreshBtn;
    QPushButton *fileBtn;
    QPushButton *sendBtn;
    QTextEdit *sendEdit;
    QListWidget *fileList;
    QListWidget *msgList;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLineEdit *porEdit;
    QLabel *portLb;
    QLabel *ipLb;
    QLineEdit *ipEdit;
    QPushButton *coonBtn;

    void setupUi(QWidget *Chat)
    {
        if (Chat->objectName().isEmpty())
            Chat->setObjectName("Chat");
        Chat->resize(700, 500);
        widget = new QWidget(Chat);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(0, 0, 700, 500));
        refreshBtn = new QPushButton(widget);
        refreshBtn->setObjectName("refreshBtn");
        refreshBtn->setGeometry(QRect(10, 460, 100, 30));
        fileBtn = new QPushButton(widget);
        fileBtn->setObjectName("fileBtn");
        fileBtn->setGeometry(QRect(220, 460, 100, 30));
        sendBtn = new QPushButton(widget);
        sendBtn->setObjectName("sendBtn");
        sendBtn->setGeometry(QRect(350, 460, 100, 30));
        sendEdit = new QTextEdit(widget);
        sendEdit->setObjectName("sendEdit");
        sendEdit->setGeometry(QRect(0, 359, 500, 91));
        fileList = new QListWidget(widget);
        fileList->setObjectName("fileList");
        fileList->setGeometry(QRect(500, 160, 200, 341));
        msgList = new QListWidget(widget);
        msgList->setObjectName("msgList");
        msgList->setGeometry(QRect(0, 9, 500, 351));
        gridLayoutWidget = new QWidget(widget);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(510, 10, 181, 141));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        porEdit = new QLineEdit(gridLayoutWidget);
        porEdit->setObjectName("porEdit");

        gridLayout->addWidget(porEdit, 1, 1, 1, 1);

        portLb = new QLabel(gridLayoutWidget);
        portLb->setObjectName("portLb");

        gridLayout->addWidget(portLb, 1, 0, 1, 1);

        ipLb = new QLabel(gridLayoutWidget);
        ipLb->setObjectName("ipLb");

        gridLayout->addWidget(ipLb, 0, 0, 1, 1);

        ipEdit = new QLineEdit(gridLayoutWidget);
        ipEdit->setObjectName("ipEdit");

        gridLayout->addWidget(ipEdit, 0, 1, 1, 1);

        coonBtn = new QPushButton(gridLayoutWidget);
        coonBtn->setObjectName("coonBtn");

        gridLayout->addWidget(coonBtn, 2, 0, 1, 2);


        retranslateUi(Chat);

        QMetaObject::connectSlotsByName(Chat);
    } // setupUi

    void retranslateUi(QWidget *Chat)
    {
        Chat->setWindowTitle(QCoreApplication::translate("Chat", "\350\201\212\345\244\251\345\256\244", nullptr));
        refreshBtn->setText(QCoreApplication::translate("Chat", "\345\210\267\346\226\260\345\210\227\350\241\250", nullptr));
        fileBtn->setText(QCoreApplication::translate("Chat", "\346\226\207\344\273\266\345\217\221\351\200\201", nullptr));
        sendBtn->setText(QCoreApplication::translate("Chat", "\345\217\221\351\200\201", nullptr));
        portLb->setText(QCoreApplication::translate("Chat", "PORT:", nullptr));
        ipLb->setText(QCoreApplication::translate("Chat", "IP:", nullptr));
        coonBtn->setText(QCoreApplication::translate("Chat", "\350\277\236\346\216\245", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Chat: public Ui_Chat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHAT_H
