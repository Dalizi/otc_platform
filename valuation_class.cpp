#include "valuation_class.h"
#include <fstream>
#include <QMessageBox>

using namespace std;

Option_Value::Option_Value(string infile_location, TradeManager *tm) : tm(tm)
{
    ifstream in_file(infile_location);

    if (!in_file.is_open()) {
        QMessageBox::about(0, "ERROR", "Error loading trade date info.");
        exit(1);
    }
    string temp;
    while (in_file >> temp)
    {
        Trade_Day.push_back(temp);
    }


    string host("10.2.6.31");
    //int iRet = my_redis.Connect(host, 6379, "Finders6");
    int iRet = my_redis.Connect("127.0.0.1", 6379);
    if (iRet != 0) {
        stringstream ss;
        ss << "Redis Error: " <<iRet;
        QMessageBox::about(0, "ERROR", QString::fromStdString(ss.str()));
        exit(1);
    }
	cout<<"REDIS数据库已连接!"<<endl;
	my_redis.Select(0);

	return;
}

Option_Value::~Option_Value()
{
	Stop();
}

void Option_Value::ParameterToMap()
{
	char * temp = new char[100];
	temp_param.clear();
	temp_param=value_parameter.other_param;

	sprintf(temp, "%f", value_parameter.Strike_Price);
	temp_param["Strike"] = temp;
	sprintf(temp, "%f", value_parameter.Spot_Price);
	temp_param["Underlying"] = temp;
	sprintf(temp, "%f", value_parameter.Free_Rate);
	temp_param["FreeRate"] = temp;
	sprintf(temp, "%f", value_parameter.Yield_Rate);
	temp_param["YiledRate"] = temp;
	sprintf(temp, "%f", value_parameter.Volatility);
	temp_param["Volatility"] = temp;
	sprintf(temp, "%f", value_parameter.TimeToMaturity);
	temp_param["Maturity"] = temp;
	sprintf(temp, "%d", value_parameter.Option_Type);
	temp_param["OptionType"] = temp;
	sprintf(temp, "%d", value_parameter.Value_Method);
	temp_param["ValueMethod"] = temp;

	delete[] temp;
	return;
}

void Option_Value::MapToParameter()
{
	param_lock.lock();
	value_parameter.Free_Rate =atof( temp_param["FreeRate"].c_str());
	value_parameter.Yield_Rate = atof(temp_param["YiledRate"].c_str());
	value_parameter.Spot_Price = atof(temp_param["Underlying"].c_str());
	value_parameter.Strike_Price =atof( temp_param["Strike"].c_str());
	value_parameter.Volatility = atof(temp_param["Volatility"].c_str());
	value_parameter.Option_Type = atoi(temp_param["OptionType"].c_str());
	value_parameter.TimeToMaturity = atof(temp_param["Maturity"].c_str());
	value_parameter.Value_Method = atoi(temp_param["ValueMethod"].c_str());
	temp_param.erase("FreeRate");
	temp_param.erase("YieldRate");
	temp_param.erase("Underlying");
	temp_param.erase("Strike");
	temp_param.erase("Volatility");
	temp_param.erase("OptionType");
	temp_param.erase("Maturity");
	temp_param.erase("ValueMethod");
	value_parameter.other_param = temp_param;
	param_lock.unlock();
	return;
}
void Option_Value::Parameter_Setting(string param_name, string param_value)
{
	my_redis.HSet(Param_Key,param_name,param_value);
	param_lock.lock();
	param_update = true;
	param_lock.unlock();
	return;
}

void Option_Value::Parameters_Setting(map<string, string> & params)
{
	my_redis.HMSet(Param_Key, params);
	param_lock.lock();
	param_update = true;
	param_lock.unlock();
	return;
}

void Option_Value::Get_Parameters()
{
	int iRet;
	iRet=my_redis.HGetAll(Param_Key, temp_param);
	return;
}

