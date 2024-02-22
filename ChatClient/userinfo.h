#ifndef USERINFO_H
#define USERINFO_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <QCoreApplication>


class UserInfo
{
public:
    QString name;//用户名
    QString account;//账号
    QString password;//密码

public:
    UserInfo();

public:
    int saveInfo(UserInfo& info);//保存用户信息
    int seekUser(UserInfo& info);//查找信息

};

#endif // USERINFO_H
