#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class ClientDialog; }
QT_END_NAMESPACE

class ClientDialog : public QDialog
{
    Q_OBJECT

public:
    ClientDialog(QWidget *parent = nullptr);
    ~ClientDialog();
    //连接

private slots:
    //发送消息按钮槽函数
    void on_sendButton_clicked();

    //连接服务器按钮槽函数
    void on_connectButton_clicked();

    //和服务器连接成功时的槽函数
    void onConnected();
    //和服务器断开连接时的槽函数
    void onDisconnected();
    //接收聊天消息的槽函数
    void onReadyRead();
    //网络异常执行的槽函数
    void onError();

private:
    Ui::ClientDialog *ui;

    bool status; //客户端状态
    QTcpSocket tcpSocket; //客户端套接字
    QHostAddress ServerIP; //服务器地址
    quint16 ServerPort; //服务器端口地址
    QString username; //客户端昵称

};
#endif // CLIENTDIALOG_H
