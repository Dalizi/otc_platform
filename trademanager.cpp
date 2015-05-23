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
                  " average_price, underlying_price)"
                  "VALUES (?, ?, ?, ?, ?, ?, ?,?)");
    query.addBindValue(pt.client_id);
    query.addBindValue((int)pt.long_short);
    query.addBindValue(pt.instr_code);
    query.addBindValue(pt.total_amount);
    query.addBindValue(pt.available_amount);
    query.addBindValue(pt.frozen_amount);
    query.addBindValue(pt.average_price);
    query.addBindValue(pt.underlying_price);
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
    setTransaction(tt);
    changeOrderStatus(ot.order_id.toStdString(), 1);
    updateBalance(ot);
    setPosition(tt);
    //errMsgBox("Order accepted.");
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

void TradeManager::setTransaction(const TransactionType &ot) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO transactions (id, instr_code, "
                  "client_id, price, amount, long_short, open_offset, underlying_price)"
                  " VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(ot.transaction_id);
    query.addBindValue(ot.instr_code);
    //query.addBindValue(ot.time.toString("yyyyMMddhhmmss"));
    query.addBindValue(ot.client_id);
    query.addBindValue(ot.price);
    query.addBindValue(ot.amount);
    query.addBindValue((int)ot.long_short);
    query.addBindValue((int)ot.open_offset);
	query.addBindValue(ot.underlying_price);

    if (!query.exec())
        qDebug() <<"INSERTION FAILED..." <<" " <<query.lastQuery() <<" " <<query.lastError();
}

void TradeManager::setPosition(const TransactionType &ot) {
    PositionType pt = getPosition(ot.client_id, ot.instr_code, ot.getPositionDirect());
	if (pt.instr_code == "") {
		pt.instr_code = ot.instr_code;
		pt.client_id = ot.client_id;
		pt.long_short = ot.long_short;
        pt.total_amount = ot.amount;
        pt.frozen_amount = 0;
        pt.available_amount = ot.amount;
        pt.average_price = ot.price;
        pt.underlying_price = calc_server->Settle_Price(pt.instr_code.toStdString(), pt.long_short);
		addPosition(pt);
    } else {
    updatePosition(pt, ot);
    }
//    updateMainPosition(pt1, ot);
//    setMainPosition(pt1);
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
	query.prepare("UPDATE balance SET total_balance=?, available_balance=?, withdrawable_balance=?, occupied_margin=?  WHERE client_id=?");
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
					"underlying_price double"
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
                    "long_short integer"
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

void TradeManager::updatePosition(const PositionType &pt, const TransactionType &ot) {
    QSqlQuery query(db);
    int adjust_param = ot.open_offset == OpenOffsetType::OPEN ? 1 : -1;
    int total_amount = pt.total_amount + ot.amount * adjust_param;
    double average_price, underlying_price;
    if (total_amount != 0) {
        average_price = (pt.average_price * pt.total_amount + ot.price * ot.amount * adjust_param) / (pt.total_amount + ot.amount * adjust_param);
        underlying_price = (pt.underlying_price * pt.total_amount + ot.underlying_price * ot.amount * adjust_param) / (pt.total_amount + ot.amount * adjust_param) ;
        query.prepare("UPDATE position SET total_amount=?, available_amount=?,average_price=?, underlying_price=? "
                      "WHERE instr_code=? AND id=? AND long_short=?");
        query.addBindValue(total_amount);
        query.addBindValue(total_amount);
        query.addBindValue(average_price);
        query.addBindValue(underlying_price);
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

void TradeManager::updateBalance(const OrderType &ot) {
	QSqlQuery query(db);
    string underlying_code = calc_server->getUnderlyingCode(ot.instr_code.toStdString());
    int multiplier = calc_server->main_contract.multiplier;
    query.prepare("UPDATE balance SET total_balance =total_balance+?,"
                  "occupied_margin=occupied_margin+?,"
                  "withdrawable_balance=withdrawable_balance+?"
                  " WHERE id=?");
    auto pt = getPosition(ot.client_id, ot.instr_code, ot.getPositionDirect());
	if (getInstrType(ot.instr_code.toStdString()) == "option") {
        if (ot.long_short == LONG) {
            if (ot.open_offset == OPEN) {
                double premium = ot.price*ot.amount*multiplier;
                query.addBindValue(-premium);
                query.addBindValue(0);
                query.addBindValue(-premium);
				query.addBindValue(ot.client_id);
			}
			else {
                double premium = pt.average_price*ot.amount*multiplier;
                double cash_flow = getCloseCashFlow(ot);
                double margin = calc_server->Settle_Price(underlying_code, ot.long_short)*ot.amount*multiplier*getMarginRate(ot.client_id, ot.instr_code.toStdString());
                query.addBindValue(cash_flow);
                query.addBindValue(-premium-margin);
                query.addBindValue(margin);
                query.addBindValue(ot.client_id);
			}
		} else {
            if (ot.open_offset == OpenOffsetType::OPEN) {
                double premium = ot.price*ot.amount*multiplier;
                double margin = calc_server->Settle_Price(underlying_code, ot.long_short)*ot.amount*multiplier*getMarginRate(ot.client_id, ot.instr_code.toStdString());
                query.addBindValue(premium);
                query.addBindValue(premium+margin);
                query.addBindValue(-margin);
				query.addBindValue(ot.client_id);
			}
			else {
                auto pos = getPosition(ot.client_id, ot.instr_code, ot.reversePosition());
                double cash_flow = getCloseCashFlow(ot);
                query.addBindValue(cash_flow);
                query.addBindValue(0);
                query.addBindValue(0);
                query.addBindValue(ot.client_id);
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
	return cb.occupied_margin / (cb.occupied_margin + cb.available_balance);
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
double TradeManager::getCloseCashFlow(const OrderType &ot) {
    auto close_price = calc_server->Position_Quote(ot.instr_code.toStdString(), ot.getPositionDirect());
    return (ot.getPositionDirect()==LONG?1:-1) * close_price * ot.amount * calc_server->main_contract.multiplier;
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
