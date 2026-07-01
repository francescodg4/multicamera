#include "themes/glass/Style.hpp"

#include "themes/glass/Theme.hpp"
#include "themes/glass/widgets/Glass.hpp"

#include <QFontDatabase>
#include <QPainter>
#include <QPainterPath>

namespace glass {

namespace {

    QColor alpha(QColor c, int a)
    {
        c.setAlpha(a);
        return c;
    }

    QPainterPath rounded(const QRectF& r, qreal radius)
    {
        QPainterPath path;
        const qreal rr = std::min(radius, std::min(r.width(), r.height()) / 2);
        path.addRoundedRect(r, rr, rr);
        return path;
    }

    /// Recessed glass well: dark diffusion fill, inner shadow at the top, light rim at the bottom.
    void recessed(QPainter& p, const QRectF& rect, qreal radius, const WidgetStyle::Look& look)
    {
        const QPainterPath shape = rounded(rect, radius);
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.fillPath(shape, QColor(0, 0, 0, look.enabled ? 72 : 40));
        p.setClipPath(shape);
        QLinearGradient inner(rect.topLeft(), QPointF(rect.left(), rect.top() + 8));
        inner.setColorAt(0, QColor(0, 0, 0, 70));
        inner.setColorAt(1, QColor(0, 0, 0, 0));
        p.fillRect(rect, inner);
        p.setClipping(false);
        QLinearGradient rim(rect.topLeft(), rect.bottomLeft());
        rim.setColorAt(0, QColor(255, 255, 255, look.hover ? 40 : 18));
        rim.setColorAt(1, QColor(255, 255, 255, look.hover ? 80 : 46));
        p.strokePath(shape, QPen(QBrush(rim), 1));
        if (look.focus) {
            p.strokePath(shape, QPen(Theme::accent, 1.5));
        }
        p.restore();
    }

    /// Accent fill running from the brand blue to cyan, with its radiance.
    void radiant(QPainter& p, const QRectF& rect, qreal radius, Qt::Orientation orientation, bool enabled)
    {
        if (rect.width() < 1 || rect.height() < 1) {
            return;
        }
        if (enabled) {
            Glass::paintGlow(p, rect, radius, Theme::glowAccent, 0.55);
        }
        QLinearGradient g(rect.topLeft(), orientation == Qt::Horizontal ? rect.topRight() : rect.bottomLeft());
        g.setColorAt(orientation == Qt::Horizontal ? 0 : 1, enabled ? Theme::accent : QColor(255, 255, 255, 50));
        g.setColorAt(orientation == Qt::Horizontal ? 1 : 0, enabled ? Theme::cyan : QColor(255, 255, 255, 70));
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.fillPath(rounded(rect, radius), g);
        p.restore();
    }

} // namespace

Style::Style()
    : WidgetStyle([] {
        Metrics m;
        m.control = 34;
        m.padding = 16;
        m.frame = 3;
        m.indicator = 20;
        m.arrow = 10;
        m.groove = 6;
        m.scroll = 8;
        m.title = 38;
        m.row = 28;
        m.margin = 16;
        m.spacing = 12;
        return m;
    }())
{
}

QFont Style::font() const
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    font.setPointSizeF(10);
    return font;
}

QPalette Style::standardPalette() const
{
    QPalette pal;
    pal.setColor(QPalette::Window, Theme::canvas);
    pal.setColor(QPalette::WindowText, Theme::text);
    pal.setColor(QPalette::Base, Qt::transparent); // views draw their own glass well
    pal.setColor(QPalette::AlternateBase, QColor(255, 255, 255, 9));
    pal.setColor(QPalette::Text, Theme::text);
    pal.setColor(QPalette::PlaceholderText, alpha(Theme::textDim, 150));
    pal.setColor(QPalette::Button, Theme::clay);
    pal.setColor(QPalette::ButtonText, Theme::text);
    pal.setColor(QPalette::BrightText, Theme::gold);
    pal.setColor(QPalette::Highlight, Theme::accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipBase, Theme::solidSurface);
    pal.setColor(QPalette::ToolTipText, Theme::text);
    pal.setColor(QPalette::Link, Theme::cyan);
    pal.setColor(QPalette::Light, QColor(255, 255, 255, 60));
    pal.setColor(QPalette::Midlight, QColor(255, 255, 255, 40));
    pal.setColor(QPalette::Mid, QColor(255, 255, 255, 30));
    pal.setColor(QPalette::Dark, QColor(0, 0, 0, 110));
    pal.setColor(QPalette::Shadow, QColor(0, 0, 0, 160));
    for (const QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
        pal.setColor(QPalette::Disabled, role, alpha(Theme::text, 95));
    }
    return pal;
}

