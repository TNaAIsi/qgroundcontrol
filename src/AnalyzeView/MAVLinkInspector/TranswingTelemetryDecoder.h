#pragma once

#include <QtCore/QString>
#include <QtCore/QtTypes>

#include "MAVLinkMessageType.h"

class QGCMAVLinkMessage;
class QmlObjectListModel;

namespace TranswingTelemetryDecoder {

static constexpr quint16 kTranswingArrayId = 22001;
static constexpr long kTranswingProtocolVersion = 4;

/// Returns true if the message is a Transwing v4 DEBUG_FLOAT_ARRAY snapshot
/// (msgid 350, array_id == 22001, name == "TRANSWING", round(data[0]) == 4).
[[nodiscard]] bool isTranswingV4Snapshot(const mavlink_message_t &message);

/// Returns true if the message belongs to the Transwing-relevant message set.
[[nodiscard]] bool isTranswingMessage(const mavlink_message_t &message);

/// Extracts a stable instance key for DEBUG_FLOAT_ARRAY messages so Transwing v4
/// does not collide with generic DEBUG_FLOAT_ARRAY streams.
[[nodiscard]] QString extractDebugFloatArrayInstance(const mavlink_message_t &message);

/// Populates human-readable QGCMAVLinkMessageField entries for all Transwing v4
/// slots and unpacked status flags.
void initTranswingV4Fields(QmlObjectListModel *fields, QGCMAVLinkMessage *parent);

/// Updates decoded Transwing v4 field values from a DEBUG_FLOAT_ARRAY message.
void updateTranswingV4Fields(const mavlink_message_t &message, QmlObjectListModel *fields);

} // namespace TranswingTelemetryDecoder
