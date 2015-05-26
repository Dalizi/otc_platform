#include "trademanager.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cassert>
#include <ctime>
#include <stdexcept>
#include <QSqlQuery>
#include <QVariant>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include <QDir>

#include "valuation_class.h"


using namespace std;

TradeManager::TradeManager(QObject *parent) :calc_server(new Option_Value("TradeDate.txt", this))
{
    openDB();
	calcRun();
    //int iRet = redis.Connect("10.2.6.31", 6379, "Finders6");
    int iRet = redis.Connect("127.0.0.1", 6379);
    if (iRet != 0) {
        stringstream ss;
        ss << "Redis Error: " <<iRet;
        QMessageBox::about(0, "ERROR", QString::fromStdString(ss.str()));
        exit(1);
    }

}

TradeManager::~TradeManager() {
	delete calc_server;
}

void TradeManager::loadContractTypeFile(const string &in_file) {
	ifstream f(in_file);
	if (f.fail()) {
		errMsgBox("Fail loading contract type file.");
	}
	string line;
	while (getline(f, line)) {
		string type;
		string code;
		stringstream ss(line);
		ss >> code;
		ss >> type;
		instr_code_type[code] = type;
	}

}

void TradeManager::calcRun() {

	calc_server->Init();
	PositionType temp_position;
	temp_position.average_price = 200;
	temp_position.client_id = 1;
	temp_position.instr_code = "OTC-IFX03C00-2015-06-03-3500";
	temp_position.total_amount = 10;
    temp_position.long_short = LONG;
	calc_server->Total_Position.push_back(temp_position);

	temp_position.average_price = 120;
	temp_position.client_id = 1;
	temp_position.instr_code = "OTC-IFX03P00-2015-06-05-3450";
	temp_position.total_amount = 5;
    temp_position.long_short = LONG;
	calc_server->Total_Position.push_back(temp_position);

	temp_position.average_price = 240;
	temp_position.client_id = 1;
	temp_position.instr_code = "OTC-IFX03C00-2015-06-011-3570";
	temp_position.total_amount = 13;
    temp_position.long_short = SHORT;
	calc_server->Total_Position.push_back(temp_position);

	calc_server->Start();
}

inline
void TradeManager::errMsgBox(const string &msg) {
    QMessageBox::warning(0, "Warning", QString::fromStdString(msg));
}

void TradeManager::addPosition(const PositionType &pt) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO position (id, long_short, instr_code, "
                  "total_amount, available_amount, frozen_amount,"
                  " average_price, underlying_price, occupied_margin)"
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(pt.client_id);
    query.addBindValue((int)pt.long_short);
    query.addBindValue(pt.instr_code);
    query.addBindValue(pt.total_amount);
    query.addBindValue(pt.available_amount);
    query.addBindValue(pt.frozen_amount);
    query.addBindValue(pt.average_price);
    query.addBindValue(pt.underlying_price);
    query.addBindValue(pt.occupied_margin);
    if (!query.exec()) {
        qDebug() <<"INSERTION FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
    }

}

void TradeManager::addMainPosition(const PositionType &pt) {
	QSqlQuery query(db);
	query.prepare("INSERT INTO total_position (long_short, instr_code, "
		"total_amount, available_amount, frozen_amount,"
		" average_price)"
		"VALUES (?, ?, ?, ?, ?, ?)");
	query.addBindValue(pt.long_short);
	query.addBindValue(pt.instr_code);
	query.addBindValue(pt.total_amount);
	query.addBindValue(pt.available_amount);
	query.addBindValue(pt.frozen_amount);
	query.addBindValue(pt.average_price);
	if (!query.exec()){
		qDebug() << "addMainPosition FAILED..." << " " << query.lastQuery() << " " << query.lastError();
	}

}

