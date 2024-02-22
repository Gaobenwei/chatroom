#ifndef HEADER_H
#define HEADER_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QThread>

/*************************协议定义******************************/
//包协议
//systemInfo:
//0x01:客户端接收包完成
//0x02:服务端接收包完成
//0x03:服务端接收文件完成
//0x04:服务端发送文件完成
//0x05:清空客户端文件列表
enum class PackType{
    msg = 1,//普通消息
    systemInfo = 2,//系统消息
    fileList = 3,//文件列表
    fileInfo = 4,//文件信息
    fileText = 5,//文件内容
    down = 6,//下载请求
};
struct PackHeader {
    PackType type;//包类型
    int32_t bodySize;//包体大小
};

//文件格式
struct FileInfo {
    char name[300];//文件名
    int64_t size;//文件大小
};
/**************************end*********************************/


#endif // HEADER_H
