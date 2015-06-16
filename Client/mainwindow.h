#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif

#include "ClientService.h"
#include "orderplacedialog.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class orderPlaceDialog;
public:
    explicit MainWindow(ClientServiceClient *csc, QWidget *parent = 0);
    ~MainWindow();

    void updateClientInfo();
    void start();

public slots:
    void onRefreshPositionButtonClicked();
    void onRefreshIntraDayTransactionButtonClicked();
    void onRefreshHistTransactionButtonClicked();
    void onRefreshIntraDayOrderButtonClicked();
    void onRefreshHistOrderButtonClicked();

private slots:
    void onPlaceOrderButtonClicked();
    void onQouteCellDoubleClicked(int row, int col);
    void onPositionCellDoubleClicked(int row, int col);
    void onUpdateTimerTimeout();
    void updateHistTransactionInfo();

signals:
    //void qouteChanged(QouteTrans qt);
    void histTransactionReady();

private:
    Ui::MainWindow *ui;
    ClientServiceClient *rpc;
    QTimer timer;
#ifdef Q_OS_LINUX
    const std::vector<QString> order_status_lookup = {QString("已报"), QString("已成"), QString("废单"), QString("已撤")};
#elif defined(Q_OS_WIN32)
    const std::vector<QString> order_status_lookup = std::vector<QString>({QString("已报"), QString("已成"), QString("废单"), QString("已撤")});
#endif
    QFuture<void> timed_future, position_future, intraday_order_future, hist_order_future, intraday_transaction_future;
    QFuture<std::vector<TransactionTypeTrans>> hist_transaction_future;
    std::shared_ptr<ClientServiceClient> timed_client;
    std::shared_ptr<ClientServiceClient> balance_client;
    std::shared_ptr<ClientServiceClient> event_client;
private:
    void setPositionLine(QTableWidget *qtw, const PositionTypeTrans &pbt, int row);
    void setOrderLine(QTableWidget *qtw, const OrderTypeTrans &ot, int row);
    void setTransactionLine(QTableWidget *qtw, const TransactionTypeTrans &tt, int row);
    void setQouteLine(QTableWidget *qtw, const QouteTrans &ot, int row);
    void initDate();
    void updateQoute();
    void updateBalance();
    void updateQouteAndBalance();
    void updatePositionInfo();
    void updateIntraDayTransactionInfo();
    std::vector<TransactionTypeTrans> getHistTransactionInfo();
    void updateIntraDayOrderInfo();
    void updateHistOrderInfo();

    std::shared_ptr<ClientServiceClient> getThriftClient();

};

#endif // MAINWINDOW_H
