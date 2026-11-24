#include "Gcj02Transform.h"

#include <cmath>
#include <numbers>

#include <QtCore/QtMath>

namespace Gcj02Transform {
using std::numbers::pi;

bool isOutOfChina(double latitude, double longitude)
{
    return longitude < 72.004 || longitude > 137.8347 || latitude < 0.8293 || latitude > 55.8271;
}

QGeoCoordinate forward(QGeoCoordinate wgs)
{
    if (!wgs.isValid() || isOutOfChina(wgs.latitude(), wgs.longitude())) {
        return wgs;
    }
    const double x = wgs.longitude() - 105;
    const double y = wgs.latitude() - 35;
    const auto wave = [](double t) { return std::sin(t * pi) * (2.0 / 3); };
    const double common = 20 * (wave(6 * x) + wave(2 * x));
    const double north = -100 + 2 * x + 3 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * std::sqrt(std::abs(x)) + common +
                         20 * wave(y) + 40 * wave(y / 3) + 160 * wave(y / 12) + 320 * wave(y / 30);
    const double east = 300 + x + 2 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * std::sqrt(std::abs(x)) + common +
                        20 * wave(x) + 40 * wave(x / 3) + 150 * wave(x / 12) + 300 * wave(x / 30);
    constexpr double axis = 6378245.0;
    constexpr double eccentricitySquared = 0.00669342162296594323;
    const double lat = wgs.latitude() * pi / 180;
    const double scale = 1 - eccentricitySquared * std::pow(std::sin(lat), 2);
    return {wgs.latitude() + north * 180 * std::pow(scale, 1.5) / (axis * (1 - eccentricitySquared) * pi),
            wgs.longitude() + east * 180 * std::sqrt(scale) / (axis * std::cos(lat) * pi), wgs.altitude()};
}

QGeoCoordinate inverse(QGeoCoordinate gcj)
{
    auto estimate = gcj;
    for (int i = 0; i < 10; ++i) {
        const auto projected = forward(estimate);
        const double latError = projected.latitude() - gcj.latitude();
        const double lonError = projected.longitude() - gcj.longitude();
        if (std::abs(latError) + std::abs(lonError) < 1e-9) {
            break;
        }
        estimate.setLatitude(estimate.latitude() - latError);
        estimate.setLongitude(estimate.longitude() - lonError);
    }
    return estimate;
}

QList<TilePart> tileParts(int x, int y, int zoom)
{
    const double count = std::ldexp(1.0, zoom);
    const double latitude = std::atan(std::sinh(pi * (1 - 2 * (y + 0.5) / count))) * 180 / pi;
    const double longitude = (x + 0.5) / count * 360 - 180;
    const auto gcj = forward({latitude, longitude});
    // Translate the raster by its centre displacement, rounded to the nearest source pixel.
    const int dx = qRound((gcj.longitude() - longitude) / 360 * count * 256);
    const int dy = qRound(((1 - std::asinh(std::tan(gcj.latitude() * pi / 180)) / pi) * count / 2 - (y + 0.5)) * 256);
    const QRect window(dx, dy, 256, 256);
    QList<TilePart> parts;
    for (int row = qFloor(dy / 256.0); row <= qFloor(window.bottom() / 256.0); ++row) {
        for (int col = qFloor(dx / 256.0); col <= qFloor(window.right() / 256.0); ++col) {
            const QPoint origin(col * 256, row * 256);
            const QRect overlap = window.intersected(QRect(origin, QSize(256, 256)));
            parts.append(
                {QPoint(x + col, y + row), overlap.translated(-origin), overlap.translated(-window.topLeft())});
        }
    }
    return parts;
}
}  // namespace Gcj02Transform
