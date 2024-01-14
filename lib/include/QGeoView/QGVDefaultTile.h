#pragma once

#include "QGVDrawItem.h"

#include <QBrush>
#include <QPen>

class QGVDefaultTile : public QGVDrawItem
{
    Q_OBJECT

public:
    explicit QGVDefaultTile(const QGV::GeoTilePos& tilePos, const QColor color = Qt::darkGray, const QString& text = QString("No Data"));

private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    void drawText(QPainter* painter);

private:
    QGV::GeoTilePos mTilePos;
    QRectF mProjRect;
    QColor mColor;
    QString mText;
};

