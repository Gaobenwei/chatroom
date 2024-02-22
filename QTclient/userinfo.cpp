#include "userinfo.h"

UserInfo::UserInfo()
{
    this->name={};
    this->account="";
    this->password="";
}

//-1:保存失败  0:成功  1:账号重复
int UserInfo::saveInfo(UserInfo& info)
{
    QString filePath=QCoreApplication::applicationDirPath(); //返回包含应用程序可执行文件的目录。
    filePath+="/userInfo.json";
    //创建错误信息收集的对象
    QJsonParseError errr;
    //打开文件描述符
    QFile file(filePath);
    if(!file.exists())
    {
        //文件不存在
        if(!file.open(QIODevice::WriteOnly))
        {
            //创建文件
            return-2;
        }
        file.close();
    }
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        return -1;
    }
    //读取JSON数据
    QJsonDocument doc=QJsonDocument::fromJson(file.readAll(),&errr);
    QJsonArray arr=doc.array();
    for(int i=0;i<arr.size();i++)
    {
        QString account=arr.at(i).toObject().value("account").toString();
        if(account==info.account)
        {
            return 1;
        }
    }
    file.close();

    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
        return -1;
    }

    QJsonObject obj;
    obj.insert("name",info.name);
    obj.insert("account",info.account);
    obj.insert("password",info.password);
    arr.append(obj);
    doc.setArray(arr);
    file.write(doc.toJson());
    file.close();
    return 0;
}

//-1:查询失败  0:成功  1:账号失败  2:密码失败
int UserInfo::seekUser(UserInfo &info)
{
    QString fileName=QCoreApplication::applicationDirPath();
    fileName+="/userInfo.json";
    QFile file(fileName);
    //创建错误信息收集的对象
    QJsonParseError errr;

    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        return -1;
    }
    QJsonDocument doc=QJsonDocument::fromJson(file.readAll(),&errr);
    QJsonArray arr=doc.array();
    for(int i=0;i<arr.size();i++)
    {
        QString account=arr.at(i).toObject().value("account").toString();
        if(account==info.account)
        {
            QString password=arr.at(i).toObject().value("password").toString();
            if(password == info.password)
            {
                info.name = arr.at(i).toObject().value("name").toString();
                return 0;
            }
            else{
                return 2;
            }
        }
    }
    file.close();
    return 1;
}
