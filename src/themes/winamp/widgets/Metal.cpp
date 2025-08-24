#include "Metal.hpp"

#include "themes/winamp/Theme.hpp"

#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QPainterPath>

#include <cmath>

namespace winamp {

namespace Metal {

namespace {

    QPainterPath rounded(const QRectF& r, qreal radius)
    {
        QPainterPath path;
        const qreal rr = std::min(radius, std::min(r.width(), r.height()) / 2);
        path.addRoundedRect(r, rr, rr);
        return path;
    }

    QColor alpha(QColor c, qreal a)
    {
        c.setAlphaF(float(a));
        return c;
    }

} // namespace

QFont uiFont(qreal pointSize, bool bold)
{
    QFont f(QStringList { QStringLiteral("Tahoma"), QStringLiteral("Segoe UI"), QStringLiteral("DejaVu Sans") });
    f.setPointSizeF(pointSize);
    f.setBold(bold);
    return f;
}

QFont digitalFont(int pixelSize, bool bold)
{
    QFont f(QStringList { QStringLiteral("Courier New"), QStringLiteral("Consolas"), QStringLiteral("DejaVu Sans Mono") });
    f.setPixelSize(pixelSize);
    f.setBold(bold);
    return f;
}

void bevel(QPainter& p, const QRectF& rect, qreal radius, bool raised)
{
    const QPainterPath shape = rounded(rect, radius);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipPath(shape);
    p.setBrush(Qt::NoBrush);
    p.strokePath(shape.translated(1, 1), QPen(raised ? Theme::bevelLight : Theme::bevelDark, 2));
    p.strokePath(shape.translated(-1, -1), QPen(raised ? Theme::bevelDark : Theme::bevelLight, 2));
    p.restore();
}

void frame(QPainter& p, const QRectF& rect, qreal radius)
{
    const QPainterPath shape = rounded(rect, radius);
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0, Theme::frameTop);
    g.setColorAt(1, Theme::frameBottom);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, g);
    p.restore();
    bevel(p, rect, radius, true);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.strokePath(shape, QPen(Theme::outline, 1));
    p.restore();
}

void insetPanel(QPainter& p, const QRectF& rect, const QColor& fill, qreal radius)
{
    const QPainterPath shape = rounded(rect, radius);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, fill);
    p.setClipPath(shape);
    // inset 2px 2px 4px rgba(0,0,0,0.6): shadow along the top and left inner edges
    QLinearGradient top(0, rect.top(), 0, rect.top() + 6);
    top.setColorAt(0, QColor(0, 0, 0, 153));
    top.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(QRectF(rect.left(), rect.top(), rect.width(), 6), top);
    QLinearGradient left(rect.left(), 0, rect.left() + 6, 0);
    left.setColorAt(0, QColor(0, 0, 0, 153));
    left.setColorAt(1, QColor(0, 0, 0, 0));
    p.fillRect(QRectF(rect.left(), rect.top(), 6, rect.height()), left);
    p.restore();
    bevel(p, rect, radius, false);
}

void lcd(QPainter& p, const QRectF& rect, qreal radius)
{
    insetPanel(p, rect, Theme::lcdBg, radius);
    p.save();
    p.setClipPath(rounded(rect, radius));
    // subtle illumination from the top and scanlines across the glass
    QLinearGradient light(rect.topLeft(), rect.bottomLeft());
    light.setColorAt(0, alpha(Theme::lcdGlow, 0.10));
    light.setColorAt(0.5, alpha(Theme::lcdGlow, 0.0));
    p.fillRect(rect, light);
    for (qreal y = rect.top() + 1; y < rect.bottom(); y += 3) {
        p.fillRect(QRectF(rect.left(), y, rect.width(), 1), QColor(0, 0, 0, 55));
    }
    p.restore();
}

