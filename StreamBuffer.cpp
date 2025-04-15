// StreamBuffer.cpp
#include "StreamBuffer.h"

StreamBuffer::StreamBuffer(const QUrl &url, QObject *parent)
    : QIODevice(parent), url_(url) 
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
    if (reply_) reply_->deleteLater();
    delete buffer_;
    delete retryTimer_;
    delete manager_;
}

qint64 StreamBuffer::readData(char *data, qint64 maxlen) 
{
    qint64 bytesRead = buffer_->read(data, maxlen);
    PRINT << "StreamBuffer: Read" << bytesRead << "bytes, buffer size:" << buffer_->size();
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
    retryTimer_->stop();
    QNetworkRequest request(url_);
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (QtMediaPlayer)"); // Mimic browser
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(15000);

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // Warning: Use cautiously
    request.setSslConfiguration(sslConfig);

    reply_ = manager_->get(request);
    connect(reply_, &QNetworkReply::readyRead, this, &StreamBuffer::DataAvailable);
    connect(reply_, &QNetworkReply::finished, this, &StreamBuffer::HandleFinished);
    connect(reply_, &QNetworkReply::errorOccurred, this, &StreamBuffer::HandleError);
    connect(reply_, &QNetworkReply::redirected, this, &StreamBuffer::HandleRedirect);
    PRINT << "StreamBuffer: Starting request for" << url_.toString();
}

void StreamBuffer::DataAvailable() 
{
    QByteArray data = reply_->readAll();
    if (buffer_->size() < 20971520 && !data.isEmpty()) {
        buffer_->write(data);
        PRINT << "StreamBuffer: Wrote" << data.size() << "bytes, buffer size:" << buffer_->size();
        emit readyRead();
    } 
    else 
    {
        PRINT << "StreamBuffer: Skipped write, buffer size:" << buffer_->size() << "data size:" << data.size();
    }
}

void StreamBuffer::HandleFinished() 
{
    QByteArray data = reply_->readAll();
    if (buffer_->size() < 20971520 && !data.isEmpty()) 
    {
        buffer_->write(data);
        PRINT << "StreamBuffer: Wrote" << data.size() << "bytes on finish";
    }
    qint64 status = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    PRINT << "StreamBuffer: Request finished, HTTP status:" << status;
    if (status >= 200 && status < 300 && buffer_->size() > 0) 
    {
        PRINT << "StreamBuffer: Valid response, buffer size:" << buffer_->size();
    } 
    else 
    {
        PRINT << "StreamBuffer: Empty or failed response, retrying";
    }
    reply_->deleteLater();
    reply_ = nullptr;
    retryTimer_->start(1000);
}

void StreamBuffer::HandleError(QNetworkReply::NetworkError) 
{
    PRINT << "StreamBuffer: Network error:" << reply_->errorString() << "URL:" << url_.toString();
    reply_->deleteLater();
    reply_ = nullptr;
    retryTimer_->start(1000);
}

void StreamBuffer::HandleRedirect(const QUrl &redirectUrl) 
{
    qDebug() << "StreamBuffer: Redirected to:" << redirectUrl;
    url_ = redirectUrl;
    reply_->deleteLater();
    reply_ = nullptr;
    StartRequest();
}