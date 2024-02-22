/********************************************************************************
** Form generated from reading UI file 'login.ui'
**
** Created by: Qt User Interface Compiler version 6.4.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGIN_H
#define UI_LOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QWidget *widget;
    QFrame *line;
    QLabel *titleLb;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *accoutLb;
    QLabel *passwordLb;
    QLabel *nameLb;
    QLineEdit *passwordEdit;
    QLineEdit *nameEdit;
    QLineEdit *accoutEdit;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QPushButton *signupBtn;
    QPushButton *loginBtn;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName("Widget");
        Widget->resize(400, 200);
        widget = new QWidget(Widget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(0, 0, 400, 200));
        line = new QFrame(widget);
        line->setObjectName("line");
        line->setGeometry(QRect(50, 45, 300, 10));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        titleLb = new QLabel(widget);
        titleLb->setObjectName("titleLb");
        titleLb->setGeometry(QRect(150, 10, 100, 35));
        titleLb->setStyleSheet(QString::fromUtf8("font: 20pt \"\351\273\221\344\275\223\";"));
        titleLb->setAlignment(Qt::AlignCenter);
        gridLayoutWidget = new QWidget(widget);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(50, 49, 301, 91));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        accoutLb = new QLabel(gridLayoutWidget);
        accoutLb->setObjectName("accoutLb");
        accoutLb->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));
        accoutLb->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(accoutLb, 0, 0, 1, 1);

        passwordLb = new QLabel(gridLayoutWidget);
        passwordLb->setObjectName("passwordLb");
        passwordLb->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));
        passwordLb->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(passwordLb, 1, 0, 1, 1);

        nameLb = new QLabel(gridLayoutWidget);
        nameLb->setObjectName("nameLb");
        nameLb->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));
        nameLb->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(nameLb, 2, 0, 1, 1);

        passwordEdit = new QLineEdit(gridLayoutWidget);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));

        gridLayout->addWidget(passwordEdit, 1, 1, 1, 3);

        nameEdit = new QLineEdit(gridLayoutWidget);
        nameEdit->setObjectName("nameEdit");
        nameEdit->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));

        gridLayout->addWidget(nameEdit, 2, 1, 1, 3);

        accoutEdit = new QLineEdit(gridLayoutWidget);
        accoutEdit->setObjectName("accoutEdit");
        accoutEdit->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));

        gridLayout->addWidget(accoutEdit, 0, 1, 1, 3);

        gridLayoutWidget_2 = new QWidget(widget);
        gridLayoutWidget_2->setObjectName("gridLayoutWidget_2");
        gridLayoutWidget_2->setGeometry(QRect(180, 150, 171, 31));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        signupBtn = new QPushButton(gridLayoutWidget_2);
        signupBtn->setObjectName("signupBtn");
        signupBtn->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));

        gridLayout_2->addWidget(signupBtn, 0, 0, 1, 1);

        loginBtn = new QPushButton(gridLayoutWidget_2);
        loginBtn->setObjectName("loginBtn");
        loginBtn->setStyleSheet(QString::fromUtf8("font: 12pt \"\351\273\221\344\275\223\";"));

        gridLayout_2->addWidget(loginBtn, 0, 1, 1, 1);

        QWidget::setTabOrder(accoutEdit, passwordEdit);
        QWidget::setTabOrder(passwordEdit, nameEdit);
        QWidget::setTabOrder(nameEdit, signupBtn);
        QWidget::setTabOrder(signupBtn, loginBtn);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Login", nullptr));
        titleLb->setText(QCoreApplication::translate("Widget", "\347\231\273\345\275\225", nullptr));
        accoutLb->setText(QCoreApplication::translate("Widget", "\350\264\246\345\217\267\357\274\232", nullptr));
        passwordLb->setText(QCoreApplication::translate("Widget", "\345\257\206\347\240\201\357\274\232", nullptr));
        nameLb->setText(QCoreApplication::translate("Widget", "\347\224\250\346\210\267\345\220\215\357\274\232", nullptr));
        signupBtn->setText(QCoreApplication::translate("Widget", "\346\263\250\345\206\214", nullptr));
        loginBtn->setText(QCoreApplication::translate("Widget", "\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGIN_H
