#include "server.h"
#include <QFile>


Server::Server()
{
    if (!(this->listen(QHostAddress::Any, 1230)))
    {
        qDebug() << "error";      
    }
}


void Server::incomingConnection(qintptr socketDescriptor)
{
    socket_ = new QTcpSocket;
    socket_->setSocketDescriptor(socketDescriptor);
    connect(socket_, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(socket_, &QTcpSocket::disconnected, this, &QTcpSocket::deleteLater);
}


void Server::onReadyRead()
{
    socket_ = (QTcpSocket*)sender();
    QDataStream inp(socket_);
    inp.setVersion(QDataStream::Qt_5_15);
    if (inp.status() == QDataStream::Ok)
    {
        QString str;
        inp >> str;
        qDebug() << str;
        QStringList strSplited = str.split("  ");  
        if (strSplited[0] == "get")
        {
            str = getClient(strSplited);
        }
        else if (strSplited[0] == "POST" && strSplited[1].right(4) == "exit") 
        {
            socket_->close();
            this->close();
            return;
        }
        sendToClient(str);
    }
    else
    {
        qDebug() << "dataStream error ";
    }
}

void Server::sendToClient(QString str)
{
    data_.clear();
    QDataStream out(&data_, QIODevice::WriteOnly);

    out << str;
    socket_->write(data_);
}

QString Server::getClient(QStringList &strSplited)  
{
    QFile file(strSplited[1]);
    QString strFile = file.readAll();
    file.close();
    return strFile;
}