double Option_Value::Maturity_Remain(string maturity_date)
{
	time_t now;
	time(&now);
	struct tm today;
	today = *localtime(&now);
	int start, end;
	double result = 0;

	char buffer[100];
	strftime(buffer, 100, "%Y-%m-%d", &today);

	string cur_date = buffer;
	for (int i = 0; i < Trade_Day.max_size(); i++)
	{
		if (strcmp(cur_date.c_str(), Trade_Day[i].c_str()) <= 0){
			start = i;
			break;
		}
	}
	for (int i = 0; i < Trade_Day.max_size(); i++)
	{
		if (strcmp(maturity_date.c_str(), Trade_Day[i].c_str()) <= 0){
			end = i;
			break;
		}
	}
	result = end - start;
	if (end - start <= 5){
		if (today.tm_hour >= 10 && today.tm_hour < 11){
			result = result - 1.25 / 4.5;
		}
		else if (today.tm_hour >= 11 && today.tm_hour < 12){
			result = result - 2 / 4.5;
		}
		else if (today.tm_hour >= 13 && today.tm_hour < 14){
			result = result - 2.5 / 4.5;
		}
		else if (today.tm_hour >= 14 && today.tm_hour < 15){
			result = result - 3.5 / 4.5;
		}
		else if (today.tm_hour >= 15 && today.tm_hour < 16){
			result = result - 1;
		}
		else{}
	}
	return result/256;
}

double Option_Value::Option_Valuation(pricing_param param)
{
	int method = param.Value_Method;
	double result=0;
	switch (method)
	{
	case 1: result=bls_vanila_option(param); break;
	}
	return result;
}

int Option_Value::Delta_Hedger(vector<PositionType> &Position,double Spot_Price)
{
	double result=0;
	double maturity,strike,basic_vola;
	pricing_param temp_param;
	
	param_lock.lock();
	temp_param = value_parameter;
	param_lock.unlock();

	basic_vola = temp_param.Volatility;
	temp_param.Spot_Price = Spot_Price;

	for (int i = 0; i < Position.size(); i++)
	{
		if (strcmp(Position[i].instr_code.toStdString().substr(0, 4).c_str(), "OTC-") == 0){
			int l = Position[i].instr_code.length();
			int pos_first = Position[i].instr_code.toStdString().substr(4, l - 4).find_first_of("-") + 4;
			int pos_last = Position[i].instr_code.toStdString().find_last_of("-");
			string temp = Position[i].instr_code.toStdString().substr(pos_first + 1, 10);
			maturity = Maturity_Remain(temp);
			temp = Position[i].instr_code.toStdString().substr(pos_last + 1, l - pos_last - 1);
			strike = atof(temp.c_str());
			temp = Position[i].instr_code.toStdString().substr(pos_first - 3, 3);

			if (strcmp(temp.c_str(), "C00") == 0){ temp_param.Option_Type = 1; }
			if (strcmp(temp.c_str(), "P00") == 0){ temp_param.Option_Type = 2; }

			temp_param.Strike_Price = strike;
			temp_param.TimeToMaturity = maturity;
			temp_param.Volatility = Volatility_Adjustment(basic_vola, maturity, strike);

			switch (temp_param.Value_Method)
			{
			case 1: result = result + 2*(-Position[i].long_short+0.5)*Position[i].total_amount*bls_vanila_delta(temp_param)*main_contract.multiplier; break;
			}
		}
		else if (strcmp(Position[i].instr_code.toStdString().substr(0, 2).c_str(), "IF") == 0)
		{
			result = result + 2 * (-Position[i].long_short + 0.5)*Position[i].total_amount;
		}
		else{}
	}

	return (int)round(result/300);
}