void Style::window(QPainter& p, const QWidget* widget, const QRect& rect) const
{
    p.fillRect(rect, Theme::canvas);
    QWidget* host = widget ? widget->window() : nullptr;
    if (!host) {
        return;
    }
    // the ambient canvas every glass layer of this window samples
    Glass::Backdrop& backdrop = Glass::Backdrop::instance();
    if (backdrop.host() != host || backdrop.canvas().size() != host->size() / 2) {
        backdrop.setHost(host);
        backdrop.render();
    }
    p.save();
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawImage(QRectF(widget->mapFrom(host, QPoint(0, 0)), QSizeF(host->size())), backdrop.canvas());
    p.restore();
}

void Style::card(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    Glass::paintSurface(p, widget, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), Theme::radiusCard, Glass::Level::Card);
    if (title.isEmpty()) {
        return;
    }
    p.save();
    p.setFont(Theme::titleFont());
    p.setPen(Theme::textDim);
    p.drawText(titleRect.adjusted(20, 12, -20, 0), Qt::AlignLeft | Qt::AlignVCenter, title);
    p.restore();
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    const qreal radius = std::min<qreal>(Theme::radiusControl, r.height() / 2);
    if (kind == Button::Flat) {
        if (look.hover || look.pressed || look.checked) {
            p.save();
            p.setRenderHint(QPainter::Antialiasing);
            p.fillPath(rounded(r, radius), QColor(255, 255, 255, look.pressed ? 34 : 20));
            p.restore();
        }
        return;
    }
    const bool accent = kind == Button::Default || look.checked;
    if (look.enabled && (look.hover || look.focus) && !look.pressed) {
        Glass::paintGlow(p, r, radius, accent ? Theme::glowAccent : Theme::glowSoft, accent ? 0.95 : 0.55);
    } else if (look.enabled && accent) {
        Glass::paintGlow(p, r, radius, Theme::glowAccent, 0.5);
    }
    const QColor base = !look.enabled ? QColor(0x2a, 0x3c, 0x52) : accent ? Theme::accent : Theme::clay;
    Glass::paintClay(p, r, radius, base, look.pressed);
}

QColor Style::buttonText(Button, const Look& look, const QPalette&) const
{
    return look.enabled ? Theme::text : alpha(Theme::text, 95);
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    const qreal radius = std::min<qreal>(12, r.height() / 2);
    if (look.focus) {
        Glass::paintGlow(p, r, radius, Theme::glowAccent, 0.45);
    }
    recessed(p, r, radius, look);
}

void Style::view(QPainter& p, const QRect& rect, const Look& look) const
{
    Look calm = look;
    calm.hover = false;
    calm.focus = false;
    recessed(p, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), 10, calm);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    if (state == Qt::Unchecked) {
        recessed(p, r, 6, look);
        return;
    }
    if (look.enabled) {
        Glass::paintGlow(p, r, 6, Theme::glowAccent, look.hover ? 1.0 : 0.6);
    }
    Glass::paintClay(p, r, 6, look.enabled ? Theme::accent : QColor(0x2a, 0x3c, 0x52), look.pressed);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    if (state == Qt::Checked) {
        p.drawPolyline(QPolygonF({ at(0.26, 0.52), at(0.44, 0.70), at(0.75, 0.32) }));
    } else {
        p.drawLine(at(0.28, 0.5), at(0.72, 0.5));
    }
    p.restore();
}

void Style::arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    const QRectF r(rect);
    QPolygonF chevron;
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    switch (type) {
    case Qt::UpArrow: chevron = { at(0, 0.7), at(0.5, 0.3), at(1, 0.7) }; break;
    case Qt::DownArrow: chevron = { at(0, 0.3), at(0.5, 0.7), at(1, 0.3) }; break;
    case Qt::LeftArrow: chevron = { at(0.7, 0), at(0.3, 0.5), at(0.7, 1) }; break;
    case Qt::RightArrow: chevron = { at(0.3, 0), at(0.7, 0.5), at(0.3, 1) }; break;
    default: return;
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(!look.enabled ? alpha(Theme::text, 80) : look.hover ? Theme::cyan : Theme::text, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPolyline(chevron);
    p.restore();
}

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation, const Look& look) const
{
    const QRectF r(rect);
    const qreal radius = std::min(r.width(), r.height()) / 2;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(r, radius), QColor(0, 0, 0, 90));
    p.strokePath(rounded(r, radius), QPen(QColor(255, 255, 255, 22), 1));
    p.restore();
    radiant(p, QRectF(filled), radius, orientation, look.enabled);
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    if (look.enabled && (look.hover || look.pressed)) {
        Glass::paintGlow(p, r, r.height() / 2, Theme::glowSoft, 1.0);
    }
    Glass::paintClay(p, r, r.height() / 2, look.enabled ? QColor(0xe4, 0xf0, 0xff) : QColor(0x6a, 0x7a, 0x8e), look.pressed);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(look.enabled ? Theme::accent : QColor(0x4a, 0x5a, 0x6e));
    p.drawEllipse(r.center(), 3, 3);
    p.restore();
}

