#ifndef DOWNFILE_H
#define DOWNFILE_H

#include <QWidget>
#include <QThread>

class DownFile : public QThread
{
    Q_OBJECT
public:
    explicit DownFile(QObject *parent = nullptr);
private:
    QVector<QByteArray> m_data;
    QString m_filePath;
    qint64 m_writeBytes;
    qint64 m_totalBytes;

signals:
    void writeOK();
public slots:
    void initData(QByteArray data);
    void initFilePath(QString file);
    void initSize(qint64 totalBytes);

protected:
    void run();
};

#endif // DOWNFILE_H
