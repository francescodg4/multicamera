#include "themes/emerald/Style.hpp"

#include "themes/emerald/Theme.hpp"
#include "themes/emerald/widgets/Pixel.hpp"

#include <QPainter>

#include <cmath>

namespace emerald {

namespace {

    using Pixel::Scale;

    /// @p r (device pixels) in the coordinates of a canvas that starts at @p origin.
    QRect low(const QRect& r, const QPoint& origin)
    {
        return QRect((r.left() - origin.x()) / Scale, (r.top() - origin.y()) / Scale, r.width() / Scale, r.height() / Scale);
    }

    /// Filled pixel disc with a one-pixel border.
    void disc(QPainter& p, const QRect& r, const QColor& fill, const QColor& border)
    {
        const qreal radius = std::min(r.width(), r.height()) / 2.0;
        const QPointF c = QRectF(r).center();
        for (int y = 0; y < r.height(); ++y) {
            const qreal dy = y + 0.5 - radius;
            const qreal outer = std::sqrt(std::max(0.0, radius * radius - dy * dy));
            const qreal inner = std::sqrt(std::max(0.0, (radius - 1) * (radius - 1) - dy * dy));
            const int x0 = int(std::lround(c.x() - outer)), x1 = int(std::lround(c.x() + outer));
            if (x1 <= x0) {
                continue;
            }
            p.fillRect(x0, r.top() + y, x1 - x0, 1, border);
            const int i0 = int(std::lround(c.x() - inner)), i1 = int(std::lround(c.x() + inner));
            if (inner > 0 && i1 > i0 && std::abs(dy) < radius - 1) {
                p.fillRect(i0, r.top() + y, i1 - i0, 1, fill);
            }
        }
    }

    /// Raised block with a hard pixel shadow (the handle of sliders), sunk into it when pressed.
    void block(QPainter& p, const QRect& r, const QColor& face, bool pressed)
    {
        const QRect body = r.adjusted(0, 0, -1, -1);
        if (!pressed) {
            Pixel::chamfer(p, body.translated(1, 1), Theme::borderDark, Theme::borderDark, 1);
        }
        const QRect top = pressed ? body.translated(1, 1) : body;
        Pixel::chamfer(p, top, face, Theme::borderDark, 1);
        p.fillRect(top.left() + 1, top.top() + 1, top.width() - 2, 1, face.lighter(130));
    }

    /// HP-bar colour: green, then yellow below half, red below a fifth.
    QColor hp(qreal fraction, bool shade)
    {
        if (fraction > 0.5) {
            return shade ? QColor(0x58, 0xd0, 0x80) : QColor(0x70, 0xf8, 0xa8);
        }
        if (fraction > 0.2) {
            return shade ? QColor(0xc8, 0xa8, 0x08) : QColor(0xf8, 0xe0, 0x38);
        }
        return shade ? QColor(0xa8, 0x40, 0x48) : QColor(0xf8, 0x58, 0x38);
    }

