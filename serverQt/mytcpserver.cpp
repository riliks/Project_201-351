#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include "functions.h"
#include <QString>

MyTcpServer::~MyTcpServer()
{
    foreach(int i, SClients.keys()) {
        QTextStream os(SClients[i]);
        SClients[i] -> close();
        SClients.remove(i);
    }
    server_status=0;
    mTcpServer->close();
}
MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        server_status=1;
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection(){
    if(server_status==1){
        QTcpSocket* clientSocket = mTcpServer ->nextPendingConnection();
        int idusersocs=(int)clientSocket->socketDescriptor();
        SClients[idusersocs] = clientSocket;
        SClients[idusersocs] -> write("Hello, World!!! I am echo server!\r\n");
        connect(SClients[idusersocs], &QTcpSocket::readyRead,this,&MyTcpServer::slotServerRead);
        connect(SClients[idusersocs], &QTcpSocket::disconnected,this,&MyTcpServer::slotClientDisconnected);
    }
}

void MyTcpServer::slotServerRead(){
    QTcpSocket *clientSocket = (QTcpSocket*)sender();
    int id = (int)clientSocket->socketDescriptor();
    QByteArray array;
    std::string message="";
    while(clientSocket->bytesAvailable()>0)
    {
        array = clientSocket->readAll();
        message += array.toStdString();
    }

    qDebug()<<QString::fromStdString(message);

    std::string func = "";
    int field[200];
    int pos = message.find("&");
    func = message.substr(0, pos);
    message.erase(0, pos+1);
    if(func == "auth") {
        std::string log = "";
        std::string pass = "";
        pos = message.find("&");
        log = message.substr(0, pos);
        message.erase(0, pos);

        pos = message.find("&");
        pass = message.substr(pos+1, message.length() - 3);
        message.erase(0);

        qDebug()<<"login ="<<QString::fromStdString(log)
                <<"password = "<< QString::fromStdString(pass)
               <<"result = " << authorize(log,pass);
        array.clear();
        array.append(authorize(log,pass));
        clientSocket -> write(array);
    }
    if(func == "start") {
        std::string params = "";
        pos = message.find("&");
        params = message.substr(pos+1, message.length() - 3);
        message.erase(0);

        array.clear();
        for(int i=0; i<200; i++) {
            field[i] = 0;
            array.append(field[i]);
        }
        qDebug()<<"params ="<<QString::fromStdString(params)
               <<"field create";

        clientSocket -> write(array);
    }
    if(func == "stats") {
        // подключение к бд
    }
    if(func == "set coord") {
        std::string player1 = "";
        std::string player2 = "";
        std::string player1_ships = "";
        std::string player2_ships = "";
        pos = message.find("&");
        player1 = message.substr(0, pos);
        message.erase(0, pos);

        pos = message.find("&");
        player1_ships = message.substr(0, pos);
        message.erase(0, pos);
        for(int i=0; i<100; i++) {
            field[i] = player1_ships[i];
        }

        pos = message.find("&");
        player2 = message.substr(0, pos);
        message.erase(0, pos);

        pos = message.find("&");
        player2_ships = message.substr(pos+1, message.length() - 3);
        message.erase(0);
        for(int i=100; i<200; i++) {
            field[i] = player2_ships[i];
        }
        qDebug()<<"field write in";
        array.clear();
        for(int i=0; i<200; i++) {
            array.append(field[i]);
        }
        clientSocket -> write(array);
    }
    if(func == "action"){
        std::string action = "";                                    // 0 - пусто
        pos = message.find("&");                                    // 1 - мимо
        int pos1 = message.find("6");                               // 2 - ранил
        action = message.substr(pos+1, message.length() - 3);       // 3 - убил
        message.erase(0);                                           // 4 - корабль
                                                                    // 5 - остров
                                                                    // 6 - отметка пользователя
        switch (field[pos1]) {
            case 0:
                field[pos1] = 1;
                break;
            case 4:
                if (field[pos1 - 1] == 4 || field[pos1 + 1] == 4)
                    field[pos1] = 2;
                else
                    field[pos1] = 3;
                break;

        int j = 0;
        for (int i=0; i<100; i++) {
            if (field[i] == 4) {
                j += 1;
                break;
            }
        }
            if (j != 0) {
               qDebug()<<"Game over, PLAYER 2 WIN";
               slotClientDisconnected();
            }
        j = 0;
        for (int i=100; i<200; i++) {
            if (field[i] == 4) {
                j += 1;
                break;
            }
        }
            if (j != 0) {
               qDebug()<<"Game over, PLAYER 1 WIN";
               slotClientDisconnected();
            }
        }
        qDebug()<<"action";
        array.clear();
        for(int i=0; i<200; i++) {
            array.append(field[i]);
        }
        clientSocket -> write(array);
    }
    if(func == "exit") {
        slotClientDisconnected();
    }
    //clientSocket -> write(array);    
}
void MyTcpServer::slotClientDisconnected(){
    QTcpSocket *clientSocket = (QTcpSocket*)sender();
    int id = (int)clientSocket->socketDescriptor();
    clientSocket->close();
    SClients.remove(id);
    qDebug() << QString::fromUtf8("Client is disconnected \n");
}

/* 04040004440444040400004444000004400404000444
0040400044404440404000044440000044004040004440
0404000444044404040000444400000440040400044400
4040004440444040400004444000004400404000444000
000444400444004040 */
