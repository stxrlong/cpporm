//
//  main.cpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//
#include <iostream>
#include "mysql/mysql.hpp"

using namespace cpporm;

int main(int argc, const char * argv[]) {
    std::cout << "Hello, World!\n";
    std::string url = "mysql://test:test@192.168.1.106:3306/test?maxConn=10";
    if(!MysqlConnPool::instance().init(url))
        return -1;
    
    MysqlPrep mysql;
    USER user;
    user.username = "zhangsan";
    user.password = "123456";
    user.state = 1;
    user.createTime = "2020-01-22 19:10:32";
    if(!mysql.insert(user)){
        std::cout<< "insert user1 error." <<std::endl;
    }

    user.username.assign("lisi");
    user.password.assign("123");
    user.createTime = "2020-01-22 19:15:10";
    if(!mysql.insert(user)){
        std::cout<< "insert user2 error." <<std::endl;
    }
    
    ParamMap pm;
    std::string x = "zhangsan";
    pm[1] = x;
    pm[2] = 1;
    
    std::pair<MysqlRet, USER> s_user = mysql.selectOne<USER>("one_user", pm);

    if (s_user.first == MysqlRet::SUCCESS) {
        std::cout<<"user.username:"<<s_user.second.username<<std::endl;
        std::cout<<"user.password:"<<s_user.second.password<<std::endl;
        std::cout<<"user.state:"<<s_user.second.state<<std::endl;
        std::cout<<"user.createTime:"<<s_user.second.createTime<<std::endl;
    }

    std::cout<<"*************************"<<std::endl;
    
    pm.clear();
    std::pair<MysqlRet, std::vector<USER>> all_user = mysql.selectList<USER>("all_user", pm);

    if (all_user.first == MysqlRet::SUCCESS) {
        int index = 1;
        for(auto user:all_user.second){
            std::cout<<"the "<<index++<<"'s user.'"<<std::endl;
            std::cout<<"user.username:"<<user.username<<std::endl;
            std::cout<<"user.password:"<<user.password<<std::endl;
            std::cout<<"user.state:"<<user.state<<std::endl;
            std::cout<<"user.createTime:"<<user.createTime<<std::endl;
        }
    }

    std::cout<<"*************************"<<std::endl;
    return 0;
}
