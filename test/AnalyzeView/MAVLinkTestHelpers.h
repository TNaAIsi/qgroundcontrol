#pragma once

#include "MAVLinkMessage.h"
#include <MAVLinkLib.h>
#include <QtCore/QObject>
#include <array>
#include <cstring>

namespace MAVLinkTestHelpers {

// Build a raw mavlink heartbeat message
inline mavlink_message_t makeHeartbeat(uint8_t sysId = 1, uint8_t compId = 1)
{
    mavlink_message_t msg{};
    mavlink_msg_heartbeat_pack_chan(
        sysId, compId, MAVLINK_COMM_0, &msg,
        MAV_TYPE_QUADROTOR, MAV_AUTOPILOT_PX4, 0, 0, MAV_STATE_ACTIVE);
    return msg;
}

// Build a QGCMAVLinkMessage wrapping a heartbeat
inline QGCMAVLinkMessage* makeHeartbeatMsg(uint8_t sysId, uint8_t compId, QObject* parent = nullptr)
{
    mavlink_message_t msg = makeHeartbeat(sysId, compId);
    return new QGCMAVLinkMessage(msg, QString(), parent);
}

inline QGCMAVLinkMessage* makeHeartbeatMsg(QObject* parent = nullptr)
{
    return makeHeartbeatMsg(1, 1, parent);
}

inline mavlink_message_t makeDebugFloatArray(uint16_t arrayId,
                                             const char *name,
                                             const std::array<float, 58> &data,
                                             uint64_t timeUsec = 1234567ULL,
                                             uint8_t sysId = 1,
                                             uint8_t compId = 1)
{
    mavlink_message_t msg{};
    char nameBuf[MAVLINK_MSG_DEBUG_FLOAT_ARRAY_FIELD_NAME_LEN]{};
    if (name) {
        std::strncpy(nameBuf, name, sizeof(nameBuf) - 1);
    }
    mavlink_msg_debug_float_array_pack_chan(
        sysId, compId, MAVLINK_COMM_0, &msg,
        timeUsec, nameBuf, arrayId, data.data());
    return msg;
}

} // namespace MAVLinkTestHelpers
