#include "themes/metro/Style.hpp"

#include <QPainter>
#include <QPainterPath>

namespace metro {

namespace {

    QColor alpha(QColor c, int a)
    {
        c.setAlpha(a * c.alpha() / 255);
        return c;
    }

    /// App-bar buttons are round: a control as wide as it is high.
    bool isRound(const QRect& r)
    {
        return std::abs(r.width() - r.height()) <= 2;
    }

    void checkMark(QPainter& p, const QRectF& r, const QColor& color, qreal width)
    {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(color, width, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
        p.drawPolyline(QPolygonF({ at(0.2, 0.52), at(0.42, 0.72), at(0.8, 0.3) }));
        p.restore();
    }

} // namespace

Style::Style(bool dark)
    : WidgetStyle([] {
        Metrics m;
        m.control = 32;
        m.padding = 14;
        m.frame = 2;
        m.indicator = 18;
        m.arrow = 10;
        m.groove = 4;
        m.scroll = 8;
        m.title = 36;
        m.row = 28;
        m.margin = 16;
        m.spacing = 10;
        return m;
    }())
    , c(Theme::palette(dark))
{
}

QFont Style::font() const
{
    return Theme::font(10);
}

QPalette Style::standardPalette() const
{
    QPalette pal;
    pal.setColor(QPalette::Window, c.canvas);
    pal.setColor(QPalette::WindowText, c.text);
    pal.setColor(QPalette::Base, c.surface);
    pal.setColor(QPalette::AlternateBase, c.dark ? c.surface.lighter(112) : c.canvas);
    pal.setColor(QPalette::Text, c.text);
    pal.setColor(QPalette::PlaceholderText, c.text2);
    pal.setColor(QPalette::Button, c.surface);
    pal.setColor(QPalette::ButtonText, c.text);
    pal.setColor(QPalette::BrightText, Theme::accent);
    pal.setColor(QPalette::Highlight, Theme::accent);
    pal.setColor(QPalette::HighlightedText, Theme::tileText);
    pal.setColor(QPalette::ToolTipBase, c.surface);
    pal.setColor(QPalette::ToolTipText, c.text);
    pal.setColor(QPalette::Link, Theme::accent);
    pal.setColor(QPalette::Light, c.surface);
    pal.setColor(QPalette::Midlight, c.line);
    pal.setColor(QPalette::Mid, c.track);
    pal.setColor(QPalette::Dark, c.border);
    pal.setColor(QPalette::Shadow, c.text);
    for (const QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
        pal.setColor(QPalette::Disabled, role, alpha(c.text2, 150));
    }
    return pal;
}

void Style::stroke(QPainter& p, const QRect& rect, const QColor& color, int width) const
{
    const QRect r = rect.adjusted(0, 0, -1, -1);
    p.fillRect(QRect(r.left(), r.top(), r.width() + 1, width), color);
    p.fillRect(QRect(r.left(), r.bottom() - width + 1, r.width() + 1, width), color);
    p.fillRect(QRect(r.left(), r.top() + width, width, r.height() + 1 - 2 * width), color);
    p.fillRect(QRect(r.right() - width + 1, r.top() + width, width, r.height() + 1 - 2 * width), color);
}

// ---- surfaces -----------------------------------------------------------------------------------

void Style::window(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, c.canvas); // solid neutral canvas, no pattern, no shadow
}

void Style::card(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    // content is chrome: a flat panel, and a light section title instead of a box header
    p.fillRect(rect, c.surface);
    if (title.isEmpty()) {
        return;
    }
    p.save();
    p.setFont(Theme::font(14, QFont::Light));
    p.setPen(c.text);
    p.drawText(titleRect.adjusted(14, 4, -14, 0), Qt::AlignLeft | Qt::AlignVCenter, title);
    p.restore();
}

void Style::tile(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title, int index) const
{
    // a live tile: flat accent colour, the name in white SemiBold (the tile header)
    p.fillRect(rect, Theme::tiles[std::size_t(std::max(0, index)) % std::size(Theme::tiles)]);
    if (title.isEmpty()) {
        return;
    }
    p.save();
    p.setFont(Theme::font(10.5, QFont::DemiBold));
    p.setPen(Theme::tileText);
    p.drawText(titleRect.adjusted(10, 2, -10, 0), Qt::AlignLeft | Qt::AlignVCenter, title);
    p.restore();
}

QColor Style::tileText() const
{
    return Theme::tileText; // everything on a live tile is white
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    const bool accent = kind == Button::Default || look.checked || look.pressed;
    QColor fill = Qt::transparent;
    QColor edge = look.enabled ? (look.hover ? c.text : c.border) : alpha(c.border, 110);
    if (!look.enabled) {
        fill = kind == Button::Default ? alpha(Theme::accent, 90) : Qt::transparent;
    } else if (accent) {
        fill = look.pressed ? Theme::accent.darker(115) : look.hover ? Theme::accent.lighter(112) : Theme::accent;
        edge = fill;
    } else if (look.hover) {
        fill = alpha(c.text, 26);
    }
    if (kind == Button::Flat && !look.hover && !look.pressed && !look.checked) {
        return; // a text command until it is touched
    }

    if (isRound(rect)) {
        // app-bar command: a circular 2px stroke round the glyph
        const QRectF r = QRectF(rect).adjusted(1.5, 1.5, -1.5, -1.5);
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(fill);
        p.setPen(QPen(edge, 2));
        p.drawEllipse(r);
        p.restore();
    } else {
        p.fillRect(rect, fill);
        stroke(p, rect, edge, 2);
    }
    if (look.focusVisible) {
        p.save();
        p.setPen(QPen(c.text, 1, Qt::DotLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(rect).adjusted(3.5, 3.5, -3.5, -3.5));
        p.restore();
    }
}

QColor Style::buttonText(Button kind, const Look& look, const QPalette&) const
{
    if (!look.enabled) {
        return alpha(c.text2, 150);
    }
    return kind == Button::Default || look.checked || look.pressed ? Theme::tileText : c.text;
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    p.fillRect(rect, look.enabled ? c.surface : alpha(c.surface, 140));
    const QColor edge = !look.enabled ? alpha(c.border, 90) : look.focus ? Theme::accent : look.hover ? c.text2 : c.border;
    stroke(p, rect, edge, 2);
}

void Style::view(QPainter& p, const QRect& rect, const Look&) const
{
    p.fillRect(rect, c.surface);
    stroke(p, rect, c.line, 1);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    const QRect r = rect.adjusted(1, 1, -1, -1);
    const QColor ink = !look.enabled ? alpha(c.text2, 130) : look.pressed ? Theme::accent : c.text;
    p.fillRect(r, look.hover ? alpha(c.text, 22) : Qt::transparent);
    stroke(p, r, ink, 2);
    if (state == Qt::Checked) {
        checkMark(p, QRectF(r).adjusted(2, 2, -2, -2), ink, 2);
    } else if (state == Qt::PartiallyChecked) {
        p.fillRect(r.adjusted(5, 5, -5, -5), ink);
    }
}

void Style::arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    const QRectF r(rect);
    QPolygonF chevron;
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    switch (type) {
    case Qt::UpArrow: chevron = { at(0, 0.72), at(0.5, 0.28), at(1, 0.72) }; break;
    case Qt::DownArrow: chevron = { at(0, 0.28), at(0.5, 0.72), at(1, 0.28) }; break;
    case Qt::LeftArrow: chevron = { at(0.72, 0), at(0.28, 0.5), at(0.72, 1) }; break;
    case Qt::RightArrow: chevron = { at(0.28, 0), at(0.72, 0.5), at(0.28, 1) }; break;
    default: return;
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(!look.enabled ? alpha(c.text2, 120) : look.hover ? Theme::accent : c.text, 1.6, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    p.drawPolyline(chevron);
    p.restore();
}

// ---- ranges -------------------------------------------------------------------------------------

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation, const Look& look) const
{
    p.fillRect(rect, c.track);
    p.fillRect(filled, look.enabled ? Theme::accent : alpha(Theme::accent, 110));
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation, const Look& look) const
{
    // the narrow thumb in the foreground colour
    p.fillRect(rect, !look.enabled ? c.border : look.pressed ? Theme::accent : c.text);
}

QSize Style::handleSize(Qt::Orientation orientation) const
{
    return orientation == Qt::Horizontal ? QSize(8, 22) : QSize(22, 8);
}

void Style::scrollBar(QPainter& p, const QRect&, const QRect& handle, Qt::Orientation, const Look& look) const
{
    p.fillRect(handle.adjusted(2, 2, -2, -2), look.pressed ? Theme::accent : look.hover ? c.text2 : alpha(c.text2, 130));
}

void Style::display(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, Theme::charms);
}

QColor Style::displayText() const
{
    return Theme::displayText;
}

// ---- menus & items ------------------------------------------------------------------------------

void Style::menuBar(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, c.canvas);
}