void titleBar(QPainter& p, const QRectF& rect)
{
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0, Theme::titleTop);
    g.setColorAt(1, Theme::titleBottom);
    const QPainterPath shape = rounded(rect, Theme::radiusPanel);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, g);
    p.setClipPath(shape);
    // horizontal reflection ridges
    for (qreal y = rect.top() + 2; y < rect.bottom(); y += 3) {
        p.fillRect(QRectF(rect.left(), y, rect.width(), 1), QColor(255, 255, 255, y < rect.center().y() ? 26 : 12));
    }
    p.fillRect(QRectF(rect.left(), rect.top(), rect.width(), 1), QColor(255, 255, 255, 90));
    p.restore();
}

void ridge(QPainter& p, const QRectF& rect)
{
    const QPainterPath shape = rounded(rect, rect.height() / 2);
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0, QColor(0x0a, 0x1c, 0x3a));
    g.setColorAt(1, QColor(0x3a, 0x6a, 0xb0));
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, g);
    p.strokePath(shape, QPen(QColor(0x9a, 0xb8, 0xe0, 160), 1));
    p.restore();
}

void glyph(QPainter& p, const QRectF& r, Glyph glyph, const QColor& color)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    const qreal w = r.width(), h = r.height(), x = r.left(), y = r.top();
    const auto tri = [&](qreal x0, qreal x1) { p.drawPolygon(QPolygonF({ QPointF(x0, y), QPointF(x1, y + h / 2), QPointF(x0, y + h) })); };
    const auto triL = [&](qreal x0, qreal x1) { p.drawPolygon(QPolygonF({ QPointF(x1, y), QPointF(x0, y + h / 2), QPointF(x1, y + h) })); };
    switch (glyph) {
    case Glyph::Play: tri(x + w * 0.15, x + w * 0.95); break;
    case Glyph::Forward:
        tri(x, x + w * 0.5);
        tri(x + w * 0.5, x + w);
        break;
    case Glyph::Rewind:
        triL(x, x + w * 0.5);
        triL(x + w * 0.5, x + w);
        break;
    case Glyph::Back: triL(x + w * 0.1, x + w * 0.85); break;
    case Glyph::Pause:
        p.drawRect(QRectF(x + w * 0.15, y, w * 0.25, h));
        p.drawRect(QRectF(x + w * 0.6, y, w * 0.25, h));
        break;
    case Glyph::Stop: p.drawRect(QRectF(x + w * 0.1, y + h * 0.1, w * 0.8, h * 0.8)); break;
    case Glyph::Eject:
        p.drawPolygon(QPolygonF({ QPointF(x, y + h * 0.62), QPointF(x + w / 2, y), QPointF(x + w, y + h * 0.62) }));
        p.drawRect(QRectF(x, y + h * 0.78, w, h * 0.22));
        break;
    case Glyph::Minimize: p.drawRect(QRectF(x + w * 0.15, y + h * 0.7, w * 0.7, h * 0.2)); break;
    case Glyph::Maximize:
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(color, 2));
        p.drawRect(QRectF(x + w * 0.18, y + h * 0.18, w * 0.64, h * 0.64));
        break;
    case Glyph::Close:
        p.setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(x + w * 0.2, y + h * 0.2), QPointF(x + w * 0.8, y + h * 0.8));
        p.drawLine(QPointF(x + w * 0.8, y + h * 0.2), QPointF(x + w * 0.2, y + h * 0.8));
        break;
    case Glyph::None: break;
    }
    p.restore();
}

void capsule(QPainter& p, const QRectF& rect, bool pressed, bool active, bool hover)
{
    const qreal radius = std::min(Theme::radiusButton, rect.height() / 2);
    const QPainterPath shape = rounded(rect, radius);
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    if (active) {
        g.setColorAt(0, pressed ? QColor(0x2d, 0x63, 0xc8) : QColor(0x7a, 0xb2, 0xff));
        g.setColorAt(1, pressed ? QColor(0x7a, 0xb2, 0xff) : QColor(0x2d, 0x63, 0xc8));
    } else {
        const QColor top = hover ? QColor(0xff, 0xff, 0xff) : QColor(0xf4, 0xf6, 0xfa);
        const QColor bottom = hover ? QColor(0xd0, 0xd6, 0xe0) : QColor(0xc3, 0xca, 0xd6);
        g.setColorAt(0, pressed ? bottom : top);
        g.setColorAt(1, pressed ? top : bottom);
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, g);
    p.restore();
    bevel(p, rect, radius, !pressed);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.strokePath(shape, QPen(Theme::outline, 1));
    p.restore();
}

