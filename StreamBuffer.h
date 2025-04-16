// StreamBuffer.h
#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QTimer>
#include <QUrl>

class StreamBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit StreamBuffer(const QUrl &url, QObject *parent = nullptr);
    ~StreamBuffer();

    void Stop();

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    bool isSequential() const override;

private slots:
    void StartRequest();
    void DataAvailable();
    void HandleFinished();
    void HandleError(QNetworkReply::NetworkError code);
    void HandleRedirect(const QUrl &url);

signals:
    void ErrorOccurred(const QString &error);
    void BufferReady();

private:
    QUrl url_;
    QBuffer *buffer_;
    QNetworkAccessManager *manager_;
    QNetworkReply *reply_ = nullptr;
    QTimer *retryTimer_;
    int retryCount_;
};

#endif // STREAMBUFFER_H