int Option_Value::Hedger_Excute(int net_delta,double price)
{
	double excute_price;
	int amount;
	net_delta = -net_delta;
	if (net_delta < 0)
	{
		excute_price = price + 1;
		amount = net_delta;
	}
	else
	{
		excute_price = price - 1;
		amount = net_delta;
	}

	int i;
	for (i = 0; i < Total_Position.size(); i++)
	{
		if (strcmp(Total_Position[i].instr_code.toStdString().c_str(), main_contract.contract_code.c_str()) == 0) break;
	}

	if (i >= Total_Position.size()){
		PositionType temp;
		memset(&temp, sizeof(temp),0);
		temp.instr_code = QString::fromStdString(main_contract.contract_code);
		Total_Position.push_back(temp);
		i = Total_Position.size() - 1;
	}

	if (Total_Position[i].total_amount == 0)
	{
		Total_Position[i].average_price = price;
		Total_Position[i].total_amount = abs(net_delta);
        Total_Position[i].long_short = (LongShortType)((net_delta > 0) ? 0 : 1);
	}
	else
	{
		if (2*(-Total_Position[i].long_short+0.5)*net_delta > 0)
		{
			Total_Position[i].average_price = Total_Position[i].average_price*Total_Position[i].total_amount + abs(net_delta)*price;
			Total_Position[i].total_amount = Total_Position[i].total_amount + abs(net_delta);
			Total_Position[i].average_price = Total_Position[i].average_price / Total_Position[i].total_amount;
		}
		else
		{
			int new_position = Total_Position[i].total_amount - abs(net_delta);
			if (new_position >= 0){
				Total_Position[i].total_amount = new_position;
				double PnL = (price - Total_Position[i].average_price)*Total_Position[i].long_short/abs(net_delta);
				Total_Position[i].average_price = Total_Position[i].average_price - PnL;
			}
			else{
				Total_Position[i].total_amount = -new_position;
                Total_Position[i].long_short = (LongShortType)(1 - Total_Position[i].long_short);
				double PnL = (price - Total_Position[i].average_price)*Total_Position[i].long_short / Total_Position[i].total_amount;
				Total_Position[i].average_price = Total_Position[i].average_price - PnL;
			}

		}

	}
	
	TradeManager temp_class;
	temp_class.setMainPosition(Total_Position[i]);
	
	cout << "Delta Hedged at: " << excute_price << " with amount " << net_delta << endl;

	return 0;
}

double Option_Value::Volatility_Adjustment(double basic_vol, double maturity, double strike)
{

	return basic_vol;
}

double Option_Value::Spread_Adjustment(double maturity, double strike, map<string, double> &position)
{
	return 0;
}

double Option_Value::Basis_Spread( map<string,string> &basis)
{
	double spread=0;
	string type = basis["Spread_Type"];
	if (strcmp(type.c_str(), "Vol") == 0 && basis.count("Basis_Vol_Spread")>0)
	{
		spread = atof(basis["Basis_Vol_Spread"].c_str());
	}
	else if (strcmp(type.c_str(), "Del") == 0 && basis.count("Basis_Delta_Spread")>0)
	{
		spread = atof(basis["Basis_Delta_Spread"].c_str());
	}
	else if (strcmp(type.c_str(), "Prc") == 0 && basis.count("Basis_Price_Spread")>0)
	{
		spread = atof(basis["Basis_Price_Spread"].c_str());
	}
	else
	{
	}
	return spread;
}

double Option_Value::Position_PnL(PositionType Position, bool isMain)
{
	double result = 0;
	double maturity, strike, basic_vola,Spot_Price;
	pricing_param temp_param;
	map<string, string> update_data;

	param_lock.lock();
	temp_param = value_parameter;
	param_lock.unlock();


	basic_vola = temp_param.Volatility;
	int iRet = my_redis.HGetAll(main_contract.contract_code, update_data);
	Spot_Price= atof(update_data["LastPrice"].c_str());
	temp_param.Spot_Price = Spot_Price;

	if (strcmp(Position.instr_code.toStdString().substr(0, 4).c_str(), "OTC-") == 0){
			int l = Position.instr_code.length();
			int pos_first = Position.instr_code.toStdString().substr(4, l - 4).find_first_of("-") + 4;
			int pos_last = Position.instr_code.toStdString().find_last_of("-");
			string temp = Position.instr_code.toStdString().substr(pos_first + 1, 10);
			maturity = Maturity_Remain(temp);
			temp = Position.instr_code.toStdString().substr(pos_last + 1, l - pos_last - 1);
			strike = atof(temp.c_str());
			temp = Position.instr_code.toStdString().substr(pos_first - 3, 3);

			if (strcmp(temp.c_str(), "C00") == 0){ temp_param.Option_Type = 1; }
			if (strcmp(temp.c_str(), "P00") == 0){ temp_param.Option_Type = 2; }

			temp_param.Strike_Price = strike;
			temp_param.TimeToMaturity = maturity;
			double spread = Basis_Spread(temp_param.other_param);
			if (isMain) {
                spread *= (Position.long_short == LONG ? 1 : -1);
			}
			else {
                spread *= (Position.long_short == LONG ? -1 : 1);
			}
			temp_param.Volatility = spread/3 + Volatility_Adjustment(basic_vola, maturity, strike);

			switch (temp_param.Value_Method)
			{
			case 1: result = 2 * (-(int)Position.long_short + 0.5)*Position.total_amount*(bls_vanila_option(temp_param)- Position.average_price)*main_contract.multiplier; break;
			}
		}
	else if (strcmp(Position.instr_code.toStdString().substr(0, 2).c_str(), "IF") == 0)
		{
			result = 2 * (-Position.long_short + 0.5)*Position.total_amount*(Spot_Price-Position.average_price)*300;
		}
	else{}

	return result;
}

