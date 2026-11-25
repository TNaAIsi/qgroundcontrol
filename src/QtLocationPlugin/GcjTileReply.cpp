#include "GcjTileReply.h"

#include <QtCore/QBuffer>
#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "Gcj02Transform.h"
#include "MapProvider.h"
#include "QGCMapUrlEngine.h"
#include "QGCNetworkHelper.h"

namespace {
class GcjTileReply final : public QNetworkReply
{
public:
    GcjTileReply(QNetworkAccessManager* manager, const QNetworkRequest& request, const MapProvider& provider,
                 const QList<Gcj02Transform::TilePart>& parts, int zoom)
        : QNetworkReply(manager)
        , _remaining(parts.size())
        , _format(provider.getMapStyle() == MapProvider::SatelliteMapDay ? "JPEG" : "PNG")
    {
        setRequest(request);
        setUrl(request.url());
        open(ReadOnly);
        for (const auto& part : parts) {
            QNetworkRequest subRequest(request);
            subRequest.setUrl(provider.getTileURL(part.index.x(), part.index.y(), zoom));
            auto* reply = manager->get(subRequest);
            // QNetworkAccessManager tears down its replies as a group. Keep them siblings.
            _sources.append(reply);
            connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
            QGCNetworkHelper::ignoreSslErrorsIfNeeded(reply);
            connect(reply, &QNetworkReply::finished, this, [this, reply, part] {
                reply->deleteLater();
                if (isFinished()) {
                    return;
                }
                if (reply->error() != NoError) {
                    _fail(reply->error(), reply->errorString());
                    return;
                }
                const QImage image = QImage::fromData(reply->readAll());
                if (image.size() != QSize(256, 256)) {
                    _fail(UnknownContentError, tr("Invalid map tile image"));
                    return;
                }
                QPainter painter(&_tile);
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(part.destination, image, part.source);
                painter.end();
                if (--_remaining == 0) {
                    if (!_buffer.open(ReadWrite) || !_tile.save(&_buffer, _format) || !_buffer.seek(0)) {
                        _fail(UnknownContentError, tr("Could not encode map tile"));
                        return;
                    }
                    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
                    setFinished(true);
                    emit finished();
                }
            });
        }
    }

    void abort() override
    {
        if (!isFinished()) {
            _fail(OperationCanceledError, tr("Map tile request canceled"));
        }
    }

    qint64 bytesAvailable() const override { return _buffer.bytesAvailable() + QNetworkReply::bytesAvailable(); }

protected:
    qint64 readData(char* data, qint64 size) override { return _buffer.read(data, size); }

private:
    void _fail(NetworkError error, const QString& message)
    {
        setFinished(true);
        setError(error, message);
        for (const auto& reply : std::as_const(_sources)) {
            if (reply && !reply->isFinished()) {
                reply->abort();
            }
        }
        emit errorOccurred(error);
        emit finished();
    }

    QList<QPointer<QNetworkReply>> _sources;
    qsizetype _remaining;
    const char* _format;
    QImage _tile{256, 256, QImage::Format_ARGB32};
    QBuffer _buffer;
};
}  // namespace

QNetworkReply* fetchMapTile(QNetworkAccessManager* manager, const QNetworkRequest& request, int mapId, int x, int y,
                            int zoom)
{
    const auto provider = UrlFactory::getMapProviderFromQtMapId(mapId);
    if (provider && provider->requiresGcj02Correction()) {
        const auto parts = Gcj02Transform::tileParts(x, y, zoom);
        if (parts.size() != 1 || parts.front().index != QPoint(x, y)) {
            return new GcjTileReply(manager, request, *provider, parts, zoom);
        }
    }
    return manager->get(request);
}
