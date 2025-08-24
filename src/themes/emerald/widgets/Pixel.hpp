#pragma once

#include "Icons.hpp"

#include <QColor>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QString>

class QWidget;

namespace emerald {

/// GBA-style pixel rendering: everything is drawn on a low-resolution grid (one "pixel" is
/// Scale device pixels), without antialiasing, and scaled up with nearest-neighbour sampling.
namespace Pixel {

constexpr int Scale = 2; ///< --pixel-scaling: integer scale for crisp pixels

/// Monospace pixel font (DejaVu Sans Mono rasterised without antialiasing).
QFont font(bool heading = false);

/// Paints a widget at 1/Scale resolution and blits it pixelated when destroyed.
class Canvas {
public:
    explicit Canvas(QWidget* widget);
    Canvas(QPainter& target, const QRect& deviceRect); ///< for item delegates
    ~Canvas();
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;

    QPainter& p() { return m_painter; }
    QRect rect() const { return m_image.rect(); }

private:
    QPainter* m_target = nullptr;
    QWidget* m_widget = nullptr;
    QPoint m_origin;
    QImage m_image;
    QPainter m_painter;
};

enum class Direction { Up, Down, Left, Right };

/// Box wallpaper themes; each has a tiled pattern, a bi-colour frame and a header landscape.
enum class Wallpaper {
    Forest,
    City,
    Desert,
    Savannah,
    Crag,
    Volcano,
    Snow,
    Cave,
    Beach,
    Seafloor,
    River,
    Sky,
    PolkaDot,
    Stripes,
    Metallic,
    Simple,
};

/// Text with a bi-colour drop shadow (right, bottom and diagonal).
void text(QPainter& p, const QRect& rect, int flags, const QString& str, const QColor& color, const QColor& shadow);
/// Pixel height needed for wrapped text of the given width.
int textHeight(const QString& str, int width, bool heading = false);

/// Rect with chamfered corners (the pixel version of a rounded rectangle).
void chamfer(QPainter& p, const QRect& rect, const QColor& fill, const QColor& border, int corner = 2);

/// Wallpaper container: bi-colour frame and a tiled texture fill.
void wallpaper(QPainter& p, const QRect& rect, Wallpaper theme);
/// Thin, pill-cornered landscape banner (optionally with a caption).
void banner(QPainter& p, const QRect& rect, Wallpaper theme, const QString& caption = {});

/// Metallic data card with a header plate; returns the CRT screen area inside it.
QRect dataWindow(QPainter& p, const QRect& rect, const QString& title);
/// Blue CRT screen with horizontal scanlines over whatever is drawn inside @p screen.
void crtFrame(QPainter& p, const QRect& screen);
void scanlines(QPainter& p, const QRect& screen);

/// Cream lined-paper box with a double-line border.
void textBox(QPainter& p, const QRect& rect);
/// Teal party deck panel and one of its slot boxes.
void deck(QPainter& p, const QRect& rect);
void deckSlot(QPainter& p, const QRect& rect, bool filled, bool selected);

/// Highlighted background tile used for the selected cell.
void highlightTile(QPainter& p, const QRect& rect);

/// Beveled capsule key: inner highlight line, hard pixel shadow; a press drops it into the shadow.
void key(QPainter& p, const QRect& rect, const QColor& face, const QString& label, bool pressed, bool enabled = true);
int keyWidth(const QString& label);
constexpr int KeyHeight = 16;

/// Pointing glove cursor; @p tip is where the finger points.
void hand(QPainter& p, const QPoint& tip, Direction direction);
/// Bi-colour arrow inside @p rect.
void arrow(QPainter& p, const QRect& rect, Direction direction, const QColor& fill, const QColor& shade);
void arrow(QPainter& p, const QRect& rect, Direction direction); ///< white with a blue shade

/// Icon turned into an outlined pixel sprite of size × size pixels.
QImage sprite(Icon icon, int size, const QColor& accent = Qt::white);
/// Image reduced to a low resolution (drawn back up it stays blocky).
QImage pixelate(const QImage& image, const QSize& lowRes);

} // namespace Pixel

} // namespace emerald