void Option_Value::Init()
{
	ifstream input_param("ConfigFile.txt");
    if (!input_param.is_open()) {
        QMessageBox::about(0, "ERROR", "Error loading config file.");
        exit(1);
	}
	string temp_str;
	input_param >> temp_str;
	main_contract.product = temp_str;
	input_param >> temp_str;
	main_contract.contract_code = temp_str;
	input_param >> main_contract.multiplier;
	Param_Key = "PARAM-" + main_contract.product;
	vector<string> keys;
	my_redis.Keys("OTC*", keys);
	my_redis.Del(Param_Key);
	for (auto key : keys) {
		my_redis.Del(key);
	}
	input_param >> temp_str;
	value_parameter.other_param["LastSettlePrice"] = temp_str;
	input_param >> temp_str;
	value_parameter.other_param["LastSettleVola"] = temp_str;
	//WriteToRedis Parameter Init

		input_param >> value_parameter.Free_Rate;
		input_param >> value_parameter.Yield_Rate;
		input_param >> value_parameter.Spot_Price;
		input_param >> value_parameter.Strike_Price;
		input_param >> value_parameter.Volatility;
		input_param >> value_parameter.Option_Type;
		input_param >> value_parameter.TimeToMaturity;
		input_param >> value_parameter.Value_Method;

		input_param >> temp_str;
		value_parameter.other_param["Basis_Vol_Spread"] = temp_str;
		input_param >> temp_str;
		value_parameter.other_param["Basis_Delta_Spread"] = temp_str;
		input_param >> temp_str;
		value_parameter.other_param["Basis_Price_Spread"] = temp_str;
		input_param >> temp_str;
		value_parameter.other_param["Spread_Type"] = temp_str;
		value_parameter.other_param["Underlying_Code"] = main_contract.contract_code;

		char buffer[1000];
		sprintf(buffer, "%d", main_contract.multiplier);
		value_parameter.other_param["Multiplier"] = buffer;
		ParameterToMap();
		my_redis.HMSet(Param_Key, temp_param);

	time_t now;
	time(&now);
	struct tm today;
	today = *localtime(&now);

	strftime(buffer, 100, "%Y-%m-%d", &today);

	string cur_date = buffer;
	string mat_date;
	for (int i = 0; i <Trade_Day.max_size(); i++)
	{
		if (strcmp(cur_date.c_str(), Trade_Day[i].c_str()) <= 0){
			mat_date = Trade_Day[i + 64];
			break;
		}
	}
	main_contract.OTC_Code["atm_vanila_call"] = main_contract.product + "C00-" + mat_date;
	main_contract.OTC_Code["atm_vanila_put"] = main_contract.product + "P00-" + mat_date;
	cout << main_contract.OTC_Code["atm_vanila_call"] << endl;
	cout << main_contract.OTC_Code["atm_vanila_put"] << endl;
	map<string, string>temp_data;
	temp_data["bid"] = "0";
	temp_data["ask"] = "0";
	temp_data["implied_vol"] = "0";
	temp_data["code"] = "0";
	temp_data["bid_volume"] = "0";
	temp_data["ask_volume"] = "0";
	temp_data["strike"] = "0";
	main_contract.Quote["atm_vanila_call"] = temp_data;
	main_contract.Quote["atm_vanila_put"] = temp_data;

	my_redis.HMSet(main_contract.OTC_Code["atm_vanila_call"], main_contract.Quote["atm_vanila_call"]);
	my_redis.HMSet(main_contract.OTC_Code["atm_vanila_put"], main_contract.Quote["atm_vanila_put"]);

	cout << "Valuation Initialized!" << endl;
	return;
}

void Option_Value::Start()
{
	value_flag = true;
	//hedge_flag = true;
    main_thread = std::thread(&Option_Value::main_value_process, this, this);
	//hedge_thread = thread(&Option_Value::Auto_Hedger, this, this);
	return;
}

