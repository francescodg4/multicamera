#pragma once

#include <QColor>
#include <QFont>
#include <QRectF>
#include <QString>

class QPainter;

namespace winamp {

/// Drawing vocabulary of the Winamp Modern skin: brushed-metal frames with a 1px bevel,
/// inset cobalt LCD glass with scanlines, capsule and round transport buttons, glowing telemetry.
namespace Metal {

QFont uiFont(qreal pointSize = 9, bool bold = false); ///< --font-ui (Tahoma / Segoe UI)
QFont digitalFont(int pixelSize = 12, bool bold = false); ///< --font-digital (Courier New / Consolas)

/// Metal frame: --winamp-frame-bg gradient with the metal bevel.
void frame(QPainter& p, const QRectF& rect, qreal radius = 6);
/// --winamp-metal-bevel: white top/left inner line, #404858 bottom/right inner line (swapped when sunken).
void bevel(QPainter& p, const QRectF& rect, qreal radius, bool raised = true);
/// Recessed panel with --shadow-inset-panel (inset 2px 2px 4px rgba(0,0,0,.6)).
void insetPanel(QPainter& p, const QRectF& rect, const QColor& fill, qreal radius = 6);
/// Illuminated LCD glass: inset panel in --winamp-lcd-bg with scanlines.
void lcd(QPainter& p, const QRectF& rect, qreal radius = 6);
/// Title bar: --winamp-titlebar-bg with horizontal reflection ridges.
void titleBar(QPainter& p, const QRectF& rect);
/// Horizontal ridged capsule (the grooves either side of the title).
void ridge(QPainter& p, const QRectF& rect);

enum class Glyph { None, Rewind, Play, Pause, Stop, Forward, Eject, Back, Minimize, Maximize, Close };
void glyph(QPainter& p, const QRectF& rect, Glyph glyph, const QColor& color);

/// Pill button (--border-radius-button); active = blue accent.
void capsule(QPainter& p, const QRectF& rect, bool pressed, bool active, bool hover);
/// Circular bevel-edged metallic transport button.
void roundButton(QPainter& p, const QRectF& rect, bool pressed, bool active, bool hover);

/// 7-segment digits (and ':').
void sevenSegment(QPainter& p, const QRectF& rect, const QString& text, const QColor& on, const QColor& off);
/// LCD text with a soft glow.
void glowText(QPainter& p, const QRectF& rect, int flags, const QString& text, const QColor& color);
/// Static dot-matrix LED line (a line too long for it ends with an ellipsis).
void dotMatrix(QPainter& p, const QRectF& rect, const QString& text, const QColor& on, const QColor& off);
/// Round indicator LED.
void led(QPainter& p, const QPointF& centre, qreal radius, const QColor& color, bool lit);
/// Slider groove with its filled part, and the metallic capsule thumb.
void sliderTrack(QPainter& p, const QRectF& track, qreal fill);
void sliderThumb(QPainter& p, const QRectF& thumb, bool pressed);

} // namespace Metal

} // namespace winamp
