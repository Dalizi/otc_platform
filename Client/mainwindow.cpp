#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QTimeZone>
#include <QVector>


#include <sstream>
#include <iomanip>
#include <vector>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;


using namespace std;

extern int ClientId;
extern string SERVER_ADDR;

MainWindow::MainWindow(ClientServiceClient *csc, QWidget *parent) :
    QMainWindow(parent),
    rpc(csc),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timed_client = getThriftClient();
    balance_client = getThriftClient();

}

MainWindow::~MainWindow()
{
    delete ui;
    timed_future.waitForFinished();
    position_future.waitForFinished();
    intraday_order_future.waitForFinished();
    hist_order_future.waitForFinished();
    intraday_transaction_future.waitForFinished();
    hist_transaction_future.waitForFinished();
}

void MainWindow::start() {
    qRegisterMetaType<QVector<int>>("QVector<int>");
    initDate();
    updateClientInfo();
    updatePositionInfo();
    updateBalance();
    connect(ui->intraDayOrderQueryPushButton, SIGNAL(clicked()), this, SLOT(onRefreshIntraDayOrderButtonClicked()));
    connect(ui->histOrderQueryPushButton, SIGNAL(clicked()), this, SLOT(onRefreshHistOrderButtonClicked()));
    connect(ui->intraDayTransactionQueryPushButton, SIGNAL(clicked()), this, SLOT(onRefreshIntraDayTransactionButtonClicked()));
    connect(ui->histTransactionQueryPushButton, SIGNAL(clicked()), this, SLOT(onRefreshHistTransactionButtonClicked()));
    connect(ui->positionRefreshPushButton, SIGNAL(clicked()), this, SLOT(onRefreshPositionButtonClicked()));
    connect(ui->placeOrderPushButton, SIGNAL(clicked()), this, SLOT(onPlaceOrderButtonClicked()));
    connect(ui->qouteTableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onQouteCellDoubleClicked(int,int)));
    connect(ui->positionTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onPositionCellDoubleClicked(int,int)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(onUpdateTimerTimeout()), Qt::DirectConnection);
    ui->qouteTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->positionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->intraDayOrderTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->histOrderTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->intraDayTransactionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->histTransactionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    timer.start(3000);
}

void MainWindow::initDate() {
    ui->histTransactionStartDateEdit->setDate(QDate::currentDate());
    ui->histTransactionStartDateEdit->setMaximumDate(QDate::currentDate());
    ui->histTransactionEndDateEdit->setDate(QDate::currentDate());
    ui->histTransactionEndDateEdit->setMaximumDate(QDate::currentDate());

    ui->histOrderStartDateEdit->setDate(QDate::currentDate());
    ui->histOrderStartDateEdit->setMaximumDate(QDate::currentDate());
    ui->histOrderEndDateEdit->setDate(QDate::currentDate());
    ui->histOrderEndDateEdit->setMaximumDate(QDate::currentDate());
}

void MainWindow::updateClientInfo() {
    ClientInfoTrans cit;
    rpc->get_clientinfo(cit, ClientId);
    stringstream ss;
    ss <<setfill('0') <<setw(8) <<cit.client_id;
    ui->clientIDLineEdit->setText(QString::fromStdString(ss.str()));
    ui->clientNameLineEdit->setText(QString::fromStdString(cit.client_name));
    ui->clientRelationshipLineEdit->setText(QString::fromStdString(cit.client_relationship));
    ui->entrustValueLineEdit->setText(QString::number(cit.trust_value));
    ui->charteredBusinessLineEdit->setText(QString::fromStdString(cit.chartered_business));
    ui->reviewMaterialLineEdit->setText(QString::fromStdString(cit.review_material));
    ui->publicInfoLineEdit->setText(QString::fromStdString(cit.public_info));
    ui->interviewInfoLineEdit->setText(QString::fromStdString(cit.interview_record));
    ui->clientLevelLineEdit->setText(QString::number(cit.client_level));
}

void MainWindow::updatePositionInfo() {
    int row =0;
    vector<PositionTypeTrans> pos_vec;
    rpc->get_position(pos_vec, ClientId);
    ui->positionTableWidget->setRowCount(pos_vec.size());
    for (auto p:pos_vec) {
        setPositionLine(ui->positionTableWidget, p, row++);
    }
    GreekRisk greeks;
    rpc->get_client_greeks(greeks, ClientId);
    ui->deltaLabel->setText(QString::number(greeks.delta));
    ui->gammaLabel->setText(QString::number(greeks.gamma));
    ui->thetaLabel->setText(QString::number(greeks.theta));
    ui->vegaLabel->setText(QString::number(greeks.vega));


}

