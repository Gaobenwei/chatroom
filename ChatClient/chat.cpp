#include "chat.h"
#include "ui_chat.h"

Chat::Chat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Chat)
{
    ui->setupUi(this);
    //ui设置
    setFixedSize(width(),height());//窗口大小
    //控件使能
    ui->sendBtn->setEnabled(false);
    ui->fileBtn->setEnabled(false);
    ui->refreshBtn->setEnabled(false);
    ui->fileList->setEnabled(false);

    //ui控件槽函数
    connect(ui->connBtn,&QPushButton::clicked,this,&Chat::connectServer);
    connect(ui->sendBtn,&QPushButton::clicked,this,&Chat::sendBtnClicked);
    connect(ui->fileBtn,&QPushButton::clicked,this,&Chat::sendFileClicked);
    connect(ui->refreshBtn,&QPushButton::clicked,this,&Chat::refreshBtnClicked);
    connect(ui->fileList,&QListWidget::itemDoubleClicked,this,&Chat::listItemDoubelClicked);

    //套接字相关初始化
    m_client = new QTcpSocket(this);
    connect(m_client,&QTcpSocket::connected,this,&Chat::clientConnected);
    connect(m_client,&QTcpSocket::errorOccurred,this,&Chat::clientError);
    connect(m_client,&QTcpSocket::readyRead,this,&Chat::clientRead);

    //读写线程相关初始化
    m_readThread = new ReadThread(this);

    //线程槽
    connect(this,&Chat::readData,m_readThread,&ReadThread::readData);//读取数据包
    connect(this,&Chat::readFilePath,m_readThread,&ReadThread::readFilePath);//读取文件路径
    connect(m_readThread,&ReadThread::showInfoDlg,this,&Chat::showInfoDlg);
    connect(m_readThread,&ReadThread::showMsg,this,&Chat::showMsg);
    connect(m_readThread,&ReadThread::clearFileList,this,&Chat::clearFileList);
    connect(m_readThread,&ReadThread::addItem,this,&Chat::addItem);
    connect(m_readThread,&ReadThread::writeData,this,&Chat::writeData);
    connect(m_readThread,&ReadThread::enableFileList,this,&Chat::enableFileList);

    //ip默认值
    //"1.13.159.149"
    //"192.168.234.129"
    ui->ipEdit->setText("192.168.234.129");
    ui->portEdit->setText("6000");
}

Chat::~Chat()
{
    m_client->close();
    delete ui;
}

void Chat::showWindow(QString name)
{
    this->show();
    m_name = name;
    m_readThread->start();//启动读取线程
}

void Chat::connectServer()
{
    if(ui->connBtn->text() == "连接"){
        ui->connBtn->setText("断开");
        QString ipAddress = ui->ipEdit->text();
        qint16 port = ui->portEdit->text().toShort();
        m_client->connectToHost(QHostAddress(ipAddress),port);
    }
    else if(ui->connBtn->text() == "断开"){
        ui->connBtn->setText("连接");
        m_client->disconnectFromHost();
        ui->sendBtn->setEnabled(false);
        ui->fileBtn->setEnabled(false);
        ui->refreshBtn->setEnabled(false);
        ui->fileList->setEnabled(false);
    }
}

void Chat::clientConnected()
{
    ui->sendBtn->setEnabled(true);
    ui->fileBtn->setEnabled(true);
    ui->refreshBtn->setEnabled(true);
    ui->fileList->setEnabled(true);
}
void Chat::clientError()
{
    ui->connBtn->setText("连接");
    ui->sendBtn->setEnabled(false);
    ui->fileBtn->setEnabled(false);
    ui->refreshBtn->setEnabled(false);
    ui->fileList->setEnabled(false);
    QMessageBox::critical(this,"error","连接出错！");
}

void Chat::writeData(PackType type, QByteArray body = NULL)
{
    //根据包类型生成包头
    PackHeader headerInfo = {};
    headerInfo.type = type;

    if(body.isNull())
        headerInfo.bodySize = 0;
    else
        headerInfo.bodySize = body.size();

    QByteArray header = QByteArray::fromRawData(reinterpret_cast<const char*>(&headerInfo),sizeof(headerInfo));

    //组织数据包
    QByteArray packet;
    if(body.isNull())
        packet = header;
    else
        packet = header + body;

    //向服务器发送数据包
    m_client->write(packet);
    m_client->flush();
}

void Chat::showInfoDlg(QString info)
{
    QMessageBox::information(this,"info",info);
}

void Chat::showMsg(QString msg)
{
    ui->msgList->addItem(msg);
}

void Chat::clearFileList()
{
    ui->fileList->clear();
}

void Chat::addItem(QString item)
{
    ui->fileList->addItem(item);
}

void Chat::enableFileList(bool flag)
{
    ui->fileList->setEnabled(flag);
}

void Chat::clientRead()
{
    QByteArray bytes = m_client->readAll();
    m_readThread->readData(bytes);
}

void Chat::sendBtnClicked()
{
    QString text = ui->sendEdit->toPlainText();

    if(text.isEmpty()){
        QMessageBox::warning(this,"warning","发送内容为空！");
        return;
    }
    ui->sendEdit->clear();

    //普通消息包体
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString msg = m_name + "  <" + time + ">\n" + text;
    QByteArray body = msg.toUtf8();
    //发送消息
    writeData(PackType::msg, body);
}

void Chat::sendFileClicked()
{
    //获取发送文件路径
    QString filePath = QFileDialog::getOpenFileName(this,"send file",\
                                 "",\
                                 "*.*;");
    if(filePath.isEmpty())
        QMessageBox::critical(this,"error","dir not found");

    QFileInfo fileInfo(filePath);

    //文件信息包体
    FileInfo bodyInfo = {};
    bodyInfo.size = fileInfo.size();
    memset(bodyInfo.name, 0 ,sizeof(bodyInfo.name));
    QByteArray fileBytes = fileInfo.fileName().toUtf8();
    memcpy(bodyInfo.name, fileBytes, fileBytes.size());
    QByteArray body = QByteArray::fromRawData(reinterpret_cast<const char*>(&bodyInfo),sizeof(bodyInfo));

    //发送文件信息包
    writeData(PackType::fileInfo, body);

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::critical(this,"error","文件打开失败");
        return;
    }
    while(!file.atEnd()){
        //读取文件内容
        QByteArray body = file.read(1000);

        //发送文件包
        writeData(PackType::fileText, body);
    }
    if(file.atEnd()){
        qDebug()<<"传输完成";
    }
    else{
        qDebug()<<"传输失败";
    }
}

void Chat::refreshBtnClicked()
{
    //向服务器询问文件列表信息
    writeData(PackType::fileList);
}

void Chat::listItemDoubelClicked(QListWidgetItem *item)
{
    //获取待下载文件名
    QString fileName = item->text();

    //设置带下载文件路径
    QString filePath = QCoreApplication::applicationDirPath();
    filePath += "/file";
    QDir dir(filePath);
    if(!dir.exists())
        dir.mkdir(filePath);

    filePath += "/" + fileName;
    m_readThread->readFilePath(filePath);

    //请求下载包体
    QByteArray body = fileName.toUtf8();

    writeData(PackType::down, body);
}