string TradeManager::addOrder(OrderType &ot) {
	QSqlQuery query(db);
	query.prepare("INSERT INTO orders (id, instr_code, client_id, price,"
        "amount, long_short, open_offset, order_status)"
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    stringstream ss;
    auto time = QDateTime::currentDateTime();
    ot.time = time;
    ss <<QDate::currentDate().toString("yyyyMMdd").toStdString() <<setfill('0') <<setw(8)<<getOrderIndex();
    query.addBindValue(QString::fromStdString(ss.str()));
	query.addBindValue(ot.instr_code);
	query.addBindValue(ot.client_id);
	query.addBindValue(ot.price);
	query.addBindValue(ot.amount);
	query.addBindValue(ot.long_short);
	query.addBindValue((int)ot.open_offset);
	query.addBindValue((int)ot.order_status);
    //query.addBindValue(time.toString("yyyy-MM-dd HH:mm:ss"));
	if (!query.exec())
		qDebug() << "addOrder FAILED..." << " " << query.lastQuery() << " " << query.lastError();
    return ss.str();
}

vector<TransactionType> TradeManager::getTransaction(int client_id) {
    vector<TransactionType> ret;
    QSqlQuery query(db);
    query.prepare("SELECT * FROM transactions WHERE client_id=?");
    query.addBindValue(client_id);
    if (!query.exec())
        qDebug() <<"GET ORDER FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
    while (query.next()) {
        TransactionType ot;
        ot.transaction_id = query.value(0).toString();
        ot.instr_code = query.value(1).toString();
        ot.time = query.value(2).toDateTime();
        ot.client_id = client_id;
        ot.price = query.value(4).toDouble();
        ot.amount = query.value(5).toInt();
        ot.long_short = (LongShortType)query.value(6).toInt();
        ot.open_offset = (OpenOffsetType)query.value(7).toInt();
		ot.underlying_price = query.value(8).toDouble();
        ot.close_pnl = query.value("close_pnl").toDouble();
        ret.push_back(ot);

    }
    return ret;
}

vector<OrderType> TradeManager::getOrder(int client_id) {
	vector<OrderType> ret;
	QSqlQuery query(db);
	query.prepare("SELECT * FROM orders WHERE client_id=?");
	query.addBindValue(client_id);
	if (!query.exec())
		qDebug() << "GET ORDER FAILED..." << " " << query.lastQuery() << " " << query.lastError();
	while (query.next()) {
		OrderType ot;
		ot.order_id = query.value(0).toString();
		ot.instr_code = query.value(1).toString();
		ot.time = query.value(2).toDateTime();
		ot.client_id = client_id;
		ot.price = query.value(4).toDouble();
		ot.amount = query.value(5).toInt();
        ot.long_short = (LongShortType)query.value(6).toInt();
        ot.open_offset = (OpenOffsetType)query.value(7).toInt();
        ot.order_status = (OrderStatusType)query.value(8).toInt();
		ret.push_back(ot);

	}
	return ret;
}

vector<OrderType> TradeManager::getAllOrders() {
	vector<OrderType> ret;
	QSqlQuery query(db);
	query.prepare("SELECT * FROM orders");
	if (!query.exec())
		qDebug() << "GET ORDER FAILED..." << " " << query.lastQuery() << " " << query.lastError();
	while (query.next()) {
		OrderType ot;
		ot.order_id = query.value(0).toString();
		ot.instr_code = query.value(1).toString();
		ot.time = query.value(2).toDateTime();
		ot.client_id = query.value(3).toInt();
		ot.price = query.value(4).toDouble();
		ot.amount = query.value(5).toInt();
        ot.long_short = (LongShortType)query.value(6).toInt();
        ot.open_offset = (OpenOffsetType)query.value(7).toInt();
        ot.order_status = (OrderStatusType)query.value(8).toInt();
		ret.push_back(ot);

	}
	return ret;
}

void TradeManager::acceptOrder(const OrderType &ot) {
    TransactionType tt;
    tt.transaction_id = ot.order_id;
    tt.amount = ot.amount;
    tt.instr_code = ot.instr_code;
    tt.client_id = ot.client_id;
    tt.long_short = ot.long_short;
    tt.open_offset = ot.open_offset;
    tt.price = ot.price;
    tt.underlying_price = calc_server->getUnderlyingPrice(ot.instr_code.toStdString());
    changeOrderStatus(ot.order_id.toStdString(), 1);
    updateBalance(tt);
    setPosition(tt);
}



void TradeManager::rejectOrder(const OrderType &ot) {
    changeOrderStatus(ot.order_id.toStdString(), 2);
}

void TradeManager::cancelOrder(const OrderType &ot) {
    changeOrderStatus(ot.order_id.toStdString(), 3);
}

inline
int TradeManager::getOrderStatus(const string &order_id) {
	QSqlQuery query(db);
	query.prepare("SELECT order_status FROM orders WHERE id=?");
	query.addBindValue(QString::fromStdString(order_id));
	if (!query.exec())
		qDebug() << "changeOrderStatus FAILED..." << " " << query.lastQuery() << " " << query.lastError();
	if (query.next())
		return query.value(0).toInt();
	else {
		stringstream ss;
		ss << "Order: " << order_id << " does not exist.";
		errMsgBox(ss.str());
	}
}

inline
void TradeManager::changeOrderStatus(const string &order_id, int status) {
	/*
	* 0:已报
	* 1:已成
	* 2:未成
	* 3:撤单
	*/
	QSqlQuery query(db);
	query.prepare("UPDATE orders SET order_status=? WHERE id=?");
	query.addBindValue(status);
	query.addBindValue(QString::fromStdString(order_id));
	if (!query.exec())
		qDebug() << "changeOrderStatus FAILED..." << " " << query.lastQuery() << " " << query.lastError();
}

PositionType TradeManager::getPosition(int client_id, const QString &instr_code, LongShortType long_short) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM position WHERE id=? AND instr_code=? AND long_short=?");
    query.addBindValue(client_id);
    query.addBindValue(instr_code);
    query.addBindValue((int)long_short);
    if (!query.exec())
        qDebug() <<"INSERTION FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
    PositionType pt;
    if (query.next()) {
        pt.client_id = query.value(0).toInt();
        pt.instr_code = query.value(1).toString();
        pt.average_price = query.value(2).toDouble();
        pt.total_amount = query.value(3).toInt();
        pt.available_amount = query.value(4).toInt();
        pt.frozen_amount = query.value(5).toInt();
        pt.long_short = (LongShortType)query.value(6).toInt();
        pt.underlying_price = query.value(7).toDouble();
        pt.occupied_margin = query.value(8).toDouble();
    }
    return pt;
}



vector<PositionType> TradeManager::getAllPosition(int client_id) {
    vector<PositionType> ret;
    QSqlQuery query(QString("SELECT * FROM position WHERE id=%1").arg(client_id), db);
    while (query.next()) {
        PositionType pt;
        pt.client_id = query.value(0).toInt();
        pt.instr_code = query.value(1).toString();
        pt.average_price = query.value(2).toDouble();
        pt.total_amount = query.value(3).toInt();
        pt.available_amount = query.value(4).toInt();
        pt.frozen_amount = query.value(5).toInt();
        pt.long_short = (LongShortType)query.value(6).toInt();
        pt.underlying_price = query.value("underlying_price").toDouble();
        pt.occupied_margin = query.value("occupied_margin").toDouble();
        ret.push_back(pt);

    }
    return ret;
}

vector<PositionType> TradeManager::getAllPosition(const QString &client_name) {
    vector<PositionType> ret;
    QSqlQuery query(db);
    auto client_id = getIDFromName(client_name);
    query.prepare("SELECT * FROM position WHERE id=:id");
    query.bindValue(":id", client_id);
    if (!query.exec())
        qDebug() <<query.lastQuery() <<" FAILED..." <<" ERROR:" <<query.lastError();
    while (query.next()) {
        PositionType pt;
        pt.client_id = query.value(0).toInt();
        pt.instr_code = query.value(1).toString();
        pt.average_price = query.value(2).toDouble();
        pt.total_amount = query.value(3).toInt();
        pt.available_amount = query.value(4).toInt();
        pt.frozen_amount = query.value(5).toInt();
        pt.long_short = (LongShortType)query.value(6).toInt();
        pt.underlying_price = query.value("underlying_price").toDouble();
        pt.occupied_margin = query.value("occupied_margin").toDouble();
        ret.push_back(pt);

    }
    return ret;
}

ClientInfo TradeManager::getClientInfo(const QString &client_name) {
    QSqlQuery query(db);
    query.prepare("SELECT * FROM client WHERE name = :name");
    query.bindValue(":name", client_name);
    if (!query.exec())
        qDebug() <<query.lastQuery() <<" FAILED..." <<" ERROR:" <<query.lastError();
    ClientInfo ci;
    if (query.next()) {
       ci.client_id =  query.value(0).toInt();
       ci.client_name = query.value(1).toString();
       ci.interview_record = query.value(2).toString();
       ci.review_material = query.value(3).toString();
       ci.public_info = query.value(4).toString();
       ci.client_relationship = query.value(5).toString();
       ci.client_level = query.value(6).toInt();
       ci.trust_value = query.value(7).toDouble();
       ci.chartered_business = query.value(8).toString();
    }
    return ci;
}

ClientInfo TradeManager::getClientInfo(const int client_id) {
	QSqlQuery query(db);
	query.prepare("SELECT * FROM client WHERE id = ?");
	query.addBindValue(client_id);
	if (!query.exec())
		qDebug() << query.lastQuery() << " FAILED..." << " ERROR:" << query.lastError();
	ClientInfo ci;
	if (query.next()) {
		ci.client_id = query.value(0).toInt();
		ci.client_name = query.value(1).toString();
		ci.interview_record = query.value(2).toString();
		ci.review_material = query.value(3).toString();
		ci.public_info = query.value(4).toString();
		ci.client_relationship = query.value(5).toString();
		ci.client_level = query.value(6).toInt();
		ci.trust_value = query.value(7).toDouble();
		ci.chartered_business = query.value(8).toString();
	}
	return ci;
}