void MainWindow::updateIntraDayTransactionInfo(){
    int row = 0;
    vector<TransactionTypeTrans> transaction_vec;
    auto start_date = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    auto end_date = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    rpc->get_transaction(transaction_vec, ClientId, start_date, end_date);
    ui->intraDayTransactionTableWidget->setRowCount(transaction_vec.size());
    for (auto o:transaction_vec) {
        setTransactionLine(ui->intraDayTransactionTableWidget, o, row++);
    }
}

void MainWindow::updateHistTransactionInfo() {
    int row = 0;
    vector<TransactionTypeTrans> transaction_vec;
    auto start_date = ui->histTransactionStartDateEdit->date().addDays(-10).toString("yyyy-MM-dd").toStdString();
    auto end_date = ui->histTransactionEndDateEdit->date().toString("yyyy-MM-dd").toStdString();
    rpc->get_transaction(transaction_vec, ClientId, start_date, end_date);
    ui->histTransactionTableWidget->setRowCount(transaction_vec.size());
    for (auto o:transaction_vec) {
        setTransactionLine(ui->histTransactionTableWidget, o, row++);
    }
}

void MainWindow::updateIntraDayOrderInfo() {
    int row = 0;
    vector<OrderTypeTrans> order_vec;
    auto start_date = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    auto end_date = QDate::currentDate().toString("yyyy-MM-dd").toStdString();
    rpc->get_order(order_vec, ClientId, start_date, end_date);
    ui->intraDayOrderTableWidget->setRowCount(order_vec.size());
    for (auto o:order_vec) {
        setOrderLine(ui->intraDayOrderTableWidget, o, row++);
    }
}

void MainWindow::updateHistOrderInfo() {
    int row = 0;
    vector<OrderTypeTrans> order_vec;
    auto start_date = ui->histOrderStartDateEdit->date().toString("yyyy-MM-dd").toStdString();
    auto end_date = ui->histOrderEndDateEdit->date().toString("yyyy-MM-dd").toStdString();
    rpc->get_order(order_vec, ClientId, start_date, end_date);
    ui->histOrderTableWidget->setRowCount(order_vec.size());
    for (auto o:order_vec) {
        setOrderLine(ui->histOrderTableWidget, o, row++);
    }
}

void MainWindow::updateQoute() {

    int row = 0;
    vector<QouteTrans> qoute_vec;
    rpc->get_qoute(qoute_vec);
    ui->qouteTableWidget->setRowCount(qoute_vec.size());
    for (auto q:qoute_vec) {
        setQouteLine(ui->qouteTableWidget, q, row++);
    }
}

void MainWindow::updateBalance() {
    map<string, double> balance_map;
    ClientBalanceTrans cbt;
    rpc->get_calculated_balance(balance_map, ClientId);
    rpc->get_balance(cbt, ClientId);
    ui->marketValueBalanceLineEdit->setText(QString::number(balance_map["market_value_balance"], 'f'));
    ui->totalBalanceLineEdit->setText(QString::number(cbt.total_balance, 'f'));
    ui->occupiedMarginLineEdit->setText(QString::number(cbt.occupied_margin, 'f'));
    ui->availableBalanceLineEdit->setText(QString::number(balance_map["available_balance"], 'f'));
    ui->frozenBalanceLineEdit->setText(QString::number(balance_map["frozen_balance"], 'f'));
    //ui->totalBalanceLineEdit->setText(QString::number(cbt.total_balance, 'f'));
    ui->marginRiskLineEdit->setText(QString::number(balance_map["margin_risk_ratio"], 'f'));
    //ui->marketValueLineEdit->setText(QString::number(balance_map["market_value"], 'f'));

}

void MainWindow::onPlaceOrderButtonClicked() {
    QouteTrans qt;
    QList<QTableWidgetItem*> cur_line = ui->qouteTableWidget->selectedItems();
    qt.instr_code = cur_line.value(0)->text().toStdString();
    qt.ask_price = cur_line.value(1)->text().toDouble();
    qt.ask_volume = cur_line.value(2)->text().toInt();
    qt.bid_price = cur_line.value(3)->text().toDouble();
    qt.bid_volume = cur_line.value(4)->text().toInt();
    orderPlaceDialog opd(rpc, &qt, false);
    connect(&opd, SIGNAL(orderPlaced()), this, SLOT(onRefreshPositionButtonClicked()));
    opd.exec();
}


void MainWindow::setPositionLine(QTableWidget *qtw, const PositionTypeTrans &pbt, int row) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(pbt.instr_code)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.total_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.available_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.frozen_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.average_price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(rpc->get_pnl(pbt))));
    qtw->setItem(row, column++, new QTableWidgetItem(pbt.long_short==LONG_ORDER?"买":"卖"));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(rpc->get_close_price(pbt), 'f')));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.occupied_margin)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.underlying_price)));

}

