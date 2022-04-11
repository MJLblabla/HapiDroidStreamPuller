//
// Created by 1 on 2022/4/7.
//

#include "IConnection.h"

IConnection::IConnection() = default;

IConnection::~IConnection() = default;

void IConnection::open() {
    isStart = false;
    exit = true;
    mConnectedStatus = CONNECTED_STATUS_START;
    connectCallBack(mConnectedStatus);
    workThread = new thread(startConnectionThread, this);
//   innerConnect();
}

void IConnection::startConnectionThread(IConnection *connection) {
    if (connection->innerConnect()) {
        connection->isStart = true;
        connection->exit = false;
        connection->mConnectedStatus = CONNECTED_STATUS_CONNECTED;
        connection->connectCallBack(connection->mConnectedStatus);
        connection->loopPack();
    } else {
        connection->mConnectedStatus = CONNECTED_STATUS_CONNECT_FAIL;
        connection->connectCallBack(connection->mConnectedStatus);
    }
}

void IConnection::close() {
    isStart = false;
    exit = true;
    mConnectedStatus = CONNECTED_STATUS_CLOSE;
    connectCallBack(mConnectedStatus);
    if (workThread != nullptr) {
        workThread->join();
        delete workThread;
        workThread = nullptr;
    }
    clear();
    innerDisconnect();
}

int IConnection::getConnectedStatus() {
    return mConnectedStatus;
}



