# cpporm
一个简单的c++ orm组件，依赖于boost(版本1.68.0)和mysql connector/c++，目前仅支持数据库插入和查询。

## Usage
1. 通过宏定义创建对应的表和针对表的查询语句的映射关系（插入语句会由组件自动生成），先用TABLE宏定义应用层的对象和数据库中的真实表，然后用FIELD宏创建应用层中对象中成员，包括成员类型，成员名及数据库中的字段类型，再用SQLSTRING宏创建该对象的sql查询语句，当然需要有个对应标识来指定sql语句，如下面的定义例子：

    TABLE(USER,user)  //（USER是应用层对象，user是数据库表名）<br>
    FIELD(USER,std::string,username,FieldType::STRING) //（USER是应用层对象，std::string是应用层成员类型，username是成员名，FIELD::STRING是数据库类型）<br>
    FIELD(USER,std::string,password,FieldType::STRING) <br>
    FIELD(USER,int,state,FieldType::INT32) <br>
    FIELD(USER,std::string,createTime,FieldType::STRING) <br>
    FIELD(USER,std::string,modifyTime,FieldType::STRING) <br>
    SQLSTRING(USER,one_user,"select * from user where username = ? and state = ?;") //（USER是应用层对象，one_user是sql语句的标识，在应用层使用只需用标识，字符串是sql查询语句，插入语句无需特殊指定，可以自动生成，当然也可以自行这样指定<br>
    SQLSTRING(USER,all_user,"select * from user;") <br>
    ENDTABLE
    
2. 初始化数据库连接数据库，方式如下：

    a).  std::string url = "mysql://username:password@ip:port/database_name?maxConn=2";<br>
        e.g.   "mysql://test:test@192.168.1.106:3306/test?maxConn=10"
    
    b). MysqlConnPool::instance().init(url) should return true.
    
3. 获取链接，然后进行插入查询操作

    MysqlPrep mysql;<br>
    USER user;<br>
    user.username = "zhangsan";<br>
    user.password = "123456";<br>
    user.state = 1;<br>
    user.createTime = "2020-01-22 19:10:32";<br>
    if(!mysql.insert(user)){<br>
        std::cout<< "insert user1 error." <<std::endl;<br>
    }
    
    ParamMap pm;<br>
    pm[1] = "zhangsan"; // username (std::string)<br>
    pm[2] = 1;  //state (int)

    std::pair<MysqlRet, USER> s_user = mysql.selectOne<USER>("one_user", pm);<br>
    if (s_user.first == MysqlRet::SUCCESS) {<br>
        std::cout<<"user.username:"<<s_user.second.username<<std::endl;<br>
        std::cout<<"user.password:"<<s_user.second.password<<std::endl;<br>
        std::cout<<"user.state:"<<s_user.second.state<<std::endl;<br>
        std::cout<<"user.createTime:"<<s_user.second.createTime<<std::endl;<br>
    }
    
## Reference
    参考了该orm模型，简化了该模型并加入了宏定义的对象数据库表映射方式，同时参考mysql connector中ConnectOptionMap方式简化了查询参数的输入
    [Tiny-ormhttps://github.com/david-pp/tiny-orm](https://github.com/david-pp/tiny-orm)



    
    
    
