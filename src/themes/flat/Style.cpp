#include "themes/flat/Style.hpp"

#include <QPainter>

namespace flat {

namespace {

    QColor alpha(QColor c, int a)
    {
        c.setAlpha(a * c.alpha() / 255);
        return c;
    }

    /// 1px line kept inside @p rect: panels are separated by hairlines, never by shadows.
    void hairline(QPainter& p, const QRect& rect, const QColor& color, int width = 1)
    {
        p.fillRect(QRect(rect.left(), rect.top(), rect.width(), width), color);
        p.fillRect(QRect(rect.left(), rect.bottom() - width + 1, rect.width(), width), color);
        p.fillRect(QRect(rect.left(), rect.top() + width, width, rect.height() - 2 * width), color);
        p.fillRect(QRect(rect.right() - width + 1, rect.top() + width, width, rect.height() - 2 * width), color);
    }

    /// Band of @p thickness centred across @p rect (flat tracks are thinner than their widgets).
    QRect band(const QRect& rect, Qt::Orientation orientation, int thickness)
    {
        if (orientation == Qt::Horizontal) {
            const int h = std::min(thickness, rect.height());
            return { rect.left(), rect.center().y() - h / 2 + 1, rect.width(), h };
        }
        const int w = std::min(thickness, rect.width());
        return { rect.center().x() - w / 2 + 1, rect.top(), w, rect.height() };
    }

    /// Panel with its section title at the top-left and a divider under it.
    void panel(QPainter& p, const QRect& rect, const QRect& titleRect, const QString& title)
    {
        p.fillRect(rect, Theme::panel);
        hairline(p, rect, Theme::panelBorder);
        if (title.isEmpty()) {
            return;
        }
        p.save();
        p.setFont(Theme::font(14));
        p.setPen(Theme::textTitle);
        p.drawText(titleRect.adjusted(12, 0, -12, 0), Qt::AlignLeft | Qt::AlignVCenter, title);
        p.restore();
        p.fillRect(QRect(rect.left() + 1, titleRect.bottom(), rect.width() - 2, 1), Theme::divider);
    }

} // namespace

Style::Style()
    : WidgetStyle([] {
        Metrics m;
        m.control = 26;
        m.padding = 10;
        m.frame = 1;
        m.indicator = 14;
        m.arrow = 8;
        m.groove = 2;
        m.scroll = 6;
        m.tab = 28;
        m.title = 30;
        m.row = 22;
        m.margin = 12;
        m.spacing = Theme::gutter;
        return m;
    }())
{
}

QFont Style::font() const
{
    return Theme::font(13);
}

QPalette Style::standardPalette() const
{
    QPalette pal;
    pal.setColor(QPalette::Window, Theme::canvas);
    pal.setColor(QPalette::WindowText, Theme::text);
    pal.setColor(QPalette::Base, Theme::sunken);
    pal.setColor(QPalette::AlternateBase, Theme::panel);
    pal.setColor(QPalette::Text, Theme::text);
    pal.setColor(QPalette::PlaceholderText, Theme::textMuted);
    pal.setColor(QPalette::Button, Theme::button);
    pal.setColor(QPalette::ButtonText, Theme::text);
    pal.setColor(QPalette::BrightText, Theme::detect);
    pal.setColor(QPalette::Highlight, Theme::selected);
    pal.setColor(QPalette::HighlightedText, Theme::text); // a selected row keeps its text colour
    pal.setColor(QPalette::ToolTipBase, Theme::canvas);
    pal.setColor(QPalette::ToolTipText, Theme::textChip);
    pal.setColor(QPalette::Link, Theme::textSelected);
    pal.setColor(QPalette::Light, Theme::panel);
    pal.setColor(QPalette::Midlight, Theme::divider);
    pal.setColor(QPalette::Mid, Theme::panelBorder);
    pal.setColor(QPalette::Dark, Theme::sunken);
    pal.setColor(QPalette::Shadow, Theme::canvas);
    for (const QPalette::ColorRole role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText }) {
        pal.setColor(QPalette::Disabled, role, Theme::textDisabled);
    }
    return pal;
}

// ---- surfaces -----------------------------------------------------------------------------------

void Style::window(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, Theme::canvas); // black edge to edge: the brightest pixels are data
}

void Style::card(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title) const
{
    panel(p, rect, titleRect, title);
}

void Style::tile(QPainter& p, const QWidget*, const QRect& rect, const QRect& titleRect, const QString& title, int) const
{
    // the cameras touch: each keeps half a gutter of canvas round its panel
    constexpr int half = Theme::gutter / 2;
    p.fillRect(rect, Theme::canvas);
    panel(p, rect.adjusted(half, half, -half, -half), titleRect.adjusted(half, half, -half, 0), title);
}

