#pragma once

#include "QGVLayerTiles.h"
#include "QGVImage.h"


class QGVTileCache
{
public:
    QGVTileCache(const QString& path);
    ~QGVTileCache();

    bool isOpen() const;
    bool isCached(const QGV::GeoTilePos& tilePos) const;
    bool cache(const QGV::GeoTilePos& tilePos, const QByteArray& rawImage, const QString &url);
    QGVImage *getCached(const QGV::GeoTilePos& tilePos) const;

private:
//    QSqlDatabase *mDB = nullptr;
    bool mOpened = false;
    QString mPath;

    bool createTable();
};