ClientBalance TradeManager::getClientBalance(int client_id) {
    QSqlQuery query(QString("SELECT * FROM balance WHERE name = %1").arg(client_id),db);
    ClientBalance cb;
    if (query.next()) {
       cb.client_id =  query.value(0).toInt();
       cb.total_balance = query.value(1).toDouble();
       cb.available_balance = query.value(2).toDouble();
       cb.withdrawable_balance = query.value(3).toDouble();
       cb.occupied_margin = query.value(4).toDouble();
    } else {
        QMessageBox mb;
        mb.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
        mb.setText("ERROR Reading database: No such client.");
    }
    return cb;
}

void TradeManager::setTransaction(const TransactionType &tt) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO transactions (id, instr_code, "
                  "client_id, price, amount, long_short, open_offset, underlying_price, close_pnl)"
                  " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(tt.transaction_id);
    query.addBindValue(tt.instr_code);
    //query.addBindValue(ot.time.toString("yyyyMMddhhmmss"));
    query.addBindValue(tt.client_id);
    query.addBindValue(tt.price);
    query.addBindValue(tt.amount);
    query.addBindValue((int)tt.long_short);
    query.addBindValue((int)tt.open_offset);
    query.addBindValue(tt.underlying_price);
    query.addBindValue(tt.close_pnl);

    if (!query.exec())
        qDebug() <<"INSERTION FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
}

void TradeManager::setPosition(TransactionType tt) {
    PositionType pt = getPosition(tt.client_id, tt.instr_code, tt.getPositionDirect());
    if (tt.open_offset == OFFSET)
        tt.close_pnl = (tt.price - pt.average_price) * tt.amount * getMultiplier(tt.instr_code.toStdString());
    else
        tt.close_pnl = 0;
    setTransaction(tt);
	if (pt.instr_code == "") {
        string underlying_code = calc_server->getUnderlyingCode(tt.instr_code.toStdString());
        int multiplier = calc_server->main_contract.multiplier;
        pt.instr_code = tt.instr_code;
        pt.client_id = tt.client_id;
        pt.long_short = tt.long_short;
        pt.total_amount = tt.amount;
        pt.frozen_amount = 0;
        pt.available_amount = tt.amount;
        pt.average_price = tt.price;
        pt.underlying_price = calc_server->getUnderlyingPrice(tt.instr_code.toStdString());
        if (tt.long_short == SHORT)
            pt.occupied_margin = calc_server->Settle_Price(underlying_code, tt.long_short)
                                * tt.amount*multiplier
                                * getMarginRate(tt.client_id, tt.instr_code.toStdString()) + tt.amount*tt.price*multiplier;
		addPosition(pt);

    } else {
        updatePosition(pt, tt);
    }
}

void TradeManager::setPosition(const PositionType &pt) {
	if (pt.total_amount == 0) {
		deletePosition(pt);
		return;
	}
    QSqlQuery query(db);
    query.prepare("UPDATE position SET average_price=?, total_amount=?, available_amount=?, frozen_amount=?, underlying_price=?"
                                "WHERE id=? AND instr_code=? AND long_short=?");
    query.addBindValue(pt.average_price);
    query.addBindValue(pt.total_amount);
    query.addBindValue(pt.available_amount);
    query.addBindValue(pt.frozen_amount);
    query.addBindValue(pt.underlying_price);
    query.addBindValue(pt.client_id);
    query.addBindValue(pt.instr_code);
    query.addBindValue((int)pt.long_short);
    if (!query.exec())
        qDebug() <<"UPDATE position FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
}

void TradeManager::deletePosition(const PositionType &pt) {
	QSqlQuery query(db);
	query.prepare("DELETE FROM position WHERE id=? AND instr_code=? AND long_short=?");
	query.addBindValue(pt.client_id);
	query.addBindValue(pt.instr_code);
	query.addBindValue((int)pt.long_short);
	if (!query.exec())
		errMsgBox("删除持仓失败。");
}

void TradeManager::setMainPosition(const PositionType &pt) {
	QSqlQuery query(db);
	query.prepare("UPDATE total_position SET average_price=?, total_amount=?, available_amount=?, frozen_amount=?"
		"WHERE instr_code=? AND long_short=?");
	query.addBindValue(pt.average_price);
	query.addBindValue(pt.total_amount);
	query.addBindValue(pt.available_amount);
	query.addBindValue(pt.frozen_amount);
	query.addBindValue(pt.instr_code);
	query.addBindValue((int)pt.long_short);
	if (!query.exec())
		qDebug() << "UPDATE total_position FAILED..." << " " << query.lastQuery() << " " << query.lastError();

}

int TradeManager::setClientInfo(const ClientInfo &ci) {
    QSqlQuery query(db);
    query.prepare(QString("INSERT INTO client (name, interview_record, "
                  "review_material, public_info, client_relationship,"
                  " client_level, entrust_value, business_domain)"
                  "VALUES (:name, :interview_record,"
                  ":review_material, :public_info, :client_relationship,"
                  ":client_level, :entrust_value, :business_domain)"));
    query.bindValue(":name", ci.client_name);
    query.bindValue(":interview_record", ci.interview_record);
    query.bindValue(":review_material", ci.review_material);
    query.bindValue(":public_info", ci.public_info);
    query.bindValue(":client_relationship", ci.client_relationship);
    query.bindValue(":client_level", ci.client_level);
    query.bindValue(":entrust_value", ci.trust_value);
    query.bindValue(":business_domain", ci.chartered_business);
    if (!query.exec())
        qDebug() <<"INSERTION client FAILED... "<<query.lastQuery()  <<" " <<query.lastError();
	query.prepare("SELECT id FROM client WHERE name=?");
	query.addBindValue(ci.client_name);
	if (!query.exec())
		qDebug() << "get client id FAILED... " << query.lastQuery() << " " << query.lastError();
	int client_id;
	if (query.next()) {
		client_id = query.value(0).toInt();
	}

	query.prepare("INSERT INTO passwd (client_id, password)"
		"VALUES (?, ?)");
	query.addBindValue(client_id);
	query.addBindValue("999999");
	if (!query.exec())
		qDebug() << "INSERTION passwd FAILED... " << query.lastQuery() << " " << query.lastError();

	return client_id;

}

void TradeManager::initClientBalance(int client_id, double init_balance) {
	QSqlQuery query(db);
	query.prepare("INSERT INTO balance (id, total_balance, available_balance, occupied_margin, withdrawable_balance)"
		"VALUES (?, ?, ?, ?, ?)");
	query.addBindValue(client_id);
	query.addBindValue(init_balance);
	query.addBindValue(init_balance);
	query.addBindValue(0);
	query.addBindValue(init_balance);
	if (!query.exec())
		qDebug() << "INITIALIZING balance FAILED... " << query.lastQuery() << " " << query.lastError();
}