void roundButton(QPainter& p, const QRectF& rect, bool pressed, bool active, bool hover)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    // outer bevel ring
    QLinearGradient ring(rect.topLeft(), rect.bottomRight());
    ring.setColorAt(0, QColor(0xf2, 0xf4, 0xf8));
    ring.setColorAt(1, QColor(0x7a, 0x84, 0x96));
    p.setPen(QPen(Theme::outline, 1));
    p.setBrush(ring);
    p.drawEllipse(rect);
    // face
    const QRectF face = rect.adjusted(rect.width() * 0.09, rect.height() * 0.09, -rect.width() * 0.09, -rect.height() * 0.09);
    QRadialGradient g(face.center() - QPointF(0, face.height() * (pressed ? -0.15 : 0.25)), face.width() * 0.75);
    const QColor light = active ? QColor(0xdc, 0xea, 0xff) : hover ? QColor(0xff, 0xff, 0xff) : QColor(0xfa, 0xfb, 0xfd);
    const QColor dark = active ? QColor(0x8c, 0xb0, 0xe8) : QColor(0xb0, 0xb8, 0xc6);
    g.setColorAt(0, pressed ? dark : light);
    g.setColorAt(1, pressed ? light : dark);
    p.setPen(QPen(pressed ? Theme::bevelDark : QColor(0xff, 0xff, 0xff, 200), 1));
    p.setBrush(g);
    p.drawEllipse(face);
    p.restore();
}

void sevenSegment(QPainter& p, const QRectF& rect, const QString& text, const QColor& on, const QColor& off)
{
    // segments a..g of each digit
    static const int Segments[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };
    const qreal h = rect.height();
    const qreal w = h * 0.52, t = h * 0.12, gap = h * 0.18;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    qreal x = rect.left();
    const auto segment = [&](qreal x0, qreal y0, qreal len, bool horizontal, bool lit) {
        QPolygonF poly;
        if (horizontal) {
            poly << QPointF(x0, y0) << QPointF(x0 + t / 2, y0 - t / 2) << QPointF(x0 + len - t / 2, y0 - t / 2)
                 << QPointF(x0 + len, y0) << QPointF(x0 + len - t / 2, y0 + t / 2) << QPointF(x0 + t / 2, y0 + t / 2);
        } else {
            poly << QPointF(x0, y0) << QPointF(x0 + t / 2, y0 + t / 2) << QPointF(x0 + t / 2, y0 + len - t / 2)
                 << QPointF(x0, y0 + len) << QPointF(x0 - t / 2, y0 + len - t / 2) << QPointF(x0 - t / 2, y0 + t / 2);
        }
        if (lit) {
            p.setBrush(QColor(on.red(), on.green(), on.blue(), 70)); // glow
            p.drawPolygon(poly.translated(0, 0.8));
        }
        p.setBrush(lit ? on : off);
        p.drawPolygon(poly);
    };
    for (const QChar ch : text) {
        if (ch == u':') {
            p.setBrush(on);
            p.drawEllipse(QPointF(x + t, rect.top() + h * 0.32), t * 0.55, t * 0.55);
            p.drawEllipse(QPointF(x + t, rect.top() + h * 0.70), t * 0.55, t * 0.55);
            x += t * 2 + gap * 0.5;
            continue;
        }
        const int mask = ch.isDigit() ? Segments[ch.digitValue()] : ch == u'-' ? 0x40 : 0;
        const qreal top = rect.top() + t / 2, mid = rect.top() + h / 2, bottom = rect.bottom() - t / 2;
        const qreal left = x + t / 2, right = x + w - t / 2;
        segment(left, top, w - t, true, mask & 0x01); // a
        segment(right, top, mid - top, false, mask & 0x02); // b
        segment(right, mid, bottom - mid, false, mask & 0x04); // c
        segment(left, bottom, w - t, true, mask & 0x08); // d
        segment(left, mid, bottom - mid, false, mask & 0x10); // e
        segment(left, top, mid - top, false, mask & 0x20); // f
        segment(left, mid, w - t, true, mask & 0x40); // g
        x += w + gap;
    }
    p.restore();
}

