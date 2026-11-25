#include "TranswingTelemetryDecoder.h"

#include "MAVLinkLib.h"
#include "MAVLinkMessage.h"
#include "MAVLinkMessageField.h"
#include "QmlObjectListModel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace TranswingTelemetryDecoder {

namespace {

struct TranswingFieldSpec {
    const char *name;
    const char *unitOrType;
};

constexpr std::array<TranswingFieldSpec, 31> kTranswingV4Fields = {{
    {"time_usec",                 "us"},
    {"protocol_version",          "version"},
    {"wing_angle",                "deg"},
    {"wing_angle_setpoint",       "deg"},
    {"wing_angle_rate_setpoint",  "deg/s"},
    {"wing_angle_accel_setpoint", "deg/s^2"},
    {"wing_angle_jerk_setpoint",  "deg/s^3"},
    {"collective_tilt_angle",     "deg"},
    {"tracking_error",            "deg"},
    {"phase",                     "enum"},
    {"fixed_wing_weight",         "0..1"},
    {"status_flags",              "bitmask"},
    {"feedback_sample_age",       "ms"},
    {"attitude_error",            "deg"},
    {"altitude_loss",             "m"},
    {"collective_command",        "0..1"},
    {"abort_reason",              "enum"},
    {"pilot_takeover_available",  "bool"},
    {"pilot_control_owner",       "enum"},
    {"transition_complete",       "bool"},
    {"handover_window_age",       "ms"},
    {"tilt_error",                "deg"},
    {"heading_error",             "deg"},
    {"flag_wing_angle_valid",     "bool"},
    {"flag_feedback_healthy",     "bool"},
    {"flag_tracking_error_ok",    "bool"},
    {"flag_flight_control_active","bool"},
    {"flag_ground_test_active",   "bool"},
    {"flag_feedback_hold_active", "bool"},
    {"flag_pilot_control_locked", "bool"},
    {"flag_status_fresh",         "bool"},
}};

QString extractDebugFloatArrayName(const mavlink_debug_float_array_t &dfa)
{
    const size_t len = strnlen(dfa.name, sizeof(dfa.name));
    return QString::fromLatin1(dfa.name, static_cast<qsizetype>(len)).trimmed();
}

QString formatFloatValue(float value, int precision = 6)
{
    if (!std::isfinite(value)) {
        return QStringLiteral("NaN");
    }
    return QString::number(static_cast<double>(value), 'g', precision);
}

QString formatPhaseEnum(long phase)
{
    switch (phase) {
    case 0:
        return QStringLiteral("0 (MC)");
    case 1:
        return QStringLiteral("1 (Accelerate)");
    case 2:
        return QStringLiteral("2 (Blend)");
    case 3:
        return QStringLiteral("3 (FixedWing)");
    case 4:
        return QStringLiteral("4 (BackTransition)");
    default:
        return QStringLiteral("%1 (Unknown)").arg(phase);
    }
}

QString formatAbortReasonEnum(long reason)
{
    switch (reason) {
    case 0:
        return QStringLiteral("0 (None)");
    case 1:
        return QStringLiteral("1 (StateUnavailable)");
    case 2:
        return QStringLiteral("2 (HeightLoss)");
    case 3:
        return QStringLiteral("3 (AttitudeError)");
    case 4:
        return QStringLiteral("4 (Timeout)");
    case 5:
        return QStringLiteral("5 (LowAirspeed)");
    case 6:
        return QStringLiteral("6 (TrackingLoss)");
    default:
        return QStringLiteral("%1 (Unknown)").arg(reason);
    }
}

QString formatControlOwnerEnum(long owner)
{
    switch (owner) {
    case 0:
        return QStringLiteral("0 (Inactive)");
    case 1:
        return QStringLiteral("1 (WaitingForPilot)");
    case 2:
        return QStringLiteral("2 (Pilot)");
    case 3:
        return QStringLiteral("3 (AutoHoldRequested)");
    default:
        return QStringLiteral("%1 (Unknown)").arg(owner);
    }
}

QString formatBoolValue(float rawValue, qreal &numericOut)
{
    if (!std::isfinite(rawValue)) {
        numericOut = static_cast<qreal>(rawValue);
        return QStringLiteral("NaN");
    }
    const bool set = (std::lround(rawValue) != 0);
    numericOut = set ? 1.0 : 0.0;
    return set ? QStringLiteral("1 (True)") : QStringLiteral("0 (False)");
}

void setFieldValue(QmlObjectListModel *fields, int index, const QString &text, qreal numeric)
{
    if (!fields || index < 0 || index >= fields->count()) {
        return;
    }
    QGCMAVLinkMessageField *const field = qobject_cast<QGCMAVLinkMessageField *>(fields->get(index));
    if (!field) {
        return;
    }
    field->updateValue(text, numeric);
}

} // namespace

bool isTranswingV4Snapshot(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        return false;
    }

    mavlink_debug_float_array_t dfa{};
    mavlink_msg_debug_float_array_decode(&message, &dfa);

    if (dfa.array_id != kTranswingArrayId) {
        return false;
    }
    if (extractDebugFloatArrayName(dfa) != QLatin1String("TRANSWING")) {
        return false;
    }
    if (!std::isfinite(dfa.data[0])) {
        return false;
    }
    return std::lround(dfa.data[0]) == kTranswingProtocolVersion;
}

