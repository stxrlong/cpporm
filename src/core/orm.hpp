//
//  orm.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef orm_hpp
#define orm_hpp

#include <iostream>

#include "reflection.hpp"

/*
 * sql parameters container, refer to variant in mysql connector/c++
 */
class ParameterBase {
public:
    ParameterBase(void *pStr, std::string pTypeName):
        paraStr(pStr), paraTypeName(pTypeName){}
    
    virtual ~ParameterBase(){}
    
    virtual ParameterBase* clone() = 0;
    
    std::string &type(){
        return paraTypeName;
    }
    
    template<typename T>
    T *get() const{
        if (typeid(T).name() == typeid(void).name()) {
          return static_cast< T * > (paraStr);
        }
        
        if (typeid(T).name() != paraTypeName) {
            std::cout<<"parameter type doesn't match."<<std::endl;
        }
        
        return static_cast<T*>(paraStr);
    }
protected:
    void *paraStr;
    std::string paraTypeName;
};

template<typename T>
class ParameterImpl: public ParameterBase{
public:
    ParameterImpl(T i): ParameterBase(new T(i),typeid(T).name()){}
    
    ParameterImpl(ParameterImpl& that): ParameterBase(that) {
        copyParam(that);
    }
    
    ParameterImpl& operator=(ParameterImpl& that) {
      if (this != &that) {
        destroyParam();
        if (paraStr == NULL) {
          copyParam(that);
        }
      }
      return *this;
    }
    
    ~ParameterImpl(){
        destroyParam();
    }
    
    virtual ParameterImpl* clone() {
        return new ParameterImpl(*this);
    }
private:
    void destroyParam(){
        T *tmp=static_cast< T * >(paraStr);
        if (tmp) {
          delete tmp;
          paraStr=NULL;
        }
    }
    
    void copyParam(ParameterImpl& that) {
        paraStr=new T(*(static_cast< T * > (that.get< void >())));
    }
    
};
    
class Parameter {
public:
    Parameter(const int& i=0)
        :param(new ParameterImpl<int>(i)){}
    
    Parameter(const std::string& i)
        :param(new ParameterImpl<std::string>(i)){}
    
    Parameter(const char* i)
        :param(new ParameterImpl<const char*>(i)){}
    
    Parameter(const bool i)
        :param(new ParameterImpl<bool>(i)){}
    
    Parameter(const Parameter& that) {
        if (this != &that) {
            param = that.param->clone();
        }
    }
    
    Parameter &operator = (const Parameter& that){
        if(this != &that){
            delete param;
            param = that.param->clone();
        }
        return *this;
    }
    
    ~Parameter(){
        if(param){
            delete param;
        }
    }
    
    std::string &type(){
        return param->type();
    }
    
    template<typename T>
    T* get() const {
        return param->get<T>();
    }
private:
    ParameterBase *param;
};

//because
using ParamMap = std::map<int, Parameter>;

#endif /* orm_hpp */
