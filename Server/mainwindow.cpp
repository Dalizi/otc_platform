#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <map>
#include <vector>
#include <QString>
#include <QStringList>
#include <QMessageBox>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

using namespace std;

MainWindow::MainWindow(boost::shared_ptr<TradeManager> tm, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tm(tm),
    acd(new AddClientDialog(tm)),
    opd(new orderPlaceDialog(tm))
{
    ui->setupUi(this);
    connect(ui->actionAdd_Client, SIGNAL(triggered()), acd, SLOT(open()));
    connect(ui->actionPlaceOrder, SIGNAL(triggered()), this, SLOT(onAddClientMenuTriggered()));
    connect(this, SIGNAL(currentClientID(int)), opd, SLOT(onAddClient(int)));
    connect(acd, SIGNAL(clientAdded(QString)), this, SLOT(onClientAdded(QString)));
    connect(ui->khmcComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCurrentIndexChanged(QString)));
    connect(ui->selectClientComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(refreshTransactionPositionInfo()));
    connect(opd, SIGNAL(orderPlaced()), this, SLOT(onOrderPlaced()));
	connect(ui->refreshPushButton, SIGNAL(clicked()), this, SLOT(onRefreshButtonClicked()));
    connect(ui->refreshPushButton_1, SIGNAL(clicked()), this, SLOT(onRefreshButtonClicked()));
    connect(&lcz_timer, SIGNAL(timeout()), this, SLOT(redisWriteClientGreeks()));
    connect(ui->initClientBalancePushButton, SIGNAL(clicked()), this, SLOT(onResetBalanceButtonClicked()));
    connect(this, SIGNAL(resetBalance(int)), tm.get(), SLOT(resetClientBalance(int)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateClientBalance()));
    connect(ui->settlePushButton, SIGNAL(clicked()), tm.get(), SLOT(settleProgram()));
    redisWriteClientGreeks();
    ui->mainAccountPositionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->reportedOrderTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->positionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->transactionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    timer.start(3000);
    lcz_timer.start(1000 * 60);
    initClientInfo();
    updateClientBalance();
    refreshTransactionPositionInfo();
	initMainAccountInfo();
	

}


MainWindow::~MainWindow()
{
    delete ui;
    delete acd;
	delete opd;
}



void MainWindow::initClientInfo() {
    auto client_list = tm->getClientList();
    if (client_list.empty()) return;
    for (auto qname:client_list) {
        cidStringList.append(qname);
    }
    ui->khmcComboBox->addItems(cidStringList);
    ui->selectClientComboBox->addItems(cidStringList);
    displayClientInfo(ui->khmcComboBox->currentText());

}

void MainWindow::initMainAccountInfo() {
    auto positions = tm->getAllMainAccountPosition();
    ui->mainAccountPositionTableWidget->setRowCount(positions.size());
    int row = 0;
	PositionRisk pr;
    for (auto p: positions) {
		auto greeks = tm->getGreeks(p);
		pr.delta += greeks.delta;
		pr.gamma += greeks.gamma;
		pr.theta += greeks.theta;
		pr.vega += greeks.vega;
        setPositionLine(ui->mainAccountPositionTableWidget, p, row++, true);
    }
	ui->deltaLabel->setText(QString::number(pr.delta));
	ui->gammaLabel->setText(QString::number(pr.gamma));
	ui->thetaLabel->setText(QString::number(pr.theta));
	ui->vegaLabel->setText(QString::number(pr.vega));
}

void MainWindow::displayClientInfo(const QString &cur_text) {
    auto cur_info = tm->getClientInfo(cur_text);
    stringstream ss;
    ss <<setfill('0') <<setw(10) <<cur_info.client_id;
    ui->khbhLineEdit->setText(QString::fromStdString(ss.str()));
    ui->ftxxLineEdit->setText(cur_info.interview_record);
    ui->syzlLineEdit->setText(cur_info.review_material);
    ui->gkxxLineEdit->setText(cur_info.public_info);
    ui->khgxLineEdit->setText(cur_info.client_relationship);
    ui->khywdjLineEdit->setText(QString::number(cur_info.client_level));
    ui->sxedLineEdit->setText(QString::number(cur_info.trust_value));
    ui->ywfwLineEdit->setText(cur_info.chartered_business);
}


void MainWindow::onClientAdded(QString client_name) {
    ui->khmcComboBox->addItem(client_name);
}

void MainWindow::onCurrentIndexChanged(const QString &cur_text) {
    displayClientInfo(cur_text);
    //refreshTransactionPositionInfo();
}

void MainWindow::refreshTransactionPositionInfo() {
    int row =0;
    auto current_client = ui->selectClientComboBox->currentText();
    auto position_info = tm->getAllPosition(current_client);
    ui->positionTableWidget->setRowCount(position_info.size());
    for (auto p:position_info) {
        setPositionLine(ui->positionTableWidget, p, row++, false);
    }
    row = 0;
    int client_id = tm->getIDFromName(current_client);
    auto transaction_info = tm->getTransaction(client_id);
    ui->transactionTableWidget->setRowCount(transaction_info.size());
    for (auto t:transaction_info) {
        setTransactionLine(ui->transactionTableWidget, t, row++);
    }

}

