
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <string>
#include <sstream>
#include <time.h>
#include <QDateTime>

#include "ClientService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int ClientId;

int main(int argc, char *argv[])
{
    string ip="10.2.6.31";

    boost::shared_ptr<TTransport> socket(new TSocket(ip, 9090));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ClientServiceClient client(protocol);

     transport->open();

     //Get All Users List
     int num=13;

     //Get Necessary Contract Parameters
     int multi=100;

     string date=QDate::currentDate().toString("yyyy-MM-dd").toStdString();

    //Get Daily Settlement File
     string settle_file_name="SettleParameter--"+date+".txt";
     ifstream settle_file(settle_file_name);

     struct settledata
     {
         map<string,string> parameter;
     };

     map<string,settledata> settle_param;
     string productID;
     while (!settle_file.eof())
     {
         string temp_msg;
         settle_file>>temp_msg;
         int pos=temp_msg.find(';');
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
     struct pricing_param
     {
         double Spot_Price,Strike_Price;
         float Free_Rate,Yield_Rate,TimeToMaturity,Volatility;
         int Option_Type,Value_Method;
         map<string,string> other_param;
     };
     pricing_param settle_parameter;
     for (auto i:settle_param)
     {
         if (i.first=="IFX03")
         {
                settle_parameter.Spot_Price=stof(i.second.parameter["Underlying"]);
                i.second.parameter.erase("UnderlyingPrice");
                settle_parameter.Free_Rate=stof(i.second.parameter["Free_Rate"]);
                settle_parameter.Yield_Rate=stof(i.second.parameter["Yield_Rate"]);
                settle_parameter.TimeToMaturity=stof(i.second.parameter["Maturity"]);
                settle_parameter.Volatility=stof(i.second.parameter["Volatility"]);
                settle_parameter.Value_Method=stoi(i.second.parameter["Value_Method"]);
                settle_parameter.other_param["LastSettlePrice"]=settle_parameter.Spot_Price;
                settle_parameter.other_param["LastSettleVola"]=settle_parameter.Volatility;
         }
         else
         {

         }
     }

     for (int i=1;i<=num;i++)
     {
         stringstream ss;
         ss<<i;
         string client_id=ss.str();
         string settlefile="SettleFile_ClientNo_"+client_id+".txt";
         ofstream outfile(settlefile.c_str());
         ClientInfoTrans ClientInfo;
         client.get_clientinfo(ClientInfo,stoi(client_id));

         outfile<<"           OTC Service Settlement File For Client         "<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<endl;
         outfile<<endl;

         outfile<<"           Client Infomation"<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<setw(20)<<setfill(' ')<<"Client No."<<setw(20)<<setfill(' ')<<"ClientName"<<setw(20)<<setfill(' ')<<"ClientLevel"<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<setw(25)<<setfill(' ')<<ClientInfo.client_id<<setw(20)<<setfill(' ')<<ClientInfo.client_name<<setw(20)<<setfill(' ')<<ClientInfo.client_level<<endl;
         outfile<<endl;

        /* outfile<<"           Client Balance"<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<setw(20)<<setfill(' ')<<"Initial Balance"<<setw(20)<<setfill(' ')<<"Current Balance"<<setw(20)<<setfill(' ')<<"Account Available"<<setw(20)<<setfill(' ')<<"Frozen Margin"<<endl;
         ClientBalanceTrans Balance;
         client.get_balance(Balance,stoi(client_id));
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<setw(25)<<setfill(' ')<<100000<<setw(25)<<setfill(' ')<<Balance.total_balance<<setw(25)<<setfill(' ')<<Balance.available_balance<<setw(25)<<setfill(' ')<<Balance.occupied_margin<<endl;
         outfile<<endl;*/

         outfile<<"           Transaction History"<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         outfile<<setw(30)<<setfill(' ')<<"Trade Date"<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort"<<setw(15)<<setfill(' ')<<"Direction";
         outfile<<setw(15)<<setfill(' ')<<"Price"<<setw(15)<<setfill(' ')<<"Amount"<<endl;
         outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
         std::vector<TransactionTypeTrans> HistTrans;
         std::vector<TransactionTypeTrans>Pos_Hold;
         float Close_PnL=0;
         float Hold_PnL=0;
        client.get_transaction(HistTrans,stoi(client_id),date,date);
        for (auto i :HistTrans)
        {
            outfile<<setw(25)<<setfill(' ')<<i.date_time<<setw(35)<<setfill(' ')<<i.instr_code<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(15)<<setfill(' ')<<(i.open_offset==0 ? "Open":"Close");
            outfile<<setw(15)<<setfill(' ')<<i.price<<setw(15)<<setfill(' ')<<i.amount<<endl;
        }

        outfile<<endl;
        outfile<<"           Account Position"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        outfile<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort"<<setw(15)<<setfill(' ')<<"Price.Avg";
        outfile<<setw(15)<<setfill(' ')<<"Total No"<<setw(15)<<setfill(' ')<<"No.Avail"<<setw(15)<<setfill(' ')<<"Margin"<<setw(15)<<setfill(' ')<<"ClosePrice"<<endl;
        outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
        std::vector<PositionTypeTrans> Position;
       client.get_position(Position,stoi(client_id));
       double New_Margin=0;
       float multiplier=stoi(settle_parameter.other_param["Multiplier"]);
       float margin_rate=0.05;
       for (auto i :Position)
       {
           float close=client.get_close_price(i);
           //float settle=get_settle_price(i,pricing_param);
           outfile<<setw(35)<<setfill(' ')<<i.instr_code<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(20)<<setfill(' ')<<i.average_price;
           outfile<<setw(20)<<setfill(' ')<<i.total_amount<<setw(20)<<setfill(' ')<<i.available_amount<<setw(20)<<setfill(' ')<<0<<setw(20)<<setfill(' ')<<close<<endl;
           Hold_PnL=Hold_PnL+i.total_amount*(i.average_price-close)*(i.long_short==0 ? -1:1)*multiplier;
           if (i.long_short==1)
           {
               double new_margin=(close+settle_parameter.Spot_Price*margin_rate)*multiplier;
                New_Margin+=new_margin;
                //set position new margin
           }
       }

     outfile<<"           Close PnL Detail"<<endl;
     outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
     outfile<<setw(30)<<setfill(' ')<<"Trade Date"<<setw(50)<<setfill(' ')<<"Contract Code"<<setw(15)<<setfill(' ')<<"LongShort";
     outfile<<setw(15)<<setfill(' ')<<"Price"<<setw(15)<<setfill(' ')<<"Amount"<<setw(15)<<setfill(' ')<<"ClosePnL"<<endl;
     outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
     for (auto i :HistTrans)
     {
        if (i.open_offset==1)
        {
            outfile<<setw(25)<<setfill(' ')<<i.date_time<<setw(35)<<setfill(' ')<<i.instr_code<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short")<<setw(15)<<setfill(' ')<<(i.open_offset==0 ? "Open":"Close");
            outfile<<setw(15)<<setfill(' ')<<i.price<<setw(15)<<setfill(' ')<<i.amount<<endl;
        }
     }
     /*int Close_Num=0;
    for (auto i :HistTrans)
    {

        bool indicator=false;
        for (auto  j:Pos_Hold)
        {
            if (i.instr_code==j.instr_code)
            {
                indicator=true;
                if (i.open_offset==0)
                {
                  j.price=j.price*j.amount+i.price*i.amount;
                  j.amount=j.amount+i.amount;
                  j.price=j.price/j.amount;
                }
                else
                {
                    float temp_PnL=(j.price-i.price)*i.amount*(j.long_short==0 ? -1:1)*multi;
                    Close_PnL=Close_PnL+temp_PnL;
                    Close_Num=Close_Num+i.amount;
                    j.amount=j.amount-i.amount;
                    outfile<<setw(25)<<setfill(' ')<<i.date_time<<setw(35)<<setfill(' ')<<i.instr_code<<setw(15)<<setfill(' ')<<(i.long_short==0 ? "Long":"Short");
                    outfile<<setw(15)<<setfill(' ')<<i.price<<setw(15)<<setfill(' ')<<i.amount<<setw(15)<<setfill(' ')<<temp_PnL<<endl;
                }
                break;
            }
        }
       if (indicator==false)
       {
           Pos_Hold.push_back(i);
       }
    }
    outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    outfile<<setw(30)<<setfill(' ')<<"Total Close Amount: "<<setw(50)<<setfill(' ')<<Close_Num;
    outfile<<setw(30)<<setfill(' ')<<"Total Close PnL: "<<setw(50)<<setfill(' ')<<Close_PnL;
    outfile<<endl;*/



     outfile<<"           Client Balance"<<endl;
     outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
     //Set Balance Occupied Margin
     //Set Balance Available_Balance
     //Set Balance Withdraw_Balance
     outfile<<setw(20)<<setfill(' ')<<"Initial Balance"<<setw(20)<<setfill(' ')<<"Total Balance"<<setw(20)<<setfill(' ')<<"Account Available"<<setw(20)<<setfill(' ')<<"Market Value";
     outfile<<setw(20)<<setfill(' ')<<"Cash WithDraw"<<setw(20)<<setfill(' ')<<"Frozen Margin"<<endl;
     ClientBalanceTrans Balance;
     client.get_balance(Balance,stoi(client_id));
     outfile<<"------------------------------------------------------------------------------------------------------------------------------------------------------------------------"<<endl;
     outfile<<setw(25)<<setfill(' ')<<100000<<setw(25)<<setfill(' ')<<Balance.total_balance<<setw(25)<<setfill(' ')<<Balance.available_balance<<setw(25)<<setfill(' ')<<100000+Close_PnL+Hold_PnL;
     outfile<<setw(25)<<setfill(' ')<<0<<setw(25)<<setfill(' ')<<Balance.occupied_margin<<endl;
     outfile<<endl;
     }


   cout<<"Settlement Program Finish!"<<endl;
   int cc;
   cin>>cc;

}