void MainWindow::setOrderLine(QTableWidget *qtw, const OrderTypeTrans &ot, int row) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.order_id)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.instr_code)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.long_short==LONG_ORDER?"买":"卖")));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.open_offset==OPEN?"开仓":"平仓")));
    //auto time = QDateTime::fromString(QString::fromStdString(ot.date_time), "yyyy-MM-dd HH:mm:ss");
    //time.setTimeZone(QTimeZone(8));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.date_time)));
    qtw->setItem(row, column++, new QTableWidgetItem(order_status_lookup[(int)ot.order_status]));

}

void MainWindow::setTransactionLine(QTableWidget *qtw, const TransactionTypeTrans &tt, int row) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(tt.transaction_id)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(tt.instr_code)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(tt.price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(tt.amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(tt.long_short==LONG_ORDER?"买":"卖")));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(tt.open_offset==OPEN?"开仓":"平仓")));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(tt.date_time)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(tt.underlying_price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(tt.close_pnl)));
}

void MainWindow::setQouteLine(QTableWidget *qtw, const QouteTrans &qt, int row) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(qt.instr_code)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(qt.ask_price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(qt.ask_volume)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(qt.bid_price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(qt.bid_volume)));
}

void MainWindow::onQouteCellDoubleClicked(int row, int col) {
    QouteTrans qt;
    qt.instr_code = ui->qouteTableWidget->item(row, 0)->text().toStdString();
    qt.ask_price = ui->qouteTableWidget->item(row, 1)->text().toDouble();
    qt.ask_volume = ui->qouteTableWidget->item(row, 2)->text().toInt();
    qt.bid_price = ui->qouteTableWidget->item(row, 3)->text().toDouble();
    qt.bid_volume = ui->qouteTableWidget->item(row, 4)->text().toInt();
    orderPlaceDialog opd(rpc, &qt, false);
    connect(&opd, SIGNAL(orderPlaced()), this, SLOT(onRefreshPositionButtonClicked()));
    opd.exec();
}

void MainWindow::onPositionCellDoubleClicked(int row, int col) {
    PositionTypeTrans ptt;
    ptt.client_id = ClientId;
    ptt.instr_code = ui->positionTableWidget->item(row, 0)->text().toStdString();
    ptt.total_amount = ui->positionTableWidget->item(row, 1)->text().toDouble();
    ptt.available_amount = ui->positionTableWidget->item(row, 2)->text().toInt();
    ptt.frozen_amount = ui->positionTableWidget->item(row, 3)->text().toDouble();
    ptt.average_price = ui->positionTableWidget->item(row, 4)->text().toDouble();
    ptt.long_short = ui->positionTableWidget->item(row, 6)->text()=="买"?LONG_ORDER : SHORT_ORDER;
    ptt.offset_price = rpc->get_close_price(ptt);
    QouteTrans qt;
    qt.instr_code = ptt.instr_code;
    qt.ask_price = ptt.offset_price;
    qt.ask_volume = ptt.total_amount;
    qt.bid_price = ptt.offset_price;
    qt.bid_volume = ptt.total_amount;


    orderPlaceDialog opd(rpc, &qt, true, ptt.long_short);
    connect(&opd, SIGNAL(orderPlaced()), this, SLOT(onRefreshPositionButtonClicked()));
    opd.exec();
}

void MainWindow::onUpdateTimerTimeout() {
    timed_future = QtConcurrent::run(this, &MainWindow::updateQouteAndBalance);
}

std::shared_ptr<ClientServiceClient> MainWindow::getThriftClient() {
    boost::shared_ptr<TTransport> socket(new TSocket(SERVER_ADDR, 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    std::shared_ptr<ClientServiceClient> client = make_shared<ClientServiceClient>(protocol);

    transport->open();
    return client;
}

void MainWindow::updateQouteAndBalance() {
    updateQoute();
    updateBalance();
}


void MainWindow::onRefreshPositionButtonClicked() {
    position_future = QtConcurrent::run(this, &MainWindow::updatePositionInfo);
}

void MainWindow::onRefreshIntraDayTransactionButtonClicked() {
    intraday_transaction_future = QtConcurrent::run(this, &MainWindow::updateIntraDayTransactionInfo);
}
void MainWindow::onRefreshHistTransactionButtonClicked() {
    hist_transaction_future = QtConcurrent::run(this, &MainWindow::updateHistTransactionInfo);
}
void MainWindow::onRefreshIntraDayOrderButtonClicked() {
    intraday_order_future = QtConcurrent::run(this, &MainWindow::updateIntraDayOrderInfo);
}
void MainWindow::onRefreshHistOrderButtonClicked() {
    hist_order_future = QtConcurrent::run(this, &MainWindow::updateHistOrderInfo);
}
