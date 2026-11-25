#pragma once

#include "MapProvider.h"

class AMapProvider final : public MapProvider
{
public:
    explicit AMapProvider(MapStyle style);

    static bool isGcj02CorrectionEnabled();

    bool requiresGcj02Correction() const final { return isGcj02CorrectionEnabled(); }

private:
    QString _getURL(int x, int y, int zoom) const final;
};
