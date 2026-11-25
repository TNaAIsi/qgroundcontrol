#pragma once

#include "UnitTest.h"

class VehicleTranswingFactGroupTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _decodeSnapshot();
    void _ignoreOtherPackets();
    void _expireSnapshot();
};
