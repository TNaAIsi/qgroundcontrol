#include "AMapProvider.h"

#include "FlightMapSettings.h"
#include "SettingsManager.h"

AMapProvider::AMapProvider(MapStyle style)
    : MapProvider(style == SatelliteMapDay ? QStringLiteral("AMap Satellite") : QStringLiteral("AMap Road"),
                  QStringLiteral("https://www.amap.com/"), style == SatelliteMapDay ? "jpg" : "png",
                  QGC_AVERAGE_TILE_SIZE, style)
{}

bool AMapProvider::isGcj02CorrectionEnabled()
{
    SettingsManager *const settingsManager = SettingsManager::instance();
    if (!settingsManager) {
        return true;
    }
    FlightMapSettings *const flightMapSettings = settingsManager->flightMapSettings();
    if (!flightMapSettings) {
        return true;
    }
    Fact *const fact = flightMapSettings->amapGcj02Correction();
    return fact ? fact->rawValue().toBool() : true;
}

QString AMapProvider::_getURL(int x, int y, int zoom) const
{
    const QString pattern =
        getMapStyle() == SatelliteMapDay
            ? QStringLiteral("https://webst0%1.is.autonavi.com/appmaptile?style=6&x=%2&y=%3&z=%4")
            : QStringLiteral(
                  "https://webrd0%1.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x=%2&y=%3&z=%4");
    return pattern.arg(1 + _getServerNum(x, y, 4)).arg(x).arg(y).arg(zoom);
}
