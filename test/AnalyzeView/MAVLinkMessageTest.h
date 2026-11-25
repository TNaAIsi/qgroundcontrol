#pragma once

#include "UnitTest.h"

class MAVLinkMessageTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _constructionTest();
    void _countStartsAtOneTest();
    void _updateIncrementsCountTest();
    void _setSelectedTest();
    void _fieldsPopulatedTest();
    void _setTargetRateHzTest();
    void _updateFreqTest();
    void _transwingV4DecodingTest();
    void _transwingInstanceSeparationTest();
    void _transwingClassificationTest();
};
