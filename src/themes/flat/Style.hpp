#pragma once

#include "WidgetStyle.hpp"
#include "themes/flat/Theme.hpp"

namespace flat {

/// The standard widgets as a flat slate operations console: "quiet chrome, loud data". Navy
/// panels with a hairline and a sentence-case title on a slate canvas, cameras separated by
/// gutters of canvas, docked tabs with a steel-blue indicator, sunken fields, and the one signal
/// colour (azure) for the camera under inspection and the primary action; the status lamps keep
/// green, amber and red. No gradients, glows, shadows or bevels.
class Style : public WidgetStyle {
public:
    Style();

    QFont font() const override;
    QPalette standardPalette() const override;

    void window(QPainter& p, const QWidget* widget, const QRect& rect) const override;
    void card(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title) const override;
    void tile(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title, int index) const override;
    QColor tileText() const override;
    void button(QPainter& p, const QRect& rect, Button kind, const Look& look) const override;
    QColor buttonText(Button kind, const Look& look, const QPalette& palette) const override;
    void field(QPainter& p, const QRect& rect, const Look& look) const override;
    void view(QPainter& p, const QRect& rect, const Look& look) const override;
    void check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const override;
    void arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const override;
    void groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation, const Look& look) const override;
    void handle(QPainter& p, const QRect& rect, Qt::Orientation orientation, const Look& look) const override;
    QSize handleSize(Qt::Orientation orientation) const override;
    void scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation orientation, const Look& look) const override;
    void display(QPainter& p, const QRect& rect) const override;
    QColor displayText() const override;
    void menuBar(QPainter& p, const QWidget* widget, const QRect& rect) const override;
    QColor menuBarText(bool active) const override;
    void menu(QPainter& p, const QRect& rect) const override;
    void highlight(QPainter& p, const QRect& rect, bool inBar) const override;
    void selection(QPainter& p, const QRect& rect, const Look& look) const override;
    void tooltip(QPainter& p, const QRect& rect) const override;
    void focusFrame(QPainter& p, const QRect& rect) const override;
    QColor ownMark() const override;
    void lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const override;

private:
    Theme::Palette c;
};

} // namespace flat
