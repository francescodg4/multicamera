#include "themes/winamp/Style.hpp"

#include "themes/winamp/Theme.hpp"
#include "themes/winamp/widgets/Metal.hpp"

#include <QPainter>
#include <QPainterPath>

namespace winamp {

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

    /// Soft LCD-blue ring round a focused or hovered control.
    void glowRing(QPainter& p, const QRectF& r, qreal radius, int strength)
    {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.strokePath(rounded(r.adjusted(-1, -1, 1, 1), radius + 1), QPen(alpha(Theme::lcdGlow, strength / 3), 3));
        p.strokePath(rounded(r, radius), QPen(alpha(Theme::lcdGlow, strength), 1.2));
        p.restore();
    }

    /// Glowing stroke drawn on LCD glass.
    void glowStroke(QPainter& p, const QPainterPath& path, qreal width)
    {
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(alpha(Theme::lcdGlow, 70), width * 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(path);
        p.setPen(QPen(Theme::lcdGlow.lighter(115), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(path);
        p.restore();
    }

    const QColor grooveDark { 0x2a, 0x34, 0x46 };
    const QColor fillTop { 0x7a, 0xb2, 0xff };
    const QColor fillBottom { 0x2d, 0x63, 0xc8 };

} // namespace

Style::Style()
    : WidgetStyle([] {
        Metrics m;
        m.control = 28;
        m.padding = 14;
        m.frame = 3;
        m.indicator = 18;
        m.arrow = 9;
        m.groove = 6;
        m.scroll = 14;
        m.tab = 28;
        m.title = 30;
        m.row = 22;
        m.margin = 12;
        m.spacing = 8;
        m.pressShift = 1;
        return m;
    }())
{
}

QFont Style::font() const
{
    return Metal::uiFont(9);
}

QPalette Style::standardPalette() const
{
    QPalette pal;
    pal.setColor(QPalette::Window, Theme::frameTop);
    pal.setColor(QPalette::WindowText, Theme::text);
    pal.setColor(QPalette::Base, Qt::transparent); // fields and views are LCD glass
    pal.setColor(QPalette::AlternateBase, alpha(Theme::lcdGlow, 14));
    pal.setColor(QPalette::Text, Theme::lcdGlow);
    pal.setColor(QPalette::PlaceholderText, alpha(Theme::lcdGlow, 110));
    pal.setColor(QPalette::Button, QColor(0xe4, 0xe8, 0xee));
    pal.setColor(QPalette::ButtonText, Theme::text);
    pal.setColor(QPalette::BrightText, Theme::lcdGlow);
    pal.setColor(QPalette::Highlight, Theme::accent);
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipBase, QColor(0xee, 0xf1, 0xf5));
    pal.setColor(QPalette::ToolTipText, Theme::text);
    pal.setColor(QPalette::Link, Theme::accent);
    pal.setColor(QPalette::Light, Theme::bevelLight);
    pal.setColor(QPalette::Midlight, Theme::frameTop);
    pal.setColor(QPalette::Mid, Theme::outline);
    pal.setColor(QPalette::Dark, Theme::bevelDark);
    pal.setColor(QPalette::Shadow, QColor(0x10, 0x14, 0x1c));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, Theme::textDim);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, Theme::textDim);
    pal.setColor(QPalette::Disabled, QPalette::Text, Theme::lcdDim.lighter(160));
    return pal;
}

void Style::window(QPainter& p, const QWidget*, const QRect& rect) const
{
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0, Theme::frameTop);
    g.setColorAt(1, Theme::frameBottom);
    p.fillRect(rect, g);
    // brushed metal: faint horizontal hairlines
    for (int y = rect.top(); y < rect.bottom(); y += 2) {
        p.fillRect(QRect(rect.left(), y, rect.width(), 1), QColor(255, 255, 255, (y / 2) % 3 == 0 ? 22 : 8));
    }
}

