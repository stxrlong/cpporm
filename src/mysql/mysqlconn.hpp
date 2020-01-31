//
//  mysqlconn.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef mysqlconn_hpp
#define mysqlconn_hpp

#include <iostream>
#include <core/orm.hpp>
#include "mysql/jdbc.h"

using MysqlConn = sql::Connection;
using MysqlPtst = sql::PreparedStatement;
using Mysqlstmt = sql::Statement;
using MysqlException = sql::SQLException;
using MysqlResult = sql::ResultSet;

namespace cpporm {

class MysqlConnection{
    enum executeType{
        SELECT,
        PREPARE_SELECT,
        INSERT,
        PREPARE_INSERT,
        UPDATE,
        PREPARE_UPDATE,
        SET_PARAM
    };
    
    using MysqlConnPrepMap = std::map<std::string, MysqlPtst*>;
    
public:
    MysqlConnection(MysqlConn* conn):conn_(conn),reties(3),currentPtst(nullptr){}

    ~MysqlConnection(){
        if(conn_!=nullptr)
            delete conn_;
        
        for(MysqlConnPrepMap::iterator it = connPtsts.begin();it!=connPtsts.end();){
            if(it->second != nullptr){
                delete it->second;
            }
            connPtsts.erase(it++);
        }
    }

    bool acquirePtst(const std::string& label);
    bool createPtst(const std::string& label, const std::string& sql);
    bool setParam(int index, Parameter condition);
    
    MysqlResult *executeQuery();
    MysqlResult *executeQuery(const std::string& sql);
    
    bool executeInsert();
    bool executeInsert(const std::string& sql);

    void clear();
    
    bool isConnected();
    bool reConnect();

private:
    bool analyzeError(const MysqlException& e, const executeType& type);
    
    MysqlConn *conn_;
    MysqlConnPrepMap connPtsts;
    
    MysqlPtst *currentPtst;
    
    int reties;
}; /*class MysqlConn*/

bool MysqlConnection::acquirePtst(const std::string& label){
    bool ret = false;
    MysqlConnPrepMap::iterator mcpm = connPtsts.find(label);
    if(mcpm != connPtsts.end() && mcpm->second != nullptr){
        currentPtst = mcpm->second;
        ret = true;
    }
    
    return ret;
}

bool MysqlConnection::createPtst(const std::string& label, const std::string& sql){
    bool ret = false;
    
    try {
        currentPtst = conn_->prepareStatement(sql);
        ret = true;
    } catch (MysqlException& e) {
        std::cout<<e.what()<<std::endl;
    }
    
    if(ret){
        connPtsts.insert(MysqlConnPrepMap::value_type(label, currentPtst));
    }
    
    return ret;
}

bool MysqlConnection::setParam(int index, Parameter condition){
    try {
        if(condition.type() == typeid(int).name()){
            currentPtst->setInt(index, *condition.get<int>());
        }else if(condition.type() == typeid(bool).name()){
            currentPtst->setBoolean(index, *condition.get<bool>());
        }else if(condition.type() == typeid(std::string).name()){
            currentPtst->setString(index, *condition.get<std::string>());
        }else if(condition.type() == typeid(const char*).name()){
            currentPtst->setString(index, *condition.get<char *>());
        }else{
            std::cout << "unknown parameter type." << std::endl;
        }
    } catch (const MysqlException& e) {
        analyzeError(e, SET_PARAM);
    }
    
    return true;
}

MysqlResult *MysqlConnection::executeQuery(){
    MysqlResult *mr = nullptr;
    
    if (currentPtst != nullptr) {
        try {
            mr = currentPtst->executeQuery();
        } catch (const MysqlException& e) {
            //rescuse to retry 3 times if any mysql error occure.
            if(analyzeError(e,PREPARE_SELECT))
                return executeQuery();
        }
    }
    
    return mr;
}

MysqlResult* MysqlConnection::executeQuery(const std::string& sql){
    MysqlResult *mr = nullptr;
    Mysqlstmt *st = nullptr;
    try {
        st = conn_->createStatement();
        mr = st->executeQuery(sql);
    } catch (const MysqlException& e) {
        //rescuse to retry 3 times if any mysql error occure.
        if(analyzeError(e,SELECT))
            return executeQuery(sql);
    }
    
    if (st)
        delete st;
    
    return mr;
}

bool MysqlConnection::executeInsert(){
    try {
        return currentPtst->execute();
    } catch (const MysqlException& e) {
        //rescuse to retry 3 times if any mysql error occure.
        if(analyzeError(e,PREPARE_INSERT))
            return executeInsert();
    }
    
    return false;
    
}

bool MysqlConnection::executeInsert(const std::string& sql){
    try {
        Mysqlstmt *st = conn_->createStatement();
        // i don't known why ret is false when execute successfully;
        return !st->execute(sql);
    } catch (const MysqlException& e) {
        //rescuse to retry 3 times if any mysql error occure.
        if(analyzeError(e,INSERT))
            return executeInsert(sql);
    }
    
    std::cout<< "execute result error."<<std::endl;
    return false;
}

void MysqlConnection::clear(){
    currentPtst = nullptr;
}

bool MysqlConnection::analyzeError(const MysqlException& e, const executeType& type){
    std::string error;
    switch (type) {
        case SELECT:
            error.assign("select error:[");
            break;
        case PREPARE_SELECT:
            error.assign("prepare select error:[");
            break;
        case INSERT:
            error.assign("insert error:[");
            break;
        case PREPARE_INSERT:
            error.assign("prepare insert error:[");
            break;
        case UPDATE:
            error.assign("update error:[");
            break;
        case PREPARE_UPDATE:
            error.assign("prepare update error:[");
            break;
        case SET_PARAM:
            error.assign("set param error:[");
            break;
        default:
            break;
    }
    
    std::cout << error.append(e.getSQLState()).append("]").append(e.what())<<std::endl;
    
    return false;
}

bool MysqlConnection::isConnected(){
    return false;
}

bool MysqlConnection::reConnect(){
    return false;
}

} /* namespace cpporm */

#endif /* mysqlconn_hpp */