bool isTranswingMessage(const mavlink_message_t &message)
{
    switch (message.msgid) {
    case MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY:
        return isTranswingV4Snapshot(message);
    case MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
    case MAVLINK_MSG_ID_VFR_HUD:
    case MAVLINK_MSG_ID_ESC_STATUS:
    case MAVLINK_MSG_ID_ESC_INFO:
    case MAVLINK_MSG_ID_BATTERY_STATUS:
    case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
    case MAVLINK_MSG_ID_STATUSTEXT:
    case MAVLINK_MSG_ID_EVENT:
        return true;
    default:
        return false;
    }
}

QString extractDebugFloatArrayInstance(const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        return QString();
    }

    mavlink_debug_float_array_t dfa{};
    mavlink_msg_debug_float_array_decode(&message, &dfa);
    const QString arrayName = extractDebugFloatArrayName(dfa);

    if (dfa.array_id == kTranswingArrayId
        && arrayName == QLatin1String("TRANSWING")
        && std::isfinite(dfa.data[0])
        && std::lround(dfa.data[0]) == kTranswingProtocolVersion) {
        return QStringLiteral("TRANSWING_v4");
    }

    if (!arrayName.isEmpty()) {
        return QStringLiteral("%1_%2").arg(arrayName).arg(dfa.array_id);
    }
    return QString::number(dfa.array_id);
}

void initTranswingV4Fields(QmlObjectListModel *fields, QGCMAVLinkMessage *parent)
{
    if (!fields || !parent) {
        return;
    }

    for (const TranswingFieldSpec &spec : kTranswingV4Fields) {
        fields->append(new QGCMAVLinkMessageField(QLatin1String(spec.name),
                                                  QLatin1String(spec.unitOrType),
                                                  parent));
    }
}

void updateTranswingV4Fields(const mavlink_message_t &message, QmlObjectListModel *fields)
{
    if (!fields || fields->count() < static_cast<int>(kTranswingV4Fields.size())) {
        return;
    }
    if (message.msgid != MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        return;
    }

    mavlink_debug_float_array_t dfa{};
    mavlink_msg_debug_float_array_decode(&message, &dfa);

    // 0: time_usec
    setFieldValue(fields, 0, QString::number(dfa.time_usec), static_cast<qreal>(dfa.time_usec));

    // Slots 0..21 map to field indices 1..22
    for (int slot = 0; slot <= 21; ++slot) {
        const int fieldIdx = slot + 1;
        const float raw = dfa.data[slot];

        switch (slot) {
        case 0: { // protocol_version
            const QString text = std::isfinite(raw) ? QString::number(std::lround(raw)) : QStringLiteral("NaN");
            setFieldValue(fields, fieldIdx, text, static_cast<qreal>(raw));
            break;
        }
        case 8: { // phase
            if (!std::isfinite(raw)) {
                setFieldValue(fields, fieldIdx, QStringLiteral("NaN"), static_cast<qreal>(raw));
            } else {
                const long val = std::lround(raw);
                setFieldValue(fields, fieldIdx, formatPhaseEnum(val), static_cast<qreal>(val));
            }
            break;
        }
        case 10: { // status_flags
            if (!std::isfinite(raw)) {
                setFieldValue(fields, fieldIdx, QStringLiteral("NaN"), static_cast<qreal>(raw));
            } else {
                const long signedMask = std::max<long>(0, std::lround(raw));
                const quint32 mask = static_cast<quint32>(signedMask);
                setFieldValue(fields,
                              fieldIdx,
                              QStringLiteral("0x%1 (%2)").arg(mask, 2, 16, QLatin1Char('0')).arg(mask),
                              static_cast<qreal>(mask));
            }
            break;
        }
        case 15: { // abort_reason
            if (!std::isfinite(raw)) {
                setFieldValue(fields, fieldIdx, QStringLiteral("NaN"), static_cast<qreal>(raw));
            } else {
                const long val = std::lround(raw);
                setFieldValue(fields, fieldIdx, formatAbortReasonEnum(val), static_cast<qreal>(val));
            }
            break;
        }
        case 16: // pilot_takeover_available
        case 18: { // transition_complete
            qreal num = 0.0;
            const QString text = formatBoolValue(raw, num);
            setFieldValue(fields, fieldIdx, text, num);
            break;
        }
        case 17: { // pilot_control_owner
            if (!std::isfinite(raw)) {
                setFieldValue(fields, fieldIdx, QStringLiteral("NaN"), static_cast<qreal>(raw));
            } else {
                const long val = std::lround(raw);
                setFieldValue(fields, fieldIdx, formatControlOwnerEnum(val), static_cast<qreal>(val));
            }
            break;
        }
        default:
            setFieldValue(fields, fieldIdx, formatFloatValue(raw), static_cast<qreal>(raw));
            break;
        }
    }

    // 8 unpacked status flags from data[10] -> field indices 23..30
    const float rawMask = dfa.data[10];
    if (!std::isfinite(rawMask)) {
        for (int bit = 0; bit < 8; ++bit) {
            setFieldValue(fields, 23 + bit, QStringLiteral("NaN"), static_cast<qreal>(rawMask));
        }
        return;
    }

    const quint32 mask = static_cast<quint32>(std::max<long>(0, std::lround(rawMask)));
    for (int bit = 0; bit < 8; ++bit) {
        const bool isSet = (mask & (1u << bit)) != 0;
        setFieldValue(fields,
                      23 + bit,
                      isSet ? QStringLiteral("1 (True)") : QStringLiteral("0 (False)"),
                      isSet ? 1.0 : 0.0);
    }
}

} // namespace TranswingTelemetryDecoder
