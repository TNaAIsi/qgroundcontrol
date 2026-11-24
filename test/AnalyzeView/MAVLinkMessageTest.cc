#include "MAVLinkMessageTest.h"

#include <QtTest/QSignalSpy>

#include "MAVLinkMessage.h"
#include "MAVLinkMessageField.h"
#include "MAVLinkMessageInfo.h"
#include "MAVLinkTestHelpers.h"
#include "QmlObjectListModel.h"

#include <array>
#include <limits>

void MAVLinkMessageTest::_constructionTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat(2, 3);
    QGCMAVLinkMessage message(msg);

    QCOMPARE(message.id(), static_cast<quint32>(MAVLINK_MSG_ID_HEARTBEAT));
    QCOMPARE(message.sysId(), static_cast<quint8>(2));
    QCOMPARE(message.compId(), static_cast<quint8>(3));
    QCOMPARE(message.name(), QStringLiteral("HEARTBEAT"));
}

void MAVLinkMessageTest::_countStartsAtOneTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    QCOMPARE(message.count(), static_cast<quint64>(1));
}

void MAVLinkMessageTest::_updateIncrementsCountTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    QSignalSpy countSpy(&message, &QGCMAVLinkMessage::countChanged);

    message.update(msg);
    QCOMPARE(message.count(), static_cast<quint64>(2));
    QCOMPARE(countSpy.count(), 1);

    message.update(msg);
    QCOMPARE(message.count(), static_cast<quint64>(3));
    QCOMPARE(countSpy.count(), 2);
}

void MAVLinkMessageTest::_setSelectedTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    QSignalSpy selectedSpy(&message, &QGCMAVLinkMessage::selectedChanged);

    QVERIFY(!message.selected());

    message.setSelected(true);
    QVERIFY(message.selected());
    QCOMPARE(selectedSpy.count(), 1);

    // Setting to the same value must not emit again
    message.setSelected(true);
    QCOMPARE(selectedSpy.count(), 1);

    message.setSelected(false);
    QVERIFY(!message.selected());
    QCOMPARE(selectedSpy.count(), 2);
}

void MAVLinkMessageTest::_fieldsPopulatedTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    const QmlObjectListModel *fields = message.fields();
    QVERIFY(fields != nullptr);

    // HEARTBEAT has 6 fields: type, autopilot, base_mode, custom_mode, system_status, mavlink_version
    const mavlink_message_info_t* info = MAVLinkMessageInfo::byId(msg.msgid);
    QVERIFY(info != nullptr);
    QCOMPARE(fields->count(), static_cast<int>(info->num_fields));
}

void MAVLinkMessageTest::_setTargetRateHzTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    QSignalSpy rateSpy(&message, &QGCMAVLinkMessage::targetRateHzChanged);

    QCOMPARE(message.targetRateHz(), static_cast<int32_t>(0));

    message.setTargetRateHz(10);
    QCOMPARE(message.targetRateHz(), static_cast<int32_t>(10));
    QCOMPARE(rateSpy.count(), 1);

    // Same value must not emit
    message.setTargetRateHz(10);
    QCOMPARE(rateSpy.count(), 1);
}

void MAVLinkMessageTest::_updateFreqTest()
{
    const mavlink_message_t msg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage message(msg);

    // Initial frequency is zero
    QCOMPARE_FUZZY(message.actualRateHz(), 0.0, 1e-9);

    // Simulate one update since last freq refresh
    message.update(msg);
    message.updateFreq();

    // _count started at 1, update() made it 2, _lastCount was 0
    // msgCount = 2 - 0 = 2; EWMA = 0.2*0 + 0.8*2 = 1.6
    QCOMPARE_FUZZY(message.actualRateHz(), 1.6, 1e-9);
}