    const QColor headerBlue { 0x28, 0x78, 0xd8 }; ///< the "MOVE / OK / BACK" strip

} // namespace

Style::Style()
    : WidgetStyle([] {
        Metrics m; // every size is even: one art pixel is Scale device pixels
        m.control = 32;
        m.padding = 16;
        m.frame = 4;
        m.indicator = 20;
        m.arrow = 12;
        m.groove = 8;
        m.scroll = 16;
        m.tab = 32;
        m.title = 40;
        m.row = 28;
        m.margin = 16;
        m.spacing = 12;
        m.pressShift = Scale;
        return m;
    }())
{
}

QFont Style::font() const
{
    QFont f(QStringLiteral("DejaVu Sans Mono"));
    f.setPixelSize(13);
    f.setBold(true);
    f.setStyleStrategy(QFont::NoAntialias); // crisp, like the indexed-palette text
    return f;
}

QPalette Style::standardPalette() const
{
    QPalette pal;
    pal.setColor(QPalette::Window, Theme::primaryGreen);
    pal.setColor(QPalette::WindowText, Theme::text);
    pal.setColor(QPalette::Base, Qt::transparent); // views draw their own paper
    pal.setColor(QPalette::AlternateBase, Theme::paperLine);
    pal.setColor(QPalette::Text, Theme::text);
    pal.setColor(QPalette::PlaceholderText, Theme::dimText);
    pal.setColor(QPalette::Button, Theme::cream);
    pal.setColor(QPalette::ButtonText, Theme::text);
    pal.setColor(QPalette::BrightText, Theme::badge);
    pal.setColor(QPalette::Highlight, Theme::highlight);
    pal.setColor(QPalette::HighlightedText, Theme::text);
    pal.setColor(QPalette::ToolTipBase, Theme::cream);
    pal.setColor(QPalette::ToolTipText, Theme::text);
    pal.setColor(QPalette::Link, Theme::darkGreen);
    pal.setColor(QPalette::Light, Theme::white);
    pal.setColor(QPalette::Midlight, Theme::metalLight);
    pal.setColor(QPalette::Mid, Theme::metal);
    pal.setColor(QPalette::Dark, Theme::metalDark);
    pal.setColor(QPalette::Shadow, Theme::borderDark);
    for (const QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
        pal.setColor(QPalette::Disabled, role, Theme::dimText);
    }
    return pal;
}

void Style::window(QPainter& p, const QWidget*, const QRect& rect) const
{
    if (m_wallpaper.size() != rect.size()) {
        m_wallpaper = QImage(rect.size(), QImage::Format_ARGB32_Premultiplied);
        m_wallpaper.fill(Theme::primaryGreen);
        QPainter wp(&m_wallpaper);
        Pixel::Canvas c(wp, m_wallpaper.rect());
        Pixel::wallpaper(c.p(), c.rect(), Pixel::Wallpaper::Forest);
    }
    p.drawImage(rect.topLeft(), m_wallpaper);
}

void Style::card(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    // the box body sits just under the banner, which is placed above it like a box header
    const int top = title.isEmpty() ? 0 : titleRect.height() / Scale / 2 - 2;
    const QRect body = r.adjusted(0, top, 0, 0);
    Pixel::chamfer(q, body, Theme::cream, Theme::borderDark, 3);
    Pixel::chamfer(q, body.adjusted(2, 2, -2, -2), Qt::transparent, Theme::kbdNumGreen, 2);
    if (!title.isEmpty()) {
        Pixel::banner(q, QRect(6, 1, r.width() - 12, 16), Pixel::Wallpaper::Sky, title);
    }
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    if (kind == Button::Flat) {
        if (look.hover || look.pressed || look.checked) {
            Pixel::highlightTile(c.p(), c.rect());
        }
        return;
    }
    QColor face = kind == Button::Default ? Theme::keyYellow : look.checked ? Theme::kbdUpperBlue : Theme::white;
    if (look.hover && !look.pressed) {
        face = face.lighter(106);
    }
    Pixel::key(c.p(), c.rect(), face, QString(), look.pressed || (look.checked && kind != Button::Default), look.enabled);
    if (look.focus) {
        // focused key: the bi-colour arrow pointing at it from the left
        Pixel::arrow(c.p(), QRect(2, c.rect().height() / 2 - 3, 4, 6), Pixel::Direction::Right, Theme::badge, Theme::badge.darker(140));
    }
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    Pixel::textBox(q, r);
    if (!look.enabled) {
        q.fillRect(r.adjusted(2, 2, -2, -2), QColor(0xd0, 0xd0, 0xc8, 120));
    } else if (look.focus) {
        Pixel::chamfer(q, r, Qt::transparent, Theme::darkGreen, 2);
        Pixel::chamfer(q, r.adjusted(1, 1, -1, -1), Qt::transparent, Theme::primaryGreen, 2);
    } else if (look.hover) {
        Pixel::chamfer(q, r.adjusted(1, 1, -1, -1), Qt::transparent, Theme::kbdUpperBlue, 2);
    }
}

void Style::view(QPainter& p, const QRect& rect, const Look&) const
{
    Pixel::Canvas c(p, rect);
    Pixel::chamfer(c.p(), c.rect(), Theme::cream, Theme::borderDark, 2);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    const QColor face = !look.enabled ? Theme::keyGray : look.hover ? Theme::highlight : Theme::white;
    Pixel::chamfer(q, r, face, Theme::borderDark, 1);
    q.fillRect(r.left() + 1, r.top() + 1, r.width() - 2, 1, Theme::metalLight);
    const QColor ink = look.enabled ? Theme::primaryGreen : Theme::metalDark;
    if (state == Qt::Checked) {
        // two-pixel stroke with its dark shadow, on the 10x10 art grid
        static const QPoint mark[] = { { 2, 4 }, { 3, 5 }, { 4, 6 }, { 5, 5 }, { 6, 4 }, { 7, 3 } };
        for (const QPoint& pt : mark) {
            q.fillRect(pt.x() + 1, pt.y() + 1, 1, 2, Theme::darkGreen);
        }
        for (const QPoint& pt : mark) {
            q.fillRect(pt.x(), pt.y(), 1, 2, ink);
        }
    } else if (state == Qt::PartiallyChecked) {
        q.fillRect(3, 5, 5, 1, Theme::darkGreen);
        q.fillRect(2, 4, 5, 1, ink);
    }
}

void Style::radio(QPainter& p, const QRect& rect, bool on, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    disc(q, r, !look.enabled ? Theme::keyGray : look.hover ? Theme::highlight : Theme::white, Theme::borderDark);
    if (on) {
        disc(q, r.adjusted(3, 3, -3, -3), look.enabled ? Theme::primaryGreen : Theme::metalDark, Theme::darkGreen);
    }
}

void Style::arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    Pixel::Direction direction = Pixel::Direction::Down;
    switch (type) {
    case Qt::UpArrow: direction = Pixel::Direction::Up; break;
    case Qt::LeftArrow: direction = Pixel::Direction::Left; break;
    case Qt::RightArrow: direction = Pixel::Direction::Right; break;
    default: break;
    }
    Pixel::Canvas c(p, rect);
    if (look.enabled) {
        Pixel::arrow(c.p(), c.rect(), direction, look.hover ? Theme::highlight : Theme::white, Theme::kbdUpperBlue);
    } else {
        Pixel::arrow(c.p(), c.rect(), direction, Theme::keyGray, Theme::metal);
    }
}

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    Pixel::chamfer(q, r, Theme::darkGreen, Theme::borderDark, 1);
    const QRect f = low(filled, rect.topLeft()).intersected(r.adjusted(1, 1, -1, -1));
    if (!f.isEmpty()) {
        const QColor fill = look.enabled ? Theme::kbdNumGreen : Theme::metal;
        q.fillRect(f, fill);
        q.fillRect(f.left(), f.top(), f.width(), 1, fill.lighter(125));
    }
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation orientation, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    const QColor face = !look.enabled ? Theme::keyGray : look.hover || look.pressed ? Theme::highlight : Theme::kbdUpperBlue;
    block(q, r, face, look.pressed);
    // grip line across the handle
    const QRect top = look.pressed ? r.translated(1, 1) : r;
    if (orientation == Qt::Horizontal) {
        q.fillRect(top.left() + (top.width() - 1) / 2, top.top() + 3, 1, top.height() - 7, face.darker(160));
    } else {
        q.fillRect(top.left() + 3, top.top() + (top.height() - 1) / 2, top.width() - 7, 1, face.darker(160));
    }
}