QColor Style::tileText() const
{
    return Theme::text;
}

void Style::button(QPainter& p, const QRect& rect, Button kind, const Look& look) const
{
    const bool primary = kind == Button::Default;
    QColor fill = primary ? Theme::detectFill : Theme::button;
    QColor edge = primary ? Theme::detectFill : Theme::panelBorder;
    if (!look.enabled) {
        fill = primary ? Theme::detectDisabled : Theme::button;
        edge = primary ? Theme::detectDisabled : Theme::buttonDisabledBorder;
    } else if (look.pressed || look.checked) {
        fill = primary ? Theme::detectPressed : Theme::buttonPressed;
    } else if (look.hover) {
        fill = primary ? Theme::detectHover : Theme::selected;
    }
    if (kind == Button::Flat && look.enabled && !look.hover && !look.pressed && !look.checked) {
        return; // a text command until it is touched
    }
    if (look.focusVisible) {
        edge = primary ? Theme::detect : Theme::selectedLine;
    }
    p.fillRect(rect, fill);
    hairline(p, rect, edge);
}

QColor Style::buttonText(Button kind, const Look& look, const QPalette&) const
{
    if (!look.enabled) {
        return Theme::textDisabled;
    }
    return kind == Button::Default ? Theme::detectText : Theme::text;
}

void Style::field(QPainter& p, const QRect& rect, const Look& look) const
{
    p.fillRect(rect, Theme::sunken);
    const QColor edge = !look.enabled ? Theme::buttonDisabledBorder : look.focus ? Theme::selectedLine : look.hover ? Theme::fieldHover : Theme::panelBorder;
    hairline(p, rect, edge);
}

void Style::view(QPainter& p, const QRect& rect, const Look&) const
{
    p.fillRect(rect, Theme::sunken);
    hairline(p, rect, Theme::panelBorder);
}

void Style::check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const
{
    const QRect& r = rect;
    p.fillRect(r, Theme::sunken);
    hairline(p, r, !look.enabled ? Theme::buttonDisabledBorder : look.focusVisible ? Theme::selectedLine : look.hover ? Theme::fieldHover : Theme::panelBorder);
    const QColor mark = look.enabled ? Theme::detect : Theme::textDisabled;
    if (state == Qt::Checked) {
        const QRectF m = QRectF(r).adjusted(3, 3, -3, -3);
        const auto at = [&](qreal x, qreal y) { return QPointF(m.left() + x * m.width(), m.top() + y * m.height()); };
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(mark, 1.8, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
        p.drawPolyline(QPolygonF({ at(0.05, 0.52), at(0.38, 0.84), at(0.95, 0.16) }));
        p.restore();
    } else if (state == Qt::PartiallyChecked) {
        p.fillRect(r.adjusted(4, 4, -4, -4), mark);
    }
}

void Style::radio(QPainter& p, const QRect& rect, bool on, const Look& look) const
{
    const QRectF r = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(Theme::sunken);
    p.setPen(QPen(!look.enabled ? Theme::buttonDisabledBorder : look.focusVisible ? Theme::selectedLine : look.hover ? Theme::fieldHover : Theme::panelBorder, 1));
    p.drawEllipse(r);
    if (on) {
        p.setPen(Qt::NoPen);
        p.setBrush(look.enabled ? Theme::detect : Theme::textDisabled);
        p.drawEllipse(r.center(), r.width() * 0.25, r.height() * 0.25);
    }
    p.restore();
}

void Style::arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    const QRectF r(rect);
    QPolygonF chevron;
    const auto at = [&](qreal x, qreal y) { return QPointF(r.left() + x * r.width(), r.top() + y * r.height()); };
    switch (type) {
    case Qt::UpArrow: chevron = { at(0.1, 0.7), at(0.5, 0.3), at(0.9, 0.7) }; break;
    case Qt::DownArrow: chevron = { at(0.1, 0.3), at(0.5, 0.7), at(0.9, 0.3) }; break;
    case Qt::LeftArrow: chevron = { at(0.7, 0.1), at(0.3, 0.5), at(0.7, 0.9) }; break;
    case Qt::RightArrow: chevron = { at(0.3, 0.1), at(0.7, 0.5), at(0.3, 0.9) }; break;
    default: return;
    }
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(!look.enabled ? Theme::textDisabled : look.hover ? Theme::text : Theme::textMuted, 1.4, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    p.drawPolyline(chevron);
    p.restore();
}

// ---- ranges -------------------------------------------------------------------------------------

void Style::groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation, const Look& look) const
{
    p.fillRect(rect, Theme::divider);
    p.fillRect(filled, look.enabled ? Theme::selectedLine : Theme::panelBorder);
}

