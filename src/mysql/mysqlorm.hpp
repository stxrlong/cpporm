//
//  mysqlorm.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef mysqlorm_hpp
#define mysqlorm_hpp

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>

#include "core/orm.hpp"

namespace cpporm {

enum class FieldType{
    INT8, INT16, INT32, INT64,

    UINT8, UINT16, UINT32, UINT64,

    BOOL,

    FLOAT, DOUBLE,

    STRING, VCHAR,

    BYTES, BYTES_TINY, BYTES_MEDIUM, BYTES_LONG,

    OBJECT
};

class SqlMake
{
public:
    SqlMake(){};
    ~SqlMake(){};

    static SqlMake &instance() {
        static SqlMake sm;
        return sm;
    }

    template <typename Obj>
    std::string makeTemInsertSql(Obj& obj);
    
    template <typename Obj>
    std::string makeTemInsertSql(std::vector<Obj>& objs);
    
    template <typename Obj>
    std::string& makeInsertSql();

    
private:
    std::map<std::string, std::string> sqls;

    std::mutex mutex_;
};

template <typename Obj>
std::string SqlMake::makeTemInsertSql(Obj& obj){
    std::ostringstream fieldsOs;
    std::ostringstream valuesOs;
    
    db_ty::tbl_class *t = Obj::get_class_ptr();
    db_ty::dbTypeMap *fields = t->get_fields();
    
    for (auto& field : *fields) {
        auto f = field.second;
        fieldsOs<<f.get_key()<<",";
        switch (f.get_fld_ty()) {
            case (int)FieldType::INT8:{
                int8_t tmp = 0;
                f.get(&obj, tmp);
                valuesOs<<tmp<<",";
                break;
            }
            case (int)FieldType::INT16:{
                int16_t tmp = 0;
                f.get(&obj, tmp);
                valuesOs<<tmp<<",";
                break;
            }
            case (int)FieldType::INT32:{
                int32_t tmp = 0;
                f.get(&obj, tmp);
                valuesOs<<tmp<<",";
                break;
            }
            case (int)FieldType::INT64:{
                int64_t tmp = 0;
                f.get(&obj, tmp);
                valuesOs<<tmp<<",";
                break;
            }
            case (int)FieldType::STRING:{
                std::string tmp;
                f.get(&obj, tmp);
                if(tmp.empty()){
                    valuesOs<<"'',";
                }else{
                    valuesOs<<"'"<<tmp<<"',";
                }
                break;
            }
            case (int)FieldType::BOOL:{
                bool tmp;
                f.get(&obj, tmp);
                valuesOs<<tmp<<",";
                break;
            }
            default:{
                std::cout<< "make insert sql error:unknown field type"<<std::endl;
            }
        }
    }
    
    std::string fieldsStr = fieldsOs.str();
    std::string valuesStr = valuesOs.str();
    std::string sql = "INSERT INTO " + obj.get_tbl_nm() + " (";
    sql.append(t->get_class()).append(fieldsStr.substr(0, fieldsStr.length() - 1)).append(") VALUES (").append(valuesStr.substr(0, valuesStr.length() - 1)).append(");");
    
    return sql;
}

template <typename Obj>
std::string SqlMake::makeTemInsertSql(std::vector<Obj>& objs){
    //to do;
    return "";
}


template <typename Obj>
std::string& SqlMake::makeInsertSql() {
    std::string key = typeid(Obj).name();
    //to do
    std::string sql;
    
    std::lock_guard<std::mutex> lock(mutex_);
    sqls.insert(std::map<std::string, std::string>::value_type(key, sql));
    return sqls[key];
}

}/*namespace cpporm*/

#endif /* mysqlorm_hpp */
