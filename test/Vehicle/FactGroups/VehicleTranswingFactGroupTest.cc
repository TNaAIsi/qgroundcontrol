#include "VehicleTranswingFactGroupTest.h"

#include <cstring>

#include "MAVLinkLib.h"
#include "Vehicle.h"
#include "VehicleTranswingFactGroup.h"

namespace {
mavlink_debug_float_array_t snapshot()
{
    mavlink_debug_float_array_t packet{};
    packet.array_id = 22001;
    std::memcpy(packet.name, "TRANSWING", sizeof(packet.name));
    packet.data[0] = 4;
    return packet;
}

mavlink_message_t encode(const mavlink_debug_float_array_t& packet, uint8_t component = MAV_COMP_ID_AUTOPILOT1)
{
    mavlink_message_t message{};
    mavlink_msg_debug_float_array_encode(1, component, &message, &packet);
    return message;
}
}  // namespace

void VehicleTranswingFactGroupTest::_decodeSnapshot()
{
    Vehicle vehicle(MAV_AUTOPILOT_PX4, MAV_TYPE_VTOL_TILTROTOR);
    vehicle.setOfflineEditingDefaultComponentId(MAV_COMP_ID_AUTOPILOT1);
    auto* group = vehicle.getFactGroup("Transwing");
    QVERIFY(group);
    QCOMPARE(group->factNames().size(), 30);
    auto packet = snapshot();
    for (int i = 1; i <= 21; ++i) {
        packet.data[i] = i;
    }
    packet.data[8] = 3;
    packet.data[9] = 0.25f;
    packet.data[10] = 255;
    packet.data[14] = 0.75f;
    packet.data[15] = 6;
    packet.data[16] = 1;
    packet.data[17] = 2;
    packet.data[18] = 0;
    group->handleMessage(&vehicle, encode(packet));
    const QMap<QString, double> expected{
        {"wingAngle", 1},
        {"wingAngleSetpoint", 2},
        {"wingAngleRateSetpoint", 3},
        {"wingAngleAccelSetpoint", 4},
        {"wingAngleJerkSetpoint", 5},
        {"collectiveTiltAngle", 6},
        {"trackingError", 7},
        {"phase", 3},
        {"fixedWingWeight", 25},
        {"feedbackSampleAge", 11},
        {"attitudeError", 12},
        {"altitudeLoss", 13},
        {"collectiveCommand", 75},
        {"abortReason", 6},
        {"pilotTakeoverAvailable", 1},
        {"pilotControlOwner", 2},
        {"transitionComplete", 0},
        {"handoverWindowAge", 19},
        {"tiltError", 20},
        {"headingError", 21},
    };
    for (auto it = expected.cbegin(); it != expected.cend(); ++it) {
        QVERIFY2(group->factExists(it.key()), qPrintable(it.key()));
        QCOMPARE(group->getFact(it.key())->rawValue().toDouble(), it.value());
    }
    const QStringList flags{"wingAngleValid",   "feedbackHealthy",    "trackingErrorOk",    "flightControlActive",
                            "groundTestActive", "feedbackHoldActive", "pilotControlLocked", "telemetryFresh"};
    for (int bit = 0; bit < flags.size(); ++bit) {
        packet.data[10] = 1 << bit;
        group->handleMessage(&vehicle, encode(packet));
        for (int i = 0; i < flags.size(); ++i) {
            QVERIFY(group->factExists(flags[i]));
            QCOMPARE(group->getFact(flags[i])->rawValue().toInt(), int(bit == i));
        }
    }
    packet.data[11] = packet.data[19] = qQNaN();
    group->handleMessage(&vehicle, encode(packet));
    for (const auto* name :
         {"wingAngle", "collectiveTiltAngle", "trackingError", "feedbackSampleAge", "handoverWindowAge"}) {
        QVERIFY(qIsNaN(group->getFact(name)->rawValue().toDouble()));
    }
}

void VehicleTranswingFactGroupTest::_ignoreOtherPackets()
{
    Vehicle vehicle(MAV_AUTOPILOT_PX4, MAV_TYPE_VTOL_TILTROTOR);
    vehicle.setOfflineEditingDefaultComponentId(MAV_COMP_ID_AUTOPILOT1);
    auto* group = vehicle.transwingFactGroup();
    const auto valid = snapshot();
    auto packet = valid;
    packet.array_id = 1;
    group->handleMessage(&vehicle, encode(packet));
    packet = valid;
    packet.name[9] = 'X';
    group->handleMessage(&vehicle, encode(packet));
    for (const float schema : {3.0f, 5.0f, float(qQNaN())}) {
        packet = valid;
        packet.data[0] = schema;
        group->handleMessage(&vehicle, encode(packet));
    }
    group->handleMessage(&vehicle, encode(valid, MAV_COMP_ID_CAMERA));
    group->handleMessage(nullptr, encode(valid));
    QVERIFY(!group->telemetryAvailable());
    QVERIFY(qIsNaN(group->getFact("headingError")->rawValue().toDouble()));
}

void VehicleTranswingFactGroupTest::_expireSnapshot()
{
    Vehicle vehicle(MAV_AUTOPILOT_PX4, MAV_TYPE_VTOL_TILTROTOR);
    vehicle.setOfflineEditingDefaultComponentId(MAV_COMP_ID_AUTOPILOT1);
    auto* group = vehicle.transwingFactGroup();
    auto packet = snapshot();
    packet.data[10] = 129;
    group->handleMessage(&vehicle, encode(packet));
    QCOMPARE(group->getFact("wingAngle")->rawValue().toDouble(), 0.0);
    QCOMPARE(group->getFact("telemetryFresh")->rawValue().toInt(), 1);

    mavlink_message_t state{};
    mavlink_msg_extended_sys_state_pack(1, MAV_COMP_ID_AUTOPILOT1, &state, MAV_VTOL_STATE_TRANSITION_TO_FW,
                                        MAV_LANDED_STATE_IN_AIR);
    group->handleMessage(&vehicle, state);
    QTRY_COMPARE_WITH_TIMEOUT(group->getFact("telemetryFresh")->rawValue().toInt(), 0, 4000);
    for (const QString& name : group->factNames()) {
        Fact* fact = group->getFact(name);
        if (name == "vtolState") {
            QCOMPARE(fact->enumStringValue(), QStringLiteral("TransitionToFW"));
        } else if (name == "landedState") {
            QCOMPARE(fact->enumStringValue(), QStringLiteral("InAir"));
        } else if (fact->type() == FactMetaData::valueTypeDouble) {
            QVERIFY(qIsNaN(fact->rawValue().toDouble()));
        } else if (name != "telemetryFresh") {
            QCOMPARE(fact->enumStringValue(), QStringLiteral("Unavailable"));
        }
    }
    group->handleMessage(&vehicle, encode(packet));
    QCOMPARE(group->getFact("headingError")->rawValue().toDouble(), 0.0);
    QCOMPARE(group->getFact("telemetryFresh")->rawValue().toInt(), 1);
}

UT_REGISTER_TEST(VehicleTranswingFactGroupTest, TestLabel::Unit)