void TradeManager::setClientBalance(const ClientBalance &cb) {
	QSqlQuery  query(db);
    query.prepare("UPDATE balance SET total_balance=?, available_balance=?, withdrawable_balance=?, occupied_margin=?  WHERE id=?");
	query.addBindValue(cb.total_balance);
	query.addBindValue(cb.available_balance);
	query.addBindValue(cb.withdrawable_balance);
	query.addBindValue(cb.occupied_margin);
	query.addBindValue(cb.client_id);
	if (!query.exec())
		qDebug() << "SET balance FAILED... " << query.lastQuery() << " " << query.lastError();


}


std::vector<QString> TradeManager::getClientList() {
    vector<QString> ret;
    QSqlQuery query("SELECT name FROM client", db);
    while (query.next()) {
        QString name = query.value(0).toString();
        ret.push_back(name);
    }
    return ret;
}

ClientBalance TradeManager::getBalance(int client_id) {
	QSqlQuery  query(db);
	ClientBalance cb;
	query.prepare("SELECT total_balance, available_balance, withdrawable_balance, occupied_margin FROM balance WHERE id=?");
	query.addBindValue(client_id);
	if (!query.exec())
		qDebug() << "GET balance FAILED... " << query.lastQuery() << " " << query.lastError();
	if (query.next()) {
		cb.total_balance = query.value(0).toDouble();
		cb.available_balance = query.value(1).toDouble();
		cb.withdrawable_balance = query.value(2).toDouble();
		cb.occupied_margin = query.value(3).toDouble();
	}
	else {
		QMessageBox qMsgBox;
		qMsgBox.setText("No balance info");
		qMsgBox.exec();
	}
	return cb;
}

double TradeManager::getTotalBalance(int client_id) {
	return 0;
}


int TradeManager::getTransactionID() {
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM transactions");
    if (!query.exec())
        qDebug() <<"Get Order ID Failed... "<<query.lastQuery()  <<" " <<query.lastError();
    if (query.next())
        return query.value(0).toInt();
}



bool TradeManager::openDB()
{
    // Find QSLite driver
    db = QSqlDatabase::addDatabase("QSQLITE");

#ifdef Q_OS_LINUX
    // NOTE: We have to store database file into user home folder in Linux
    QString path("OTC.db");
    //path.append(QDir::separator()).append("OTC.db");
    //path = QDir::toNativeSeparators(path);
    db.setDatabaseName(path);
#else
    // NOTE: File exists in the application private folder, in Symbian Qt implementation
    db.setDatabaseName("OTC.db");
#endif

    // Open databasee
    bool ret = db.open();
	QSqlQuery query(db);
    query.prepare("CREATE table if not exists client ("
                    "id integer PRIMARY KEY AUTOINCREMENT,"
                    "name varchar(128),"
                    "interview_record varchar(2048),"
                    "review_material varchar(2048),"
                    "public_info varchar(2048),"
                    "client_relationship varchar(2048),"
                    "client_level integer,"
                    "entrust_value double,"
                    "business_domain varcha(2048)"
                    ");");
    if (!query.exec())
		qDebug() << "CREATE client table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

    query.prepare("CREATE table if not exists balance ("
                    "id integer PRIMARY KEY,"
                    "total_balance double,"
                    "available_balance double,"
                    "withdrawable_balance double,"
					"occupied_margin double"
                    ");");
    if (!query.exec())
		qDebug() << "CREATE balance table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

    query.prepare("CREATE table if not exists transactions ("
                    "id varchar(33),"
                    "instr_code varchar(20),"
					"time timestamp NOT NULL DEFAULT (datetime(CURRENT_TIMESTAMP, 'localtime')),"
                    "client_id integer,"
                    "price double,"
                    "amount integer,"
                    "long_short integer,"
                    "open_offset integer,"
                    "underlying_price double,"
                    "close_pnl double"
                    ");");
	if (!query.exec())
		qDebug() << "CREATE transaction table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

	query.prepare("CREATE table if not exists orders ("
		"id varchar(33),"
		"instr_code varchar(20),"
		"time timestamp NOT NULL DEFAULT (datetime(CURRENT_TIMESTAMP, 'localtime')),"
		"client_id integer,"
		"price double,"
		"amount integer,"
		"long_short integer,"
		"open_offset integer,"
		"order_status integer"
		");");
	if (!query.exec())
		qDebug() << "CREATE transaction table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

    query.prepare("CREATE table if not exists position ("
                    "id integer,"
                    "instr_code varchar(20),"
                    "average_price double,"
                    "total_amount integer,"
                    "available_amount integer,"
                    "frozen_amount integer,"
                    "long_short integer,"
                    "occupied_margin double"
                    ");");
    if (!query.exec())
        qDebug() << "CREATE position table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

	query.prepare("CREATE table if not exists passwd ("
		"client_id integer,"
		"password varchar(20) NOT NULL DEFAULT '999999'"
		");");
	if (!query.exec())
		qDebug() << "CREATE passwd table FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();

	query.prepare("CREATE table if not exists margin_rate ("
		"client_id integer,"
		"instr_code varchar(30),"
		"margin_rate double"
		");");
	if (!query.exec())
		errMsgBox("CREATE margin_rate table FAILED...");
    return ret;
}

QSqlError TradeManager::lastError()
{
    // If opening database has failed user can ask
    // error description by QSqlError::text()
    return db.lastError();
}

int TradeManager::getIDFromName(const QString &client_name) {
    QSqlQuery query(QString("SELECT id FROM client WHERE name='%1'").arg(client_name), db);
    int client_id;
    if (query.next()) {
        client_id = query.value(0).toInt();
    }
    return client_id;
}

bool TradeManager::isPositionExist(int client_id, const QString &instr_code, int long_short) {
    QSqlQuery query(QString("SELECT * FROM position WHERE id=%1 AND instr_code=%2 AND long_short=%3").arg(client_id).arg(instr_code).arg(long_short));
    if (query.next()) return true;
    return false;
}

double TradeManager::getPnL(const PositionType &pt, bool isMain) {
	return calc_server->Position_PnL(pt, isMain);
}

vector<Qoute> TradeManager::getQoute() {
	vector<Qoute> ret;
	vector<string> keys;
	redis.Keys("OTC-*", keys);
	for (auto key : keys) {
		Qoute q;
		map<string, string> qoute_map;
		redis.HGetAll(key, qoute_map);
		if (qoute_map.find("code") == qoute_map.end() || qoute_map.find("ask") == qoute_map.end() ||
			qoute_map.find("ask_volume") == qoute_map.end() || qoute_map.find("bid") == qoute_map.end()
			|| qoute_map.find("bid_volume") == qoute_map.end()) continue;

		q.instr_code = QString::fromStdString(qoute_map["code"]);
		q.ask_price = stod(qoute_map["ask"]);
		q.ask_volume = stoi(qoute_map["ask_volume"]);
		q.bid_price = stod(qoute_map["bid"]);
		q.bid_volume = stoi(qoute_map["bid_volume"]);
		ret.push_back(q);
		
	}
	return ret;
}

