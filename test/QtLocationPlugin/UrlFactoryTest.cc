#include "UrlFactoryTest.h"

#include <QtCore/QBuffer>
#include <QtCore/QUrlQuery>
#include <QtGui/QImage>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtTest/QSignalSpy>

#include "Gcj02Transform.h"
#include "GcjTileReply.h"
#include "MapProvider.h"
#include "QGCMapUrlEngine.h"
#include "QGCTileSet.h"

static const QString kBingRoad = QStringLiteral("Bing Road");
static const QString kBingSatellite = QStringLiteral("Bing Satellite");
static const QString kBingHybrid = QStringLiteral("Bing Hybrid");
static const QString kCopernicus = QStringLiteral("Copernicus");
static const QString kTerrarium = QStringLiteral("Terrarium");

// --- Provider registry ---

void UrlFactoryTest::_testGetProviderTypesNotEmpty()
{
    const QStringList types = UrlFactory::getProviderTypes();
    QVERIFY(!types.isEmpty());
}

void UrlFactoryTest::_testGetProviderTypesContainsBing()
{
    const QStringList types = UrlFactory::getProviderTypes();
    QVERIFY(types.contains(kBingRoad));
    QVERIFY(types.contains(kBingSatellite));
    QVERIFY(types.contains(kBingHybrid));
}

void UrlFactoryTest::_testGetElevationProviderTypes()
{
    const QStringList elevTypes = UrlFactory::getElevationProviderTypes();
    QVERIFY(!elevTypes.isEmpty());
    QVERIFY(elevTypes.contains(kCopernicus));

    const QStringList allTypes = UrlFactory::getProviderTypes();
    for (const auto& t : elevTypes) {
        QVERIFY2(allTypes.contains(t), qPrintable(QStringLiteral("Elevation type '%1' not in allTypes").arg(t)));
    }
}

// --- Type <-> MapId roundtrip ---

void UrlFactoryTest::_testMapIdFromProviderTypeValid()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    QVERIFY(id > 0);
}

void UrlFactoryTest::_testProviderTypeFromMapIdRoundtrip()
{
    const QStringList types = UrlFactory::getProviderTypes();
    for (const auto& type : types) {
        const int id = UrlFactory::getQtMapIdFromProviderType(type);
        QVERIFY2(id > 0, qPrintable(QStringLiteral("Bad id for type: %1").arg(type)));
        const QString recovered = UrlFactory::getProviderTypeFromQtMapId(id);
        QCOMPARE(recovered, type);
    }
}

void UrlFactoryTest::_testMapIdFromEmptyType()
{
    QCOMPARE(UrlFactory::getQtMapIdFromProviderType(QString()), -1);
}

void UrlFactoryTest::_testMapIdFromInvalidType()
{
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("type not found:"));
    QCOMPARE(UrlFactory::getQtMapIdFromProviderType(QStringLiteral("Nonexistent")), -1);
    verifyExpectedLogMessage();
}

void UrlFactoryTest::_testProviderTypeFromInvalidId()
{
    QVERIFY(UrlFactory::getProviderTypeFromQtMapId(-1).isEmpty());
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("map id not found:"));
    QVERIFY(UrlFactory::getProviderTypeFromQtMapId(99999).isEmpty());
    verifyExpectedLogMessage();
}

// --- Hash <-> ProviderType roundtrip ---

void UrlFactoryTest::_testHashFromProviderTypeValid()
{
    const int hash = UrlFactory::hashFromProviderType(kBingRoad);
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    QCOMPARE(hash, id);
}

void UrlFactoryTest::_testProviderTypeFromHashRoundtrip()
{
    const QStringList types = UrlFactory::getProviderTypes();
    for (const auto& type : types) {
        const int hash = UrlFactory::hashFromProviderType(type);
        QVERIFY2(hash > 0, qPrintable(QStringLiteral("Bad hash for: %1").arg(type)));
        const QString recovered = UrlFactory::providerTypeFromHash(hash);
        QCOMPARE(recovered, type);
    }
}

