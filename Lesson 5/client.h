#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();

private slots:
    void on_connect_server_clicked();
    void on_send_to_clicked();

    void on_exit_clicked();

public slots:
    void onReadyRead();
private:
    Ui::Client *ui;
    QByteArray data;
    QTcpSocket *socket;
    void sendToServer(QString str);
};
#endif 