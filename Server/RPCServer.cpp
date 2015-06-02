#include "RPCServer.h"

#include <stdexcept>
#include <qdatetime.h>
#include <qstring.h>

using namespace std;

ClientServiceHandler::ClientServiceHandler(TradeManager *tm):tm(tm) {

}

ClientServiceHandler::~ClientServiceHandler() {

}

bool ClientServiceHandler::auth(const string& client_id, const std::string& password) {
    //cout << "auth" << endl;
	auto client_idn = stoi(client_id);
	int ret =  tm->authPassword(client_idn, password);
	if (ret != -1) return ret;
	InvalidQuery iq;
    iq.error_no = 0;
	iq.why = "Client doesn't exists.";
	throw iq;
	return false;
}

double ClientServiceHandler::get_pnl(const PositionTypeTrans& ptt){
	PositionType pt;
	pt << ptt;
	return tm->getPnL(pt, false);
}

double ClientServiceHandler::get_close_price(const PositionTypeTrans& ptt) {
	PositionType pt = tm->getPosition(ptt.client_id, QString::fromStdString(ptt.instr_code), ptt.long_short);
	return tm->getClosePrice(pt);
}

void ClientServiceHandler::place_order(const OrderTypeTrans& order) {
    assert(order.order_status == REPORTED);
	OrderType ot;
	ot << order;
    ot.order_id = QString::fromStdString(tm->addOrder(ot));

    tm->processSingleOrder(ot);
}

void ClientServiceHandler::get_order(std::vector<OrderTypeTrans> & _return, const int32_t client_id, const std::string& start_date, const std::string& end_date) {
	auto order_vec = tm->getOrder(client_id);
	auto qstart_date = QDate::fromString(QString::fromStdString(start_date), "yyyy-MM-dd");
	auto qend_date = QDate::fromString(QString::fromStdString(end_date), "yyyy-MM-dd");
	for (auto order : order_vec) {
		OrderTypeTrans ott;
		/*
		auto test1 = order.time.date().toString("yy-MM-dd").toStdString();
		bool test2 = order.time.date() >= qstart_date;
		bool test3 = order.time.date() <= qend_date;
		auto test4 = qend_date.toString("yy-MM-dd").toStdString();
		*/
        if (order.time.date() >= qstart_date && order.time.date() <= qend_date) {
			ott << order;
            _return.push_back(ott);
		}

	}
}

void ClientServiceHandler::get_calculated_balance(std::map<std::string, double> & _return, const int32_t client_id) {
	_return = tm->getCalculatedBalance(client_id);
}

void ClientServiceHandler::get_transaction(std::vector<TransactionTypeTrans> & _return, const int32_t client_id, const std::string& start_date, const std::string& end_date) {
	auto transaction_vec = tm->getTransaction(client_id);
	auto qstart_date = QDate::fromString(QString::fromStdString(start_date), "yyyy-MM-dd");
	auto qend_date = QDate::fromString(QString::fromStdString(end_date), "yyyy-MM-dd");
	for (auto transaction : transaction_vec) {
        TransactionTypeTrans ttt;
		/*
		auto test1 = order.time.date().toString("yy-MM-dd").toStdString();
		bool test2 = order.time.date() >= qstart_date;
		bool test3 = order.time.date() <= qend_date;
		auto test4 = qend_date.toString("yy-MM-dd").toStdString();
		*/
		if (transaction.time.date() >= qstart_date && transaction.time.date() <= qend_date) {
            ttt << transaction;
            _return.push_back(ttt);
		}
		
	}
	
}

void ClientServiceHandler::get_position(vector<PositionTypeTrans> & _return, const int32_t client_id){
	auto pos_vec = tm->getAllPosition(client_id);
	for (auto pos : pos_vec) {
		PositionTypeTrans ptt;
        ptt << pos;
		_return.push_back(ptt);
	}
}

void ClientServiceHandler::get_balance(ClientBalanceTrans& _return, const int32_t client_id) {
	auto cb = tm->getBalance(client_id);
	_return.client_id = cb.client_id;
    _return.total_balance = cb.total_balance;
	_return.occupied_margin = cb.occupied_margin;
	_return.withdrawable_balance = cb.withdrawable_balance;
}

void ClientServiceHandler::get_clientinfo(ClientInfoTrans& _return, const int32_t client_id) {
	auto ci = tm->getClientInfo(client_id);
	_return.client_id = ci.client_id;
	_return.client_level = ci.client_level;
	_return.client_name = ci.client_name.toStdString();
	_return.client_relationship = ci.client_relationship.toStdString();
	_return.review_material = ci.review_material.toStdString();
	_return.interview_record = ci.interview_record.toStdString();
	_return.public_info = ci.public_info.toStdString();
	_return.trust_value = ci.trust_value;
	_return.chartered_business = ci.chartered_business.toStdString();
}

void ClientServiceHandler::get_qoute(std::vector<QouteTrans> & _return) {
	auto qoute_vec = tm->getQoute();
    if (QTime::currentTime()>QTime::fromString("23:59","hh:mm") || QTime::currentTime()<QTime::fromString("00:00","hh:mm"))
    {
        return;
    }
	for (auto qoute : qoute_vec) {
		QouteTrans qt;
		qt.instr_code = qoute.instr_code.toStdString();
		qt.ask_price = qoute.ask_price;
		qt.ask_volume = qoute.ask_volume;
		qt.bid_price = qoute.bid_price;
		qt.bid_volume = qoute.bid_volume;
		_return.push_back(qt);
	}
}

void ClientServiceHandler::get_greeks(GreekRisk& _return) {
	auto greeks = tm->getGreeksSum();
	_return.delta = greeks.delta;
	_return.gamma = greeks.gamma;
	_return.theta = greeks.theta;
	_return.vega = greeks.vega;
}

void ClientServiceHandler::update_hedge_position(const vector<PositionTypeTrans> &ptt){
	tm->updateHedgePosition(ptt);
}

void ClientServiceHandler::get_client_greeks(GreekRisk& _return, const int32_t client_id) {
	auto greeks = tm->getClientGreeksSum(client_id);
	_return.delta = greeks.delta;
	_return.gamma = greeks.gamma;
	_return.theta = greeks.theta;
	_return.vega = greeks.vega;
}
