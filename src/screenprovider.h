#ifndef SCREENPROVIDER_H
#define SCREENPROVIDER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QQueue>
#include <QByteArray>
#include <QHostAddress>
#include <QFutureWatcher>
#include <QString>
#include <QTimer>

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <unistd.h>


void streamLoop(qintptr socketDesc, QQueue<QByteArray> &queue, bool& streaming);

class ScreenProvider : public QTcpServer
{
    Q_OBJECT
public:
    ScreenProvider(QObject* parent = 0);
    ~ScreenProvider();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stopStreaming();
    void incomingConnection(qintptr handle) Q_DECL_OVERRIDE;

    QQueue<QByteArray> queue_;

signals:
    void clientConnected();
    void clientDisconnected();

public slots:
    void handleEndedStream();
    void onClientConnected();

private:

    QFutureWatcher<void> *watcher_;
    QFuture<void> future_;
    bool streaming_;
    QHostAddress ipAddress_;
    int port_;
};

#endif // SCREENPROVIDER_H