QSize Style::handleSize(Qt::Orientation) const
{
    return { 22, 22 };
}

void Style::scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation orientation, const Look& look) const
{
    const QRectF track = orientation == Qt::Horizontal ? QRectF(groove).adjusted(2, 1, -2, -1) : QRectF(groove).adjusted(1, 2, -1, -2);
    const QRectF r = orientation == Qt::Horizontal ? QRectF(handle).adjusted(2, 1, -2, -1) : QRectF(handle).adjusted(1, 2, -1, -2);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(track, std::min(track.width(), track.height()) / 2), QColor(255, 255, 255, 12));
    p.fillPath(rounded(r, std::min(r.width(), r.height()) / 2), QColor(255, 255, 255, look.pressed ? 150 : look.hover ? 110 : 60));
    p.restore();
}

void Style::display(QPainter& p, const QRect& rect) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    recessed(p, r, 14, Look {});
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QRadialGradient light(r.center(), r.width() * 0.6);
    light.setColorAt(0, alpha(Theme::cyan, 26));
    light.setColorAt(1, alpha(Theme::cyan, 0));
    p.fillPath(rounded(r, 14), light);
    p.restore();
}

QColor Style::displayText() const
{
    return Theme::cyan;
}

void Style::menuBar(QPainter& p, const QWidget* widget, const QRect& rect) const
{
    // a strip of liquid glass across the top of the window
    Glass::paintSurface(p, widget, QRectF(rect).adjusted(-1, -1, 1, 0), 0, Glass::Level::Bar);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), QColor(255, 255, 255, 26));
}

QColor Style::menuBarText(bool active) const
{
    return active ? Qt::white : Theme::text;
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, Theme::solidSurface);
    p.setPen(QColor(255, 255, 255, 38));
    p.drawRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    const QRectF r = inBar ? QRectF(rect).adjusted(0, 3, 0, -3) : QRectF(rect).adjusted(3, 1, -3, -1);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(r, 8), inBar ? QColor(255, 255, 255, 30) : alpha(Theme::accent, 190));
    p.restore();
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1.5, 1.5, -1.5, -1.5);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    if (look.checked) {
        p.fillPath(rounded(r, 8), alpha(Theme::accent, 120));
        p.strokePath(rounded(r, 8), QPen(alpha(Theme::cyan, 120), 1));
    } else {
        p.fillPath(rounded(r, 8), QColor(255, 255, 255, 16));
    }
    p.restore();
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    menu(p, rect);
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // radiance instead of an outline: a soft exterior glow round the card and a luminous rim
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath outside;
    outside.addRect(r.adjusted(-40, -40, 40, 40));
    outside = outside.subtracted(rounded(r, Theme::radiusCard));
    p.save();
    p.setClipPath(outside, Qt::IntersectClip);
    Glass::paintGlow(p, r, Theme::radiusCard, mark(), 0.9);
    p.restore();

    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient rim(r.topLeft(), r.bottomRight());
    rim.setColorAt(0, markColor().isValid() ? mark().lighter(130) : Theme::cyan);
    rim.setColorAt(1, markColor().isValid() ? mark() : Theme::accent);
    p.strokePath(rounded(r, Theme::radiusCard), QPen(QBrush(rim), 1.5));
    p.restore();
}

QColor Style::ownMark() const
{
    return Theme::glowAccent;
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    if (lit) {
        Glass::paintGlow(p, r, r.width() / 2, alpha(color, 200), 0.6);
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    QRadialGradient g(r.center() - QPointF(r.width() * 0.2, r.height() * 0.2), r.width() * 0.8);
    g.setColorAt(0, lit ? color.lighter(160) : QColor(255, 255, 255, 60));
    g.setColorAt(1, lit ? color : QColor(255, 255, 255, 20));
    p.setPen(QPen(QColor(255, 255, 255, lit ? 110 : 45), 1));
    p.setBrush(g);
    p.drawEllipse(r);
    p.restore();
}

} // namespace glass
