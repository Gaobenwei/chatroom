#ifndef CHAT_H
#define CHAT_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QThread>
#include <QtEndian>
#include <qplatformdefs.h>
#include <QListWidget>
#include "readthread.h"

namespace Ui {
class Chat;
}

class Chat : public QWidget
{
    Q_OBJECT
private:
    //私有成员变量
    QTcpSocket *m_client;//客户端通信套接字，服务端可能会有监听套接字QTcpServer;
    QString m_name;//用户名
    ReadThread *m_readThread;//读取线程

public:
    explicit Chat(QWidget *parent = nullptr);
    ~Chat();

signals:
    void readData(QByteArray data);
    void readFilePath(QString filePath);

public slots:

    void writeData(PackType type, QByteArray body);
    //子线程槽
    void showInfoDlg(QString info);
    void showMsg(QString msg);
    void clearFileList();
    void addItem(QString item);
    void enableFileList(bool flag);

    //主线程槽
    void showWindow(QString name);//显示窗口
    void connectServer();//连接服务器
    void clientConnected();//服务器已连接
    void clientError();//连接服务器出错
    void clientRead();//读取输入缓冲区
    void sendBtnClicked();
    void sendFileClicked();
    void refreshBtnClicked();
    void listItemDoubelClicked(QListWidgetItem *item);

private:
    Ui::Chat *ui;

protected:

};



#endif // CHAT_H