void Style::handle(QPainter& p, const QRect& rect, Qt::Orientation, const Look& look) const
{
    // a 10px square, no shadow
    QRect r(QPoint(), QSize(10, 10));
    r.moveCenter(rect.center());
    p.fillRect(r, !look.enabled ? Theme::textDisabled : look.pressed || look.hover ? Theme::textTitle : Theme::text);
    if (look.focusVisible) {
        hairline(p, r.adjusted(-2, -2, 2, 2), Theme::selectedLine);
    }
}

QSize Style::handleSize(Qt::Orientation) const
{
    return { 14, 14 }; // room round the 10px square for its focus outline
}

void Style::scrollBar(QPainter& p, const QRect&, const QRect& handle, Qt::Orientation, const Look& look) const
{
    // no arrows, no track: the handle alone
    p.fillRect(handle.adjusted(1, 1, -1, -1), look.pressed || look.hover ? Theme::selectedLine : Theme::panelBorder);
}

void Style::progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation) const
{
    p.fillRect(band(rect, orientation, 6), Theme::divider);
    if (!filled.isEmpty()) {
        p.fillRect(band(filled, orientation, 6), Theme::detect);
    }
}

void Style::tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const
{
    // docked tabs: the selected one on a slate fill over a steel-blue indicator
    if (selected) {
        p.fillRect(rect, Theme::selected);
        p.fillRect(QRect(rect.left(), rect.bottom() - 1, rect.width(), 2), Theme::selectedLine);
    }
    if (look.focusVisible) {
        hairline(p, rect, Theme::selectedLine);
    }
}

void Style::tabPane(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, Theme::panel);
    hairline(p, rect, Theme::panelBorder);
}

void Style::dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const
{
    const QRectF ring = QRectF(rect).adjusted(8, 8, -8, -8);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Theme::divider, 2, Qt::SolidLine, Qt::FlatCap));
    p.drawArc(ring, 225 * 16, -270 * 16);
    if (value > 0) {
        p.setPen(QPen(look.enabled ? Theme::selectedLine : Theme::panelBorder, 2, Qt::SolidLine, Qt::FlatCap));
        p.drawArc(ring, 225 * 16, int(-270 * 16 * value));
    }
    const QPointF at = dialPoint(ring, value, ring.width() / 2);
    p.setPen(Qt::NoPen);
    p.setBrush(!look.enabled ? Theme::textDisabled : look.pressed || look.hover ? Theme::textTitle : Theme::text);
    p.drawRect(QRectF(at - QPointF(5, 5), QSizeF(10, 10)));
    p.restore();
}

void Style::display(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, Theme::sunken);
    hairline(p, rect, Theme::panelBorder);
}

QColor Style::displayText() const
{
    return Theme::textOverlay;
}

// ---- menus & items ------------------------------------------------------------------------------

void Style::menuBar(QPainter& p, const QWidget*, const QRect& rect) const
{
    p.fillRect(rect, Theme::canvas);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), Theme::divider);
}

QColor Style::menuBarText(bool active) const
{
    return active ? Theme::textSelected : Theme::text;
}

void Style::menu(QPainter& p, const QRect& rect) const
{
    p.fillRect(rect, Theme::panel);
    hairline(p, rect, Theme::panelBorder);
}

void Style::highlight(QPainter& p, const QRect& rect, bool inBar) const
{
    p.fillRect(inBar ? rect : rect.adjusted(1, 0, -1, 0), Theme::selected);
}

void Style::selection(QPainter& p, const QRect& rect, const Look& look) const
{
    p.fillRect(rect, look.checked ? Theme::selected : Theme::rowHover);
}

void Style::header(QPainter& p, const QRect& rect, const Look& look) const
{
    // no fill, no bold: a divider under the column names
    p.fillRect(rect, look.hover ? Theme::rowHover : Theme::panel);
    p.fillRect(QRect(rect.left(), rect.bottom(), rect.width(), 1), Theme::divider);
}

void Style::tooltip(QPainter& p, const QRect& rect) const
{
    // the overlay caption chip, made opaque (tooltips float over anything)
    p.fillRect(rect, Theme::canvas);
    hairline(p, rect, Theme::panelBorder);
}

void Style::focusFrame(QPainter& p, const QRect& rect) const
{
    // the camera under inspection is the tracked object: a 2px detection-green box, square corners
    hairline(p, rect, Theme::detect, 2);
}

void Style::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    // a flat status marker: a small filled circle, no rim, no glow
    const QRectF r = QRectF(rect).adjusted(1, 1, -1, -1);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(lit ? color : alpha(color, 70));
    p.drawEllipse(r);
    p.restore();
}

} // namespace flat
