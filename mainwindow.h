#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "trademanager.h"
#include "addclientdialog.h"
#include "orderplacedialog.h"
#include "valuation_class.h"

#include <map>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QTimer>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class TradeManager;

public:
    explicit MainWindow(boost::shared_ptr<TradeManager> tm, QWidget *parent = 0);
    ~MainWindow();

private:
    void initClientInfo();
    void initMainAccountInfo();
    void displayClientInfo(const QString &cur_text);
    void refreshTransactionPositionInfo();
	void refreshOrderInfo();
	void setOrderLine(QTableWidget *qtw, OrderType pbt, int row);
    void setPositionLine(QTableWidget *qtw, PositionType pbt, int row, bool is_main);
    void setTransactionLine(QTableWidget *qtw, TransactionType ot, int row);
    void refreshRevenue(QTableWidget* qtw, bool isMain);


private:
    Ui::MainWindow *ui;
    boost::shared_ptr<TradeManager> tm;
    AddClientDialog *acd;
    orderPlaceDialog *opd;
	QStringList cidStringList;
	const vector<QString> order_status_strs = vector<QString>{ "Reported", "Accepted", "Rejected", "Canceled" };
	QTimer timer;
    QTimer flush_timer;
	QTimer lcz_timer;


public slots:
    //void onAddClientButtonClicked();
    //void onPlaceOrderButtonClicked();
    void onClientAdded(QString client_name);
    void onOrderPlaced();
	void onRefreshButtonClicked();
	void onProcessOrderRequested();
	void onErrOccured(QString);

private slots:
    void onCurrentIndexChanged(const QString cur_text);
    void onAddClientMenuTriggered();
	void redisWriteClientGreeks();

signals:
    void currentClientID(int client_id);


};

#endif // MAINWINDOW_H
