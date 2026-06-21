#include "themes/revolut/Style.hpp"

#include <QPainter>
#include <QPainterPath>

namespace revolut {

namespace {

    QColor alpha(QColor c, int a)
    {
        c.setAlpha(a * c.alpha() / 255);
        return c;
    }

    QPainterPath rounded(const QRectF& r, qreal radius)
    {
        QPainterPath path;
        const qreal rr = std::min(radius, std::min(r.width(), r.height()) / 2);
        path.addRoundedRect(r, rr, rr);
        return path;
    }

    qreal pill(const QRectF& r)
    {
        return std::min(r.width(), r.height()) / 2; // --rui-radius-round
    }

} // namespace

Style::Style(bool dark)
    : WidgetStyle([] {
        Metrics m;
        m.control = 36;
        m.padding = 18;
        m.frame = 2;
        m.indicator = 18;
        m.arrow = 10;
        m.groove = 6;
        m.scroll = 10;
        m.tab = 34;
        m.title = 36;
        m.row = 28;
        m.margin = 16;
        m.spacing = 12;
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
    pal.setColor(QPalette::Base, c.widget);
    pal.setColor(QPalette::AlternateBase, alpha(c.tint, c.dark ? 150 : 255));
    pal.setColor(QPalette::Text, c.text);
    pal.setColor(QPalette::PlaceholderText, c.text3);
    pal.setColor(QPalette::Button, c.chip);
    pal.setColor(QPalette::ButtonText, c.text);
    pal.setColor(QPalette::BrightText, c.accent);
    pal.setColor(QPalette::Highlight, c.accent);
    pal.setColor(QPalette::HighlightedText, Theme::onAccent);
    pal.setColor(QPalette::ToolTipBase, c.dark ? c.widget : c.text);
    pal.setColor(QPalette::ToolTipText, c.dark ? c.text : c.canvas);
    pal.setColor(QPalette::Link, c.accentHover);
    pal.setColor(QPalette::Light, c.widget);
    pal.setColor(QPalette::Midlight, c.card);
    pal.setColor(QPalette::Mid, c.track);
    pal.setColor(QPalette::Dark, c.text3);
    pal.setColor(QPalette::Shadow, c.text);
    for (const QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
        pal.setColor(QPalette::Disabled, role, alpha(c.text3, 150));
    }
    return pal;
}

void Style::surface(QPainter& p, const QRectF& rect, qreal radius, const QColor& fill, const QColor& border, bool focusRing) const
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(rect, radius), fill);
    if (border.alpha() > 0) {
        p.strokePath(rounded(rect.adjusted(0.5, 0.5, -0.5, -0.5), radius), QPen(border, 1));
    }
    if (focusRing) {
        p.strokePath(rounded(rect.adjusted(-1, -1, 1, 1), radius + 1), QPen(Theme::focus, 2));
    }
    p.restore();
}

void Style::shadow(QPainter& p, const QRectF& rect, qreal radius, int level) const
{
    if (c.dark) {
        return; // dark surfaces are told apart by tone, not shadow
    }
    // --rui-shadow-level1..3: 0 2..3px 3..30px rgba(0, 0, 0, .06..12), as a few soft layers
    const qreal offset = level == 1 ? 1.5 : 2.5;
    const int spread = level == 1 ? 2 : level == 2 ? 4 : 10;
    const int strength = level == 1 ? 14 : level == 2 ? 16 : 12;
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    for (int i = spread; i > 0; --i) {
        const QRectF r = rect.translated(0, offset).adjusted(-i * 0.5, -i * 0.5, i * 0.5, i * 0.5);
        p.fillPath(rounded(r, radius + i * 0.5), QColor(26, 15, 51, strength / i + 1));
    }
    p.restore();
}

// ---- surfaces -----------------------------------------------------------------------------------

void Style::window(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, c.canvas);
    if (c.dark) {
        // the dark stage: a violet radial glow from the top
        QRadialGradient glow(QPointF(rect.left() + rect.width() * 0.72, rect.top() - rect.height() * 0.1), std::max(rect.width(), rect.height()) * 0.75);
        glow.setColorAt(0, alpha(c.accent, 70));
        glow.setColorAt(0.5, alpha(c.accent, 18));
        glow.setColorAt(1, alpha(c.accent, 0));
        p.fillRect(rect, glow);
    }
}

