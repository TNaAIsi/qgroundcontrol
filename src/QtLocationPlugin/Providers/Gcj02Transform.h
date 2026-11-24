#pragma once

#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtPositioning/QGeoCoordinate>

namespace Gcj02Transform {
bool isOutOfChina(double latitude, double longitude);
QGeoCoordinate forward(QGeoCoordinate wgs);
QGeoCoordinate inverse(QGeoCoordinate gcj);

struct TilePart
{
    QPoint index;
    QRect source;
    QRect destination;
};

QList<TilePart> tileParts(int x, int y, int zoom);
}  // namespace Gcj02Transform
