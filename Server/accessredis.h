#ifndef CACCESSREDIS_H
#define CACCESSREDIS_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <cstring>
#include <mutex>
#include <time.h>
using namespace std;

#include "hiredis/hiredis.h"



class CAccessRedis
{
public:
    CAccessRedis(void);
    ~CAccessRedis(void);

public:
    // 连接
    // return: 0:OK -1:net error  -2:pwd error  -10:command err
    int Connect(const string &strHost, int iPort, const string &strPwd = "", time_t timeout = 5);

    // select 选择数据库
    int Select(int iIndex);

    // set 创建或设置值
    int Set(const string& strKey,const string& strVal);

    // get 获取值
    int Get(const string& key,vector<string> &vec);

	int Flush();

    // del 删除key
    int Del(const string& strkey);

    // keys 列出所有值
    int Keys(const string& strkey,vector<string> &vec);

    //////////////////////////////////////////////////////////////////////////
    // 哈希表

    // hset 设置单值
    int HSet(const string& strKey,const string& strField,const string& strVal);

    // hmset 设置多值
    int HMSet(const string& strKey,map<string,string>& mapVals);

    // hget 获取单值
    int HGet(const string& strKey,const string& strField,string& strVal);

    // hmget 获取多值
    int HMGet(const string& strKey,vector<string>& vecField,map<string,string>& mapKV);

    // hgetall 获取所有值
    int HGetAll(const string& strKey,map<string,string>& mapKV);

    // hdel 删除指定值
    int HDel(const string& strKey,vector<string>& vecField);

    //////////////////////////////////////////////////////////////////////////
    // 列表

    // rpush 尾端添加一个或多个值
    int RPush(const string& strkey,vector<string>& vecVals);

    // lrange
    int LRange(const string& strkey,int iBegin,int iEnd,vector<string>& vecVals);

    int CommitNoRetCommond(const string& strCmd);

    void ClientSetName(const string& name){
        stringstream ss;
        ss<<"CLIENT SETNAME "<<name;
        CommitNoRetCommond(ss.str());
    }

    void Subscribe(const string& channel, function<void(const string&, const string&, void*)>, void* privdata);

    int UnSubscribe(const string &channel="");
    //
    template <typename T>
    int Publish(const string& key, T msg) {
        stringstream ss;
        ss <<"Publish " <<key <<" " <<msg;

        return CommitNoRetCommond(ss.str());
    }
private:
    //
    int CommitHaveRetCommond(const string& strCmd,vector<string>& vecVals);

    //
    int GetData(redisReply* r,vector<string>& vec);

private:
    redisContext* m_context;
	mutex redis_mtx;

};

#endif // CACCESSREDIS_H
