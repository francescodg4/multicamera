#pragma once

#include <QColor>
#include <QImage>
#include <QPointer>
#include <QWidget>

class QPainter;

namespace glass {

/// Material system of the interface (see the design rules in the project brief):
/// liquid / frosted glass over a live ambient canvas, tactile clay controls and radiant glows.
namespace Glass {

/// Elevation of a glass layer: higher layers blur more (backdrop blur 12 / 18 / 24 px).
enum class Level {
    Card, ///< bento cards: frosted diffusion layer
    Bar, ///< floating tab bar: liquid glass
    Modal, ///< overlays such as the photo viewer: liquid glass
};

/// Accessibility switches (the equivalent of prefers-reduced-motion / -transparency).
struct Settings {
    bool reducedMotion = false;
    bool reducedTransparency = false;
};
Settings& settings();

/// Seconds since start (frozen at 0 with reduced motion).
qreal time();

/// Slow 0..1 pulse used by radiant "active" states (1 with reduced motion).
qreal pulse();

/// The ambient canvas under every glass layer, in host (window) coordinates.
/// Rendered once per frame; the blurred copies are shared by all surfaces.
class Backdrop {
public:
    static Backdrop& instance();

    void setHost(QWidget* host) { m_host = host; }
    QWidget* host() const { return m_host; }
    /// The host if @p widget is inside it, else null (a widget of another window must not sample it).
    QWidget* hostFor(const QWidget* widget) const { return m_host && widget && widget->window() == m_host ? m_host.data() : nullptr; }

    /// Re-renders the canvas for the host size at the current time.
    void render();

    const QImage& canvas() const { return m_canvas; } ///< half resolution
    const QImage& blurred(Level level) const; ///< quarter resolution
    qreal luminance(const QRectF& hostRect) const; ///< 0..1, of the blurred backdrop

private:
    QPointer<QWidget> m_host;
    QImage m_canvas;
    QImage m_levels[3];
};

/// Gaussian-like blur (three box passes) of a premultiplied image.
QImage blur(QImage image, qreal sigma);

/// Blurred, refracted backdrop of @p widget's area, clipped to the rounded rect.
void paintBackdrop(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level);
/// Adaptive tint plus the low-contrast border with its top/left highlight.
void paintTint(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level);
/// paintBackdrop + paintTint.
void paintSurface(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level);

/// Tactile clay surface: soft outer shadow, inner light and dark passes.
/// While pressed it scales down to 0.98 and its inner highlights flatten.
void paintClay(QPainter& p, const QRectF& rect, qreal radius, const QColor& base, bool pressed = false);

/// Soft exterior luminescence (box-shadow: 0 0 24px -2px color), scaled by @p intensity.
void paintGlow(QPainter& p, const QRectF& rect, qreal radius, const QColor& color, qreal intensity = 1.0);

/// Rotating conic gradient edge with a blurred halo, for active processing.
void paintSweep(QPainter& p, const QRectF& rect, qreal radius, const QColor& color);

/// Rect scaled about its centre (pressed controls use Theme::pressedScale).
QRectF scaled(const QRectF& rect, qreal factor);

/// Frosted bento card.
class GlassCard : public QWidget {
public:
    explicit GlassCard(Level level = Level::Card, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Level m_level;
};

} // namespace Glass

} // namespace glass