void Style::card(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    Metal::frame(p, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), Theme::radiusPanel);
    if (title.isEmpty()) {
        return;
    }
    // bold caption followed by a ridged groove, like the grooves beside the title bar's name
    QFont f = Metal::uiFont(8, true);
    f.setCapitalization(QFont::AllUppercase);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0.8);
    p.save();
    p.setFont(f);
    const QRect text = titleRect.adjusted(12, 4, -12, 0);
    p.setPen(QColor(255, 255, 255, 170));
    p.drawText(text.translated(0, 1), Qt::AlignLeft | Qt::AlignVCenter, title);
    p.setPen(Theme::text);
    p.drawText(text, Qt::AlignLeft | Qt::AlignVCenter, title);
    const int end = text.left() + QFontMetrics(f).horizontalAdvance(title.toUpper()) + 10;
    if (end < text.right() - 20) {
        Metal::ridge(p, QRectF(end, text.center().y() - 3, text.right() - end, 6));
    }
    p.restore();
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    if (kind == Button::Flat && !(look.hover || look.pressed || look.checked)) {
        return;
    }
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.55);
    }
    Metal::capsule(p, r, look.pressed, look.enabled && (kind == Button::Default || look.checked), look.hover);
    p.restore();
    if (look.focus) {
        glowRing(p, r, std::min(Theme::radiusButton, r.height() / 2), 170);
    }
}

QColor Style::buttonText(Button kind, const Look& look, const QPalette&) const
{
    if (!look.enabled) {
        return Theme::textDim;
    }
    return kind == Button::Default || look.checked ? Qt::white : Theme::text;
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    Metal::lcd(p, r, Theme::radiusPanel);
    if (look.focus) {
        glowRing(p, r, Theme::radiusPanel, 200);
    } else if (look.hover) {
        glowRing(p, r, Theme::radiusPanel, 90);
    }
}

void Style::view(QPainter& p, const QRect& rect, const Look&) const
{
    Metal::lcd(p, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), Theme::radiusPanel);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    Metal::lcd(p, r, 4);
    if (look.hover || look.focus) {
        glowRing(p, r, 4, look.focus ? 170 : 100);
    }
    if (state == Qt::Unchecked) {
        return;
    }
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    QPainterPath mark;
    if (state == Qt::Checked) {
        mark.moveTo(at(0.25, 0.52));
        mark.lineTo(at(0.43, 0.70));
        mark.lineTo(at(0.76, 0.30));
    } else {
        mark.moveTo(at(0.28, 0.5));
        mark.lineTo(at(0.72, 0.5));
    }
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.4);
    }
    glowStroke(p, mark, 1.8);
    p.restore();
}

void Style::radio(QPainter& p, const QRect& rect, bool on, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.55);
    }
    Metal::roundButton(p, r, look.pressed, false, look.hover);
    Metal::led(p, r.center(), r.width() * 0.2, Theme::lcdGlow, on && look.enabled);
    p.restore();
}

void Style::arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    const QRectF r(rect);
    QPolygonF tri;
    switch (type) {
    case Qt::UpArrow: tri = { QPointF(r.left(), r.bottom() - r.height() * 0.2), QPointF(r.center().x(), r.top() + r.height() * 0.2), QPointF(r.right(), r.bottom() - r.height() * 0.2) }; break;
    case Qt::DownArrow: tri = { QPointF(r.left(), r.top() + r.height() * 0.2), QPointF(r.center().x(), r.bottom() - r.height() * 0.2), QPointF(r.right(), r.top() + r.height() * 0.2) }; break;
    case Qt::LeftArrow: tri = { QPointF(r.right() - r.width() * 0.2, r.top()), QPointF(r.left() + r.width() * 0.2, r.center().y()), QPointF(r.right() - r.width() * 0.2, r.bottom()) }; break;
    case Qt::RightArrow: tri = { QPointF(r.left() + r.width() * 0.2, r.top()), QPointF(r.right() - r.width() * 0.2, r.center().y()), QPointF(r.left() + r.width() * 0.2, r.bottom()) }; break;
    default: return;
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(look.enabled ? (look.hover ? Theme::accent : Theme::text) : Theme::textDim);
    p.drawPolygon(tri);
    p.restore();
}

