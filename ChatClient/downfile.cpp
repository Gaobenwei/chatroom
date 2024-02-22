#include "downfile.h"

#include <QDataStream>
#include <QFile>

DownFile::DownFile(QWidget *parent)
{

}

void DownFile::initData(QByteArray data)
{
    //向vector中添加数据
    m_data.emplace_back(data);
}

void DownFile::initFilePath(QString file)
{
    //获取保存路径
    m_filePath = file;
}

void DownFile::initSize(qint64 totalBytes)
{
    //获取文件大小
    m_totalBytes = totalBytes;
    m_writedBytes = 0;
}

void DownFile::run()
{
    //打开文件
    QFile file(m_filePath);
    if(file.exists()){
        file.remove();
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)){
        return;
    }

    //写入文件
    while(m_writedBytes < m_totalBytes){
        if(m_data.size()<=0)
            continue;

        QByteArray data = m_data.takeFirst();
        file.write(data);
        //统计已写入大小
        m_writedBytes += data.size();
    }
    //写入完成
    file.close();
    writeOk();
}
