#include "mainwindow.h"
#include "logindialog.h"
#include "orderplacedialog.h"
#include <QApplication>
#include <QMessageBox>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "ClientService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int ClientId;
string SERVER_ADDR;
int main(int argc, char *argv[])
{
    ifstream f("config.ini");
    getline(f, SERVER_ADDR);
    QApplication a(argc, argv);
    boost::shared_ptr<TTransport> socket(new TSocket(SERVER_ADDR, 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ClientServiceClient client(protocol);

    try {
        transport->open();
        stringstream ss;
        ss <<"连接到服务器: "<<SERVER_ADDR;
        QMessageBox::about(0, "已连接", QString::fromStdString(ss.str()));
        LoginDialog loginDialog(&client);
        MainWindow w(&client);
        int login = 0;
        login = loginDialog.exec();
        while (login == 2) {
            login = loginDialog.exec();
        }
        if (login == 1) {
            w.start();
            w.show();
            return a.exec();
        }
    } catch(TException& te) {
        QMessageBox msgBox;
        msgBox.setText(te.what());
        msgBox.exec();
     }
}