vector<PositionType> TradeManager::getAllMainAccountPosition() {
    /*
     * 获得所有总账户中的持仓
     */
    vector<PositionType> ret;
    QSqlQuery query(db);
    query.prepare("SELECT instr_code, sum(average_price*total_amount)/sum(total_amount), sum(total_amount), sum(available_amount), sum(frozen_amount), long_short "
                  "FROM position GROUP BY instr_code, long_short");
    if (!query.exec())
        qDebug() << "GET main account positions FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
    while (query.next()) {
        PositionType pt;
        pt.instr_code = query.value(0).toString();
        pt.average_price = query.value(1).toDouble();
        pt.total_amount = query.value(2).toInt();
        pt.available_amount = query.value(3).toInt();
        pt.frozen_amount = query.value(4).toInt();
        pt.long_short = (LongShortType)(query.value(5).toInt()==0?1:0);
        ret.push_back(pt);

    }
    return ret;
}

void TradeManager::updatePosition(const PositionType &pt, const TransactionType &tt) {
    QSqlQuery query(db);
    int adjust_param = tt.open_offset == OpenOffsetType::OPEN ? 1 : -1;
    int total_amount = pt.total_amount + tt.amount * adjust_param;
    string underlying_code = calc_server->getUnderlyingCode(tt.instr_code.toStdString());
    int multiplier = calc_server->main_contract.multiplier;


    double average_price, underlying_price;
    if (total_amount != 0) {
        if (tt.open_offset == OPEN) {
            average_price = (pt.average_price * pt.total_amount + tt.price * tt.amount)/ (pt.total_amount + tt.amount);
            underlying_price = (pt.underlying_price * pt.total_amount + tt.underlying_price * tt.amount) / (pt.total_amount + tt.amount) ;
        } else {
            average_price = pt.average_price;
            underlying_price = pt.underlying_price;
        }

        double occupied_margin = calc_server->Settle_Price(underlying_code, tt.long_short)
                            * tt.amount*multiplier
                            * getMarginRate(tt.client_id, tt.instr_code.toStdString()) + pt.average_price * tt.amount;
        double margin_chng = pt.occupied_margin + (pt.long_short==LONG?0:((tt.open_offset==OPEN?1:-1)*occupied_margin));
        query.prepare("UPDATE position SET total_amount=?, available_amount=?,average_price=?, underlying_price=?, occupied_margin=?"
                      "WHERE instr_code=? AND id=? AND long_short=?");
        query.addBindValue(total_amount);
        query.addBindValue(total_amount);
        query.addBindValue(average_price);
        query.addBindValue(underlying_price);
        query.addBindValue(margin_chng);
        query.addBindValue(pt.instr_code);
        query.addBindValue(pt.client_id);
        query.addBindValue(static_cast<int>(pt.long_short));
    } else {
        query.prepare("DELETE FROM position WHERE instr_code=? AND id=? AND long_short=?");
        query.addBindValue(pt.instr_code);
        query.addBindValue(pt.client_id);
        query.addBindValue(static_cast<int>(pt.long_short));
    }

    if (!query.exec())
        qDebug() << "Update position FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
}

void TradeManager::updateBalance(const TransactionType &tt) {
	QSqlQuery query(db);
    string underlying_code = calc_server->getUnderlyingCode(tt.instr_code.toStdString());
    int multiplier = calc_server->main_contract.multiplier;
    query.prepare("UPDATE balance SET total_balance =total_balance+?,"
                  "occupied_margin=occupied_margin+?,"
                  "withdrawable_balance=withdrawable_balance+?"
                  " WHERE id=?");
    auto pt = getPosition(tt.client_id, tt.instr_code, tt.getPositionDirect());
    if (getInstrType(tt.instr_code.toStdString()) == "option") {
        if (tt.long_short == LONG) {
            if (tt.open_offset == OPEN) {
                double premium = tt.price*tt.amount*multiplier;
                query.addBindValue(-premium);
                query.addBindValue(0);
                query.addBindValue(-premium);
                query.addBindValue(tt.client_id);
			}
			else {
                double premium = pt.average_price*tt.amount*multiplier;
                double cash_flow = getCloseCashFlow(tt);
                double margin = calc_server->Settle_Price(underlying_code, tt.long_short)*tt.amount*multiplier*getMarginRate(tt.client_id, tt.instr_code.toStdString());
                query.addBindValue(cash_flow);
                query.addBindValue(-premium-margin);
                query.addBindValue(margin);
                query.addBindValue(tt.client_id);
			}
		} else {
            if (tt.open_offset == OpenOffsetType::OPEN) {
                double premium = tt.price*tt.amount*multiplier;
                double margin = calc_server->Settle_Price(underlying_code, tt.long_short)*tt.amount*multiplier*getMarginRate(tt.client_id, tt.instr_code.toStdString());
                query.addBindValue(premium);
                query.addBindValue(premium+margin);
                query.addBindValue(-margin);
                query.addBindValue(tt.client_id);
			}
			else {
                auto pos = getPosition(tt.client_id, tt.instr_code, tt.reversePosition());
                double cash_flow = getCloseCashFlow(tt);
                query.addBindValue(cash_flow);
                query.addBindValue(0);
                query.addBindValue(0);
                query.addBindValue(tt.client_id);
			}
		}
	}
	if (!query.exec())
		qDebug() << "Update balance FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
}

void TradeManager::updateBalance(int client_id, double adjust_value) {
	QSqlQuery query(db);

	query.prepare("UPDATE balance SET available_balance=available_balance+?, withdrawable_balance=available_balance, frozen_balance=frozen_balance+? WHERE id=?");
	query.addBindValue(adjust_value);
	query.addBindValue(adjust_value);
	query.addBindValue(adjust_value);
	query.addBindValue(client_id);
	if (!query.exec())
		qDebug() << "Update balance FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
}


int TradeManager::authPassword(int client_id, const string &passwd) {
	QSqlQuery query(db);
	query.prepare("SELECT password FROM passwd WHERE client_id=?");
	query.addBindValue(client_id);
	if (!query.exec())
		qDebug() << "GET password FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
	if (query.next()) {
		auto password = query.value(0).toString().toStdString();
		return password == passwd;
	}
	return -1;
}

int TradeManager::getOrderIndex() {
    QSqlQuery query(db);
    query.prepare("SELECT count(*) FROM orders WHERE time >= ?");
    query.addBindValue(QDate::currentDate().toString("yyyy-MM-dd 00:00:00"));
    if (!query.exec())
        qDebug() << "GET order index FAILED..." << " QUERY is: " << query.lastQuery() << " ERROR is: " << query.lastError();
    if (query.next()) {
        return query.value(0).toInt();
    }
}


