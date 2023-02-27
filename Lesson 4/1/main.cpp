// IN PRO FILE: LIBS += D:\Qt\QT6\Tools\mingw1120_64\x86_64-w64-mingw32\lib\libws2_32.a  !!!


#include <QApplication>
#include <winsock2.h>
#include <QDebug>
#include <thread>


const auto buffer_size = 1024;
SOCKET Connection;

std::thread Client_Handler([](){
    int SizeMsg{0};
    QVector<char> clientBuff;
    clientBuff.resize(buffer_size);
    while(true){
        if(recv(Connection, clientBuff.data(), clientBuff.size(), NULL) != SOCKET_ERROR){
            char* msg = new char[SizeMsg + 1];
            msg[SizeMsg] = '\0';
            if( recv(Connection, msg, SizeMsg, 1) != SOCKET_ERROR){
                qDebug() << "MSG:" << clientBuff.data() << "SIZE:" << clientBuff.size();
            }
            delete[] msg;
        }
    }
});



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2,2);
    if(WSAStartup(DllVersion, &wsaData) !=0){
        qDebug() << "Error: WSAStartup";
        return a.exec();
    }

    SOCKADDR_IN addr;
    addr.sin_addr.S_un.S_addr =inet_addr("127.0.0.1");
    addr.sin_port = htons(1989);
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    // connect != Qt connect because function is reset! :)
//    if(connect(Connection,(SOCKADDR*)&addr,sizeof(addr)) != 0){
//        qDebug() << "Error: failed connect to server";
//        return a.exec();
//    }
//    else{
//        qDebug() << "Connected!";
//        qDebug() << "Client discriptor " << Connection;
//        Sleep(100);

//        //CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ClientHandler,NULL,NULL,NULL);
//        Client_Handler.detach();
//    }

    while(connect(Connection,(SOCKADDR*)&addr,sizeof(addr)) != 0){
        qDebug() << "Error: failed connect to server";
        Sleep(1000);
    }

    qDebug() << "Connected!";
    qDebug() << "Client discriptor " << Connection;
    Sleep(100);

    //CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ClientHandler,NULL,NULL,NULL);
    Client_Handler.detach();

    //QTextStream cin(stdin);
    //QString ClientMsg;
    //int SizeClientMsg;

    QVector<char> buffer;
    QString ClientMsg = "HELLO MY";// SERVER!!!";

    buffer.resize(ClientMsg.size());

    for(int i = 0; i < ClientMsg.size();i++){
        buffer[i] = ClientMsg.toStdString()[i];
    }

    while(true){
        //ClientMsg = cin.readLine();
        //SizeClientMsg = ClientMsg.size();
        //send(Connection, (char*)SizeClientMsg, sizeof(int),NULL);
        //send(Connection, ClientMsg.toStdString().c_str(), sizeof(ClientMsg), NULL);

        send(Connection, buffer.data(), buffer.size(), NULL);
        Sleep(1000);
    }

    qDebug() << "exit";
    return a.exec();
}
