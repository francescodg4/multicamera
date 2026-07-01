#pragma once

#include "WidgetStyle.hpp"

namespace glass {

/// The standard widgets in Liquid Glass: frosted cards over the ambient canvas, recessed glass
/// fields, clay buttons and knobs that compress when pressed, radiant glows for active states.
class Style : public WidgetStyle {
public:
    Style();

    QFont font() const override;
    QPalette standardPalette() const override;

    void window(QPainter& p, const QWidget* widget, const QRect& rect) const override;
    void card(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title) const override;
    void button(QPainter& p, const QRect& rect, Button kind, const Look& look) const override;
    QColor buttonText(Button kind, const Look& look, const QPalette& palette) const override;
    void field(QPainter& p, const QRect& rect, const Look& look) const override;
    void view(QPainter& p, const QRect& rect, const Look& look) const override;
    void check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const override;
    void radio(QPainter& p, const QRect& rect, bool on, const Look& look) const override;
    void arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const override;
    void groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation, const Look& look) const override;
    void handle(QPainter& p, const QRect& rect, Qt::Orientation orientation, const Look& look) const override;
    QSize handleSize(Qt::Orientation orientation) const override;
    void scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation orientation, const Look& look) const override;
    void progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation) const override;
    void tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const override;
    void tabPane(QPainter& p, const QWidget* widget, const QRect& rect) const override;
    void dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const override;
    void display(QPainter& p, const QRect& rect) const override;
    QColor displayText() const override;
    void menuBar(QPainter& p, const QWidget* widget, const QRect& rect) const override;
    QColor menuBarText(bool active) const override;
    void menu(QPainter& p, const QRect& rect) const override;
    void highlight(QPainter& p, const QRect& rect, bool inBar) const override;
    void selection(QPainter& p, const QRect& rect, const Look& look) const override;
    void header(QPainter& p, const QRect& rect, const Look& look) const override;
    void tooltip(QPainter& p, const QRect& rect) const override;
    void focusFrame(QPainter& p, const QRect& rect) const override;
    QColor ownMark() const override;
    void lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const override;
};

} // namespace glass
