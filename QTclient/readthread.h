#ifndef READTHREAD_H
#define READTHREAD_H

#include "header.h"
#include "downfile.h"

class ReadThread : public QThread
{
    Q_OBJECT
public:
    explicit ReadThread(QObject *parent = nullptr);

private:
    //私有成员变量
    DownFile *m_downThread;//文件下载线程
    QByteArray m_data;//接收到的数据

signals:
    void writeData(PackType type, QByteArray body);
    void showInfoDlg(QString info);//消息提示框
    void showMsg(QString msg);//显示普通消息
    void clearFileList();//清除文件列表
    void addItem(QString item);//向文件列表中添加项
    void enableFileList(bool flag);//文件列表状态

    //file
    void initData(QByteArray data);
    void initFilePath(QString file);
    void initSize(qint64 totalBytes);

public slots:
    void readData(QByteArray data);//获取缓冲区数据
    void readFilePath(QString filePath);//获取文件路径

    void writeOk();

protected:
    void run();//线程功能函数，在创建线程的函数里start启动
};

#endif // READTHREAD_H
