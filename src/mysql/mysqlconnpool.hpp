//
//  mysqlconnpool.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef mysqlconnpool_hpp
#define mysqlconnpool_hpp

#include "core/pool.hpp"
#include "utils/stringutil.hpp"
#include "mysqlconn.hpp"

namespace cpporm {

class MysqlConnPool: public db_ty::ConnectionPool<MysqlConnection>
{
	using ConnPool = db_ty::ConnectionPool<MysqlConnection>;
    using MysqlConnectOption = sql::ConnectOptionsMap;
public:
    MysqlConnPool(){};
    ~MysqlConnPool(){};
    
    static MysqlConnPool &instance(){
        static MysqlConnPool mcp;
        return mcp;
    }

public:
    // url format: [mysql://username:password@ip:port/db?key=value&key=value...]
    bool init(std::string& url){
        return parseMysqlUrl(url);
    }
    
    int retConn(MysqlConnection *conn){
        conn->clear();
        return this->release(conn);
    }

private:
    bool parseMysqlUrl(std::string& url);
    
	MysqlConnection* create() final;

    MysqlConnectOption options_;
}; /*MysqlConnPool*/

// url format: [mysql://username:password@ip:port/db?key=value&key=value...]
bool MysqlConnPool::parseMysqlUrl(std::string &url){
    std::string scheme("mysql://");
    if (url.find(scheme.c_str(),0)==std::string::npos) {
        std::cout<<"not a mysql url"<<std::endl;
        return false;
    }
    
    std::string mysqlUrl = url.substr(scheme.length());
    stringVector conn_infos = StringUtil::split(mysqlUrl, "?");
    stringVector conn_body = StringUtil::split(conn_infos.front(),"/");
    stringVector conn_addr = StringUtil::split(conn_body.front(),"@");
    stringVector conn_host = StringUtil::split(conn_addr.back(),":");
    stringVector conn_usr_pwd = StringUtil::split(conn_addr.front(),":");
    
    options_["hostName"] = conn_host.front();
    options_["port"] = StringUtil::string2int(conn_host.back());
    options_["userName"] = conn_usr_pwd.front();
    options_["password"] = conn_usr_pwd.back();
    options_["schema"] = conn_body.back();
    
    stringVector conn_params = StringUtil::split(conn_infos.back(),"&");
    int maxConn = 0;
    for (std::string s : conn_params){
        stringVector tmp = StringUtil::split(s,"=");
        if(tmp.front().compare("maxConn")==0)
            maxConn = StringUtil::string2int(tmp.back());
    }
    
    std::cout << "parse mysql url ok, maxConn is "<<maxConn<<std::endl;
    setDefConnNum(maxConn);
    return true;
}

MysqlConnection* MysqlConnPool::create() {
    MysqlConn *mc = nullptr;
    try {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        if(driver == nullptr){
            std::cout<< "get mysql driver instance error"<<std::endl;
            return nullptr;
        }
        mc = driver->connect(options_);
    } catch (MysqlException& e) {
        std::cout<<"create mysql connection error" <<std::endl;
        std::cout<<"["<<e.getErrorCode()<<","<<e.getSQLStateCStr()<<"]"<<e.what()<<std::endl;
        return nullptr;
    }
    
    if(mc == nullptr){
//        std::cout<<"create mysql connection error" <<std::endl;
        return nullptr;
    }
    
    return new MysqlConnection(mc);
}

} /*namespace cpporm */

#endif /* mysqlconnpool_hpp */
