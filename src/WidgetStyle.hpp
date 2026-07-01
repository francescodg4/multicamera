#pragma once

#include <QProxyStyle>

/// Base of each theme's QStyle for the standard widgets (buttons, fields, sliders, item views...).
///
/// It does the QStyle plumbing once (widget states, geometry, which primitive a widget asks for)
/// and hands every surface to a small drawing vocabulary that each theme implements from its design
/// rules. It is built on Fusion, so whatever is not drawn here still looks coherent.
class WidgetStyle : public QProxyStyle {
    Q_OBJECT
public:
    /// State of the control being drawn.
    struct Look {
        bool enabled = true;
        bool hover = false;
        bool pressed = false;
        bool focus = false;
        bool focusVisible = false; ///< focus that came from the keyboard (the web's :focus-visible)
        bool checked = false; ///< toggled on, selected
    };

    enum class Button {
        Normal,
        Default, ///< the default action (the accent)
        Flat, ///< no surface until hovered
    };

    /// Sizes of the design, in device pixels.
    struct Metrics {
        int control = 30; ///< height of buttons, fields, combo and spin boxes
        int padding = 12; ///< horizontal padding inside them
        int frame = 2; ///< frame width of fields and views
        int indicator = 18; ///< check box and radio button
        int arrow = 10; ///< arrows of combo and spin boxes
        int groove = 6; ///< slider track thickness
        int scroll = 10; ///< scroll bar thickness
        int tab = 30; ///< tab height
        int title = 30; ///< group box title band
        int row = 24; ///< minimum height of an item view row
        int margin = 14; ///< layout margins
        int spacing = 10; ///< layout spacing
        int pressShift = 0; ///< label offset of a pressed button
    };

    explicit WidgetStyle(const Metrics& metrics);

    const Metrics& metrics() const { return m; }

    /// Application font of the design.
    virtual QFont font() const = 0;
    QPalette standardPalette() const override = 0;

    // ---- drawing vocabulary; `widget` may be null -----------------------------------------------

    /// Window background (painted by the window itself).
    virtual void window(QPainter& p, const QWidget* widget, const QRect& rect) const = 0;
    /// Group box: the container and its title (drawn inside @p titleRect, the top band).
    virtual void card(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title) const = 0;
    /// A card showing item @p index of a board (a camera): a design may give each its own colour.
    /// By default, a card().
    virtual void tile(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title, int index) const;
    /// Colour of glyphs drawn on a tile() (its controls' icons). By default, the button text.
    virtual QColor tileText() const;
    virtual void button(QPainter& p, const QRect& rect, Button kind, const Look& look) const = 0;
    virtual QColor buttonText(Button kind, const Look& look, const QPalette& palette) const;
    /// Line edits, spin boxes and editable combo boxes.
    virtual void field(QPainter& p, const QRect& rect, const Look& look) const = 0;
    /// Frame of item views and text edits (also fills their background).
    virtual void view(QPainter& p, const QRect& rect, const Look& look) const = 0;
    virtual void check(QPainter& p, const QRect& rect, Qt::CheckState state, const Look& look) const = 0;
    virtual void radio(QPainter& p, const QRect& rect, bool on, const Look& look) const = 0;
    virtual void arrow(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const = 0;
    virtual void spinButton(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const;
    /// Slider track; @p filled is the part between the minimum and the handle.
    virtual void groove(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation, const Look& look) const = 0;
    virtual void handle(QPainter& p, const QRect& rect, Qt::Orientation orientation, const Look& look) const = 0;
    /// Size of the slider handle (width × height) for the orientation.
    virtual QSize handleSize(Qt::Orientation orientation) const = 0;
    virtual void scrollBar(QPainter& p, const QRect& groove, const QRect& handle, Qt::Orientation orientation, const Look& look) const = 0;
    virtual void progress(QPainter& p, const QRect& rect, const QRect& filled, Qt::Orientation orientation) const = 0;
    virtual void tab(QPainter& p, const QRect& rect, bool selected, const Look& look) const = 0;
    virtual void tabPane(QPainter& p, const QWidget* widget, const QRect& rect) const = 0;
    /// Rotary knob; @p value goes 0..1 clockwise from 7:30 to 4:30 (see dialPoint()).
    virtual void dial(QPainter& p, const QRect& rect, qreal value, const Look& look) const = 0;
    /// Frame of a QLCDNumber, and the colour of its segments.
    virtual void display(QPainter& p, const QRect& rect) const = 0;
    virtual QColor displayText() const = 0;
    /// Menu bar background; @p rect is always the whole bar (the painter may be clipped to a part).
    virtual void menuBar(QPainter& p, const QWidget* widget, const QRect& rect) const = 0;
    virtual QColor menuBarText(bool active) const;
    virtual void menu(QPainter& p, const QRect& rect) const = 0;
    /// Active menu item (@p inBar: an item of the menu bar).
    virtual void highlight(QPainter& p, const QRect& rect, bool inBar) const = 0;
    /// Selected (look.checked) or hovered item of an item view.
    virtual void selection(QPainter& p, const QRect& rect, const Look& look) const = 0;
    virtual void header(QPainter& p, const QRect& rect, const Look& look) const = 0;
    virtual void tooltip(QPainter& p, const QRect& rect) const = 0;
    /// Marks the selected card of a board (e.g. the camera being inspected) in mark(); the mark
    /// may spill a few pixels outside @p rect.
    virtual void focusFrame(QPainter& p, const QRect& rect) const;
    /// The design's own colour for the selection mark. By default, the highlight.
    virtual QColor ownMark() const;
    /// Colour of the selection mark: the user's choice, else the design's own.
    QColor mark() const;
    /// The user's colour for the selection mark in every design (invalid: each design's own).
    static void setMarkColor(const QColor& color);
    static QColor markColor();
    /// Round status lamp in @p color, lit or dark.
    virtual void lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const;

    /// Point on the dial's circle of @p radius for @p value (0..1).
    static QPointF dialPoint(const QRectF& rect, qreal value, qreal radius);

    // ---- QStyle ---------------------------------------------------------------------------------

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = nullptr) const override;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl sc, const QWidget* widget = nullptr) const override;
    QRect subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option = nullptr, const QWidget* widget = nullptr) const override;
    int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr, QStyleHintReturn* returnData = nullptr) const override;

    void polish(QWidget* widget) override;
    void unpolish(QWidget* widget) override;
    using QProxyStyle::polish;
    using QProxyStyle::unpolish;

private:
    Metrics m;
};
