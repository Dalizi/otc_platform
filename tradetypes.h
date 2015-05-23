#ifndef TRADETYPES_H
#define TRADETYPES_H

#include "otc_types.h"

#include <string>
#include <QString>
#include <QDateTime>


enum ClientLevelType {

};

struct TransactionType {

	QString instr_code = ""; //合约代码
    QString transaction_id = "";	//成交编号
    QDateTime time;	//成交时间
    int client_id = 0;	//客户编号
    double price = 0;	//成交价格
    int amount = 0;	//成交数量
    LongShortType long_short = LONG;	//买卖方向
    OpenOffsetType open_offset = OPEN;	//开平方向
	double underlying_price; //标的价格
    LongShortType reversePosition() const { return long_short==LONG?SHORT:LONG; }
    LongShortType getPositionDirect() const { return open_offset == OPEN ? long_short:reversePosition(); }

};

struct OrderType {
	QString instr_code = ""; //合约代码
	QString order_id = "";	//委托编号
	QDateTime time;	//成交时间
	int client_id = 0;	//客户编号
	double price = 0;	//委托价格
	int amount = 0;	//委托数量
    LongShortType long_short = LONG;	//买卖方向
    OpenOffsetType open_offset = OPEN;	//开平方向
    OrderStatusType order_status = REPORTED; //委托状态
    LongShortType reversePosition() const { return long_short==LONG?SHORT:LONG; }
    LongShortType getPositionDirect() const { return open_offset == OPEN ? long_short:reversePosition(); }
};

struct PositionType {
    int client_id = -1;	//客户编号
    QString instr_code = ""; //合约代码
    double average_price = 0;	//平均价格
    int total_amount = 0;	//总量
    int available_amount = 0; //可用数量
    int frozen_amount = 0; //冻结数量
    double underlying_price;
    LongShortType long_short = LONG; //买卖方向
};

struct ClientInfo {
    QString client_name;
    int client_id;
    QString interview_record;
    QString review_material;
    QString public_info;
    QString client_relationship;
    unsigned client_level;
    double trust_value;
    QString chartered_business;
};

struct ClientBalance {
    int client_id;
    double total_balance;
    double available_balance;
    double withdrawable_balance;
    double occupied_margin;
};

struct Qoute {
	QString instr_code;
	double ask_price;
	int ask_volume;
	double bid_price;
	int bid_volume;

};

struct PositionRisk
{
	double delta=0, gamma=0, vega=0, theta=0;
};

OrderType &operator<<(OrderType &ot, const OrderTypeTrans &ott);
OrderTypeTrans &operator<<(OrderTypeTrans &ott, const OrderType &ot);

PositionType &operator<<(PositionType &ot, const PositionTypeTrans &ott);
PositionTypeTrans &operator<<(PositionTypeTrans &ott, const PositionType &ot);
#endif // TRADETYPES_H

