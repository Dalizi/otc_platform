#pragma once
#include <errmsgbase.h>
#include <string>
#include <vector>
#include "base_function.h"
#include "accessredis.h"
#include <time.h>
#include <thread>
#include <mutex>
#include "tradetypes.h"
#include "trademanager.h"
#include <unistd.h>

using namespace std;

struct Contract_Info
{
	string contract_code,product;
	int multiplier;
	map<string, string> OTC_Code;
	map<string,map<string, string>> Quote;
};

class Option_Value: public ErrMsgBase
{
    Q_OBJECT
public:
    Option_Value(string infile_location, TradeManager *tm);
	~Option_Value();

	void Init();
	void Start();
	void Stop();

	void main_value_process(void * stParam);
	void Auto_Hedger(void *stParam);

	void Parameter_Setting(string param_name,string param_value);
	void Parameters_Setting(map<string, string> & params);
	void Get_Parameters();
	double Maturity_Remain(string maturity_date);
	double Option_Valuation(pricing_param param);
	double Volatility_Adjustment(double basic_vol, double maturity, double strike);
	double Spread_Adjustment(double maturity, double strike, map<string, double> &position);
	double Basis_Spread(map<string, string> &basis);
	double Position_PnL(PositionType Position, bool isMain);
    string getUnderlyingCode(const string &instr_code);
	double getUnderlyingPrice(const string &instr_code);

	int Delta_Hedger(vector<PositionType> &Position, double Spot_Price);
	int Hedger_Excute(int net_delta, double price);

    double Position_Quote(const string &instr_code, LongShortType ls);
    double Settle_Price(const string &instr_code, LongShortType long_short);
	PositionRisk PositionGreeks(const PositionType &Position);

	double Price_Qoute(const string &instr_code);
	vector<string> Get_Main_Contract_Codes(); //Get all main contract code

	vector<string> Trade_Day;
	pricing_param value_parameter;
	CAccessRedis my_redis;
	map<string, string> temp_param;
	Contract_Info main_contract;
	string Param_Key;
    std::thread main_thread, hedge_thread;
	bool value_flag,hedge_flag,param_update;
	mutex param_lock;
	vector<PositionType> Total_Position;
    TradeManager *tm;

private:
	void ParameterToMap();
	void MapToParameter();
};