void UrlFactoryTest::_testHashFromInvalidType()
{
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("provider not found for type:"));
    QCOMPARE(UrlFactory::hashFromProviderType(QString()), -1);
    verifyExpectedLogMessage();
}

void UrlFactoryTest::_testProviderTypeFromInvalidHash()
{
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("provider not found from hash:"));
    QVERIFY(UrlFactory::providerTypeFromHash(0).isEmpty());
    verifyExpectedLogMessage();
}

// --- Tile hash encode/decode ---

void UrlFactoryTest::_testGetTileHashFormat()
{
    const QString hash = UrlFactory::getTileHash(kBingRoad, 100, 200, 5);
    // Format: "%010d%08d%08d%03d" = 10+8+8+3 = 29 chars
    QCOMPARE(hash.length(), 29);
}

void UrlFactoryTest::_testTileHashToTypeRoundtrip()
{
    const QStringList types = UrlFactory::getProviderTypes();
    for (const auto& type : types) {
        const QString hash = UrlFactory::getTileHash(type, 42, 99, 7);
        const QString recovered = UrlFactory::tileHashToType(hash);
        QCOMPARE(recovered, type);
    }
}

void UrlFactoryTest::_testTileHashToTypeInvalid()
{
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("provider not found from hash:"));
    QVERIFY(UrlFactory::tileHashToType(QStringLiteral("garbage")).isEmpty());
    verifyExpectedLogMessage();
}

// --- Image format via facade ---

void UrlFactoryTest::_testGetImageFormatByType()
{
    const QByteArray png(
        "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
        "data",
        12);
    QCOMPARE(UrlFactory::getImageFormat(kBingRoad, png), QStringLiteral("png"));
}

void UrlFactoryTest::_testGetImageFormatByMapId()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    const QByteArray jpeg(
        "\xFF\xD8\xFF\xE0"
        "data",
        8);
    QCOMPARE(UrlFactory::getImageFormat(id, jpeg), QStringLiteral("jpg"));
}

void UrlFactoryTest::_testGetImageFormatInvalidInputs()
{
    const QByteArray png("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
    QVERIFY(UrlFactory::getImageFormat(QString(), png).isEmpty());
    QVERIFY(UrlFactory::getImageFormat(-1, png).isEmpty());
}

void UrlFactoryTest::_testGetTileURLByType()
{
    const QUrl url = UrlFactory::getTileURL(kBingRoad, 301, 385, 10);
    QVERIFY(url.isValid());
    QVERIFY(!url.isEmpty());
    QVERIFY(url.scheme().startsWith(QStringLiteral("http")));
}

void UrlFactoryTest::_testGetTileURLByMapId()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    QVERIFY(id > 0);
    const QUrl url = UrlFactory::getTileURL(id, 301, 385, 10);
    QVERIFY(url.isValid());
    QVERIFY(!url.isEmpty());
    QVERIFY(url.scheme().startsWith(QStringLiteral("http")));
}

void UrlFactoryTest::_testGetTileURLInvalidInputs()
{
    QVERIFY(UrlFactory::getTileURL(QString(), 0, 0, 1).isEmpty());
    QVERIFY(UrlFactory::getTileURL(-1, 0, 0, 1).isEmpty());
}

// --- averageSizeForType ---

void UrlFactoryTest::_testAverageSizeForKnownProviders()
{
    QCOMPARE(UrlFactory::averageSizeForType(kBingRoad), static_cast<quint32>(1297));
    QCOMPARE(UrlFactory::averageSizeForType(kBingSatellite), static_cast<quint32>(19597));
    QCOMPARE(UrlFactory::averageSizeForType(kCopernicus), static_cast<quint32>(2786));
}

void UrlFactoryTest::_testAverageSizeForInvalidType()
{
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("type not found:"));
    QCOMPARE(UrlFactory::averageSizeForType(QStringLiteral("Nonexistent")), QGC_AVERAGE_TILE_SIZE);
    verifyExpectedLogMessage();
}

// --- isElevation ---