QSize Style::handleSize(Qt::Orientation orientation) const
{
    return orientation == Qt::Horizontal ? QSize(16, 24) : QSize(24, 16);
}

void Style::scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation, const Look& look) const
{
    Pixel::Canvas c(p, groove);
    QPainter& q = c.p();
    Pixel::chamfer(q, c.rect(), Theme::cream, Theme::borderDark, 0);
    const QRect h = low(handle, groove.topLeft());
    const QColor face = look.pressed ? Theme::darkGreen : look.hover ? Theme::primaryGreen.lighter(115) : Theme::primaryGreen;
    Pixel::chamfer(q, h, face, Theme::darkGreen, 1);
    q.fillRect(h.left() + 1, h.top() + 1, h.width() - 2, 1, face.lighter(130));
}

void Style::progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    // HP bar: dark frame, cream when empty, green / yellow / red with the fill level
    Pixel::chamfer(q, r, Theme::metalDark, Theme::borderDark, 1);
    const QRect inner = r.adjusted(2, 2, -2, -2);
    q.fillRect(inner, Theme::cream);
    const QRect f = low(filled, rect.topLeft()).intersected(inner);
    if (f.isEmpty()) {
        return;
    }
    const qreal fraction = orientation == Qt::Horizontal ? qreal(filled.width()) / std::max(1, rect.width()) : qreal(filled.height()) / std::max(1, rect.height());
    q.fillRect(f, hp(fraction, false));
    if (orientation == Qt::Horizontal) {
        q.fillRect(f.left(), f.bottom(), f.width(), 1, hp(fraction, true));
        q.fillRect(f.left(), f.top(), f.width(), 1, hp(fraction, false).lighter(120));
    } else {
        q.fillRect(f.right(), f.top(), 1, f.height(), hp(fraction, true));
    }
}

void Style::tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    // a tile open at the bottom, where it meets the deck
    const QRect tile = r.adjusted(1, selected ? 0 : 2, -1, 4);
    const QColor face = selected ? Theme::highlight : look.hover ? Theme::white : Theme::metalLight;
    Pixel::chamfer(q, tile, face, Theme::borderDark, 2);
    q.fillRect(tile.left() + 2, tile.top() + 1, tile.width() - 4, 1, face.lighter(115));
}

void Style::tabPane(QPainter& p, const QWidget*, const QRect& rect) const
{
    Pixel::Canvas c(p, rect);
    Pixel::deck(c.p(), c.rect());
}

