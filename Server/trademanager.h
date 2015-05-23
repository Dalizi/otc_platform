#ifndef TRADEMANAGER_H
#define TRADEMANAGER_H

#include "tradetypes.h"
#include "accessredis.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QFile>

#include <map>
#include <set>
#include <string>
#include <mutex>
#include <vector>

typedef vector<TransactionType>::size_type order_index;


class Option_Value;
class MainWindow;

class TradeManager: public QObject
{
	Q_OBJECT

    friend class MainWindow;
public:

    TradeManager(QObject *parent = 0);
    ~TradeManager();

	void addPosition(const PositionType &pt);
	void addMainPosition(const PositionType &pt);
    string addOrder(OrderType &ot); //Return order id

	void removeOrder(const string &order_id);

    vector<TransactionType> getTransaction(int client_id);
	vector<OrderType> getOrder(int client_id);
	vector<OrderType> getAllOrders();
    PositionType getPosition(int client_id, const QString &instr_code, LongShortType long_short);
    vector<PositionType> getAllPosition(int client_id); //获取某客户的所有持仓
    vector<PositionType> getAllPosition(const QString &client_name);
    vector<PositionType> getAllMainAccountPosition();   //获取总账户的所有持仓
	ClientBalance getBalance(int client_id);

    //获取实时计算的客户权益
	double getTotalBalance(int client_id);
    double getAvailableBalance(int client_id);
    double getFrozenBalance(int client_id);

    ClientInfo getClientInfo(const QString &client_name);
	ClientInfo getClientInfo(const int client_id);
    ClientBalance getClientBalance(int client_id);
    int getIDFromName(const QString &client_name);

    double getPnL(const PositionType &pt, bool isMain);
	double getMargin(int client_id);
	double getMarginRiskRatio(int client_id);
	double getRTMargin(int client_id);
	double getRTMarginRiskRatio(int client_id);
    double getMarketValueBalance(int client_id);
	double getClosePrice(const PositionType &pt);
    double getCloseCashFlow(const PositionType &pt);
    double getCloseCashFlow(const OrderType &ot);
	PositionRisk getGreeksSum();
	PositionRisk getClientGreeksSum(int client_id);
	PositionRisk getGreeks(const PositionType &pt);
	map<string, double> getCalculatedBalance(int client_id);
	vector<Qoute> getQoute();
    
    void setTransaction(const TransactionType &ot);
    void setPosition(const TransactionType &ot);
    void setPosition(const PositionType &pt);
	void setMainPosition(const PositionType &pt);
    int setClientInfo(const ClientInfo &ci);
    void setClientBalance(const ClientBalance &cb);
    std::vector<QString> getClientList();

	void deletePosition(const PositionType &pt);

    int getTransactionID();
	double getMultiplier(const string &instr_code);
	double getMarginRate(int client_id, const string &instr_code);


    bool openDB();
    QSqlError lastError();

    bool isPositionExist(int client_id, const QString &instr_code, int long_short);

    void updatePosition(const PositionType &pt, const TransactionType &ot);
	void updateBalance(const OrderType &ot);
	void updateBalance(int client_id, double adjust_value);

    int authPassword(int client_id, const string &passwd);
    void processSingleOrder(const OrderType &ot);  //处理收到的委托

	void initClientBalance(int client_id, double init_balance);

	void updateHedgePosition(const vector<PositionTypeTrans> &hedge_positions);

	void redisWriteClientGreeks(int client_id);

private:
    //处理委托
    void acceptOrder(const OrderType &ot);
    void rejectOrder(const OrderType &ot);
    void cancelOrder(const OrderType &ot);

	int getOrderStatus(const string &order_id);
    int getOrderIndex();
	void changeOrderStatus(const string &order_id, int status);
	void errMsgBox(const string &msg);

	bool isOrderValid(const string &order_id);
    bool isOrderValid(const OrderType &ot);

	void calcRun();
	void loadContractTypeFile(const string &in_file);
	string getInstrType(const string &instr_code);
	void setHedgePosition(const PositionTypeTrans &ptt);
	void flushRedis();


private:
    QSqlDatabase db;
    CAccessRedis redis;
	Option_Value *calc_server;
	map<string, string> instr_code_type;

};

#endif // TRADEMANAGER_H