void UrlFactoryTest::_testIsElevationTrue()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kCopernicus);
    QVERIFY(UrlFactory::isElevation(id));
}

void UrlFactoryTest::_testIsElevationFalse()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    QVERIFY(!UrlFactory::isElevation(id));
    QVERIFY(!UrlFactory::isElevation(-1));
}

// --- Coordinate conversion via facade ---

void UrlFactoryTest::_testLong2tileXDelegates()
{
    // NYC at z=10
    QCOMPARE(UrlFactory::long2tileX(kBingRoad, -73.9857, 10), 301);
}

void UrlFactoryTest::_testLat2tileYDelegates()
{
    // NYC at z=10
    QCOMPARE(UrlFactory::lat2tileY(kBingRoad, 40.7128, 10), 385);
}

void UrlFactoryTest::_testCoordConversionInvalidType()
{
    QCOMPARE(UrlFactory::long2tileX(QString(), -73.9857, 10), 0);
    QCOMPARE(UrlFactory::lat2tileY(QString(), 40.7128, 10), 0);
}

// --- Copernicus elevation specifics ---

void UrlFactoryTest::_testCopernicusLong2tileX()
{
    // floor((0+180)/0.01) = 18000
    QCOMPARE(UrlFactory::long2tileX(kCopernicus, 0.0, 1), 18000);
    // floor((-180+180)/0.01) = 0
    QCOMPARE(UrlFactory::long2tileX(kCopernicus, -180.0, 1), 0);
}

void UrlFactoryTest::_testCopernicusLat2tileY()
{
    // floor((0+90)/0.01) = 9000
    QCOMPARE(UrlFactory::lat2tileY(kCopernicus, 0.0, 1), 9000);
    // floor((-90+90)/0.01) = 0
    QCOMPARE(UrlFactory::lat2tileY(kCopernicus, -90.0, 1), 0);
}

void UrlFactoryTest::_testCopernicusTileCount()
{
    // Small 0.05° box near origin
    // Copernicus getTileCount swaps lat semantics: tileY0 = lat2tileY(bottomRightLat), tileY1 = lat2tileY(topleftLat)
    const auto set = UrlFactory::getTileCount(1, 0.0, 0.05, 0.05, 0.0, kCopernicus);
    // tileX: floor((0+180)/0.01)=18000 to floor((0.05+180)/0.01)=18005 → 6 tiles
    // tileY: floor((0+90)/0.01)=9000 to floor((0.05+90)/0.01)=9005 → 6 tiles
    // total = 6*6 = 36
    QCOMPARE(set.tileCount, static_cast<quint64>(36));
}

// --- Terrarium elevation provider ---

void UrlFactoryTest::_testTerrariumRegistered()
{
    QVERIFY(UrlFactory::getElevationProviderTypes().contains(kTerrarium));

    auto provider = UrlFactory::getMapProviderFromProviderType(kTerrarium);
    QVERIFY(provider != nullptr);
    QVERIFY(provider->isElevationProvider());
    // Unrecognized payload falls back to the provider's declared format
    QCOMPARE(UrlFactory::getImageFormat(kTerrarium, QByteArrayView("xxx", 3)), QStringLiteral("png"));
}

void UrlFactoryTest::_testTerrariumTileURL()
{
    const QUrl url = UrlFactory::getTileURL(kTerrarium, 1308, 2865, 13);
    QCOMPARE(url.toString(),
             QStringLiteral("https://s3.amazonaws.com/elevation-tiles-prod/terrarium/13/1308/2865.png"));

    // Standard slippy tile math, unlike Copernicus's fixed 0.01° grid
    QCOMPARE(UrlFactory::long2tileX(kTerrarium, 0.0, 1), 1);
    QCOMPARE(UrlFactory::lat2tileY(kTerrarium, 0.0, 1), 1);
}

