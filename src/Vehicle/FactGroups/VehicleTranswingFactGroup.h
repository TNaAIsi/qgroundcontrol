#pragma once

#include <QtCore/QElapsedTimer>

#include "FactGroup.h"

class VehicleTranswingFactGroup : public FactGroup
{
    Q_OBJECT
    Q_PROPERTY(Fact* wingAngle READ wingAngle CONSTANT)
    Q_PROPERTY(Fact* wingAngleSetpoint READ wingAngleSetpoint CONSTANT)
    Q_PROPERTY(Fact* wingAngleRateSetpoint READ wingAngleRateSetpoint CONSTANT)
    Q_PROPERTY(Fact* wingAngleAccelSetpoint READ wingAngleAccelSetpoint CONSTANT)
    Q_PROPERTY(Fact* wingAngleJerkSetpoint READ wingAngleJerkSetpoint CONSTANT)
    Q_PROPERTY(Fact* collectiveTiltAngle READ collectiveTiltAngle CONSTANT)
    Q_PROPERTY(Fact* trackingError READ trackingError CONSTANT)
    Q_PROPERTY(Fact* phase READ phase CONSTANT)
    Q_PROPERTY(Fact* fixedWingWeight READ fixedWingWeight CONSTANT)
    Q_PROPERTY(Fact* feedbackSampleAge READ feedbackSampleAge CONSTANT)
    Q_PROPERTY(Fact* attitudeError READ attitudeError CONSTANT)
    Q_PROPERTY(Fact* altitudeLoss READ altitudeLoss CONSTANT)
    Q_PROPERTY(Fact* collectiveCommand READ collectiveCommand CONSTANT)
    Q_PROPERTY(Fact* abortReason READ abortReason CONSTANT)
    Q_PROPERTY(Fact* pilotTakeoverAvailable READ pilotTakeoverAvailable CONSTANT)
    Q_PROPERTY(Fact* pilotControlOwner READ pilotControlOwner CONSTANT)
    Q_PROPERTY(Fact* transitionComplete READ transitionComplete CONSTANT)
    Q_PROPERTY(Fact* handoverWindowAge READ handoverWindowAge CONSTANT)
    Q_PROPERTY(Fact* tiltError READ tiltError CONSTANT)
    Q_PROPERTY(Fact* headingError READ headingError CONSTANT)
    Q_PROPERTY(Fact* wingAngleValid READ wingAngleValid CONSTANT)
    Q_PROPERTY(Fact* feedbackHealthy READ feedbackHealthy CONSTANT)
    Q_PROPERTY(Fact* trackingErrorOk READ trackingErrorOk CONSTANT)
    Q_PROPERTY(Fact* flightControlActive READ flightControlActive CONSTANT)
    Q_PROPERTY(Fact* groundTestActive READ groundTestActive CONSTANT)
    Q_PROPERTY(Fact* feedbackHoldActive READ feedbackHoldActive CONSTANT)
    Q_PROPERTY(Fact* pilotControlLocked READ pilotControlLocked CONSTANT)
    Q_PROPERTY(Fact* telemetryFresh READ telemetryFresh CONSTANT)
    Q_PROPERTY(Fact* vtolState READ vtolState CONSTANT)
    Q_PROPERTY(Fact* landedState READ landedState CONSTANT)

public:
    explicit VehicleTranswingFactGroup(QObject* parent = nullptr);
    void handleMessage(Vehicle* vehicle, const mavlink_message_t& message) final;

    Fact* wingAngle() { return &_wingAngle; }

    Fact* wingAngleSetpoint() { return &_wingAngleSetpoint; }

    Fact* wingAngleRateSetpoint() { return &_wingAngleRateSetpoint; }

    Fact* wingAngleAccelSetpoint() { return &_wingAngleAccelSetpoint; }

    Fact* wingAngleJerkSetpoint() { return &_wingAngleJerkSetpoint; }

    Fact* collectiveTiltAngle() { return &_collectiveTiltAngle; }

    Fact* trackingError() { return &_trackingError; }

    Fact* phase() { return &_phase; }

    Fact* fixedWingWeight() { return &_fixedWingWeight; }

    Fact* feedbackSampleAge() { return &_feedbackSampleAge; }

    Fact* attitudeError() { return &_attitudeError; }

    Fact* altitudeLoss() { return &_altitudeLoss; }

    Fact* collectiveCommand() { return &_collectiveCommand; }

    Fact* abortReason() { return &_abortReason; }

    Fact* pilotTakeoverAvailable() { return &_pilotTakeoverAvailable; }

    Fact* pilotControlOwner() { return &_pilotControlOwner; }

    Fact* transitionComplete() { return &_transitionComplete; }