QColor Style::menuBarText(bool) const
{
    return c.text;
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, c.surface);
    stroke(p, rect, c.border, 1);
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    p.fillRect(inBar ? rect : rect.adjusted(1, 0, -1, 0), inBar ? alpha(c.text, 30) : Theme::accent);
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    p.fillRect(rect, look.checked ? Theme::accent : alpha(c.text, 22));
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, c.surface);
    stroke(p, rect, c.border, 1);
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // the selected tile: an inner border, and a check mark in a triangle at the top-right corner
    stroke(p, rect, mark(), 3);
    constexpr int Corner = 26;
    QPainterPath triangle;
    triangle.moveTo(rect.right() + 1 - Corner, rect.top());
    triangle.lineTo(rect.right() + 1, rect.top());
    triangle.lineTo(rect.right() + 1, rect.top() + Corner);
    triangle.closeSubpath();
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(triangle, mark());
    p.restore();
    checkMark(p, QRectF(rect.right() + 1 - Corner * 0.52, rect.top() + 2, Corner * 0.48, Corner * 0.48), mark().lightness() > 140 ? Theme::charms : Theme::tileText, 1.8);
}

QColor Style::ownMark() const
{
    return Theme::tileText;
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    // flat: a square of colour, no glow
    const QRect r = rect.adjusted(1, 1, -1, -1);
    p.fillRect(r, lit ? color : alpha(color, 80));
}

} // namespace metro
