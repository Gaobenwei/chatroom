#ifndef WIDGET_H
#define WIDGET_H

#include "userinfo.h"
#include <QWidget>
#include <QMessageBox>
#include"chat.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

signals:
    void loginSuccess(QString name);

public slots:
    void signup();
    void login();

private:
    Ui::Widget *ui;
    UserInfo userInfo;
};
#endif // WIDGET_H
