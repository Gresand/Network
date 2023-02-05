#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

class Server : public QTcpServer
{
    Q_OBJECT
private:
    QByteArray data_;
    void sendToClient(QString str);
    QString getClient(QStringList &strSplited);

public:
    QTcpSocket *socket_;
    Server();

public slots:
    void incomingConnection(qintptr socketDescriptor);      
    void onReadyRead();                                     

};

#endif 