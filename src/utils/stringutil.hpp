//
//  stringutil.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/11.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef stringutil_hpp
#define stringutil_hpp

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using stringVector = std::vector<std::string>;
using stringMap = std::map<std::string,std::string>;
using intMap = std::map<int, std::string>;

class StringUtil{
public:
    static stringVector split(const std::string &str, std::string sep = " "){
        stringVector v;
        std::string::size_type pos1, pos2;
        pos2 = str.find(sep);
        pos1 = 0;
        while(std::string::npos != pos2){
            v.push_back(str.substr(pos1, pos2 - pos1));

            pos1 = pos2 + sep.size();
            pos2 = str.find(sep, pos1);
        }
        if(pos1 != str.length())
            v.push_back(str.substr(pos1));
        
        return v;
    }
    
    static int string2int(const std::string& str){
        std::istringstream is(str);
        int i;
        is >> i;
        return i;
    }
    
    static int charNumInString(const std::string& str, std::string ch = " "){
        int len = (int)str.length();
        int count = 0;
        const char *s = str.c_str();
        const char *c = ch.c_str();
        for(int i=0; i<len; i++){
            if (s[i] == *c) count++;
        }
        
        return count;
    }
    
};

#endif /* stringutil_hpp */