    Fact* handoverWindowAge() { return &_handoverWindowAge; }

    Fact* tiltError() { return &_tiltError; }

    Fact* headingError() { return &_headingError; }

    Fact* wingAngleValid() { return &_wingAngleValid; }

    Fact* feedbackHealthy() { return &_feedbackHealthy; }

    Fact* trackingErrorOk() { return &_trackingErrorOk; }

    Fact* flightControlActive() { return &_flightControlActive; }

    Fact* groundTestActive() { return &_groundTestActive; }

    Fact* feedbackHoldActive() { return &_feedbackHoldActive; }

    Fact* pilotControlLocked() { return &_pilotControlLocked; }

    Fact* telemetryFresh() { return &_telemetryFresh; }

    Fact* vtolState() { return &_vtolState; }

    Fact* landedState() { return &_landedState; }

private:
    void _clearSnapshot();
    void _expireSnapshot();

    QElapsedTimer _lastSnapshot;
    QTimer _expiryTimer;
    Fact _wingAngle{0, QStringLiteral("wingAngle"), FactMetaData::valueTypeDouble};
    Fact _wingAngleSetpoint{0, QStringLiteral("wingAngleSetpoint"), FactMetaData::valueTypeDouble};
    Fact _wingAngleRateSetpoint{0, QStringLiteral("wingAngleRateSetpoint"), FactMetaData::valueTypeDouble};
    Fact _wingAngleAccelSetpoint{0, QStringLiteral("wingAngleAccelSetpoint"), FactMetaData::valueTypeDouble};
    Fact _wingAngleJerkSetpoint{0, QStringLiteral("wingAngleJerkSetpoint"), FactMetaData::valueTypeDouble};
    Fact _collectiveTiltAngle{0, QStringLiteral("collectiveTiltAngle"), FactMetaData::valueTypeDouble};
    Fact _trackingError{0, QStringLiteral("trackingError"), FactMetaData::valueTypeDouble};
    Fact _phase{0, QStringLiteral("phase"), FactMetaData::valueTypeUint8};
    Fact _fixedWingWeight{0, QStringLiteral("fixedWingWeight"), FactMetaData::valueTypeDouble};
    Fact _feedbackSampleAge{0, QStringLiteral("feedbackSampleAge"), FactMetaData::valueTypeDouble};
    Fact _attitudeError{0, QStringLiteral("attitudeError"), FactMetaData::valueTypeDouble};
    Fact _altitudeLoss{0, QStringLiteral("altitudeLoss"), FactMetaData::valueTypeDouble};
    Fact _collectiveCommand{0, QStringLiteral("collectiveCommand"), FactMetaData::valueTypeDouble};
    Fact _abortReason{0, QStringLiteral("abortReason"), FactMetaData::valueTypeUint8};
    Fact _pilotTakeoverAvailable{0, QStringLiteral("pilotTakeoverAvailable"), FactMetaData::valueTypeUint8};
    Fact _pilotControlOwner{0, QStringLiteral("pilotControlOwner"), FactMetaData::valueTypeUint8};
    Fact _transitionComplete{0, QStringLiteral("transitionComplete"), FactMetaData::valueTypeUint8};
    Fact _handoverWindowAge{0, QStringLiteral("handoverWindowAge"), FactMetaData::valueTypeDouble};
    Fact _tiltError{0, QStringLiteral("tiltError"), FactMetaData::valueTypeDouble};
    Fact _headingError{0, QStringLiteral("headingError"), FactMetaData::valueTypeDouble};
    Fact _wingAngleValid{0, QStringLiteral("wingAngleValid"), FactMetaData::valueTypeUint8};
    Fact _feedbackHealthy{0, QStringLiteral("feedbackHealthy"), FactMetaData::valueTypeUint8};
    Fact _trackingErrorOk{0, QStringLiteral("trackingErrorOk"), FactMetaData::valueTypeUint8};
    Fact _flightControlActive{0, QStringLiteral("flightControlActive"), FactMetaData::valueTypeUint8};
    Fact _groundTestActive{0, QStringLiteral("groundTestActive"), FactMetaData::valueTypeUint8};
    Fact _feedbackHoldActive{0, QStringLiteral("feedbackHoldActive"), FactMetaData::valueTypeUint8};
    Fact _pilotControlLocked{0, QStringLiteral("pilotControlLocked"), FactMetaData::valueTypeUint8};
    Fact _telemetryFresh{0, QStringLiteral("telemetryFresh"), FactMetaData::valueTypeUint8};
    Fact _vtolState{0, QStringLiteral("vtolState"), FactMetaData::valueTypeUint8};
    Fact _landedState{0, QStringLiteral("landedState"), FactMetaData::valueTypeUint8};
};
