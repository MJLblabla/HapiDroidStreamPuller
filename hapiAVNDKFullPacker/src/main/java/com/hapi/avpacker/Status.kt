package com.hapi.avpacker

enum class MuxerStatus  (val intStatus: Int) {

    STATE_UNKNOWN(-1),
    STATE_DECODING(0),
    STATE_PAUSE(1),
    STATE_STOP(2),
    STATE_FINISH(3);

}

enum class ConnectedStatus(val intStatus: Int) {
    CONNECTED_STATUS_NULL(1),
    CONNECTED_STATUS_START(2),
    CONNECTED_STATUS_CONNECTED(3),
    CONNECTED_STATUS_CONNECT_FAIL(4),
    CONNECTED_STATUS_TIMEOUT_PACKET(5),
    CONNECTED_STATUS_TIMEOUT_RESET(6),
    CONNECTED_STATUS_OFFLINE(7),
    CONNECTED_STATUS_CLOSE(8)
};
