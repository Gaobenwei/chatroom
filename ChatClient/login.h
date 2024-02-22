#ifndef LOGIN_H
#define LOGIN_H

#include "userinfo.h"
#include <QWidget>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();

signals:
    void loginSuccess(QString name);

public slots:
    void signup();
    void login();

private:
    Ui::Widget *ui;
    UserInfo userInfo;

};
#endif // LOGIN_H
