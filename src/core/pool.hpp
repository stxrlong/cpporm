//
//  pool.hpp
//  orm_for_c++
//
//  Created by sjlfx on 2020/1/4.
//  Copyright © 2020 sjlfx. All rights reserved.
//

#ifndef pool_hpp
#define pool_hpp

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <list>
#include <algorithm>

namespace db_ty {

template <typename Connection>
class ConnectionPool {
protected:
    struct ConnectionInfo {
        Connection *_conn;
        time_t time;
        bool in_use = false;

        ConnectionInfo(Connection* conn_):
            _conn(conn_), time(time_t()) {
        }

        ~ConnectionInfo() {
            delete this->_conn;
        }

        bool operator < (const ConnectionInfo *_rconn) {
            const ConnectionInfo *_lconn = &this;
            return (_lconn->in_use == _rconn->in_use) ? _lconn->time < _rconn->time : !_lconn->in_use;
        }
    };

    using ConnList = std::list<ConnectionInfo*>;
public:
    ConnectionPool(): defConnNum(2), usedConnNum(0), waitTime(0) {};

    virtual ~ConnectionPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (typename ConnList::iterator it = connectionInfos.begin(); it != connectionInfos.end();) {
            ConnectionInfo *ci = *it;
            connectionInfos.erase(it++);
            delete ci;
        }
    }

    void setWaitTime(int _milliSeconds) {
        this->waitTime = _milliSeconds;
    }

    void setDefConnNum(int _defConnNum) {
        this->defConnNum = _defConnNum;
    }



protected:
    int release(Connection* conn) {
        bool found = false;
        for (typename ConnList::iterator it = connectionInfos.begin(); it != connectionInfos.end();it++) {
            if ((*it)->_conn == conn ) {
                found = true;
                if ((*it)->in_use == true) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    usedConnNum--;
                    (*it)->in_use = false;
                    (*it)->time = time_t();
                }
            }
        }

        if (!found) {
            return destroy(conn);
        }

        return 0;
    }

public:
    virtual Connection* acquire() {
        if (usedConnNum >= defConnNum) {
            if (waitTime >= 0) {
                int tries = 0;
                while (usedConnNum >= defConnNum) {
                    std::chrono::milliseconds ms(waitTime);
                    std::this_thread::sleep_for(ms);// defConnNum must be large than the cpu num;
                    if (tries++ > 1000)
                        break;
                }
            }else{
                while (usedConnNum >= defConnNum) {
                    std::chrono::milliseconds ms(waitTime);
                    std::this_thread::sleep_for(ms);// defConnNum must be large than the cpu num;
                }
            }
        }

        std::lock_guard<std::mutex> lock(mutex_);
        ConnectionInfo *ci = find();
        if (ci != nullptr) {
            ci->in_use = true;
            usedConnNum++;
            return ci->_conn;
        } else {
            if (usedConnNum >= defConnNum)
                return nullptr;
            
            Connection *c = create();
            if(c==nullptr)
                return nullptr;
            ci = new ConnectionInfo(c);
            connectionInfos.push_back(ci);
            ci->in_use = true;
            usedConnNum++;
            return c;
        }
    }
    
    virtual int destroy(Connection* conn) {
        for (typename ConnList::iterator it = connectionInfos.begin(); it != connectionInfos.end();) {
            if ((*it)->_conn == conn) {
                std::lock_guard<std::mutex> lock(mutex_);
                ConnectionInfo *ci = *it;
                connectionInfos.erase(it);
                delete ci;            }
        }
        if (conn != nullptr) {
            delete conn;
            //std::cout<<"unknown connection"<<std::endl;
        }

        return 0;
    }

protected:
    virtual Connection* create() = 0;

private:
    ConnectionInfo* find() {
        typename ConnList::iterator ci = std::min_element(connectionInfos.begin(),connectionInfos.end());
        if (ci != connectionInfos.end() && !(*ci)->in_use) {
            return *ci;
        }

        return nullptr;
    }

protected:
    ConnList connectionInfos;

    std::mutex mutex_;

    int defConnNum;
    int usedConnNum;
    int waitTime;
};
            
}/*namespace db_ty*/
#endif /* pool_hpp */
