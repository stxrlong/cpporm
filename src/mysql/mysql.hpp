//
//  mysql.h
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef mysql_h
#define mysql_h

#include "mysqlconnpool.hpp"
#include "models/mysqltable.hpp"

namespace cpporm {

enum class MysqlRet{
    SUCCESS,
    IS_NULL,
    UNKNOWN_FIELD,
    UNKNOWN_FIELD_TYPE
};

#define ASSERTMYSQL(mysql) \
    if(mysql_ == nullptr){      \
        std::cout << "mysql is nullptr" << std::endl;       \
        return false;       \
    }

class Mysql {
public:
    Mysql(MysqlConnPool *mcp = &cpporm::MysqlConnPool::instance()){
        if(mcp){
            mcp_ = mcp;
            mysql_ = mcp_->acquire();
        }
    }
    
    ~Mysql(){
        if(mysql_ != nullptr){
            mcp_->retConn(mysql_);
        }
    }

public:
    template<typename Result>
    std::pair<MysqlRet, Result> selectOne(const std::string& sql){
        Result r;
        MysqlRet ret = MysqlRet::IS_NULL;
        
        if(mysql_ != nullptr){
            MysqlResult *mr = mysql_->executeQuery(sql);
            if(mr != nullptr && mr->next()){
                ret = fetchResult(mr, r);
            }else {
                std::cout<< "execute sql error" << std::endl;
            }
        }else
            std::cout<<"mysqlconnection is null" <<std::endl;
        
        return std::make_pair(ret, std::move(r));
    }
    
    template <typename Result>
    std::pair<MysqlRet, std::vector<Result>> selectList(const std::string& sql){
        std::vector<Result> results;
        MysqlRet ret = MysqlRet::IS_NULL;
        
        if(mysql_ != nullptr){
            MysqlResult *mr = mysql_->executeQuery(sql);
            if(mr != nullptr){
                while(mr->next()){
                    Result r;
                    if((ret=fetchResult(mr, r))==MysqlRet::SUCCESS)
                        results.push_back(std::move(r));
                }
            }
        }
        
        return std::make_pair(ret, std::move(results));
    }
    
    template<typename Obj>
    bool insert(Obj& val){
        ASSERTMYSQL(mysql_)
        std::string sql = SqlMake::instance().makeTemInsertSql(val);
        std::cout<<"insert sql:"<<sql<<std::endl;
        return mysql_->executeInsert(sql);
    }
    
    template<typename Obj>
    bool insert(std::vector<Obj> vals){
        ASSERTMYSQL(mysql_)
        
        return true;
    }

protected:
    template<typename Result>
    MysqlRet fetchResult(MysqlResult *mr, Result& result){
        db_ty::tbl_class *t = Result::get_class_ptr();
        db_ty::dbTypeMap *fields = t->get_fields();
        
        for (auto& field : *fields) {
            auto f = field.second;
            switch (f.get_fld_ty()) {
                case (int)FieldType::INT8:{
                    int8_t tmp = (int8_t)mr->getInt(f.get_key());
                    f.set(&result, tmp);
                    break;
                }
                case (int)FieldType::INT16:{
                    int16_t tmp = (int16_t)mr->getInt(f.get_key());
                    f.set(&result, tmp);
                    break;
                }
                case (int)FieldType::INT32:{
                    int32_t tmp = mr->getInt(f.get_key());
                    f.set(&result, tmp);
                    break;
                }
                case (int)FieldType::INT64:{
                    int64_t tmp = mr->getInt64(f.get_key());
                    f.set(&result,tmp);
                    break;
                }
                case (int)FieldType::STRING:{
                    std::string tmp = mr->getString(f.get_key());
                    f.set(&result,tmp);
                    break;
                }
                case (int)FieldType::BOOL:{
                    int64_t tmp = mr->getBoolean(f.get_key());
                    f.set(&result,tmp);
                    break;
                }
                default:{
                    std::cout<< "fetch result error:unknown field type"<<std::endl;
                    return MysqlRet::UNKNOWN_FIELD_TYPE;
                }
            }
        }
        
        std::cout<<"fetch result success"<<std::endl;
        return MysqlRet::SUCCESS;
    }
    
    MysqlConnPool *mcp_ = nullptr;
    MysqlConnection *mysql_ = nullptr;
};

class MysqlPrep: public Mysql{
public:
    MysqlPrep(){}
    ~MysqlPrep(){}

    template<typename Result>
    std::pair<MysqlRet, Result> selectOne(const std::string& label, ParamMap& conditions){
        Result result;
        MysqlRet ret = MysqlRet::IS_NULL;
        
        //prepare before execute
        if(prePrepStat<Result>(label, conditions)){
            //execute and fetch result
            MysqlResult *mr = nullptr;
            mr = mysql_->executeQuery();
            //if more than 1 entry is found, only the last one will be return.
            while(mr->next()) {
                ret = fetchResult(mr, result);
            }
            
            delete mr;
        }
        
        return std::make_pair(ret, std::move(result));
    }
    
    template<typename Result>
    std::pair<MysqlRet, std::vector<Result>> selectList(const std::string& label, ParamMap& conditions){
        std::vector<Result> results;
        MysqlRet ret = MysqlRet::IS_NULL;
        
        //prepare before execute
        if(prePrepStat<Result>(label, conditions)){
            //execute and fetch result
            MysqlResult *mr = nullptr;
            mr = mysql_->executeQuery();
            if (mr != nullptr) {
                while(mr->next()){
                    Result result;
                    if((ret = fetchResult(mr, result)) != MysqlRet::SUCCESS){
                        std::cout<<"fetch result error." << std::endl;
                        break;
                    }
                    results.push_back(std::move(result));
                }
            }
        }

        return std::make_pair(ret, std::move(results));
    }
    
private:
    template<typename Result>
    bool prePrepStat(const std::string& label, ParamMap& conditions){
        ASSERTMYSQL(mysql_)
        
        bool ret = true;
        // create the preparedstatement
        db_ty::tbl_class *t = nullptr;
        if(!mysql_->acquirePtst(label)){
            t = Result::get_class_ptr();
            std::string sql = t->getSql(label);
            if(!mysql_->createPtst(label, sql)){
                std::cout<<"create preparedstatement failed."<<std::endl;
                ret = false;
            }
        }
        
        //set parameters
        int conditionSize = (int)conditions.size();
        int index = 1;
        for(int i = 1; i <= conditionSize; i++){
            if(!mysql_->setParam(index++, conditions[i])){
                std::cout<<"set parameter error: condition ["<<i<<"]"<<std::endl;
                ret = false;
                break;
            }
        }
        
        return ret;
    }

};


}/*namespace cpporm*/
#endif /* mysql_h */
