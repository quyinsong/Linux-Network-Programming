#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class ServerDialog; }
QT_END_NAMESPACE

class ServerDialog : public QDialog
{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent = nullptr);
    ~ServerDialog();

private slots:
    //创建按钮槽函数
    void on_createButton_clicked();
    //响应客户端连接请求的槽函数
    void onNewConnection();
    //接受客户端消息的槽函数
    void onReadyRead();
    //转发聊天消息到客户端的槽函数
    void sendMessage(const QByteArray& buf);

    void onTimeout(void);

private:
    Ui::ServerDialog *ui;
    QTcpServer tcpServer;
    quint16 port;//服务器端口
    QList<QTcpSocket *> tcpClientList; //保存所有和客户端通信的套接字

    QTimer timer;
};
#endif // SERVERDIALOG_H