void MainWindow::refreshOrderInfo() {
	int row = 0;
	auto order_info = tm->getAllOrders();
	ui->reportedOrderTableWidget->setRowCount(order_info.size());
	for (auto o : order_info) {
		setOrderLine(ui->reportedOrderTableWidget, o, row++);
	}
}

void MainWindow::setPositionLine(QTableWidget *qtw, PositionType pbt, int row, bool is_main) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(pbt.instr_code));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.total_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.available_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.frozen_amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.average_price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(tm->getPnL(pbt, is_main))));
    qtw->setItem(row, column++, new QTableWidgetItem((int)pbt.long_short==0?"Buy":"Sell"));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(pbt.occupied_margin)));
	if (is_main) {
		auto greeks = tm->getGreeks(pbt);
		qtw->setItem(row, column++, new QTableWidgetItem(QString::number(greeks.delta)));
		qtw->setItem(row, column++, new QTableWidgetItem(QString::number(greeks.gamma)));
		qtw->setItem(row, column++, new QTableWidgetItem(QString::number(greeks.vega)));
		qtw->setItem(row, column++, new QTableWidgetItem(QString::number(greeks.theta)));
	}
}

void MainWindow::setTransactionLine(QTableWidget *qtw, TransactionType ot, int row) {
    int column = 0;
    qtw->setItem(row, column++, new QTableWidgetItem(ot.transaction_id));
    qtw->setItem(row, column++, new QTableWidgetItem(ot.instr_code));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.price)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.long_short == LONG ? "Buy" : "Sell")));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.open_offset==OPEN?"Open":"Offset")));
    qtw->setItem(row, column++, new QTableWidgetItem(ot.time.toString("yyyy-MM-dd hh:mm:ss")));

}

void MainWindow::setOrderLine(QTableWidget *qtw, OrderType ot, int row) {
	int column = 0;
	qtw->setItem(row, column++, new QTableWidgetItem(ot.order_id));
	qtw->setItem(row, column++, new QTableWidgetItem(ot.instr_code));
	qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.price)));
	qtw->setItem(row, column++, new QTableWidgetItem(QString::number(ot.amount)));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.long_short == LONG ? "Buy" : "Sell")));
    qtw->setItem(row, column++, new QTableWidgetItem(QString::fromStdString(ot.open_offset == OPEN ? "Open" : "Offset")));
	qtw->setItem(row, column++, new QTableWidgetItem(ot.time.toString("yyyy-MM-dd hh:mm:ss")));
	qtw->setItem(row, column++, new QTableWidgetItem(order_status_strs[(int)ot.order_status]));
}

void MainWindow::onOrderPlaced() {
	refreshTransactionPositionInfo();
	initMainAccountInfo();
}

void MainWindow::onAddClientMenuTriggered() {
    emit currentClientID(ui->khbhLineEdit->text().toInt());
}

void MainWindow::refreshRevenue(QTableWidget *qtw, bool isMain) {
    auto nrow = qtw->rowCount();
    for (auto i = 0; i != nrow; ++i) {
		PositionType pt;
		pt.instr_code = qtw->item(i, 0)->text();
        pt.total_amount = qtw->item(i, 1)->text().toInt();
        pt.average_price = qtw->item(i, 4)->text().toDouble();
        pt.long_short = qtw->item(i, 6)->text()=="Buy"?LONG:SHORT;
		auto revenue = tm->getPnL(pt, isMain);
        qtw->item(i, 5)->setText(QString::number(revenue));
    }
}

void MainWindow::onRefreshButtonClicked() {
	refreshTransactionPositionInfo();
	initMainAccountInfo();
    refreshRevenue(ui->positionTableWidget, false);
    refreshRevenue(ui->mainAccountPositionTableWidget, true);
}


void MainWindow::onErrOccured(QString msg) {
	QMessageBox msgBox;
	msgBox.setText(msg);
	msgBox.exec();
}

void MainWindow::redisWriteClientGreeks() {
	tm->redisWriteClientGreeks(2);
	
}

void MainWindow::onResetBalanceButtonClicked() {
    int client_id = ui->khbhLineEdit->text().toInt();
    emit resetBalance(client_id);
}

void MainWindow::updateClientBalance() {
    int client_id = ui->khbhLineEdit->text().toInt();
    auto balance = tm->getBalance(client_id);
    ui->marketValueBalanceLineEdit->setText(QString::number(tm->getMarketValueBalance(client_id)));
    ui->totalBalanceLineEdit->setText(QString::number(balance.total_balance));
    ui->occupiedMarginLineEdit->setText(QString::number(balance.occupied_margin));
    ui->withdrawableBalanceLineEdit->setText(QString::number(balance.withdrawable_balance));
    ui->availableBalanceLineEdit->setText(QString::number(tm->getAvailableBalance(client_id)));
    ui->frozenBalanceLineEdit->setText(QString::number(tm->getFrozenBalance(client_id)));
    ui->marginRatioLineEdit->setText(QString::number(tm->getMarginRiskRatio(client_id)));
}