void Style::card(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    // tinted bento card: no border, no shadow at rest
    surface(p, QRectF(rect), Theme::radiusCard, c.card);
    if (title.isEmpty()) {
        return;
    }
    p.save();
    p.setFont(Theme::eyebrowFont());
    p.setPen(c.accent);
    p.drawText(titleRect.adjusted(20, 8, -20, 0), Qt::AlignLeft | Qt::AlignVCenter, title);
    p.restore();
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    const qreal radius = pill(r);
    const bool accent = kind == Button::Default || look.checked;
    QColor fill;
    if (!look.enabled) {
        fill = alpha(accent ? c.accent : c.chip, 110);
    } else if (accent) {
        fill = look.pressed ? c.accentHover.darker(110) : look.hover ? c.accentHover : c.accent;
    } else if (kind == Button::Flat && !(look.hover || look.pressed)) {
        fill = Qt::transparent;
    } else {
        fill = look.pressed ? c.chipHover.darker(106) : look.hover ? c.chipHover : c.chip;
    }
    // pressed pills compress a little
    const QRectF face = look.pressed ? r.adjusted(r.width() * 0.01, r.height() * 0.02, -r.width() * 0.01, -r.height() * 0.02) : r;
    surface(p, face, radius, fill, Qt::transparent, look.focusVisible);
}

QColor Style::buttonText(Button kind, const Look& look, const QPalette&) const
{
    if (!look.enabled) {
        return alpha(c.text3, 170);
    }
    return kind == Button::Default || look.checked ? Theme::onAccent : c.text;
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    // the field being typed in is outlined in violet; the amber ring only follows the keyboard
    const QColor border = look.focus ? c.accent : look.hover && look.enabled ? alpha(c.accent, 150) : c.line;
    surface(p, r, std::min(Theme::radiusField, r.height() / 2), look.enabled ? c.widget : alpha(c.widget, 150), border, look.focusVisible);
}

void Style::view(QPainter& p, const QRect& rect, const Look&) const
{
    surface(p, QRectF(rect), Theme::radiusWidget, c.widget, c.line);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    if (state == Qt::Unchecked) {
        surface(p, r, 6, c.widget, look.hover ? c.accent : alpha(c.text3, 170), look.focusVisible);
        return;
    }
    surface(p, r, 6, look.enabled ? (look.hover ? c.accentHover : c.accent) : alpha(c.accent, 110), Qt::transparent, look.focusVisible);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Theme::onAccent, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    if (state == Qt::Checked) {
        p.drawPolyline(QPolygonF({ at(0.26, 0.52), at(0.44, 0.70), at(0.75, 0.32) }));
    } else {
        p.drawLine(at(0.28, 0.5), at(0.72, 0.5));
    }
    p.restore();
}

void Style::radio(QPainter& p, const QRect& rect, bool on, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    if (!on) {
        surface(p, r, pill(r), c.widget, look.hover ? c.accent : alpha(c.text3, 170), look.focusVisible);
        return;
    }
    surface(p, r, pill(r), look.enabled ? c.accent : alpha(c.accent, 110), Qt::transparent, look.focusVisible);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::onAccent);
    p.drawEllipse(r.center(), r.width() * 0.2, r.width() * 0.2);
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
    p.setPen(QPen(!look.enabled ? alpha(c.text3, 120) : look.hover ? c.accent : c.text2, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawPolyline(chevron);
    p.restore();
}

// ---- ranges -------------------------------------------------------------------------------------

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation, const Look& look) const
{
    const QRectF r(rect);
    surface(p, r, pill(r), c.track);
    if (!filled.isEmpty()) {
        surface(p, QRectF(filled), pill(r), look.enabled ? c.accent : alpha(c.accent, 110));
    }
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    shadow(p, r, pill(r), 2);
    const QColor face = c.dark ? QColor(0xf1, 0xed, 0xf9) : QColor(Qt::white);
    surface(p, r, pill(r), look.enabled ? face : alpha(face, 140), look.hover || look.pressed ? c.accent : c.line, look.focusVisible);
    if (look.pressed) {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(c.accent);
        p.drawEllipse(r.center(), r.width() * 0.18, r.height() * 0.18);
        p.restore();
    }
}

QSize Style::handleSize(Qt::Orientation) const
{
    return { 20, 20 };
}