void Style::dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect().adjusted(1, 1, -1, -1);
    disc(q, r, look.hover ? Theme::kbdUpperBlue : Theme::metal, Theme::borderDark);
    const QRect face = r.adjusted(4, 4, -4, -4);
    disc(q, face, look.enabled ? Theme::metalLight : Theme::keyGray, Theme::metalDark);

    // notch lamps round the rim, lit up to the value
    const QRectF rf(r);
    constexpr int Notches = 11;
    for (int i = 0; i < Notches; ++i) {
        const qreal v = qreal(i) / (Notches - 1);
        const QPointF pt = dialPoint(rf, v, rf.width() / 2 - 2.5);
        const bool lit = look.enabled && v <= value + 1e-6;
        q.fillRect(QRectF(pt.x() - 1, pt.y() - 1, 2, 2).toRect(), lit ? QColor(0x70, 0xf8, 0xa8) : Theme::darkGreen);
    }
    // pointer: an aliased line is already a pixel line
    const QRectF ff(face);
    q.setPen(look.enabled ? Theme::borderDark : Theme::metalDark);
    q.drawLine(ff.center().toPoint(), dialPoint(ff, value, ff.width() / 2 - 2).toPoint());
    q.fillRect(QRect(ff.center().toPoint() - QPoint(1, 1), QSize(2, 2)), Theme::borderDark);
}

void Style::display(QPainter& p, const QRect& rect) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect screen = c.rect().adjusted(2, 2, -2, -2);
    Pixel::crtFrame(q, screen);
    Pixel::scanlines(q, screen);
}

QColor Style::displayText() const
{
    return Theme::white;
}

void Style::menuBar(QPainter& p, const QWidget*, const QRect& rect) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    q.fillRect(r, headerBlue);
    q.fillRect(r.left(), r.top(), r.width(), 1, headerBlue.lighter(135));
    q.fillRect(r.left(), r.bottom(), r.width(), 1, Theme::borderDark);
}

QColor Style::menuBarText(bool active) const
{
    return active ? Theme::text : Theme::white;
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    Pixel::Canvas c(p, rect);
    Pixel::chamfer(c.p(), c.rect(), Theme::cream, Theme::borderDark, 0);
    Pixel::chamfer(c.p(), c.rect().adjusted(1, 1, -1, -1), Qt::transparent, Theme::lcdBlue, 0);
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    Pixel::Canvas c(p, inBar ? rect.adjusted(0, 2, 0, -2) : rect.adjusted(4, 0, -4, 0));
    Pixel::highlightTile(c.p(), c.rect());
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    if (look.checked) {
        Pixel::highlightTile(c.p(), c.rect());
    } else {
        Pixel::chamfer(c.p(), c.rect(), Qt::transparent, Theme::kbdUpperBlue, 2);
    }
}

void Style::header(QPainter& p, const QRect& rect, const Look& look) const
{
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    const QColor face = look.hover ? Theme::kbdUpperBlue.lighter(110) : Theme::kbdUpperBlue;
    q.fillRect(r, face);
    q.fillRect(r.left(), r.top(), r.width(), 1, face.lighter(120));
    q.fillRect(r.left(), r.bottom(), r.width(), 1, Theme::borderDark);
    q.fillRect(r.right(), r.top() + 2, 1, r.height() - 4, face.darker(140));
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    Pixel::Canvas c(p, rect);
    Pixel::chamfer(c.p(), c.rect(), Theme::cream, Theme::borderDark, 0);
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // the storage box cursor: a highlighted tile border and the glove pointing down at the box
    Pixel::Canvas c(p, rect);
    QPainter& q = c.p();
    const QRect r = c.rect();
    const int top = metrics().title / Scale / 2 - 2; // where card() starts the box body
    const QRect body = r.adjusted(0, top, 0, 0);
    q.setClipRegion(QRegion(r).subtracted(QRegion(6, 1, r.width() - 12, 16))); // the box's banner stays on top
    Pixel::chamfer(q, body, Qt::transparent, Theme::borderDark, 3);
    Pixel::chamfer(q, body.adjusted(1, 1, -1, -1), Qt::transparent, Theme::highlight, 3);
    Pixel::chamfer(q, body.adjusted(2, 2, -2, -2), Qt::transparent, Theme::keyYellow, 2);
    q.setClipping(false);
    Pixel::hand(q, QPoint(r.right() - 16, 14), Pixel::Direction::Down);
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    Pixel::Canvas c(p, rect);
    const QColor face = lit ? color : color.darker(250);
    disc(c.p(), c.rect(), face, Theme::borderDark);
    if (lit && c.rect().width() >= 4) {
        c.p().fillRect(c.rect().left() + 1, c.rect().top() + 1, 1, 1, Theme::white); // glint
    }
}

} // namespace emerald