void Option_Value::Stop()
{
	value_flag = false;
	main_thread.join();
	//hedge_flag = false;
	//hedge_thread.join();
	return;
}

void Option_Value::main_value_process(void *stParam)
{
	Option_Value *ptr = (Option_Value *)stParam;
	pricing_param temp_param;
	time_t now;
	struct tm current;
	map<string, string>update_data;
	int atm_strike,iRet;
	char *buffer = new char[1000];
	string temp_code;
	double spread;

	ptr->Get_Parameters();
	ptr->MapToParameter();
	atm_strike = value_parameter.Strike_Price;
	sprintf(buffer, "%d", atm_strike);
	temp_code = buffer;
	ptr->param_update = true;

	cout << "Main Pricing Process Start!" << endl;
	while (ptr->value_flag)
	{
		time(&now);
		current = *localtime(&now);

		iRet = my_redis.HGetAll(ptr->main_contract.contract_code, update_data);
		float last_spot = atof(update_data["LastPrice"].c_str());
        //cout << update_data["UpdateTime"]<<": "<<last_spot << endl;
        if (abs(last_spot - atm_strike)>10)
		{
			atm_strike = round(last_spot / 10) * 10;
			sprintf(buffer, "%d", atm_strike);
			temp_code = buffer;
			ptr->Parameter_Setting("Strike", temp_code);
		}
		if (param_update==true){
			ptr->main_contract.Quote["atm_vanila_call"]["strike"] = temp_code;
			ptr->main_contract.Quote["atm_vanila_put"]["strike"] = temp_code;
			//strftime(buffer, 100, "%H%M%S", &current);
			
			ptr->main_contract.Quote["atm_vanila_call"]["code"] = ptr->main_contract.OTC_Code["atm_vanila_call"]+"-"+temp_code;
			ptr->main_contract.Quote["atm_vanila_put"]["code"] = ptr->main_contract.OTC_Code["atm_vanila_put"] + "-"+temp_code;

			ptr->Get_Parameters();
			ptr->MapToParameter();

			ptr->param_lock.lock();
			temp_param = value_parameter;
			ptr->param_lock.unlock();


			temp_param.Spot_Price = last_spot;
			spread = Basis_Spread(temp_param.other_param);

			temp_param.Option_Type = 1;
			temp_param.Volatility = temp_param.Volatility - spread;
			sprintf(buffer, "%f", ptr->Option_Valuation(temp_param));
			ptr->main_contract.Quote["atm_vanila_call"]["bid"] = buffer;
			temp_param.Volatility = temp_param.Volatility + 2*spread;
			sprintf(buffer, "%f", ptr->Option_Valuation(temp_param));
			ptr->main_contract.Quote["atm_vanila_call"]["ask"] = buffer;
			ptr->main_contract.Quote["atm_vanila_call"]["bid_volume"] = "500";
			ptr->main_contract.Quote["atm_vanila_call"]["ask_volume"] = "500";

			temp_param.Option_Type = 2;
			temp_param.Volatility = temp_param.Volatility - 2 * spread;
			sprintf(buffer, "%f", ptr->Option_Valuation(temp_param));
			ptr->main_contract.Quote["atm_vanila_put"]["bid"] = buffer;
			temp_param.Volatility = temp_param.Volatility + 2 * spread;
			sprintf(buffer, "%f", ptr->Option_Valuation(temp_param));
			ptr->main_contract.Quote["atm_vanila_put"]["ask"] = buffer;
			ptr->main_contract.Quote["atm_vanila_put"]["bid_volume"] = "500";
			ptr->main_contract.Quote["atm_vanila_put"]["ask_volume"] = "500";

			sprintf(buffer, "%f", temp_param.Volatility);
			ptr->main_contract.Quote["atm_vanila_call"]["implied_vol"] = buffer;
			ptr->main_contract.Quote["atm_vanila_put"]["implied_vol"] = buffer;
			my_redis.HMSet(ptr->main_contract.OTC_Code["atm_vanila_call"], ptr->main_contract.Quote["atm_vanila_call"]);
			my_redis.HMSet(ptr->main_contract.OTC_Code["atm_vanila_put"], ptr->main_contract.Quote["atm_vanila_put"]);

			param_lock.lock();
			param_update = false;
			param_lock.unlock();
		}
        sleep(1);
	}

	delete[] buffer;
	cout << "Main Pricing Process Quit!" << endl;
	return;
}

