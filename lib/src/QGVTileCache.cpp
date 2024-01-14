#include "QGVTileCache.h"
#include <QSqlQuery>

QGVTileCache::QGVTileCache(const QString& path)
{
    QString filename(path);
    if (path != ":memory:") {
        filename.append(".db");
    }
    mDB = QSqlDatabase::addDatabase("QSQLITE", path);
    mDB.setDatabaseName(filename);
    if (mDB.open()) {
        qgvDebug() << "opened database at" << filename;
        createTable();
    } else {
        qgvDebug() << "failed to open database at " << filename;
    }
}

QGVTileCache::~QGVTileCache()
{
    // There is no point in trying to close all connection,
    // because changing in name can happen 3 or more times
    // during QGVLayersTilesOnline child construction.
//    QString connectionName = mDB.connectionName();
//    mDB.close();
//    QSqlDatabase::removeDatabase(connectionName);
}

bool QGVTileCache::createTable() {
    QSqlQuery query(mDB);
    query.prepare("CREATE TABLE tiles("
                    "   zoom INTEGER NOT NULL,"
                    "   x INTEGER NOT NULL,"
                    "   y INTEGER NOT NULL,"
                    "   image BLOB NOT NULL,"
                    "   debug TEXT NOT NULL,"
                    "   PRIMARY KEY (zoom, x, y)"
                    ") STRICT;");
    bool success = query.exec();
    if (success) {
        qgvDebug() << "created table for cached tiles";
    } else {
        qgvDebug() << "failed to create table. already exists?";
    }
    return success;
}

bool QGVTileCache::isOpen() const {
    return mDB.isOpen();
}

bool QGVTileCache::isCached(const QGV::GeoTilePos& tilePos) const {
    QSqlQuery query(mDB);
    query.prepare("SELECT x FROM tiles WHERE zoom = (:zoom) AND x = (:x) AND y = (:y)");
    query.bindValue(":zoom", tilePos.zoom());
    const auto pos = tilePos.pos();
    query.bindValue(":x", pos.x());
    query.bindValue(":y", pos.y());
    return query.exec() && query.next();
}

bool QGVTileCache::cache(const QGV::GeoTilePos& tilePos, const QByteArray& rawImage, const QString &url) {
    QSqlQuery query(mDB);
    query.prepare("INSERT INTO tiles (zoom, x, y, image, debug) VALUES (:zoom, :x, :y, :image, :debug)");
    query.bindValue(":zoom", tilePos.zoom());
    const auto pos = tilePos.pos();
    query.bindValue(":x", pos.x());
    query.bindValue(":y", pos.y());
    query.bindValue(":image", rawImage);
    query.bindValue(":debug",
                    QString("%1\ncached tile(%2,%3,%4)")
                        .arg(url)
                        .arg(tilePos.zoom())
                        .arg(pos.x())
                        .arg(pos.y()));
    bool success = query.exec();
    if (success) {
        qgvDebug() << "cached" << tilePos;
    }
    return success;
}

QGVImage *QGVTileCache::getCached(const QGV::GeoTilePos& tilePos) const {
    QSqlQuery query(mDB);
    query.prepare("SELECT image, debug FROM tiles WHERE zoom = (:zoom) AND x = (:x) AND y = (:y)");
    query.bindValue(":zoom", tilePos.zoom());
    const auto pos = tilePos.pos();
    query.bindValue(":x", pos.x());
    query.bindValue(":y", pos.y());
    if (!query.exec() || !query.next()) {
        return nullptr;
    }
    auto tile = new QGVImage();
    tile->setGeometry(tilePos.toGeoRect());
    tile->loadImage(query.value("image").toByteArray());
    tile->setProperty("drawDebug", query.value("debug").toString());
    qgvDebug() << "accessed cached" << tilePos;
    return tile;
}
