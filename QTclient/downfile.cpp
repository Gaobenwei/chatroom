#include "downfile.h"
#include<QDataStream>
#include<QFile>

DownFile::DownFile(QObject *parent)
    : QThread{parent}
{

}

void DownFile::initData(QByteArray data)
{
    //向vector中添加数据
    m_data.emplace_back(data);
}


void DownFile::initFilePath(QString file)
{
    //获取文件路径
    m_filePath=file;
}

void DownFile::initSize(qint64 totalBytes)
{
    //获取文件大小
    m_totalBytes=totalBytes;
    m_writeBytes=0;
}

void DownFile::run() //实现工能
{
    //打开文件
    QFile file(m_filePath);
    if(file.exists())
    {
        //如果同名文件存在，则移除它
        file.remove();
    }
    if(!file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        return;
    }
    //写入文件内容
    while(m_writeBytes<m_totalBytes)
    {
        //缓存的数据写完了，还没有达到目标数量，继续循环等待
        if(m_data.size()<=0)
        {
            continue;
        }
        /*移除vector中的第一项并返回它。这个函数假定向量不为空。
         * 为避免失败，请在调用此函数之前调用isEmpty()。*/
        QByteArray data=m_data.takeFirst();
        file.write(data);
        m_writeBytes+=data.size();
    }
    //写入完成
    file.close();
    writeOK();
    quit();
}
