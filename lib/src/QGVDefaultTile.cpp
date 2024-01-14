#include "QGVDefaultTile.h"

#include <QBrush>
#include <QPainter>
#include <QPen>

QGVDefaultTile::QGVDefaultTile(const QGV::GeoTilePos& tilePos, const QColor color, const QString& text)
    : mTilePos(tilePos)
    , mColor(color)
    , mText(text)
{
}

void QGVDefaultTile::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    mProjRect = geoMap->getProjection()->geoToProj(mTilePos.toGeoRect());
}

QPainterPath QGVDefaultTile::projShape() const
{
    QPainterPath path;
    path.addRect(mProjRect);
    return path;
}

void QGVDefaultTile::projPaint(QPainter* painter)
{
    QPen pen = QPen(QBrush(Qt::black), 1);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(QBrush(mColor));
    painter->drawRect(mProjRect);
    drawText(painter);
}

void QGVDefaultTile::drawText(QPainter* painter)
{
    QPen pen = QPen(Qt::black);
    pen.setWidth(1);
    pen.setCosmetic(true);
    QBrush brush = QBrush(Qt::white);
    painter->setPen(pen);
    painter->setBrush(brush);
    auto path = QGV::createTextPath(mProjRect.toRect(), mText, QFont(), pen.width());
    path = QGV::createTransfromScale(mProjRect.center(), 0.75).map(path);
    painter->drawPath(path);
}