void MAVLinkMessageTest::_transwingV4DecodingTest()
{
    std::array<float, 58> data{};
    data[0] = 4.0f;                                          // protocol_version
    data[1] = 45.5f;                                         // wing_angle
    data[2] = 50.0f;                                         // wing_angle_setpoint
    data[3] = 12.5f;                                         // wing_angle_rate_setpoint
    data[4] = 3.0f;                                          // wing_angle_accel_setpoint
    data[5] = 0.5f;                                          // wing_angle_jerk_setpoint
    data[6] = 28.0f;                                         // collective_tilt_angle
    data[7] = 4.5f;                                          // tracking_error
    data[8] = 1.0f;                                          // phase = Accelerate
    data[9] = 0.35f;                                         // fixed_wing_weight
    data[10] = 135.0f;                                       // status_flags = 0x87 (bits 0,1,2,7)
    data[11] = 8.0f;                                         // feedback_sample_age
    data[12] = 2.1f;                                         // attitude_error
    data[13] = 0.4f;                                         // altitude_loss
    data[14] = 0.62f;                                        // collective_command
    data[15] = 5.0f;                                         // abort_reason = LowAirspeed
    data[16] = 1.0f;                                         // pilot_takeover_available
    data[17] = 2.0f;                                         // pilot_control_owner = Pilot
    data[18] = 0.0f;                                         // transition_complete
    data[19] = std::numeric_limits<float>::quiet_NaN();      // handover_window_age = NaN
    data[20] = 1.2f;                                         // tilt_error
    data[21] = 3.4f;                                         // heading_error

    const mavlink_message_t rawMsg = MAVLinkTestHelpers::makeDebugFloatArray(22001, "TRANSWING", data, 9876543ULL);
    QGCMAVLinkMessage message(rawMsg, QGCMAVLinkMessage::extractInstanceValue(rawMsg));
    message.setSelected(true);

    QVERIFY(message.isTranswing());
    QVERIFY(message.isTranswingStatus());
    QCOMPARE(message.name(), QStringLiteral("TRANSWING_STATUS [v4]"));
    QCOMPARE(message.fields()->count(), 31);

    auto fieldValue = [&message](int index) -> QString {
        const auto *f = qobject_cast<const QGCMAVLinkMessageField *>(message.fields()->get(index));
        return f ? f->value() : QString();
    };
    auto fieldName = [&message](int index) -> QString {
        const auto *f = qobject_cast<const QGCMAVLinkMessageField *>(message.fields()->get(index));
        return f ? f->name() : QString();
    };

    QCOMPARE(fieldName(0), QStringLiteral("time_usec"));
    QCOMPARE(fieldValue(0), QStringLiteral("9876543"));
    QCOMPARE(fieldValue(1), QStringLiteral("4"));
    QCOMPARE(fieldValue(2), QStringLiteral("45.5"));
    QCOMPARE(fieldValue(9), QStringLiteral("1 (Accelerate)"));
    QCOMPARE(fieldValue(11), QStringLiteral("0x87 (135)"));
    QCOMPARE(fieldValue(16), QStringLiteral("5 (LowAirspeed)"));
    QCOMPARE(fieldValue(17), QStringLiteral("1 (True)"));
    QCOMPARE(fieldValue(18), QStringLiteral("2 (Pilot)"));
    QCOMPARE(fieldValue(19), QStringLiteral("0 (False)"));
    QCOMPARE(fieldValue(20), QStringLiteral("NaN"));

    // Unpacked 8 status flags from data[10] = 135 (bits 0, 1, 2, 7)
    QCOMPARE(fieldName(23), QStringLiteral("flag_wing_angle_valid"));
    QCOMPARE(fieldValue(23), QStringLiteral("1 (True)"));
    QCOMPARE(fieldValue(24), QStringLiteral("1 (True)"));
    QCOMPARE(fieldValue(25), QStringLiteral("1 (True)"));
    QCOMPARE(fieldValue(26), QStringLiteral("0 (False)"));
    QCOMPARE(fieldValue(27), QStringLiteral("0 (False)"));
    QCOMPARE(fieldValue(28), QStringLiteral("0 (False)"));
    QCOMPARE(fieldValue(29), QStringLiteral("0 (False)"));
    QCOMPARE(fieldName(30), QStringLiteral("flag_status_fresh"));
    QCOMPARE(fieldValue(30), QStringLiteral("1 (True)"));
}