void TradeManager::processSingleOrder(const OrderType &ot) {
    if (isOrderValid(ot))
        acceptOrder(ot);
    else
        rejectOrder(ot);
}

bool TradeManager::isOrderValid(const string &order_id) {
	QSqlQuery query(db);
	query.prepare("SELECT client_id, instr_code, open_offset, long_short, amount, price FROM orders WHERE id=?");
	query.addBindValue(QString::fromStdString(order_id));
	if (!query.exec()) {
		stringstream ss;
		ss << "Fail reading order info" << query.lastError().text().toStdString();
		errMsgBox(ss.str());
	}
	OrderType ot;
	if (query.next()) {
		ot.client_id = query.value(0).toInt();
		ot.instr_code = query.value(1).toString();
        ot.open_offset = (OpenOffsetType)query.value(2).toInt();
        ot.long_short = (LongShortType)query.value(3).toInt();
		ot.amount = query.value(4).toInt();
		ot.price = query.value(5).toDouble();
	}
    auto pt = getPosition(ot.client_id, ot.instr_code, ot.getPositionDirect());
    if (ot.open_offset == OFFSET && ot.amount > pt.available_amount)
		return false;
	auto cb = getBalance(ot.client_id);
    if (ot.open_offset == OPEN && ot.long_short == LONG && ot.amount * ot.price > cb.available_balance)
		return false;
    if (ot.open_offset == OPEN && ot.long_short == SHORT &&
		ot.amount * calc_server->Price_Qoute(ot.instr_code.toStdString()) / getMultiplier(ot.instr_code.toStdString()) - ot.amount *ot.price > cb.available_balance)
		return false;
	return true;

}

bool TradeManager::isOrderValid(const OrderType &ot) {
    auto multiplier = calc_server->main_contract.multiplier;
    auto pt = getPosition(ot.client_id, ot.instr_code, ot.getPositionDirect());
    if (ot.open_offset == OFFSET && ot.amount > pt.available_amount)
        return false;
    auto cb = getBalance(ot.client_id);
    if (ot.open_offset == OPEN && ot.long_short == LONG && ot.amount * ot.price*multiplier > getAvailableBalance(ot.client_id))
        return false;
    if (ot.open_offset == OPEN && ot.long_short == SHORT &&
        ot.amount * calc_server->Price_Qoute(ot.instr_code.toStdString()) / getMultiplier(ot.instr_code.toStdString()) - ot.amount *ot.price*multiplier > getAvailableBalance(ot.client_id))
        return false;
    return true;

}


double TradeManager::getMultiplier(const string &instr_code) {
	stringstream ss;
	ss << "PARAM-" << instr_code.substr(0, 9);
	string str_multi;
	redis.HGet(ss.str(), "Multiplier", str_multi);
	return stod(str_multi);
}

/*
double TradeManager::getGrossBalance(int client_id) {
	auto cb = getBalance(client_id);
	return cb.available_balance + cb.occupied_margin + getMarketValue(client_id);
}
*/
double TradeManager::getMarginRiskRatio(int client_id) {
	auto cb = getBalance(client_id);
    return cb.occupied_margin / getMarketValueBalance(client_id);
}

double TradeManager::getMarketValueBalance(int client_id) {
	auto vec_pos = getAllPosition(client_id);
    auto balance = getBalance(client_id);
	double ret=0;
	for (auto pt : vec_pos) {
        ret += getCloseCashFlow(pt);
	}
    return ret+balance.total_balance;
}

inline
double TradeManager::getCloseCashFlow(const PositionType &pt) {
    auto close_price = calc_server->Position_Quote(pt.instr_code.toStdString(), pt.long_short);
    return (pt.long_short==LONG?1:-1) * close_price * pt.total_amount * calc_server->main_contract.multiplier;
}

inline
double TradeManager::getCloseCashFlow(const TransactionType &tt) {
    //auto close_price = calc_server->Position_Quote(ot.instr_code.toStdString(), ot.getPositionDirect());
    return (tt.getPositionDirect()==LONG?1:-1) * tt.price * tt.amount * calc_server->main_contract.multiplier;
}

inline
double TradeManager::getAvailableBalance(int client_id) {
    auto balance = getBalance(client_id);
    return balance.total_balance - balance.occupied_margin - getFrozenBalance(client_id);
}


double TradeManager::getFrozenBalance(int client_id) {
    auto vec_pos = getAllPosition(client_id);
    double ret=0;
    for (auto pt : vec_pos) {
        ret += getPnL(pt, false);
    }
    return max(-ret, 0);
}

map<string, double> TradeManager::getCalculatedBalance(int client_id) {
	map<string, double> ret;
    ret["market_value_balance"] = getMarketValueBalance(client_id);
    ret["available_balance"] = getAvailableBalance(client_id);
    ret["margin_risk_ratio"] = getMarginRiskRatio(client_id);
    ret["frozen_balance"] = getFrozenBalance(client_id);
	return ret;
}

inline
string TradeManager::getInstrType(const string &instr_code) {
	string::size_type pos;
	pos = instr_code.find('-');
	pos = instr_code.find('-', pos+1);
	//return instr_code_type[instr_code.substr(0, pos)];
	return "option";
}

inline
double TradeManager::getMarginRate(int client_id, const string &instr_code) {
	/*QSqlQuery query(db);
	query.prepare("SELECT * FROM margin_rate WHERE client_id=? AND instr_code=?");
	query.addBindValue(client_id);
	query.addBindValue(QString::fromStdString(instr_code));
	if (!query.exec())
		errMsgBox("ERROR reading margin rate");
	if (query.next()) {
		return query.value(1).toDouble();
	}
	else {
		errMsgBox("No margin rate info.");
		return -1;
	}*/
	return 0.05;
}

double TradeManager::getClosePrice(const PositionType &pt) {
	return calc_server->Position_Quote(pt.instr_code.toStdString(), pt.long_short);
}

PositionRisk TradeManager::getGreeks(const PositionType &pt) {
	return calc_server->PositionGreeks(pt);
}

PositionRisk TradeManager::getGreeksSum() {
	auto main_pos_vec = getAllMainAccountPosition();
	PositionRisk ret;
	for (auto pos : main_pos_vec) {
		auto tmp = getGreeks(pos);
		ret.delta += tmp.delta;
		ret.gamma += tmp.gamma;
		ret.theta += tmp.theta;
		ret.vega += tmp.vega;
	}
	return ret;
}


PositionRisk TradeManager::getClientGreeksSum(int client_id) {
	auto client_pos_vec = getAllPosition(client_id);
	PositionRisk ret;
	for (auto pos : client_pos_vec) {
		auto tmp = getGreeks(pos);
		ret.delta += tmp.delta;
		ret.gamma += tmp.gamma;
		ret.theta += tmp.theta;
		ret.vega += tmp.vega;
	}
	return ret;
}

