#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ClientService.h"
#include "orderplacedialog.h"

#include <QMainWindow>
#include <QTableWidget>
#include <QTimer>

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
    void updatePositionInfo();
    void updateIntraDayTransactionInfo();
    void updateHistTransactionInfo();
    void updateIntraDayOrderInfo();
    void updateHistOrderInfo();
    void updateQoute();
    void updateBalance();

private slots:
    void onPlaceOrderButtonClicked();
    void onQouteCellDoubleClicked(int row, int col);
    void onPositionCellDoubleClicked(int row, int col);

signals:
    //void qouteChanged(QouteTrans qt);

private:
    Ui::MainWindow *ui;
    ClientServiceClient *rpc;
    QTimer timer;
#ifdef Q_OS_LINUX
    const std::vector<QString> order_status_lookup = {QString("Reported"), QString("Accepted"), QString("Rejected"), QString("Canceled")};
#elif defined(Q_OS_WIN32)
    const std::vector<QString> order_status_lookup = std::vector<QString>({QString("Reported"), QString("Accepted"), QString("Rejected"), QString("Canceled")});
#endif

private:
    void setPositionLine(QTableWidget *qtw, const PositionTypeTrans &pbt, int row);
    void setOrderLine(QTableWidget *qtw, const OrderTypeTrans &ot, int row);
    void setTransactionLine(QTableWidget *qtw, const TransactionTypeTrans &tt, int row);
    void setQouteLine(QTableWidget *qtw, const QouteTrans &ot, int row);
    void initDate();

};

#endif // MAINWINDOW_H