void UrlFactoryTest::_testTerrariumNotUserSelectable()
{
    // GeoMap-only provider: hidden from the elevation dropdown and the
    // imagery provider list (which subtracts elevation types)
    auto provider = UrlFactory::getMapProviderFromProviderType(kTerrarium);
    QVERIFY(provider != nullptr);
    QVERIFY(!provider->isUserSelectable());

    auto copernicus = UrlFactory::getMapProviderFromProviderType(kCopernicus);
    QVERIFY(copernicus != nullptr);
    QVERIFY(copernicus->isUserSelectable());
}

// --- getTileCount ---

void UrlFactoryTest::_testGetTileCountValid()
{
    // z=1 full world via Bing Road
    const auto set = UrlFactory::getTileCount(1, -179.9, 85.0, 179.9, -85.0, kBingRoad);
    QVERIFY(set.tileCount > 0);
    QCOMPARE(set.tileCount, static_cast<quint64>(4));
}

void UrlFactoryTest::_testGetTileCountInvalidType()
{
    const auto set = UrlFactory::getTileCount(10, -73.0, 41.0, -72.0, 40.0, QString());
    QCOMPARE(set.tileCount, static_cast<quint64>(0));
    QCOMPARE(set.tileSize, static_cast<quint64>(0));
}

void UrlFactoryTest::_testGetTileCountZoomClamped()
{
    // zoom=0 should be clamped to 1 internally
    const auto setLow = UrlFactory::getTileCount(0, -179.9, 85.0, 179.9, -85.0, kBingRoad);
    const auto setOne = UrlFactory::getTileCount(1, -179.9, 85.0, 179.9, -85.0, kBingRoad);
    QCOMPARE(setLow.tileCount, setOne.tileCount);

    // zoom=99 should be clamped to 23
    const auto setHigh = UrlFactory::getTileCount(99, 0.0, 0.001, 0.001, 0.0, kBingRoad);
    const auto set23 = UrlFactory::getTileCount(23, 0.0, 0.001, 0.001, 0.0, kBingRoad);
    QCOMPARE(setHigh.tileCount, set23.tileCount);
}

// --- Provider lookup ---

void UrlFactoryTest::_testGetMapProviderValid()
{
    const int id = UrlFactory::getQtMapIdFromProviderType(kBingRoad);
    auto provider = UrlFactory::getMapProviderFromQtMapId(id);
    QVERIFY(provider != nullptr);
    QCOMPARE(provider->getMapName(), kBingRoad);
    QVERIFY(provider->isBingProvider());

    auto byType = UrlFactory::getMapProviderFromProviderType(kBingRoad);
    QVERIFY(byType != nullptr);
    QCOMPARE(byType->getMapName(), kBingRoad);
}

void UrlFactoryTest::_testGetMapProviderInvalid()
{
    QVERIFY(UrlFactory::getMapProviderFromQtMapId(-1) == nullptr);
    QVERIFY(UrlFactory::getMapProviderFromProviderType(QString()) == nullptr);
    expectLogMessage("QtLocationPlugin.QGCMapUrlEngine", QtWarningMsg, QRegularExpression("type not found:"));
    QVERIFY(UrlFactory::getMapProviderFromProviderType(QStringLiteral("Nonexistent")) == nullptr);
    verifyExpectedLogMessage();
}

void UrlFactoryTest::_testGcj02Cities()
{
    struct Reference
    {
        QGeoCoordinate wgs;
        QGeoCoordinate gcj;
    };

    const Reference cities[] = {
        {{39.9042, 116.4074}, {39.905603, 116.413642}},
        {{31.2304, 121.4737}, {31.228458, 121.478223}},
        {{22.5431, 114.0579}, {22.540383, 114.063014}},
        {{30.5728, 104.0668}, {30.570346, 104.069305}},
    };
    for (const auto& city : cities) {
        QVERIFY(Gcj02Transform::forward(city.wgs).distanceTo(city.gcj) < 1);
        QVERIFY(Gcj02Transform::inverse(city.gcj).distanceTo(city.wgs) < 1);
    }
    const QGeoCoordinate outside(48.86, 2.35);
    QCOMPARE(Gcj02Transform::forward(outside), outside);
    QCOMPARE(Gcj02Transform::inverse(outside), outside);
}

