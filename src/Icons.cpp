#include "Icons.hpp"

#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>

namespace Icons {

namespace {

    QPainterPath bar(qreal x, qreal y, qreal w, qreal h, qreal radius)
    {
        QPainterPath p;
        p.addRoundedRect(QRectF(x, y, w, h), radius, radius);
        return p;
    }

    /// Triangle outline with round corners.
    void triangle(QPainter& p, const QPolygonF& corners, const QColor& color)
    {
        p.setPen(QPen(color, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        p.drawPolygon(corners);
    }

    /// Draws @p icon on a 24 × 24 grid.
    void draw(QPainter& p, Icon icon, const QColor& color)
    {
        switch (icon) {
        case Icon::Play:
            triangle(p, { { 8, 5 }, { 8, 19 }, { 19, 12 } }, color);
            break;
        case Icon::Pause:
            p.fillPath(bar(6.5, 5, 4, 14, 1.2), color);
            p.fillPath(bar(13.5, 5, 4, 14, 1.2), color);
            break;
        case Icon::Stop:
            p.fillPath(bar(7, 7, 10, 10, 1.5), color);
            break;
        case Icon::FramePrevious:
            triangle(p, { { 18, 6 }, { 18, 18 }, { 9, 12 } }, color);
            p.fillPath(bar(5, 6, 2.6, 12, 1), color);
            break;
        case Icon::FrameNext:
            triangle(p, { { 6, 6 }, { 6, 18 }, { 15, 12 } }, color);
            p.fillPath(bar(16.4, 6, 2.6, 12, 1), color);
            break;
        }
    }

    QPixmap pixmap(Icon icon, int side, const QColor& color)
    {
        const qreal dpr = qGuiApp ? qGuiApp->devicePixelRatio() : 1.0;
        QPixmap pm(QSize(side, side) * dpr);
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.scale(side / 24.0, side / 24.0);
        draw(p, icon, color);
        return pm;
    }

} // namespace

QIcon icon(Icon icon, const QColor& color)
{
    QColor disabled = color;
    disabled.setAlphaF(0.3f);
    QIcon result;
    result.addPixmap(pixmap(icon, 48, color));
    result.addPixmap(pixmap(icon, 48, disabled), QIcon::Disabled);
    return result;
}

} // namespace Icons
