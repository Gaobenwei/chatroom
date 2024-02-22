#include "readthread.h"

ReadThread::ReadThread(QWidget *parent)
{
    m_data.clear();

    //File thread
    m_downThread = new DownFile();
    connect(m_downThread,&DownFile::writeOk,this,&ReadThread::writeOk);
    connect(this,&ReadThread::initData,m_downThread,&DownFile::initData);
    connect(this,&ReadThread::initFilePath,m_downThread,&DownFile::initFilePath);
    connect(this,&ReadThread::initSize,m_downThread,&DownFile::initSize);
}

void ReadThread::readData(QByteArray data)
{
    m_data += data;
}

void ReadThread::readFilePath(QString filePath)
{
    initFilePath(filePath);
}

void ReadThread::writeOk()
{
    showInfoDlg("文件下载完成！");
    enableFileList(true);
}

void ReadThread::run()
{
    while(1){
        //等待输入缓冲区有一个包头的大小
        while(m_data.size() < sizeof(PackHeader))
            usleep(1000);

        //获取一个包头
        QByteArray header = m_data.left(sizeof(PackHeader));
        m_data.remove(0,sizeof(PackHeader));
        PackHeader* pheader = (PackHeader*)header.data();

        //等待输入缓冲区有一个包体的大小
        while(m_data.size() < pheader->bodySize)
            usleep(1000);

        //获取包体
        QByteArray msg = m_data.left(pheader->bodySize);
        m_data.remove(0,pheader->bodySize);

        //处理包
        if(pheader->type == PackType::msg){
            //显示普通消息
            showMsg(msg);
        }
        else if(pheader->type == PackType::systemInfo){
            char info = msg[0];
            //0x01:客户端接收包完成
            //0x02:服务端接收包完成
            //0x03:服务端接收文件完成
            //0x04:服务端发送文件完成
            //0x05:清空客户端文件列表
            if(info == 0x01){

            }
            else if(info == 0x02){
            }
            else if(info == 0x03){
                showInfoDlg("文件上传完成！");
            }
            else if(info == 0x04){

            }
            else if(info == 0x05){
                clearFileList();
            }
        }
        else if(pheader->type == PackType::fileList){
            addItem(msg);
        }
        else if(pheader->type == PackType::fileInfo){
            //获取文件信息
            FileInfo *pFileInfo = (FileInfo*)msg.data();
            qint64 totalRBytes = pFileInfo->size;
            initSize(totalRBytes);

            enableFileList(false);
            m_downThread->start();
        }
        else if(pheader->type == PackType::fileText){
            //将接收文件内容写入文件
            initData(msg);

            //向服务器发送包接收完成消息
            QByteArray body;
            QDataStream stream(&body,QIODevice::WriteOnly);
            char info = 0x01;
            stream << info;
            writeData(PackType::systemInfo, body);
        }
    }
}