inline
void TradeManager::setHedgePosition(const PositionTypeTrans &ptt) {
	QSqlQuery query(db);
	query.prepare("INSERT INTO hedge_position (instr_code, average_price, total_amount, available_amount, frozen_amount, long_short)"
		"VALUES (?, ?, ?, ?, ?, ?)");
	query.addBindValue(QString::fromStdString(ptt.instr_code));
	query.addBindValue(ptt.average_price);
	query.addBindValue(ptt.total_amount);
	query.addBindValue(ptt.available_amount);
	query.addBindValue(ptt.frozen_amount);
	query.addBindValue((int)ptt.long_short);
	if (!query.exec())
		errMsgBox("Insert hedge position failed.");
}

void TradeManager::updateHedgePosition(const vector<PositionTypeTrans> &ptt) {
	QSqlQuery query(db);
	query.prepare("DELETE FROM hedge_position");
	if (!query.exec())
		errMsgBox("Flush hedge position info failed.");
	for (auto p : ptt)
		setHedgePosition(p);

}

void TradeManager::flushRedis() {
	redis.Flush();
}

void TradeManager::redisWriteClientGreeks(int client_id) {
	auto greeks = getClientGreeksSum(client_id);
	stringstream ss;
	map<string, string> m;
    string lprice;
    redis.HGet("IF1506","LastPrice",lprice);
    cout<<"Time: "<<QDateTime::currentDateTime().toLocalTime().toString().toStdString() <<" "
       <<"Delta "<<greeks.delta<<" IF Price:"<<lprice<<endl;
    ss << greeks.delta;

	m["Delta"] = ss.str();
	ss.str("");
	ss << greeks.gamma;
	m["Gamma"] = ss.str();
	ss.str("");
	ss << greeks.theta;
	m["Theta"] = ss.str();
	ss.str("");
	ss << greeks.vega;
	m["Vega"] = ss.str();
    redis.HMSet("lcz", m);
}

void TradeManager::resetClientBalance(int client_id) {
    QSqlQuery query(db);
    query.prepare("UPDATE balance SET total_balance=100000, available_balance=100000, withdrawable_balance=100000, occupied_margin=0"
                  " WHERE id=?");
    query.addBindValue(client_id);
    if (!query.exec()) {
        QMessageBox::warning(0, "Warning", query.lastError().text());
    }
}


inline
double TradeManager::getTransactionPremium(const TransactionType &tt) {
    /*多方支付权力金，空方收取权利金*/
    return (tt.long_short == LONG ? -1:1) * tt.price * tt.amount * calc_server->getMultiplier(tt.instr_code.toStdString());
}

inline
double TradeManager::getTransactionMargin(const TransactionType &tt) {
    return 0;
}

