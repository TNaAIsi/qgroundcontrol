#include "VehicleTranswingFactGroup.h"

#include <cmath>
#include <cstring>

#include "MAVLinkLib.h"
#include "Vehicle.h"

namespace {
constexpr int snapshotTimeoutMs = 2000;
constexpr int unavailable = 255;

int enumValue(float value, int maximum)
{
    return (value >= 0 && value <= maximum && std::trunc(value) == value) ? static_cast<int>(value) : unavailable;
}
}  // namespace

VehicleTranswingFactGroup::VehicleTranswingFactGroup(QObject* parent)
    : FactGroup(0, QStringLiteral(":/json/Vehicle/TranswingFact.json"), parent)
{
    _addFact(&_wingAngle);
    _addFact(&_wingAngleSetpoint);
    _addFact(&_wingAngleRateSetpoint);
    _addFact(&_wingAngleAccelSetpoint);
    _addFact(&_wingAngleJerkSetpoint);
    _addFact(&_collectiveTiltAngle);
    _addFact(&_trackingError);
    _addFact(&_phase);
    _addFact(&_fixedWingWeight);
    _addFact(&_feedbackSampleAge);
    _addFact(&_attitudeError);
    _addFact(&_altitudeLoss);
    _addFact(&_collectiveCommand);
    _addFact(&_abortReason);
    _addFact(&_pilotTakeoverAvailable);
    _addFact(&_pilotControlOwner);
    _addFact(&_transitionComplete);
    _addFact(&_handoverWindowAge);
    _addFact(&_tiltError);
    _addFact(&_headingError);
    _addFact(&_wingAngleValid);
    _addFact(&_feedbackHealthy);
    _addFact(&_trackingErrorOk);
    _addFact(&_flightControlActive);
    _addFact(&_groundTestActive);
    _addFact(&_feedbackHoldActive);
    _addFact(&_pilotControlLocked);
    _addFact(&_telemetryFresh);
    _addFact(&_vtolState);
    _addFact(&_landedState);

    _expiryTimer.setSingleShot(true);
    connect(&_expiryTimer, &QTimer::timeout, this, &VehicleTranswingFactGroup::_expireSnapshot);
    _clearSnapshot();
}

void VehicleTranswingFactGroup::_clearSnapshot()
{
    for (Fact* fact : _nameToFactMap) {
        if (fact == &_vtolState || fact == &_landedState) {
            continue;
        }
        if (fact->type() == FactMetaData::valueTypeDouble) {
            fact->setRawValue(qQNaN());
        } else {
            fact->setRawValue(fact == &_telemetryFresh ? 0 : unavailable);
        }
    }
}

void VehicleTranswingFactGroup::_expireSnapshot()
{
    const auto remaining = snapshotTimeoutMs - _lastSnapshot.elapsed();
    if (remaining > 0) {
        _expiryTimer.start(static_cast<int>(remaining));
        return;
    }
    _clearSnapshot();
}

void VehicleTranswingFactGroup::handleMessage(Vehicle* vehicle, const mavlink_message_t& message)
{
    if (!vehicle || message.compid != vehicle->defaultComponentId()) {
        return;
    }

    if (message.msgid == MAVLINK_MSG_ID_EXTENDED_SYS_STATE) {
        mavlink_extended_sys_state_t state{};
        mavlink_msg_extended_sys_state_decode(&message, &state);
        _vtolState.setRawValue(state.vtol_state);
        _landedState.setRawValue(state.landed_state);
        _setTelemetryAvailable(true);
        return;
    }
    if (message.msgid != MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        return;
    }

    mavlink_debug_float_array_t packet{};
    mavlink_msg_debug_float_array_decode(&message, &packet);
    if (packet.array_id != 22001 || std::memcmp(packet.name, "TRANSWING", sizeof(packet.name)) != 0 ||
        packet.data[0] != 4.0f) {
        return;
    }

    _lastSnapshot.start();
    _expiryTimer.start(snapshotTimeoutMs);
    const float status = packet.data[10];
    const bool hasFlags = status >= 0 && status <= 255;
    const int flags = hasFlags ? static_cast<int>(std::lround(status)) : 0;
    const auto flag = [flags, hasFlags](int bit) { return hasFlags ? ((flags >> bit) & 1) : unavailable; };
    _wingAngleValid.setRawValue(flag(0));
    _feedbackHealthy.setRawValue(flag(1));
    _trackingErrorOk.setRawValue(flag(2));
    _flightControlActive.setRawValue(flag(3));
    _groundTestActive.setRawValue(flag(4));
    _feedbackHoldActive.setRawValue(flag(5));
    _pilotControlLocked.setRawValue(flag(6));
    _telemetryFresh.setRawValue(flag(7));

    // The sender masks measured geometry when wing feedback is invalid.
    const bool validWing = (flags & 1) != 0;
    _wingAngle.setRawValue(validWing ? packet.data[1] : qQNaN());
    _wingAngleSetpoint.setRawValue(packet.data[2]);
    _wingAngleRateSetpoint.setRawValue(packet.data[3]);
    _wingAngleAccelSetpoint.setRawValue(packet.data[4]);
    _wingAngleJerkSetpoint.setRawValue(packet.data[5]);
    _collectiveTiltAngle.setRawValue(validWing ? packet.data[6] : qQNaN());
    _trackingError.setRawValue(validWing ? packet.data[7] : qQNaN());
    _phase.setRawValue(enumValue(packet.data[8], 4));
    _fixedWingWeight.setRawValue(packet.data[9] * 100.0);
    _feedbackSampleAge.setRawValue(packet.data[11]);
    _attitudeError.setRawValue(packet.data[12]);
    _altitudeLoss.setRawValue(packet.data[13]);
    _collectiveCommand.setRawValue(packet.data[14] * 100.0);
    _abortReason.setRawValue(enumValue(packet.data[15], 6));
    _pilotTakeoverAvailable.setRawValue(enumValue(packet.data[16], 1));
    _pilotControlOwner.setRawValue(enumValue(packet.data[17], 3));
    _transitionComplete.setRawValue(enumValue(packet.data[18], 1));
    _handoverWindowAge.setRawValue(packet.data[19]);
    _tiltError.setRawValue(packet.data[20]);
    _headingError.setRawValue(packet.data[21]);
    _setTelemetryAvailable(true);
}
