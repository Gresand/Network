#include <QDataStream>
#include "client.h"
#include "ui_client.h"

Client::Client(QWidget *parent) : QWidget(parent), ui(new Ui::Client)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &QTcpSocket::deleteLater);
}

Client::~Client()
{
    delete ui;
}

void Client::on_connect_server_clicked()
{
    socket->connectToHost("127.0.0.1", 1230);
    ui->textBrowser->append("connect");
}

void Client::onReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    if (in.status() == QDataStream::Ok)
    {
        QString str;
        in >> str;
        ui->textBrowser->append(str);

    }
    else
    {
        ui->textBrowser->append("error");
    }
}

void Client::sendToServer(QString str)
{
    data.clear();
    QDataStream out(&data, QIODevice::WriteOnly);
    out << str;

    socket->write(data);
}


void Client::on_send_to_clicked()
{
    sendToServer(ui->lineEdit->text());
    ui->lineEdit->clear();
}


void Client::on_exit_clicked()
{
    socket->close();
    this->close();
}