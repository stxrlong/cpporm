//
//  mysqltable.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/11.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef mysqltable_hpp
#define mysqltable_hpp

#include "mysql/mysqlorm.hpp"

namespace cpporm {

TABLE(USER,user)
FIELD(USER,std::string,username,FieldType::STRING)
FIELD(USER,std::string,password,FieldType::STRING)
FIELD(USER,int,state,FieldType::INT32)
FIELD(USER,std::string,createTime,FieldType::STRING)
FIELD(USER,std::string,modifyTime,FieldType::STRING)
SQLSTRING(USER,one_user,"select * from user where username = ? and state = ?;")
SQLSTRING(USER,all_user,"select * from user;")
ENDTABLE

TABLE(test,test)
FIELD(test,int,role,FieldType::INT32)
FIELD(test,std::string,ip,FieldType::STRING)
FIELD(test,std::string,resc,FieldType::STRING)
FIELD(test,int,method,FieldType::INT32)
SQLSTRING(test,one_test,"select * from test where ip = ? and method = ?;")
SQLSTRING(test,one_test1,"select * from test where resc = ?;")
SQLSTRING(test,all_test,"select * from test;")
ENDTABLE

//make a temporarily table when you need to query by 'join'.
TABLE(temp,test)
FIELD(temp,std::string,username,FieldType::STRING)
FIELD(temp,std::string,ip,FieldType::STRING)
SQLSTRING(temp,one_test,"select username, ip from user left join test on user.state = test.role where test.method = ?;")
ENDTABLE



}/*namespace cpporm*/

#endif /* mysqltable_hpp */
