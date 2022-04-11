//
// Created by 1 on 2022/4/7.
//

#ifndef HAPI_ICONNECTION_H
#define HAPI_ICONNECTION_H

#include "istream"
#include "ThreadSafeQueue.h"
#include "thread"
#include "LogUtil.h"

using namespace std;
typedef std::function<void(int)> ConnectCallBack;

enum ConnectedStatus {
    CONNECTED_STATUS_NULL = 1,
    CONNECTED_STATUS_START = 2,
    CONNECTED_STATUS_CONNECTED = 3,
    CONNECTED_STATUS_CONNECT_FAIL = 4,
    CONNECTED_STATUS_TIMEOUT_PACKET = 5,
    CONNECTED_STATUS_TIMEOUT_RESET = 6,
    CONNECTED_STATUS_OFFLINE = 7,
    CONNECTED_STATUS_CLOSE = 8
};

class IConnection {

protected:
    int mConnectedStatus = CONNECTED_STATUS_NULL;

    //编码器线程
    thread *workThread = nullptr;
    volatile bool exit = true;
    volatile bool isStart = false;

    //ThreadSafeQueue<void *> packQueue;
    static void startConnectionThread(IConnection *connection);

    virtual void loopPack() = 0;
    virtual int innerConnect() = 0;
    virtual void innerDisconnect()=0;
    virtual void clear()=0;
public:
    ConnectCallBack connectCallBack;
    IConnection();

    virtual ~IConnection();

    int getConnectedStatus();

    void open();

    void close();
};


#endif //HAPISRT_ICONNECTION_H
