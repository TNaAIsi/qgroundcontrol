#include "AMapProvider.h"

AMapProvider::AMapProvider(MapStyle style)
    : MapProvider(style == SatelliteMapDay ? QStringLiteral("AMap Satellite") : QStringLiteral("AMap Road"),
                  QStringLiteral("https://www.amap.com/"), style == SatelliteMapDay ? "jpg" : "png",
                  QGC_AVERAGE_TILE_SIZE, style)
{}

QString AMapProvider::_getURL(int x, int y, int zoom) const
{
    const QString pattern =
        getMapStyle() == SatelliteMapDay
            ? QStringLiteral("https://webst0%1.is.autonavi.com/appmaptile?style=6&x=%2&y=%3&z=%4")
            : QStringLiteral(
                  "https://webrd0%1.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x=%2&y=%3&z=%4");
    return pattern.arg(1 + _getServerNum(x, y, 4)).arg(x).arg(y).arg(zoom);
}