void TradeManager::settleProgram()
{
    //Get All Users List
    int num=13;

    //Get Necessary Contract Parameters


    QDate date=QDate::currentDate();
    string date_str=date.toString("yyyy-MM-DD").toStdString();

   //Get Daily Settlement File
    string settle_file_name="/home/jiangfeng/OTC_FILE/Settle_Parameter/SettleParam-"+date.toString("yyyy-MM-dd").toStdString()+".txt";
    ifstream settle_file(settle_file_name);
    if (!settle_file.is_open())
        throw runtime_error("Settle File Not Opened ! ");
    struct settledata
    {
        map<string,string> parameter;
    };

    map<string,settledata> settle_param;
    string productID;
    string temp_msg;
    while (getline(settle_file,temp_msg))
    {
        int pos=temp_msg.find(' ');
        string key=temp_msg.substr(0,pos);
        string value=temp_msg.substr(pos+1);
        if (key=="ProductID")
        {
               settledata temp;
               settle_param[value]=temp;
               productID=value;
        }
        else
        {
               settle_param[productID].parameter[key]=value;
        }
    }


    //Change Parameters
    //pricing_param settle_parameter;
    float multiplier;
    float settle_underlying;

    for (auto i:settle_param)
    {
        if (i.first=="IFX03")
        {
               calc_server->param_lock.lock();
               calc_server->value_parameter.Spot_Price=stof(i.second.parameter["Underlying"]);
               multiplier=stoi(calc_server->value_parameter.other_param["Multiplier"]);
               settle_underlying=calc_server->value_parameter.Spot_Price;
               calc_server->value_parameter.other_param["LastSettlePrice"]=i.second.parameter["Underlying"];
               calc_server->value_parameter.other_param["LastSettleVola"]=i.second.parameter["Volatility"];
               calc_server->value_parameter.Free_Rate=stof(i.second.parameter["Free_Rate"]);
               calc_server->value_parameter.Yield_Rate=stof(i.second.parameter["Yield_Rate"]);
               calc_server->value_parameter.Volatility=stof(i.second.parameter["Volatility"]);
               calc_server->value_parameter.Value_Method=stoi(i.second.parameter["Value_Method"]);   
               calc_server->param_lock.unlock();
        }
        else
        {
        }
    }


    //Settle for Each Client
    for (int i=1;i<=num;i++)
    {
        stringstream ss;
        ss<<i;
        string client_id=ss.str();
        string settlefile="/home/jiangfeng/OTC_FILE/Settle_Files/ClientNo_"+client_id+"_"+date.toString("yyyy-MM-dd").toStdString()+".txt";
        ofstream outfile(settlefile.c_str());
        ClientInfo ClientInfo;
        ClientInfo=getClientInfo(stoi(client_id));

        outfile<<"           OTC Service Settlement File For Client         "<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        outfile<<endl;
        outfile<<endl;

        outfile<<"           Client Infomation"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        outfile<<setw(20)<<setfill(' ')<<"Client No."<<setw(20)<<setfill(' ')<<"ClientName"<<setw(20)<<setfill(' ')<<"ClientLevel"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        outfile<<setw(25)<<setfill(' ')<<ClientInfo.client_id<<setw(20)<<setfill(' ')<<ClientInfo.client_name.toStdString()<<setw(20)<<setfill(' ')<<ClientInfo.client_level<<endl;
        outfile<<endl;

        outfile<<"           Transaction History"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        outfile<<setw(30)<<setfill(' ')<<"Trade Date"<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort"<<setw(15)<<setfill(' ')<<"Direction";
        outfile<<setw(15)<<setfill(' ')<<"Price"<<setw(15)<<setfill(' ')<<"Amount"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;

        std::vector<TransactionType> HistTrans;
        int Close_Amount=0;
        float Close_PnL=0;
        float Hold_PnL=0;

       HistTrans=getTransaction(stoi(client_id));
       for (auto i :HistTrans)
       {
           if (i.time.date()==date){
           outfile<<setw(25)<<setfill(' ')<<i.time.toString("yyyy-MM-dd hh:mm").toStdString()<<setw(35)<<setfill(' ')<<i.instr_code.toStdString()<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(15)<<setfill(' ')<<(i.open_offset==0 ? "Open":"Close");
           outfile<<setw(15)<<setfill(' ')<<i.price<<setw(15)<<setfill(' ')<<i.amount<<endl;
           }
       }

       outfile<<endl;
       outfile<<"           Account Position"<<endl;
       outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
       outfile<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort"<<setw(15)<<setfill(' ')<<"Price.Avg";
       outfile<<setw(15)<<setfill(' ')<<"Total No"<<setw(15)<<setfill(' ')<<"No.Avail"<<setw(15)<<setfill(' ')<<"Margin"<<setw(15)<<setfill(' ')<<"SettlePrice"<<endl;
       outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
       std::vector<PositionType> Position;
       Position=getAllPosition(stoi(client_id));
      double New_Margin=0;

      float margin_rate=0.05;
      for (auto i :Position)
      {
          float settle_price=calc_server->Settle_Price(i.instr_code.toStdString(),i.long_short);
         double new_margin=0;
          Hold_PnL=Hold_PnL+i.total_amount*(i.average_price-settle_price)*(i.long_short==0 ? -1:1)*multiplier;
          if (i.long_short==1)
          {
              new_margin=(settle_price+settle_underlying*margin_rate)*multiplier;
              New_Margin+=new_margin;
              ss.clear();
              ss<<new_margin;
              setDB_position(stoi(client_id),i.instr_code.toStdString(),"occupied_margin",ss.str());
          }
          outfile<<setw(35)<<setfill(' ')<<i.instr_code.toStdString()<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(20)<<setfill(' ')<<i.average_price;
          outfile<<setw(20)<<setfill(' ')<<i.total_amount<<setw(20)<<setfill(' ')<<i.available_amount<<setw(20)<<setfill(' ')<<new_margin<<setw(20)<<setfill(' ')<<settle_price<<endl;
      }

    outfile<<endl;
    outfile<<"           Close PnL Detail"<<endl;
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    outfile<<setw(30)<<setfill(' ')<<"Trade Date"<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort";
    outfile<<setw(15)<<setfill(' ')<<"Price"<<setw(15)<<setfill(' ')<<"Amount"<<setw(15)<<setfill(' ')<<"ClosePnL"<<endl;
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    for (auto i :HistTrans)
    {
       if (i.time.date()==date && i.open_offset==1)
       {
           outfile<<setw(25)<<setfill(' ')<<i.time.toString("yyyy-MM-dd hh:mm").toStdString()<<setw(35)<<setfill(' ')<<i.instr_code.toStdString()<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(15)<<setfill(' ')<<(i.open_offset==0 ? "Open":"Close");
           outfile<<setw(15)<<setfill(' ')<<i.price<<setw(15)<<setfill(' ')<<i.amount<<setw(35)<<setfill(' ')<<i.close_pnl<<endl;
           Close_Amount+=i.amount;
           Close_PnL+=i.close_pnl;
       }
    }

   outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
   outfile<<setw(30)<<setfill(' ')<<"Total Close Amount: "<<setw(50)<<setfill(' ')<<Close_Amount;
   outfile<<setw(30)<<setfill(' ')<<"Total Close PnL: "<<setw(50)<<setfill(' ')<<Close_PnL;
   outfile<<endl;


    outfile<<endl;
    outfile<<"           Client Balance"<<endl;
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    ClientBalance Balance;
    Balance=getBalance(stoi(client_id));
    //Set Balance Occupied Margin
    Balance.occupied_margin=New_Margin;
    //Set Balance Available_Balance
    Balance.available_balance=Balance.total_balance-Balance.occupied_margin;
    //Set Balance Withdraw_Balance
    Balance.withdrawable_balance=Balance.available_balance;
    setClientBalance(Balance);
    outfile<<setw(20)<<setfill(' ')<<"Initial Balance"<<setw(20)<<setfill(' ')<<"Total Balance"<<setw(20)<<setfill(' ')<<"Account Available"<<setw(20)<<setfill(' ')<<"Market Value";
    outfile<<setw(20)<<setfill(' ')<<"Cash WithDraw"<<setw(20)<<setfill(' ')<<"Frozen Margin"<<endl;

    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    outfile<<setw(25)<<setfill(' ')<<100000<<setw(25)<<setfill(' ')<<Balance.total_balance<<setw(25)<<setfill(' ')<<Balance.available_balance<<setw(25)<<setfill(' ')<<100000+Close_PnL+Hold_PnL;
    outfile<<setw(25)<<setfill(' ')<<0<<setw(25)<<setfill(' ')<<Balance.occupied_margin<<endl;
    outfile<<endl;

    outfile<<"           Client Risks"<<endl;
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    outfile<<setw(20)<<setfill(' ')<<"Total Delta"<<setw(20)<<setfill(' ')<<"Total Gamma"<<setw(20)<<setfill(' ')<<"Total Vega"<<setw(20)<<setfill(' ')<<"Total Theta";
    outfile<<setw(20)<<setfill(' ')<<"Risk Ratio"<<endl;
    PositionRisk Risk=getClientGreeksSum(stoi(client_id));
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    outfile<<setw(25)<<setfill(' ')<<Risk.delta<<setw(25)<<setfill(' ')<<Risk.gamma<<setw(25)<<setfill(' ')<<Risk.vega<<setw(25)<<setfill(' ')<<Risk.theta;
    outfile<<setw(25)<<setfill(' ')<<round(Balance.occupied_margin/Balance.total_balance*10000)/100<<"%"<<endl;
    outfile<<endl;
    }


  cout<<"Settlement Program Finish!"<<endl;
  int cc;
  cin>>cc;
}

void TradeManager::setDB_change(int client_id,string table,string key,string value)
{
    QSqlQuery  query(db);
    stringstream ss;
    ss<<"UPDATE "<<table<<" SET "<<key<<"=? WHERE id=?";
    query.prepare(QString::fromStdString(ss.str()));
    query.addBindValue(QString::fromStdString(value));
    query.addBindValue(client_id);
    if (!query.exec())
        qDebug() << "SET balance FAILED... " << query.lastQuery() << " " << query.lastError();

    return;
}

void TradeManager::setDB_position(int client_id,string instr_code,string key,string value)
{
    QSqlQuery  query(db);
    stringstream ss;
    ss<<"UPDATE position SET "<<key<<"=? WHERE id=? AND instr_code=?";
    query.prepare(QString::fromStdString(ss.str()));
    query.addBindValue(QString::fromStdString(value));
    query.addBindValue(client_id);
    query.addBindValue(QString::fromStdString(instr_code));
    if (!query.exec())
        qDebug() << "SET balance FAILED... " << query.lastQuery() << " " << query.lastError();

    return;
}