void Style::spinButton(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    // small metal capsules standing on the LCD
    const QRectF r = QRectF(rect).adjusted(1.5, 1.5, -2.5, -1.5);
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.55);
    }
    Metal::capsule(p, r, look.pressed, false, look.hover);
    p.restore();
    QRect a(0, 0, 7, 7);
    a.moveCenter(r.center().toPoint());
    arrow(p, a, type, look);
}

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation, const Look& look) const
{
    const QRectF r(rect);
    const qreal radius = std::min(r.width(), r.height()) / 2;
    Metal::insetPanel(p, r, grooveDark, radius);
    const QRectF f = QRectF(filled).adjusted(1, 1, -1, -1);
    if (f.width() <= 0 || f.height() <= 0) {
        return;
    }
    QLinearGradient g(f.topLeft(), orientation == Qt::Horizontal ? f.bottomLeft() : f.topRight());
    g.setColorAt(0, look.enabled ? fillTop : Theme::outline);
    g.setColorAt(1, look.enabled ? fillBottom : Theme::bevelDark);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(rounded(f, radius - 1), g);
    p.restore();
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation orientation, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.6);
    }
    Metal::capsule(p, r, look.pressed, false, look.hover);
    p.setPen(QPen(Theme::textDim, 1));
    if (orientation == Qt::Horizontal) {
        for (int i = -1; i <= 1; ++i) { // grip lines
            const qreal x = std::round(r.center().x()) + i * 3 + 0.5;
            p.drawLine(QPointF(x, r.top() + 4), QPointF(x, r.bottom() - 4));
        }
    } else {
        const qreal y = std::round(r.center().y()) + 0.5; // the EQ thumb's centre line
        p.drawLine(QPointF(r.left() + 4, y), QPointF(r.right() - 4, y));
    }
    p.restore();
}

QSize Style::handleSize(Qt::Orientation orientation) const
{
    // the position slider's wide capsule; the EQ bands' upright thumbs
    return orientation == Qt::Horizontal ? QSize(36, 16) : QSize(18, 30);
}

void Style::scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation orientation, const Look& look) const
{
    const QRectF g = QRectF(groove).adjusted(0.5, 0.5, -0.5, -0.5);
    Metal::insetPanel(p, g, grooveDark, std::min(g.width(), g.height()) / 2);
    this->handle(p, orientation == Qt::Horizontal ? handle.adjusted(0, 1, 0, -1) : handle.adjusted(1, 0, -1, 0), orientation, look);
}

void Style::progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    Metal::lcd(p, r, Theme::radiusPanel);
    // VU-meter segments, lit up to the value
    const QRectF inner = r.adjusted(4, 4, -4, -4);
    const QRectF lit(filled);
    constexpr qreal Segment = 4, Gap = 2;
    p.save();
    const bool horizontal = orientation == Qt::Horizontal;
    const qreal length = horizontal ? inner.width() : inner.height();
    for (qreal s = 0; s + Segment <= length; s += Segment + Gap) {
        const QRectF seg = horizontal ? QRectF(inner.left() + s, inner.top(), Segment, inner.height())
                                      : QRectF(inner.left(), inner.bottom() - s - Segment, inner.width(), Segment);
        const bool on = horizontal ? seg.center().x() <= lit.right() && !lit.isEmpty() : seg.center().y() >= lit.top() && !lit.isEmpty();
        if (on) {
            QLinearGradient g(seg.topLeft(), seg.bottomLeft());
            g.setColorAt(0, fillTop);
            g.setColorAt(1, Theme::accent);
            p.fillRect(seg, g);
        } else {
            p.fillRect(seg, Theme::lcdDim);
        }
    }
    p.restore();
}

void Style::tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const
{
    // the skin's slanted tabs, wider at the base
    const QRectF r = QRectF(rect).adjusted(-4, selected ? 1.5 : 4.5, 4, 0.5);
    const qreal slant = r.height() * 0.45;
    QPainterPath shape;
    shape.moveTo(r.left(), r.bottom());
    shape.lineTo(r.left() + slant, r.top() + 3);
    shape.quadTo(r.left() + slant + 2, r.top(), r.left() + slant + 6, r.top());
    shape.lineTo(r.right() - slant - 6, r.top());
    shape.quadTo(r.right() - slant - 2, r.top(), r.right() - slant, r.top() + 3);
    shape.lineTo(r.right(), r.bottom());
    QLinearGradient g(r.topLeft(), r.bottomLeft());
    g.setColorAt(0, selected ? QColor(0xf8, 0xfa, 0xfd) : look.hover ? QColor(0xe8, 0xec, 0xf2) : QColor(0xc8, 0xcf, 0xda));
    g.setColorAt(1, selected ? Theme::frameTop : QColor(0xa8, 0xb1, 0xbf));
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(shape, g);
    p.strokePath(shape, QPen(Theme::outline, 1));
    p.strokePath(shape.translated(0, 1), QPen(QColor(255, 255, 255, 140), 1));
    p.restore();
}

