#pragma once

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

QNetworkReply* fetchMapTile(QNetworkAccessManager* manager, const QNetworkRequest& request, int mapId, int x, int y,
                            int zoom);
