//
//  reflection.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef reflection_hpp
#define reflection_hpp

#include <iostream>

#include "stringutil.hpp"

#define _OFFSET_(_Obj_Ty,_Key)      \
    ((unsigned long)(&((_Obj_Ty *)0)->_Key))

#define TABLE(_Obj_Ty,_Tbl_nm)      \
class _Obj_Ty {     \
    std::string _tbl_nm=#_Tbl_nm;     \
public:      \
    _Obj_Ty() = default;        \
    ~_Obj_Ty() = default;       \
    static db_ty::tbl_class * get_class_ptr(){       \
        static db_ty::tbl_class __class_##_Obj_Key##__;     \
        return &__class_##_Obj_Key##__;     \
    }       \
    class __class_register_##_Obj_Ty##__{       \
    public:     \
        __class_register_##_Obj_Ty##__(){       \
            static db_ty::__class_register__ reg_##_Obj_Ty(     \
                #_Tbl_nm,_Obj_Ty::get_class_ptr());      \
        }       \
    }_Obj_Ty##_register;      \
    std::string& get_tbl_nm(){       \
        return _tbl_nm;     \
    }

#define FIELD(_Obj_Ty,_Fld_Ty,_Fld_Key,_SLF_FLD_KEY)     \
public:     \
    _Fld_Ty _Fld_Key;       \
private:        \
    class __field_register_##_Fld_Key##__{       \
    public:     \
        __field_register_##_Fld_Key##__(){       \
            static db_ty::__field_register__ reg_##_Fld_Key(      \
                _Obj_Ty::get_class_ptr(),       \
                _OFFSET_(_Obj_Ty,_Fld_Key),       \
                #_Fld_Key,      \
                (int8_t)_SLF_FLD_KEY);        \
        }       \
    }_Fld_Key##_register;

#define SQLSTRING(_Obj_Ty,label,sql)     \
private:        \
    class __sql_register_##label##__{       \
    public:         \
        __sql_register_##label##__(){        \
            static db_ty::__sql_register__ reg_##label(     \
                _Obj_Ty::get_class_ptr(), _label, _sql);        \
        }        \
    private:        \
        std::string _label = #label;        \
        std::string _sql = sql;        \
    }label##_register;

#define ENDTABLE };

namespace db_ty
{
class tbl_field
{
private:
    unsigned long _offset;
    std::string _key;
    int _field_ty;
public:
    tbl_field(unsigned long& offset, std::string& key, int8_t field_ty)
        : _offset(offset), _key(key), _field_ty(field_ty) {}
    
    tbl_field(const tbl_field &field){
        this->_offset = field._offset;
        this->_key = field._key;
        this->_field_ty = field._field_ty;
    }
public:
    template<typename _Obj_Ty, typename _Value_Ty>
    void get(_Obj_Ty *obj, _Value_Ty &value){
        value = *((_Value_Ty *)((unsigned char *)obj + _offset));
    }
    
    template<typename _Obj_Ty, typename _Value_Ty>
    void set(_Obj_Ty *obj, const _Value_Ty &value){
        *((_Value_Ty *)((unsigned char *)obj + _offset)) = value;
    }
    
    std::string get_key() const{
        return this->_key;
    }
    
    int get_fld_ty() const{
        return this->_field_ty;
    }
};

using dbTypeMap = std::map<std::string, tbl_field>;

class tbl_class{
private:
    dbTypeMap _field_map;
    stringMap _sqls;
    std::string _key;
    
public:
    dbTypeMap* get_fields(){
        return &this->_field_map;
    }
    
    tbl_field get_field(const std::string& key){
        dbTypeMap::iterator itr = _field_map.find(key);
        if (itr == _field_map.end())
            std::cout << "throw an field null exception" << std::endl;

        return itr->second;
    }
    
    void add_field(const tbl_field& field){
        _field_map.insert(dbTypeMap::value_type(field.get_key(), field));
    }
    
    std::string &getSql(const std::string& label){
        stringMap::iterator itr = _sqls.find(label);
        if(itr == _sqls.end()){
            std::cout << "can not find sql label:"<< label << std::endl;
        }
        return itr->second;
    }
    
    void add_sql(const std::string& label, const std::string& sql){
        _sqls.insert(stringMap::value_type(label, sql));
    }

    std::string get_class() const{
        return this->_key;
    }
    
private:
    intMap analyseSql(const std::string& sql){
        intMap im;
        // todo
        return im;
    }
};

using TableMap = std::map<std::string, tbl_class*>;
static TableMap tables;

class __field_register__{
public:
    __field_register__(tbl_class *class_ptr, unsigned long offset
                       , std::string key, int8_t type){
        tbl_field field(offset, key, type);
        class_ptr->add_field(field);
    }
};

class __class_register__{
public:
    __class_register__(std::string table, tbl_class *class_ptr){
        tables[table]= class_ptr;
    }
};


class __sql_register__{
public:
    __sql_register__(tbl_class *class_ptr, std::string& label, std::string& sql){
        class_ptr->add_sql(label,sql);
    }
};

};

#endif /* reflection_hpp */
