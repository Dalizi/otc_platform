#if _MSC_VER >= 1600
#endif 

#include "mainwindow.h"
#include "trademanager.h"
#include "ClientService.h"
#include "RPCAsyncServer.h"

#include <QApplication>
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QTcpServer>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>
#include <thrift/qt/TQTcpServer.h>

#include <fstream>

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using namespace ::apache::thrift;
using namespace apache::thrift::concurrency;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::async;

int getCWD() {
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        return errno;
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    printf ("The current working directory is %s.\n", cCurrentPath);
}

string REDIS_ADDR;
int REDIS_PORT;
string REDIS_PASSWD;

int getRedisInfo() {
    ifstream redis_info("redis_info.ini");
    if (!redis_info.is_open()) {
        QMessageBox::warning(0, "Warning", "读取redis连接配置文件失败。");
        exit(1);
    }
    string line;
    if (getline(redis_info, line)) REDIS_ADDR = line;
    if (getline(redis_info, line)) REDIS_PORT = stod(line);
    if (getline(redis_info, line)) REDIS_PASSWD = line;
}

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);
    getRedisInfo();
    TradeManager tm;
    int port = 9090;

    boost::shared_ptr<QTcpServer> tcp_server( new QTcpServer() );
    boost::shared_ptr<ClientServiceAsyncHandler> handler(new ClientServiceAsyncHandler(&tm));
    boost::shared_ptr<TAsyncProcessor> processor(new ClientServiceAsyncProcessor(handler));
    boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    boost::shared_ptr<TQTcpServer> server( new TQTcpServer(tcp_server, processor, protocolFactory) );
    if( !tcp_server->isListening() && !tcp_server->listen(QHostAddress::Any, port) )
    {
        throw runtime_error("ERROR: Listen to TCP server.");
        return 1;
    }

    MainWindow w(&tm);
    w.show();

    return a.exec();
}
