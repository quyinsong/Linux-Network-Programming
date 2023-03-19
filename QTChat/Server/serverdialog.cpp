#include "serverdialog.h"
#include "ui_serverdialog.h"

ServerDialog::ServerDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ServerDialog)
{
    ui->setupUi(this);

    //监听客户端连接请求信号
    connect(&tcpServer,SIGNAL(newConnection()),this,SLOT(onNewConnection()));

    //监听定时器信号
    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));

}

ServerDialog::~ServerDialog()
{
    delete ui;
}

//创建按钮槽函数
void ServerDialog::on_createButton_clicked()
{
    //获取服务器端口地址
    this->port = ui->portEdit->text().toInt();
    //设置服务器IP和端口
    if(tcpServer.listen(QHostAddress::Any,this->port)==true){
        qDebug()<<"创建服务器成功";
        //禁用服务器创建按钮
        ui->createButton->setEnabled(false);
        ui->portEdit->setEnabled(false);
        //开启定时器
        timer.start(3000);
    }
    else{
        qDebug()<<"创建服务器失败";
    }
}

//响应客户端连接请求的槽函数
void ServerDialog::onNewConnection()
{
    //获取和客户端通信的套接字
    QTcpSocket* tcpClient = tcpServer.nextPendingConnection();
    //保存套接字到容器
    this->tcpClientList.append(tcpClient);
    //监听客户端发送消息信号
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
}
//接收客户端消息的槽函数
void ServerDialog::onReadyRead()
{
    //遍历容器检查是那个客户端发送消息
    for(int i=0;i<this->tcpClientList.size();i++){
        if(this->tcpClientList.at(i)->bytesAvailable()){
            //读取消息并保存
            QByteArray buf = tcpClientList.at(i)->readAll();
            //显示聊天消息
            ui->listWidget->addItem(buf);
            ui->listWidget->scrollToBottom();//显示最新消息
            //转发消息给所有在线客户端
            sendMessage(buf);
        }
    }

}
//转发聊天消息到客户端的槽函数
void ServerDialog::sendMessage(const QByteArray& buf)
{
    for(int i=0;i<tcpClientList.size();i++){
        tcpClientList.at(i)->write(buf);
    }
}

//定时器槽函数，定期检测断开连接的客户端套接字并删除
void ServerDialog::onTimeout()
{
    //循环检测
    for(int i=0;i<this->tcpClientList.size();i++){
        //检测客户端套接字是否断开连接
        if(this->tcpClientList.at(i)->state()==QAbstractSocket::UnconnectedState){
            tcpClientList.removeAt(i);
            --i;
        }
    }
}