void Style::scrollBar(QPainter& p, const QRect&, const QRect& handle, Qt::Orientation, const Look& look) const
{
    // no track: a thin pill floating over the content
    const QRectF r = QRectF(handle).adjusted(2, 2, -2, -2);
    surface(p, r, pill(r), look.pressed ? c.accent : look.hover ? c.text3 : alpha(c.text3, 120));
}

void Style::progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    surface(p, r, pill(r), c.track);
    if (!filled.isEmpty()) {
        const QRectF f = QRectF(filled).adjusted(0.5, 0.5, -0.5, -0.5);
        surface(p, f, pill(r), c.accent);
    }
}

void Style::tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const
{
    // segmented pills: the selected one is a raised app widget
    const QRectF r = QRectF(rect).adjusted(2, 3, -2, -3);
    if (selected) {
        shadow(p, r, pill(r), 1);
        surface(p, r, pill(r), c.dark ? c.chipHover : c.widget, Qt::transparent, look.focusVisible);
    } else if (look.hover) {
        surface(p, r, pill(r), c.chip);
    }
}

void Style::tabPane(QPainter& p, const QWidget*, const QRect& rect) const
{
    surface(p, QRectF(rect), Theme::radiusWidget, c.widget, c.line);
}

void Style::dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const
{
    const QRectF ring = QRectF(rect).adjusted(8, 8, -8, -8);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(c.track, 6, Qt::SolidLine, Qt::RoundCap));
    p.drawArc(ring, 225 * 16, -270 * 16);
    if (value > 0) {
        p.setPen(QPen(look.enabled ? c.accent : alpha(c.accent, 110), 6, Qt::SolidLine, Qt::RoundCap));
        p.drawArc(ring, 225 * 16, int(-270 * 16 * value));
    }
    p.restore();

    const QRectF knob = ring.adjusted(ring.width() * 0.2, ring.height() * 0.2, -ring.width() * 0.2, -ring.height() * 0.2);
    shadow(p, knob, pill(knob), 2);
    surface(p, knob, pill(knob), c.widget, look.hover ? c.accent : c.line, look.focusVisible);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(look.enabled ? c.accent : c.text3);
    p.drawEllipse(dialPoint(knob, value, knob.width() * 0.3), 3.5, 3.5);
    p.restore();
}

void Style::display(QPainter& p, const QRect& rect) const
{
    // an app widget: numbers stand out in violet on the widget surface
    surface(p, QRectF(rect), std::min(Theme::radiusWidget, rect.height() / 2.0), c.widget, c.line);
}

QColor Style::displayText() const
{
    return c.accent;
}

// ---- menus & items ------------------------------------------------------------------------------

void Style::menuBar(QPainter& p, const QWidget*, const QRect& rect) const
{
    // solid navigation bar with a hairline under it
    p.fillRect(rect, c.dark ? c.canvas : c.widget);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), c.line);
}

QColor Style::menuBarText(bool) const
{
    return c.text;
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, c.widget);
    p.setPen(c.dark ? QColor(255, 255, 255, 30) : c.line);
    p.drawRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    if (inBar) {
        const QRectF r = QRectF(rect).adjusted(0, 4, 0, -4);
        surface(p, r, pill(r), c.chip);
    } else {
        surface(p, QRectF(rect).adjusted(4, 1, -4, -1), Theme::radiusItem, c.accent);
    }
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    surface(p, r, Theme::radiusItem, look.checked ? c.accent : c.chip);
}

void Style::header(QPainter& p, const QRect& rect, const Look& look) const
{
    p.fillRect(rect, look.hover ? alpha(c.tint, c.dark ? 200 : 255) : c.widget);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), c.line);
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, c.dark ? c.widget : c.text);
    if (c.dark) {
        p.setPen(QColor(255, 255, 255, 30));
        p.drawRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
    }
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // the card in focus: the amber focus ring round it
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.strokePath(rounded(r.adjusted(-2, -2, 2, 2), Theme::radiusCard + 2), QPen(alpha(Theme::focus, 70), 3));
    p.strokePath(rounded(r, Theme::radiusCard), QPen(Theme::focus, 2));
    p.restore();
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    if (lit) {
        p.setBrush(alpha(color, 70)); // soft tint halo, like the accent / tint pairs
        p.drawEllipse(r.adjusted(-2, -2, 2, 2));
    }
    p.setBrush(lit ? color : alpha(color, 90));
    p.drawEllipse(r);
    p.restore();
}

} // namespace revolut
