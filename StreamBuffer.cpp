// StreamBuffer.cpp
#include "StreamBuffer.h"
#include <QDebug>

StreamBuffer::StreamBuffer(const QUrl &url, QObject *parent)
    : QIODevice(parent), url_(url), retryCount_(0)
{
    open(QIODevice::ReadOnly);
    buffer_ = new QBuffer(this);
    buffer_->open(QIODevice::ReadWrite);
    buffer_->seek(0); // Ensure the buffer starts at position 0
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

// In StreamBuffer.cpp
void StreamBuffer::DataAvailable()
{
    if (!reply_)
        return;
    QByteArray data = reply_->readAll();
    if (!data.isEmpty())
    {
        qint64 bytesWritten = buffer_->write(data);
        qDebug() << "StreamBuffer: Wrote" << bytesWritten << "bytes, buffer size:" << buffer_->size() << "data size:" << data.size();
        if (buffer_->size() >= 1048576) // 1 MB
            emit BufferReady();
        emit readyRead(); // Emit readyRead even for small data chunks
    }
    else
    {
        qDebug() << "StreamBuffer: No data available to write";
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
    QString errorString = reply_->errorString();
    qint64 status = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "StreamBuffer: Network error:" << errorString << "Code:" << code << "HTTP Status:" << status << "URL:" << url_.toString();
    reply_->deleteLater();
    reply_ = nullptr;
    if (status == 404 || status == 403) // Unrecoverable errors
    {
        emit ErrorOccurred("Unrecoverable network error: " + errorString + " (HTTP " + QString::number(status) + ")");
        Stop();
    }
    else if (retryCount_ < 5)
    {
        retryTimer_->start(2000);
    }
    else
    {
        emit ErrorOccurred("Max retries reached in StreamBuffer: " + errorString);
        Stop();
    }
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