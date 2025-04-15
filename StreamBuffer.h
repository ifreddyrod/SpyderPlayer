// StreamBuffer.h
#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#include "Global.h"
#include <QIODevice>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QBuffer>
#include <QTimer>
#include <QUrl>

class StreamBuffer : public QIODevice 
{
    Q_OBJECT
public:
    StreamBuffer(const QUrl &url, QObject *parent = nullptr);
    ~StreamBuffer();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override;
    bool isSequential() const override;

private slots:
    void StartRequest();
    void DataAvailable();
    void HandleFinished();
    void HandleError(QNetworkReply::NetworkError);
    void HandleRedirect(const QUrl &redirectUrl);

private:
    QNetworkAccessManager *manager_;
    QNetworkReply *reply_ = nullptr;
    QUrl url_;
    QBuffer *buffer_;
    QTimer *retryTimer_;
};

#endif // STREAMBUFFER_H