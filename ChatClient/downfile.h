#ifndef DOWNFILE_H
#define DOWNFILE_H

#include <QWidget>
#include <QThread>

class DownFile : public QThread
{
    Q_OBJECT
private:
    QVector<QByteArray> m_data;
    QString m_filePath;
    qint64 m_writedBytes;
    qint64 m_totalBytes;
public:
    DownFile(QWidget *parent = nullptr);

signals:
    void writeOk();

public slots:
    void initData(QByteArray data);
    void initFilePath(QString file);
    void initSize(qint64 totalBytes);

protected:
    void run();
};

#endif // DOWNFILE_H
