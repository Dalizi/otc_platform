#include "accessredis.h"

#include <cstring>


CAccessRedis::CAccessRedis(void)
{
    m_context = NULL;

}


CAccessRedis::~CAccessRedis(void){
    if (m_context != NULL)
    {
        redisFree(m_context);
    }

}


// 连接
int CAccessRedis::Connect(const string &strHost,int iPort, const string &strPwd, time_t timeout)
{
    //redis默认监听端口为6387 可以再配置文件中修改
    struct timeval t;
    t.tv_sec = timeout;
    t.tv_usec = 0;
    if(m_context == NULL)
    {
        m_context = redisConnectWithTimeout(strHost.c_str(), iPort, t);
        if (m_context->err)
        {
            int err = m_context->err;
            redisFree(m_context);
            m_context = NULL;
            return err;
        }
    }

    if (strPwd == "")
        return 0;

    stringstream ss;
    ss<<"AUTH "<<strPwd;
    return CommitNoRetCommond(ss.str());
}

// select 选择数据库
int CAccessRedis::Select(int iIndex)
{
    stringstream ss;
    ss<<"select "<<iIndex;
    return CommitNoRetCommond(ss.str());
}

// set
int CAccessRedis::Set(const string& strKey,const string& strVal)
{
    stringstream ss;
    ss<<"set "<<strKey<<" "<<strVal;
    return CommitNoRetCommond(ss.str());
}

// get
int CAccessRedis::Get(const string&key,vector<string> &vec)
{
    stringstream ss;
    vec.clear();

    ss<<"get "<<key;
    return CommitHaveRetCommond(ss.str(),vec);
}

// del
int CAccessRedis::Del(const string&strkey)
{
    stringstream ss;
    ss<<"del "<<strkey;
    return CommitNoRetCommond(ss.str());
}

// keys
int CAccessRedis::Keys(const string& strkey,vector<string> &vec)
{
    stringstream ss;
    vec.clear();
    ss<<"keys "<<strkey;
    return CommitHaveRetCommond(ss.str(),vec);
}

// hset 设置单值
int CAccessRedis::HSet(const string& strKey,const string& strField,const string& strVal)
{
    stringstream ss;
    ss<<"hset "<<strKey<<" "<<strField<<" "<<strVal;
    return CommitNoRetCommond(ss.str());
}

// hmset 设置多值
int CAccessRedis::HMSet(const string& strKey,map<string,string>& mapVals)
{
    stringstream ss;
    stringstream msg;
    map<string,string>::iterator itr;

    ss<<"hmset "<<strKey;

    for (itr = mapVals.begin();itr != mapVals.end();itr++)
    {
        ss<<" "<<itr->first<<" "<<itr->second;
    }

    for (itr = mapVals.begin(); itr != mapVals.end(); ++itr) {
        if (itr == mapVals.begin())
            msg <<itr->first <<":" <<itr->second;
        else
            msg <<"," <<itr->first <<":" <<itr->second;
    }

    int iRet = CommitNoRetCommond(ss.str());
    Publish(strKey, msg.str());
    return iRet;
}

// hget 获取单值
int CAccessRedis::HGet(const string& strKey,const string& field,string& strVal)
{
    stringstream ss;
    vector<string> vec;

    ss<<"hget "<<strKey<<" "<<field;

    int iRet = CommitHaveRetCommond(ss.str(),vec);
    if (iRet == 0)
    {
        if (vec.size() == 0) {
            throw runtime_error("Field doesn't exists.");
        }
        strVal = vec[0];
    }

    return iRet;
}

// hmget 获取多值
int CAccessRedis::HMGet(const string& strKey,vector<string>& vecField,map<string,string>& mapKV)
{
    stringstream ss;
    vector<string> vec;
    vector<string>::iterator itrVec;

    mapKV.clear();
    ss<<"hmget "<<strKey;
    for (itrVec = vecField.begin();itrVec != vecField.end();itrVec++)
    {
        ss<<" "<<*itrVec;
    }

    int iRet = CommitHaveRetCommond(ss.str(),vec);
    if (iRet == 0)
    {
        int iLen = vec.size();
        for (int i=0;i<iLen;i+=2)
        {
            mapKV[vec[i]] = vec[i+1];
        }
    }

    return iRet;
}

// hgetall 获取所有值
int CAccessRedis::HGetAll(const string& strKey,map<string,string>& mapKV)
{
    stringstream ss;
    vector<string> vec;
    vector<string>::iterator itrVec;

    mapKV.clear();
    ss<<"hgetall "<<strKey;

    int iRet = CommitHaveRetCommond(ss.str(),vec);
    if (iRet == 0)
    {
        int iLen = vec.size();
        for (int i=0;i<iLen;i+=2)
        {
            mapKV[vec[i]] = vec[i+1];
        }
    }

    return iRet;
}