void MAVLinkMessageTest::_transwingInstanceSeparationTest()
{
    std::array<float, 58> twData{};
    twData[0] = 4.0f;
    const mavlink_message_t twMsg = MAVLinkTestHelpers::makeDebugFloatArray(22001, "TRANSWING", twData);

    std::array<float, 58> genericData{};
    genericData[0] = 1.23f;
    const mavlink_message_t genericMsg = MAVLinkTestHelpers::makeDebugFloatArray(100, "OTHER_DBG", genericData);

    std::array<float, 58> oldSchemaData{};
    oldSchemaData[0] = 3.0f;
    const mavlink_message_t oldSchemaMsg = MAVLinkTestHelpers::makeDebugFloatArray(22001, "TRANSWING", oldSchemaData);

    const QString twInst = QGCMAVLinkMessage::extractInstanceValue(twMsg);
    const QString genInst = QGCMAVLinkMessage::extractInstanceValue(genericMsg);
    const QString oldInst = QGCMAVLinkMessage::extractInstanceValue(oldSchemaMsg);

    QCOMPARE(twInst, QStringLiteral("TRANSWING_v4"));
    QVERIFY(twInst != genInst);
    QVERIFY(twInst != oldInst);

    QGCMAVLinkMessage genericMessage(genericMsg, genInst);
    QVERIFY(!genericMessage.isTranswing());
    QVERIFY(!genericMessage.isTranswingStatus());

    QGCMAVLinkMessage oldSchemaMessage(oldSchemaMsg, oldInst);
    QVERIFY(!oldSchemaMessage.isTranswing());
    QVERIFY(!oldSchemaMessage.isTranswingStatus());
}

void MAVLinkMessageTest::_transwingClassificationTest()
{
    const mavlink_message_t hbMsg = MAVLinkTestHelpers::makeHeartbeat();
    QGCMAVLinkMessage hb(hbMsg);
    QVERIFY(!hb.isTranswing());
    QVERIFY(!hb.isTranswingStatus());

    auto checkTranswingMsgId = [](auto packFn) {
        mavlink_message_t raw{};
        packFn(&raw);
        QGCMAVLinkMessage msg(raw, QGCMAVLinkMessage::extractInstanceValue(raw));
        QVERIFY(msg.isTranswing());
        QVERIFY(!msg.isTranswingStatus());
    };

    checkTranswingMsgId([](mavlink_message_t *raw) {
        mavlink_msg_extended_sys_state_pack_chan(1, 1, MAVLINK_COMM_0, raw, MAV_VTOL_STATE_TRANSITION_TO_FW, MAV_LANDED_STATE_IN_AIR);
    });
    checkTranswingMsgId([](mavlink_message_t *raw) {
        mavlink_msg_vfr_hud_pack_chan(1, 1, MAVLINK_COMM_0, raw, 18.0f, 20.0f, 90, 50, 120.0f, 0.5f);
    });
    checkTranswingMsgId([](mavlink_message_t *raw) {
        const int32_t rpms[4] = {1000, 1000, 1000, 1000};
        const float volts[4] = {24.0f, 24.0f, 24.0f, 24.0f};
        const float currs[4] = {10.0f, 10.0f, 10.0f, 10.0f};
        mavlink_msg_esc_status_pack_chan(1, 1, MAVLINK_COMM_0, raw, 0, 0, rpms, volts, currs);
    });
    checkTranswingMsgId([](mavlink_message_t *raw) {
        const uint16_t voltages[10]{};
        const uint16_t voltagesExt[4]{};
        mavlink_msg_battery_status_pack_chan(1, 1, MAVLINK_COMM_0, raw, 0, MAV_BATTERY_FUNCTION_ALL, MAV_BATTERY_TYPE_LIPO, 2500, voltages, 100, 50, 0, 80, 300, MAV_BATTERY_CHARGE_STATE_OK, voltagesExt, 0, 0);
    });
    checkTranswingMsgId([](mavlink_message_t *raw) {
        mavlink_msg_servo_output_raw_pack_chan(1, 1, MAVLINK_COMM_0, raw, 0, 0, 1500, 1500, 1500, 1500, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    });
    checkTranswingMsgId([](mavlink_message_t *raw) {
        mavlink_msg_statustext_pack_chan(1, 1, MAVLINK_COMM_0, raw, MAV_SEVERITY_INFO, "test", 0, 0);
    });
}

UT_REGISTER_TEST(MAVLinkMessageTest, TestLabel::Unit, TestLabel::AnalyzeView)