void Style::tabPane(QPainter& p, const QWidget*, const QRect& rect) const
{
    Metal::frame(p, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), Theme::radiusPanel);
}

void Style::dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(4, 4, -4, -4);
    // LED ring, lit up to the value
    constexpr int Leds = 13;
    for (int i = 0; i < Leds; ++i) {
        const qreal v = qreal(i) / (Leds - 1);
        Metal::led(p, dialPoint(r, v, r.width() / 2 - 3), 2.2, Theme::lcdGlow, look.enabled && v <= value + 1e-6);
    }
    // bevel-edged metal knob with its pointer
    const QRectF knob = r.adjusted(r.width() * 0.17, r.height() * 0.17, -r.width() * 0.17, -r.height() * 0.17);
    p.save();
    if (!look.enabled) {
        p.setOpacity(0.6);
    }
    Metal::roundButton(p, knob, look.pressed, false, look.hover);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(look.enabled ? Theme::accent : Theme::textDim, 3, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(dialPoint(knob, value, knob.width() * 0.12), dialPoint(knob, value, knob.width() * 0.36));
    p.restore();
    if (look.focus) {
        glowRing(p, knob, knob.width() / 2, 150);
    }
}

void Style::display(QPainter& p, const QRect& rect) const
{
    Metal::lcd(p, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), Theme::radiusPanel);
}

QColor Style::displayText() const
{
    return Theme::lcdGlow;
}

void Style::menuBar(QPainter& p, const QWidget*, const QRect& rect) const
{
    // the textual menu sits on the metal; a bevelled groove closes it
    p.fillRect(QRect(rect.left(), rect.bottom() - 1, rect.width(), 1), alpha(Theme::bevelDark, 90));
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), QColor(255, 255, 255, 170));
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    Metal::lcd(p, QRectF(rect), 0);
    p.setPen(Theme::outline);
    p.drawRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    if (inBar) {
        Metal::capsule(p, QRectF(rect).adjusted(0.5, 2.5, -0.5, -2.5), false, false, true);
    } else {
        Metal::titleBar(p, QRectF(rect).adjusted(3, 1, -3, -1));
    }
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(1.5, 0.5, -1.5, -0.5);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    if (look.checked) {
        p.fillPath(rounded(r, 3), Theme::lcdRow.lighter(130));
        p.strokePath(rounded(r, 3), QPen(alpha(Theme::lcdGlow, 110), 1));
    } else {
        p.fillPath(rounded(r, 3), alpha(Theme::lcdGlow, 22));
    }
    p.restore();
}

void Style::header(QPainter& p, const QRect& rect, const Look& look) const
{
    QLinearGradient g(rect.topLeft(), rect.bottomLeft());
    g.setColorAt(0, look.hover ? QColor(0xff, 0xff, 0xff) : QColor(0xf4, 0xf6, 0xfa));
    g.setColorAt(1, look.hover ? QColor(0xd0, 0xd6, 0xe0) : QColor(0xc3, 0xca, 0xd6));
    p.fillRect(rect, g);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), Theme::outline);
    p.fillRect(QRect(rect.right(), rect.top() + 3, 1, rect.height() - 6), alpha(Theme::bevelDark, 120));
    p.fillRect(QRect(rect.left(), rect.top(), rect.width(), 1), Qt::white);
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    Metal::frame(p, QRectF(rect), 0);
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // the selected module lights up in LCD blue
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    glowRing(p, r, Theme::radiusPanel, 255);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.strokePath(rounded(r.adjusted(1.5, 1.5, -1.5, -1.5), Theme::radiusPanel - 1), QPen(alpha(Theme::accent, 200), 1.5));
    p.restore();
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    const QRectF r(rect);
    Metal::led(p, r.center(), std::min(r.width(), r.height()) / 2 - 0.5, color, lit);
}

} // namespace winamp