void glowText(QPainter& p, const QRectF& rect, int flags, const QString& text, const QColor& color)
{
    p.save();
    p.setPen(alpha(color, 0.25));
    for (const QPointF d : { QPointF(-1, 0), QPointF(1, 0), QPointF(0, -1), QPointF(0, 1) }) {
        p.drawText(rect.translated(d), flags, text);
    }
    p.setPen(color);
    p.drawText(rect, flags, text);
    p.restore();
}

void dotMatrix(QPainter& p, const QRectF& rect, const QString& text, const QColor& on, const QColor& off)
{
    constexpr int Rows = 11;
    const qreal dot = rect.height() / Rows;
    QFont font(QStringLiteral("DejaVu Sans Mono"));
    font.setPixelSize(10);
    font.setBold(true);
    font.setStyleStrategy(QFont::NoAntialias);
    const int columns = int(rect.width() / dot);

    // text rasterised at one pixel per dot; too long a line is cut with an ellipsis, never scrolled
    QImage bitmap(columns, Rows, QImage::Format_Grayscale8);
    bitmap.fill(0);
    {
        QPainter b(&bitmap);
        b.setFont(font);
        b.setPen(Qt::white);
        const QString shown = QFontMetrics(font).elidedText(text, Qt::ElideRight, columns);
        b.drawText(QRect(0, 0, columns, Rows), Qt::AlignVCenter | Qt::AlignLeft, shown);
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    for (int row = 0; row < Rows; ++row) {
        const uchar* bits = bitmap.constScanLine(row);
        for (int col = 0; col < columns; ++col) {
            const bool lit = bits[col] > 127;
            p.setBrush(lit ? on.lighter(125) : off.darker(135));
            p.drawEllipse(QPointF(rect.left() + (col + 0.5) * dot, rect.top() + (row + 0.5) * dot), dot * (lit ? 0.46 : 0.34), dot * (lit ? 0.46 : 0.34));
        }
    }
    p.restore();
}

void led(QPainter& p, const QPointF& centre, qreal radius, const QColor& color, bool lit)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    if (lit) {
        QRadialGradient halo(centre, radius * 2.6);
        halo.setColorAt(0, alpha(color, 0.55));
        halo.setColorAt(1, alpha(color, 0.0));
        p.setBrush(halo);
        p.drawEllipse(centre, radius * 2.6, radius * 2.6);
    }
    QRadialGradient g(centre - QPointF(radius * 0.3, radius * 0.3), radius * 1.2);
    g.setColorAt(0, lit ? color.lighter(170) : color.darker(170));
    g.setColorAt(1, lit ? color : color.darker(320));
    p.setBrush(g);
    p.setPen(QPen(Theme::bevelDark, 0.8));
    p.drawEllipse(centre, radius, radius);
    p.restore();
}

void sliderTrack(QPainter& p, const QRectF& track, qreal fill)
{
    insetPanel(p, track, QColor(0x2a, 0x34, 0x46), track.height() / 2);
    const QRectF filled(track.left() + 1, track.top() + 1, (track.width() - 2) * std::clamp(fill, 0.0, 1.0), track.height() - 2);
    QLinearGradient g(filled.topLeft(), filled.bottomLeft());
    g.setColorAt(0, QColor(0x7a, 0xb2, 0xff));
    g.setColorAt(1, QColor(0x2d, 0x63, 0xc8));
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(filled, filled.height() / 2), g);
    p.restore();
}

void sliderThumb(QPainter& p, const QRectF& thumb, bool pressed)
{
    capsule(p, thumb, pressed, false, false);
    p.save();
    p.setPen(QPen(Theme::textDim, 1));
    for (int i = -1; i <= 1; ++i) { // grip lines
        const qreal x = thumb.center().x() + i * 3;
        p.drawLine(QPointF(x, thumb.top() + 4), QPointF(x, thumb.bottom() - 4));
    }
    p.restore();
}

} // namespace Metal

} // namespace winamp
