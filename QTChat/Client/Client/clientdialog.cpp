#include "clientdialog.h"
#include "ui_clientdialog.h"

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
{
    ui->setupUi(this);

    //

    connect(&tcpSocket,SIGNAL(connected()),this,SLOT(onConnected()));

    connect(&tcpSocket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));

    connect(&tcpSocket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));

    connect(&tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onError()));


}

ClientDialog::~ClientDialog()
{
    delete ui;
}

//发送消息按钮槽函数
void ClientDialog::on_sendButton_clicked()
{
    QString str = ui->messageEdit->text();

    if(str == ""){
        return;
    }
    else{
        str = username+":"+str;
        tcpSocket.write(str.toUtf8());
    }

    ui->messageEdit->clear();


}

//连接服务器按钮槽函数
void ClientDialog::on_connectButton_clicked()
{
    //如果客户端离线，建立连接，否则，断开连接
    if(status==false){
        this->ServerIP.setAddress(ui->ipEdit->text());
        this->ServerPort = ui->portEdit->text().toUShort();
        this->username = ui->nameEdit->text();

        this->tcpSocket.connectToHost(this->ServerIP,this->ServerPort);

    }
    else{
        this->tcpSocket.disconnectFromHost();
    }

}

//和服务器连接成功时的槽函数
void ClientDialog::onConnected()
{
    status = true;
    ui->nameEdit->setEnabled(false);
    ui->ipEdit->setEnabled(false);
    ui->portEdit->setEnabled(false);
    ui->messageEdit->setEnabled(true);
    ui->sendButton->setEnabled(true);
    ui->connectButton->setEnabled(true);
    ui->connectButton->setText("离开聊天室");

    QString str = this->username + "来到聊天室";
    tcpSocket.write(str.toUtf8());
}
//和服务器断开连接时的槽函数
void ClientDialog::onDisconnected()
{
    status = false;

    ui->nameEdit->setEnabled(true);
    ui->ipEdit->setEnabled(true);
    ui->portEdit->setEnabled(true);
    ui->messageEdit->setEnabled(false);
    ui->sendButton->setEnabled(false);
    ui->connectButton->setEnabled(true);
    ui->connectButton->setText("连接服务器");

    QString str = this->username + "离开聊天室";
    tcpSocket.write(str.toUtf8());
}
//接收聊天消息的槽函数
void ClientDialog::onReadyRead()
{
    if(tcpSocket.bytesAvailable()){
        QByteArray buf = tcpSocket.readAll();

        ui->listWidget->addItem(buf);
        ui->listWidget->scrollToBottom();
    }
}
//网络异常执行的槽函数
void ClientDialog::onError()
{
    QMessageBox::critical(this,"ERROR",tcpSocket.errorString());
}