void Option_Value::Auto_Hedger(void *stParam)
{
	Option_Value *ptr = (Option_Value *)stParam;
	time_t now;
	struct tm current;
	map<string, string>update_data;
	int net_delta, iRet;
	char *buffer = new char[1000];
	string temp_code;

	cout << "Auto Hedger Start!" << endl;
	while (ptr->hedge_flag)
	{
		time(&now);
		current = *localtime(&now);

		Total_Position = tm->getAllMainAccountPosition();
		iRet = my_redis.HGetAll(ptr->main_contract.contract_code, update_data);
		float last_spot = atof(update_data["LastPrice"].c_str());
		net_delta=ptr->Delta_Hedger(Total_Position,last_spot);
		cout << update_data["UpdateTime"] << ": " << last_spot <<"  Position Delta: "<<net_delta<< endl;
		if (abs(net_delta)>5){
			ptr->Hedger_Excute(net_delta, last_spot);
		}
        sleep(1);
	}

	delete[] buffer;

	cout << "Auto Hedger Stopped!" << endl;
	return;
}

double Option_Value::Position_Quote(const string &instr_code, LongShortType ls)
{
	double result = 0;
	double maturity, strike, basic_vola, Spot_Price;
	pricing_param temp_param;
	map<string, string> update_data;

	param_lock.lock();
	temp_param = value_parameter;
	param_lock.unlock();


	basic_vola = temp_param.Volatility;
	int iRet = my_redis.HGetAll(main_contract.contract_code, update_data);
	Spot_Price = atof(update_data["LastPrice"].c_str());
	temp_param.Spot_Price = Spot_Price;

	if (strcmp(instr_code.substr(0, 4).c_str(), "OTC-") == 0){
		int l = instr_code.length();
		int pos_first = instr_code.substr(4, l - 4).find_first_of("-") + 4;
		int pos_last = instr_code.find_last_of("-");
		string temp = instr_code.substr(pos_first + 1, 10);
		maturity = Maturity_Remain(temp);
		temp = instr_code.substr(pos_last + 1, l - pos_last - 1);
		strike = atof(temp.c_str());
		temp = instr_code.substr(pos_first - 3, 3);

		if (strcmp(temp.c_str(), "C00") == 0){ temp_param.Option_Type = 1; }
		if (strcmp(temp.c_str(), "P00") == 0){ temp_param.Option_Type = 2; }

		temp_param.Strike_Price = strike;
		temp_param.TimeToMaturity = maturity;
        double spread = Basis_Spread(temp_param.other_param);
        temp_param.Volatility = (ls == LONG ? -1 : 1) * spread/3 + Volatility_Adjustment(basic_vola, maturity, strike);

		switch (temp_param.Value_Method)
		{
		case 1: result = bls_vanila_option(temp_param); break;
		}
	}
	else if (strcmp(instr_code.substr(0, 2).c_str(), "IF") == 0)
	{
		result = Spot_Price;
	}
	else{}

	return result;
}

double Option_Value::Settle_Price(const string &instr_code, LongShortType long_short)
{
	double result = 0;
	double maturity, strike, basic_vola, Spot_Price;
	pricing_param temp_param;

	param_lock.lock();
	temp_param = value_parameter;
	param_lock.unlock();


    temp_param.Spot_Price = atof(temp_param.other_param["LastSettlePrice"].c_str());
    temp_param.Volatility = atof(temp_param.other_param["LastSettleVola"].c_str());
	Spot_Price = temp_param.Spot_Price;
	basic_vola = temp_param.Volatility;

    if (strcmp(instr_code.substr(0, 4).c_str(), "OTC-") == 0){
        int l = instr_code.length();
        int pos_first = instr_code.substr(4, l - 4).find_first_of("-") + 4;
        int pos_last = instr_code.find_last_of("-");
        string temp = instr_code.substr(pos_first + 1, 10);
		maturity = Maturity_Remain(temp);
        temp = instr_code.substr(pos_last + 1, l - pos_last - 1);
		strike = atof(temp.c_str());
        temp = instr_code.substr(pos_first - 3, 3);

		if (strcmp(temp.c_str(), "C00") == 0){ temp_param.Option_Type = 1; }
		if (strcmp(temp.c_str(), "P00") == 0){ temp_param.Option_Type = 2; }

		temp_param.Strike_Price = strike;
		temp_param.TimeToMaturity = maturity;
		double spread = Basis_Spread(temp_param.other_param);
        temp_param.Volatility = (long_short == LONG ? -1 : 1) * spread + Volatility_Adjustment(basic_vola, maturity, strike);

		switch (temp_param.Value_Method)
		{
		case 1: result = bls_vanila_option(temp_param); break;
		}
	}
    else if (strcmp(instr_code.substr(0, 2).c_str(), "IF") == 0)
	{
		result = Spot_Price;
	}
	else{}

	return result;
}

