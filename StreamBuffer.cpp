// StreamBuffer.cpp
#include "StreamBuffer.h"
#include <QDebug>

StreamBuffer::StreamBuffer(const QUrl &url, QObject *parent)
    : QIODevice(parent), url_(url), retryCount_(0)
{
    open(QIODevice::ReadOnly);
    buffer_ = new QBuffer(this);
    buffer_->open(QIODevice::ReadWrite);
    manager_ = new QNetworkAccessManager(this);
    retryTimer_ = new QTimer(this);
    connect(retryTimer_, &QTimer::timeout, this, &StreamBuffer::StartRequest);
    StartRequest();
}

StreamBuffer::~StreamBuffer()
{
    Stop();
    delete buffer_;
    delete retryTimer_;
    delete manager_;
}

void StreamBuffer::Stop()
{
    retryTimer_->stop();
    if (reply_)
    {
        disconnect(reply_, nullptr, this, nullptr);
        reply_->abort();
        reply_->deleteLater();
        reply_ = nullptr;
    }
}

qint64 StreamBuffer::readData(char *data, qint64 maxlen)
{
    qint64 bytesRead = buffer_->read(data, maxlen);
    qDebug() << "StreamBuffer: Read" << bytesRead << "bytes, buffer size:" << buffer_->size();
    return bytesRead;
}

qint64 StreamBuffer::writeData(const char *, qint64)
{
    return -1;
}

bool StreamBuffer::isSequential() const
{
    return true;
}

void StreamBuffer::StartRequest()
{
    if (retryCount_ >= 5)
    {
        qDebug() << "StreamBuffer: Max retries reached, stopping";
        Stop();
        emit ErrorOccurred("Max retries reached in StreamBuffer");
        return;
    }

    retryTimer_->stop();
    QNetworkRequest request(url_);
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (QtMediaPlayer)");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(15000);

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);

    reply_ = manager_->get(request);
    connect(reply_, &QNetworkReply::readyRead, this, &StreamBuffer::DataAvailable);
    connect(reply_, &QNetworkReply::finished, this, &StreamBuffer::HandleFinished);
    connect(reply_, &QNetworkReply::errorOccurred, this, &StreamBuffer::HandleError);
    connect(reply_, &QNetworkReply::redirected, this, &StreamBuffer::HandleRedirect);
    qDebug() << "StreamBuffer: Starting request for" << url_.toString();
    retryCount_++;
}

void StreamBuffer::DataAvailable()
{
    if (!reply_)
        return;
    QByteArray data = reply_->readAll();
    if (buffer_->size() < 20971520 && !data.isEmpty())
    {
        buffer_->write(data);
        qDebug() << "StreamBuffer: Wrote" << data.size() << "bytes, buffer size:" << buffer_->size();
        if (buffer_->size() >= 1048576)
            emit BufferReady();
        emit readyRead();
    }
    else
    {
        qDebug() << "StreamBuffer: Skipped write, buffer size:" << buffer_->size() << "data size:" << data.size();
    }
}

void StreamBuffer::HandleFinished()
{
    if (!reply_)
        return;
    QByteArray data = reply_->readAll();
    qint64 status = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "StreamBuffer: Request finished, HTTP status:" << status;

    if (!data.isEmpty() && buffer_->size() < 20971520)
    {
        buffer_->write(data);
        qDebug() << "StreamBuffer: Wrote" << data.size() << "bytes on finish";
    }

    if (status >= 200 && status < 300 && buffer_->size() > 0)
    {
        qDebug() << "StreamBuffer: Valid response, buffer size:" << buffer_->size();
        retryCount_ = 0;
    }
    else
    {
        qDebug() << "StreamBuffer: Empty or failed response, retrying";
        reply_->deleteLater();
        reply_ = nullptr;
        retryTimer_->start(2000);
    }

    reply_->deleteLater();
    reply_ = nullptr;
}

void StreamBuffer::HandleError(QNetworkReply::NetworkError code)
{
    if (!reply_)
        return;
    qDebug() << "StreamBuffer: Network error:" << reply_->errorString() << "Code:" << code << "URL:" << url_.toString();
    reply_->deleteLater();
    reply_ = nullptr;
    retryTimer_->start(2000);
}

void StreamBuffer::HandleRedirect(const QUrl &redirectUrl)
{
    if (!reply_)
        return;
    qDebug() << "StreamBuffer: Redirected to:" << redirectUrl;
    url_ = redirectUrl;
    reply_->deleteLater();
    reply_ = nullptr;
    retryCount_ = 0;
    StartRequest();
}