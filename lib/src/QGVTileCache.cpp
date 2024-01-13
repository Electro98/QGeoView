#include "QGVTileCache.h"
#include <QSqlDatabase>
#include <QSqlQuery>

QGVTileCache::QGVTileCache(const QString& path): mPath(path)
{
    QSqlDatabase mDB = QSqlDatabase::addDatabase("QSQLITE");
    mDB.setDatabaseName(path);
    if (mDB.open()) {
        qDebug() << "opened database at" << path;
        mOpened = true;
        createTable();
    } else {
        qDebug() << "failed to open database at " << path;
    }
}

QGVTileCache::~QGVTileCache()
{
    QSqlDatabase::removeDatabase(mPath);
}

bool QGVTileCache::createTable() {
    QSqlQuery query;
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
        qDebug() << "created table for cached tiles";
    } else {
        qDebug() << "failed to create table. already exists?";
    }
    return success;
}

bool QGVTileCache::isOpen() const {
    return mOpened;
}

bool QGVTileCache::isCached(const QGV::GeoTilePos& tilePos) const {
    QSqlQuery query;
    query.prepare("SELECT x FROM tiles WHERE zoom = (:zoom) AND x = (:x) AND y = (:y)");
    query.bindValue(":zoom", tilePos.zoom());
    const auto pos = tilePos.pos();
    query.bindValue(":x", pos.x());
    query.bindValue(":y", pos.y());
    return query.exec() && query.next();
}

bool QGVTileCache::cache(const QGV::GeoTilePos& tilePos, const QByteArray& rawImage, const QString &url) {
    QSqlQuery query;
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
    QSqlQuery query;
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