void UrlFactoryTest::_testAMapTiles()
{
    const auto types = UrlFactory::getProviderTypes();
    QCOMPARE(types.first(4), QStringList({"AMap Satellite", "AMap Road", "TianDiTu Road", "TianDiTu Satellite"}));
    QVERIFY(types.contains("Bing Road"));
    QVERIFY(!types.filter("Esri ").isEmpty());
    QVERIFY(!types.filter("Mapbox ").isEmpty());
    QCOMPARE(UrlFactory::getTileURL(u"AMap Satellite", 1, 2, 3).toString(),
             QStringLiteral("https://webst02.is.autonavi.com/appmaptile?style=6&x=1&y=2&z=3"));
    QCOMPARE(
        UrlFactory::getTileURL(u"AMap Road", 1, 2, 3).toString(),
        QStringLiteral("https://webrd02.is.autonavi.com/appmaptile?lang=zh_cn&size=1&scale=1&style=8&x=1&y=2&z=3"));

    const auto parts = Gcj02Transform::tileParts(53959, 24833, 16);
    QCOMPARE(parts.size(), 4);
    QCOMPARE(parts.first().index, QPoint(53960, 24832));
    QCOMPARE(parts.first().source, QRect(35, 171, 221, 85));

    class SyntheticTiles final : public QNetworkAccessManager
    {
    public:
        bool corrupt = false;

    private:
        QNetworkReply* createRequest(Operation op, const QNetworkRequest& request, QIODevice* body) override
        {
            const QUrlQuery query(request.url());
            const int col = query.queryItemValue("x").toInt() - 53960;
            const int row = query.queryItemValue("y").toInt() - 24832;
            QImage source(256, 256, QImage::Format_RGB32);
            for (int y = 0; y < 256; ++y) {
                for (int x = 0; x < 256; ++x) {
                    source.setPixel(x, y, qRgb(x, y, col + 2 * row));
                }
            }
            QBuffer buffer;
            buffer.open(QIODevice::WriteOnly);
            source.save(&buffer, "PNG");
            const QByteArray bytes = corrupt && col == 0 && row == 0 ? QByteArray("invalid") : buffer.data();
            QNetworkRequest local(QUrl("data:image/png;base64," + bytes.toBase64()));
            return QNetworkAccessManager::createRequest(op, local, body);
        }
    } network;

    const auto provider = UrlFactory::getMapProviderFromProviderType(u"AMap Road");
    QVERIFY(provider);
    auto* reply = fetchMapTile(&network, QNetworkRequest(provider->getTileURL(53959, 24833, 16)), provider->getMapId(),
                               53959, 24833, 16);
    QTRY_VERIFY_WITH_TIMEOUT(reply->isFinished(), 5000);
    QCOMPARE(reply->error(), QNetworkReply::NoError);
    const QImage stitched = QImage::fromData(reply->readAll());
    QCOMPARE(stitched.size(), QSize(256, 256));
    for (int y = 0; y < 256; ++y) {
        for (int x = 0; x < 256; ++x) {
            QCOMPARE(stitched.pixel(x, y),
                     qRgb((x + 35) % 256, (y + 171) % 256, (x + 35) / 256 + 2 * ((y + 171) / 256)));
        }
    }

    network.corrupt = true;
    const QNetworkRequest request(provider->getTileURL(53959, 24833, 16));
    auto* failed = fetchMapTile(&network, request, provider->getMapId(), 53959, 24833, 16);
    QSignalSpy finished(failed, &QNetworkReply::finished);
    QTRY_COMPARE_WITH_TIMEOUT(finished.count(), 1, 5000);
    QCOMPARE(failed->error(), QNetworkReply::UnknownContentError);
    auto* canceled = fetchMapTile(&network, request, provider->getMapId(), 53959, 24833, 16);
    canceled->abort();
    QVERIFY(canceled->isFinished());
    QCOMPARE(canceled->error(), QNetworkReply::OperationCanceledError);
}

UT_REGISTER_TEST(UrlFactoryTest, TestLabel::Unit)