PositionRisk Option_Value::PositionGreeks(const PositionType &Position)
{
	pricing_param temp_param;
	map<string, string> update_data;
	int iRet = my_redis.HGetAll(main_contract.contract_code, update_data);
	float last_spot = atof(update_data["LastPrice"].c_str());

	param_lock.lock();
	temp_param = value_parameter;
	param_lock.unlock();

	temp_param.Spot_Price = last_spot;
	PositionRisk result;
	memset(&result, 0, sizeof(PositionRisk));
	double maturity, strike, basic_vola, Spot_Price;
	basic_vola = temp_param.Volatility;

	if (strcmp(Position.instr_code.toStdString().substr(0, 4).c_str(), "OTC-") == 0){
		int l = Position.instr_code.toStdString().length();
		int pos_first = Position.instr_code.toStdString().substr(4, l - 4).find_first_of("-") + 4;
		int pos_last = Position.instr_code.toStdString().find_last_of("-");
		string temp = Position.instr_code.toStdString().substr(pos_first + 1, 10);
		maturity = Maturity_Remain(temp);
		temp = Position.instr_code.toStdString().substr(pos_last + 1, l - pos_last - 1);
		strike = atof(temp.c_str());
		temp = Position.instr_code.toStdString().substr(pos_first - 3, 3);

		if (strcmp(temp.c_str(), "C00") == 0){ temp_param.Option_Type = 1; }
		if (strcmp(temp.c_str(), "P00") == 0){ temp_param.Option_Type = 2; }

		temp_param.Strike_Price = strike;
		temp_param.TimeToMaturity = maturity;
		temp_param.Volatility = Volatility_Adjustment(basic_vola, maturity, strike);

		switch (temp_param.Value_Method)
		{
		case 1:
		{
				  result.delta = 2 * (-Position.long_short + 0.5)*Position.total_amount*main_contract.multiplier*bls_vanila_delta(temp_param);
				  result.gamma = 2 * (-Position.long_short + 0.5)*Position.total_amount*main_contract.multiplier*bls_vanila_gamma(temp_param);
				  result.vega = 2 * (-Position.long_short + 0.5)*Position.total_amount*main_contract.multiplier*bls_vanila_vega(temp_param);
				  result.theta = 2 * (-Position.long_short + 0.5)*Position.total_amount*main_contract.multiplier*bls_vanila_theta(temp_param);
				  break;
		}
		}
	}
	else if (strcmp(Position.instr_code.toStdString().substr(0, 2).c_str(), "IF") == 0)
	{
		result.delta = 2 * (-Position.long_short + 0.5)*Position.total_amount*main_contract.multiplier * 300;
	}
	else{}

	return result;

}

double Option_Value::Price_Qoute(const string &instr_code) {
	string price;
	my_redis.HGet(main_contract.contract_code, "LastPrice", price);
	return stod(price);
}

vector<string> Option_Value::Get_Main_Contract_Codes() {
	vector<string> ret;
	ret.push_back(main_contract.contract_code);
	return ret;
}

inline
string Option_Value::getUnderlyingCode(const string &option_code) {
    string underlying_code;
    my_redis.HGet("PARAM-OTC-IFX03", "Underlying_Code", underlying_code);
    return underlying_code;
}

double Option_Value::getUnderlyingPrice(const string &instr_code) {
    string underlying_code = getUnderlyingCode(instr_code);
	string str_price;
	my_redis.HGet(underlying_code, "LastPrice", str_price);
	return stod(str_price);
}