// hdel 删除指定值
int CAccessRedis::HDel(const string& strKey,vector<string>& vecField)
{
    stringstream ss;
    vector<string>::iterator itr;

    ss<<"hdel "<<strKey;

    for (itr = vecField.begin();itr != vecField.end();itr++)
    {
        ss<<" "<<*itr;
    }

    return CommitNoRetCommond(ss.str());
}

//////////////////////////////////////////////////////////////////////////
// 列表

// rpush 尾端添加一个或多个值
int CAccessRedis::RPush(const string& strKey,vector<string>& vecVals)
{
    stringstream ss;
    vector<string>::iterator itr;

    ss<<"rpush "<<strKey;

    for (itr = vecVals.begin();itr != vecVals.end();itr++)
    {
        ss<<" "<<*itr;
    }

    return CommitNoRetCommond(ss.str());
}

// lrange
int CAccessRedis::LRange(const string& strKey,int iBegin,int iEnd,vector<string>& vecVals)
{
    stringstream ss;
    vecVals.clear();
    ss<<"lrange "<<strKey<<" "<<iBegin<<" "<<iEnd;

    return CommitHaveRetCommond(ss.str(),vecVals);
}

//////////////////////////////////////////////////////////////////////////
//
int CAccessRedis::CommitNoRetCommond(const string& strCmd)
{
	unique_lock<mutex> lck(redis_mtx);
    redisReply* r = (redisReply*)redisCommand(m_context,strCmd.c_str());

    if( NULL == r)
        return -10;

    if(r->type == REDIS_REPLY_ERROR)
    {
        cerr <<"Redis Error: "<<r->str <<endl;
        freeReplyObject(r);
        return -1;
    }
    freeReplyObject(r);
    return 0;
}

void CAccessRedis::Subscribe(const string& channel, function<void(const string&, const string&, void*)> fn, void* privdata) {
    stringstream ss;
    ss << "SUBSCRIBE " <<channel;

    redisReply *reply = (redisReply*) redisCommand(m_context, ss.str().c_str());
    void *tmp = NULL;
    freeReplyObject(reply);
    while(redisGetReply(m_context, &tmp) == REDIS_OK) {
        reply = (redisReply*) tmp;
        if (reply->type == REDIS_REPLY_ARRAY && reply->elements ==3){
            if (strcmp(reply->element[2]->str, "KILL") == 0) {
                cout <<"结束监听频道: " <<reply->element[1]->str <<endl;
                break;
            }
            string channel(reply->element[1]->str);
            string msg(reply->element[2]->str);
            fn(channel, msg, privdata);
        }
        else {
            freeReplyObject(reply);
        }
    }
}

/*
template <typename T>
int CAccessRedis::Publish(const string& key, T msg) {
    stringstream ss;
    ss <<"Publish " <<key <<" " <<msg;

    return CommitNoRetCommond(ss.str());
}*/

int CAccessRedis::UnSubscribe(const string& channel) {
    stringstream ss;
    ss <<"UNSUBSCRIBE " <<channel;

    return redisAppendCommand(m_context, ss.str().c_str());
}
//
int CAccessRedis::CommitHaveRetCommond(const string& strCmd,vector<string>& vecVals)
{
	unique_lock<mutex> lck(redis_mtx);
    redisReply* r = (redisReply*)redisCommand(m_context,strCmd.c_str());

    if( NULL == r)
        return -10;

    if(r->type == REDIS_REPLY_ERROR)
    {
        cerr <<"Redis Error: "<<r->str <<endl;
        freeReplyObject(r);
        return -1;
    }
    GetData(r,vecVals);
    freeReplyObject(r);
    return 0;
}


//
int CAccessRedis::GetData(redisReply* r,vector<string>& vec)
{
    stringstream ss;
    switch (r->type)
    {
    case REDIS_REPLY_NIL:
        {}
        break;
    case REDIS_REPLY_ARRAY:
        {
            redisReply* pTmp = NULL;
            int iLen = r->elements;
            for (int i = 0;i<iLen;i++)
            {
                pTmp = *(r->element + i);
                GetData(pTmp,vec);
            }

        }
        break;
    case REDIS_REPLY_INTEGER:
        {
            ss<<r->integer;
            vec.push_back(ss.str());
            ss.str("");
        }
        break;
    default:
        {
            ss<<r->str;
            vec.push_back(ss.str());
            ss.str("");
        }
        break;
    }

    return 0;
}

int CAccessRedis::Flush() {
	stringstream ss;
	ss << "FLUSHDB";
	return  CommitNoRetCommond(ss.str());
}
