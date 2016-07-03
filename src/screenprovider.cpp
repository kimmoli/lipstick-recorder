#include "screenprovider.h"
#include "qstandardpaths.h"
#include "qnetworkinterface.h"
#include "qlist.h"
#include "QtConcurrent/QtConcurrent"
#include "QFuture"
#include "qdebug.h"
#include <QBuffer>
#include <QThread>
#include <QImage>
#include <QElapsedTimer>

ScreenProvider::ScreenProvider(QObject* parent): QTcpServer(parent)
{
    streaming_ = false;

    connect(this, SIGNAL(clientConnected()), this, SLOT(onClientConnected()));
}

ScreenProvider::~ScreenProvider()
{
    if(streaming_) {
        stopStreaming();
    }

    while(future_.isRunning()) {} // wait threads to exit
}

void ScreenProvider::start()
{

    if (!this->listen()) {
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }

    if (ipAddress.isEmpty()) {
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }

    ipAddress_ = ipAddress;
    port_ = this->serverPort();
    this->setMaxPendingConnections(1);

    qDebug() << "Link:" << QString("http://%1:%2").arg(ipAddress_.toString()).arg(port_);
}

void ScreenProvider::incomingConnection(qintptr handle) {

    if(!streaming_) {

        streaming_ = true;
        emit clientConnected();

        watcher_ = new QFutureWatcher<void>();
        connect(watcher_, SIGNAL(finished()), this, SLOT(handleEndedStream()));
        future_ = QtConcurrent::run(streamLoop,
                                     handle,
                                     std::ref(queue_),
                                     std::ref(streaming_));
        watcher_->setFuture(future_);
    }
}

void ScreenProvider::stopStreaming()
{
    streaming_ = false;
    this->close();
}

void ScreenProvider::handleEndedStream()
{
    emit clientDisconnected();
    streaming_ = false;
    disconnect(watcher_, SIGNAL(finished()), this, SLOT(handleEndedStream()));
    delete watcher_;
    watcher_ = 0;
}

void ScreenProvider::onClientConnected()
{
    qDebug() << "client connected";
}

void streamLoop(qintptr socketDesc, QQueue<QByteArray> &queue, bool& streaming) {

    QTcpSocket* socket = new QTcpSocket();
    // TCP_NODELAY + disable Nagle's algorithm
    socket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant::fromValue(1));
    // Internetwork control
    socket->setSocketOption(QAbstractSocket::TypeOfServiceOption, QVariant::fromValue(192));
    socket->setSocketDescriptor(socketDesc);
    socket->readAll();

    QByteArray ContentType = ("HTTP/1.1 200 OK\r\n" \
                              "Server: test\r\n" \
                              "Cache-Control: no-cache\r\n" \
                              "Cache-Control: private\r\n" \
                              "Connection: close\r\n"\
                              "Pragma: no-cache\r\n"\
                              "Content-Type: multipart/x-mixed-replace; boundary=--boundary\r\n\r\n");

    socket->write(ContentType);

    while((socket->state() != QAbstractSocket::ClosingState ||
           socket->state() != QAbstractSocket::UnconnectedState) &&
           socket->state() == QAbstractSocket::ConnectedState &&
           streaming) {

        if(queue.empty()) { // no new frame available
            continue;
        }

        // make sure that the queue doesn't grow too big or
        // the OOM killer will kick in
        if(queue.length() > 20) {
            queue.clear();
            continue;
        }

        QByteArray boundary = ("--boundary\r\n" \
                               "Content-Type: image/jpeg\r\n" \
                               "Content-Length: ");

        QByteArray img  = queue.dequeue();
        boundary.append(QString::number(img.length()));
        boundary.append("\r\n\r\n");

        socket->write(boundary);
        socket->waitForBytesWritten();
        boundary.clear();
        socket->write(img);
        socket->waitForBytesWritten();
        img.clear();
    }

    socket->flush();
    socket->abort();
    socket->deleteLater();
    streaming = false;
    queue.clear();
    return;
}
