#include "Glass.hpp"

#include "themes/glass/Theme.hpp"

#include <QConicalGradient>
#include <QElapsedTimer>
#include <QHash>
#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace glass {

namespace Glass {

namespace {

    // backdrop-filter blur per elevation (px, applied at quarter resolution)
    constexpr qreal LevelBlur[] = { 12, 18, 24 };
    constexpr int Downscale = 4;

    QElapsedTimer& clock()
    {
        static QElapsedTimer timer;
        if (!timer.isValid()) {
            timer.start();
        }
        return timer;
    }

    QPainterPath roundedRect(const QRectF& rect, qreal radius)
    {
        QPainterPath path;
        const qreal r = std::min(radius, std::min(rect.width(), rect.height()) / 2);
        path.addRoundedRect(rect, r, r);
        return path;
    }

    /// One horizontal or vertical box pass of radius @p r from @p src into @p dst.
    void boxPass(const QImage& src, QImage& dst, int r, bool horizontal)
    {
        const int w = src.width(), h = src.height();
        const int n = horizontal ? w : h;
        const int lines = horizontal ? h : w;
        const int span = 2 * r + 1;
        std::vector<QRgb> in(n);
        for (int line = 0; line < lines; ++line) {
            for (int i = 0; i < n; ++i) {
                in[i] = horizontal ? reinterpret_cast<const QRgb*>(src.constScanLine(line))[i]
                                   : reinterpret_cast<const QRgb*>(src.constScanLine(i))[line];
            }
            int sa = 0, sr = 0, sg = 0, sb = 0;
            for (int i = -r; i <= r; ++i) {
                const QRgb c = in[std::clamp(i, 0, n - 1)];
                sa += qAlpha(c), sr += qRed(c), sg += qGreen(c), sb += qBlue(c);
            }
            for (int i = 0; i < n; ++i) {
                const QRgb out = qRgba(sr / span, sg / span, sb / span, sa / span);
                if (horizontal) {
                    reinterpret_cast<QRgb*>(dst.scanLine(line))[i] = out;
                } else {
                    reinterpret_cast<QRgb*>(dst.scanLine(i))[line] = out;
                }
                const QRgb add = in[std::min(i + r + 1, n - 1)];
                const QRgb sub = in[std::max(i - r, 0)];
                sa += qAlpha(add) - qAlpha(sub);
                sr += qRed(add) - qRed(sub);
                sg += qGreen(add) - qGreen(sub);
                sb += qBlue(add) - qBlue(sub);
            }
        }
    }

    /// Cache of pre-rendered effect images (shadows, highlights, glows) keyed by their parameters.
    QHash<QString, QImage>& effects()
    {
        static QHash<QString, QImage> cache;
        if (cache.size() > 400) {
            cache.clear();
        }
        return cache;
    }

    /// Outer shadow image of a rounded rect, @p pad pixels larger on every side.
    QImage dropShadow(const QSize& size, qreal radius, qreal spread, qreal sigma, const QColor& color, int pad)
    {
        QImage img(size + QSize(2 * pad, 2 * pad), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF shape = QRectF(pad, pad, size.width(), size.height()).adjusted(-spread, -spread, spread, spread);
        p.fillPath(roundedRect(shape, radius + spread), color);
        p.end();
        return blur(img, sigma);
    }

    /// Inset shadow (CSS "inset x y blur color") of a rounded rect, clipped to it.
    QImage insetShadow(const QSize& size, qreal radius, const QPointF& offset, qreal sigma, const QColor& color)
    {
        const int pad = int(std::ceil(3 * sigma + std::abs(offset.y()) + std::abs(offset.x()))) + 1;
        QImage ring(size + QSize(2 * pad, 2 * pad), QImage::Format_ARGB32_Premultiplied);
        ring.fill(Qt::transparent);
        QPainter p(&ring);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath outside;
        outside.addRect(QRectF(ring.rect()));
        p.fillPath(outside.subtracted(roundedRect(QRectF(pad, pad, size.width(), size.height()).translated(offset), radius)), color);
        p.end();
        ring = blur(ring, sigma);

        QImage out(size, QImage::Format_ARGB32_Premultiplied);
        out.fill(Qt::transparent);
        QPainter q(&out);
        q.setRenderHint(QPainter::Antialiasing);
        q.setClipPath(roundedRect(QRectF(QPointF(), QSizeF(size)), radius));
        q.drawImage(-pad, -pad, ring);
        return out;
    }

} // namespace

Settings& settings()
{
    static Settings s;
    return s;
}

qreal time()
{
    return settings().reducedMotion ? 0.0 : clock().elapsed() / 1000.0;
}

qreal pulse()
{
    return settings().reducedMotion ? 1.0 : 0.5 + 0.5 * std::sin(time() * 2 * M_PI / 1.6);
}

QImage blur(QImage image, qreal sigma)
{
    const int r = int(std::lround(sigma));
    if (r < 1 || image.isNull()) {
        return image;
    }
    image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage tmp(image.size(), QImage::Format_ARGB32_Premultiplied);
    for (int pass = 0; pass < 3; ++pass) {
        boxPass(image, tmp, r, true);
        boxPass(tmp, image, r, false);
    }
    return image;
}

// ---- backdrop ------------------------------------------------------------------------------------

Backdrop& Backdrop::instance()
{
    static Backdrop backdrop;
    return backdrop;
}

void Backdrop::render()
{
    if (!m_host || m_host->size().isEmpty()) {
        return;
    }
    const QSize size = m_host->size() / 2;
    const qreal t = time();
    m_canvas = QImage(size, QImage::Format_ARGB32_Premultiplied);
    m_canvas.fill(Theme::canvas);
    QPainter p(&m_canvas);
    p.setRenderHint(QPainter::Antialiasing);

    // brand-coloured light fields drifting slowly
    struct Blob {
        qreal x, y, r, fx, fy, phase;
    };
    const Blob blobs[] = {
        { 0.22, 0.30, 0.45, 0.11, 0.07, 0.0 },
        { 0.78, 0.25, 0.40, 0.09, 0.12, 1.7 },
        { 0.60, 0.80, 0.50, 0.07, 0.10, 3.1 },
        { 0.15, 0.85, 0.38, 0.13, 0.08, 4.4 },
    };
    for (int i = 0; i < 4; ++i) {
        const Blob& b = blobs[i];
        const QPointF c((b.x + 0.12 * std::sin(t * b.fx * 2 * M_PI + b.phase)) * size.width(),
            (b.y + 0.10 * std::cos(t * b.fy * 2 * M_PI + b.phase)) * size.height());
        const qreal radius = b.r * size.width();
        QRadialGradient g(c, radius);
        QColor inner = Theme::ambient[i];
        inner.setAlphaF(0.55f);
        QColor outer = inner;
        outer.setAlpha(0);
        g.setColorAt(0, inner);
        g.setColorAt(1, outer);
        p.fillRect(m_canvas.rect(), g);
    }

    // faint caustic bands, so refraction has something to bend
    p.setPen(QPen(QColor(200, 240, 255, 22), 2));
    for (int k = 0; k < 6; ++k) {
        QPainterPath wave;
        const qreal y0 = size.height() * (0.1 + 0.16 * k);
        wave.moveTo(0, y0);
        for (qreal x = 0; x <= size.width(); x += 12) {
            wave.lineTo(x, y0 + 9 * std::sin(x / 48.0 + t * 0.6 + k));
        }
        p.drawPath(wave);
    }

    // dot grid texture
    p.setPen(QPen(QColor(255, 255, 255, 22), 1.5, Qt::SolidLine, Qt::RoundCap));
    for (int y = 8; y < size.height(); y += 15) {
        for (int x = 8; x < size.width(); x += 15) {
            p.drawPoint(x, y);
        }
    }
    p.end();

    // one shared blurred copy per elevation (keeps the number of backdrop filters bounded)
    const QImage small = m_canvas.scaled(m_host->size() / Downscale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    for (int i = 0; i < 3; ++i) {
        m_levels[i] = blur(small, LevelBlur[i] / Downscale);
    }
}

const QImage& Backdrop::blurred(Level level) const
{
    return m_levels[int(level)];
}

qreal Backdrop::luminance(const QRectF& hostRect) const
{
    const QImage& img = m_levels[0];
    if (img.isNull()) {
        return 0;
    }
    const QRect r = QRectF(hostRect.topLeft() / Downscale, hostRect.size() / Downscale).toAlignedRect().intersected(img.rect());
    if (r.isEmpty()) {
        return 0;
    }
    qreal sum = 0;
    int count = 0;
    const int step = std::max(1, std::min(r.width(), r.height()) / 8);
    for (int y = r.top(); y <= r.bottom(); y += step) {
        const QRgb* line = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        for (int x = r.left(); x <= r.right(); x += step) {
            sum += (0.2126 * qRed(line[x]) + 0.7152 * qGreen(line[x]) + 0.0722 * qBlue(line[x])) / 255.0;
            ++count;
        }
    }
    return count ? sum / count : 0;
}

// ---- glass -----------------------------------------------------------------------------------------

void paintBackdrop(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level)
{
    const Backdrop& backdrop = Backdrop::instance();
    QWidget* host = backdrop.hostFor(widget);
    const QImage& img = backdrop.blurred(level);
    if (settings().reducedTransparency || !host || img.isNull()) {
        p.fillPath(roundedRect(rect, radius), Theme::solidSurface);
        return;
    }
    const QPointF offset = widget->mapTo(host, QPoint(0, 0));
    const QRectF hostRect(-offset, QSizeF(host->size()));
    const QPainterPath shape = roundedRect(rect, radius);

    p.save();
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setClipPath(shape, Qt::IntersectClip);
    if (level == Level::Card) {
        p.drawImage(hostRect, img);
    } else {
        // liquid glass: the rim bends the content more than the centre, like a thick lens
        const QPointF c = rect.center();
        const auto lens = [&](qreal k) {
            p.drawImage(QRectF(c + (hostRect.topLeft() - c) * k, hostRect.size() * k), img);
        };
        lens(1.10);
        const qreal bevel = std::min(10.0, std::min(rect.width(), rect.height()) / 4);
        p.setClipPath(roundedRect(rect.adjusted(bevel, bevel, -bevel, -bevel), radius - bevel), Qt::IntersectClip);
        lens(1.035);
    }
    p.restore();
}

void paintTint(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level)
{
    const QPainterPath shape = roundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), radius);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    if (settings().reducedTransparency) {
        p.strokePath(shape, QPen(QColor(255, 255, 255, 60), 1));
        p.restore();
        return;
    }

    // adaptive opacity: brighter backdrop -> denser tint, keeping text contrast
    const Backdrop& backdrop = Backdrop::instance();
    qreal lum = 0.25;
    if (QWidget* host = backdrop.hostFor(widget)) {
        lum = backdrop.luminance(QRectF(widget->mapTo(host, rect.topLeft().toPoint()), rect.size()));
    }
    const qreal base = level == Level::Card ? 0.40 : 0.34;
    QColor tint = Theme::glassTint;
    tint.setAlphaF(float(std::clamp(base + (lum - 0.25) * 0.9, 0.28, 0.75)));
    p.fillPath(shape, tint);

    p.strokePath(shape, QPen(Theme::glassBorder, 1));
    QLinearGradient light(rect.topLeft(), rect.bottomRight());
    light.setColorAt(0, Theme::glassHighlight);
    light.setColorAt(0.45, QColor(255, 255, 255, 0));
    p.strokePath(shape, QPen(QBrush(light), 1));
    p.restore();
}

void paintSurface(QPainter& p, const QWidget* widget, const QRectF& rect, qreal radius, Level level)
{
    paintBackdrop(p, widget, rect, radius, level);
    paintTint(p, widget, rect, radius, level);
}

// ---- clay & radiance -------------------------------------------------------------------------------

QRectF scaled(const QRectF& rect, qreal factor)
{
    QRectF r(0, 0, rect.width() * factor, rect.height() * factor);
    r.moveCenter(rect.center());
    return r;
}

void paintClay(QPainter& p, const QRectF& rect, qreal radius, const QColor& base, bool pressed)
{
    const QRectF r = pressed ? scaled(rect, Theme::pressedScale) : rect;
    const QSize size = r.size().toSize();
    if (size.isEmpty()) {
        return;
    }
    radius = std::min(radius, std::min(r.width(), r.height()) / 2);
    const QString key = QStringLiteral("%1x%2r%3").arg(size.width()).arg(size.height()).arg(radius);
    auto& cache = effects();

    // box-shadow: 0 10px 20px -5px rgba(0,0,0,0.08)
    constexpr int ShadowPad = 32;
    const QString shadowKey = QStringLiteral("shadow") + key;
    if (!cache.contains(shadowKey)) {
        cache.insert(shadowKey, dropShadow(size, radius, -5, 10, QColor(0, 0, 0, 20), ShadowPad));
    }
    p.drawImage(r.topLeft() + QPointF(-ShadowPad, -ShadowPad + 10), cache.value(shadowKey));

    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.fillPath(roundedRect(r, radius), base);
    p.restore();

    if (!pressed) {
        // inset 0 2px 4px rgba(255,255,255,0.7), inset 0 -2px 4px rgba(0,0,0,0.12)
        const QString lightKey = QStringLiteral("light") + key;
        const QString darkKey = QStringLiteral("dark") + key;
        if (!cache.contains(lightKey)) {
            cache.insert(lightKey, insetShadow(size, radius, QPointF(0, 2), 2, QColor(255, 255, 255, 178)));
            cache.insert(darkKey, insetShadow(size, radius, QPointF(0, -2), 2, QColor(0, 0, 0, 31)));
        }
        p.drawImage(r.topLeft(), cache.value(lightKey));
        p.drawImage(r.topLeft(), cache.value(darkKey));
    }
}

void paintGlow(QPainter& p, const QRectF& rect, qreal radius, const QColor& color, qreal intensity)
{
    if (intensity <= 0 || rect.isEmpty()) {
        return;
    }
    // box-shadow: 0 0 24px -2px color
    constexpr int Pad = 40;
    const QSize size = rect.size().toSize();
    radius = std::min(radius, std::min(rect.width(), rect.height()) / 2);
    const QString key = QStringLiteral("glow%1x%2r%3c%4").arg(size.width()).arg(size.height()).arg(radius).arg(color.rgba(), 0, 16);
    auto& cache = effects();
    if (!cache.contains(key)) {
        cache.insert(key, dropShadow(size, radius, -2, 12, color, Pad));
    }
    p.save();
    p.setOpacity(p.opacity() * std::clamp(intensity, 0.0, 1.0));
    p.drawImage(rect.topLeft() - QPointF(Pad, Pad), cache.value(key));
    p.restore();
}

void paintSweep(QPainter& p, const QRectF& rect, qreal radius, const QColor& color)
{
    QConicalGradient sweep(rect.center(), -std::fmod(time() * 240.0, 360.0));
    QColor clear = color;
    clear.setAlpha(0);
    sweep.setColorAt(0, color);
    sweep.setColorAt(0.35, clear);
    sweep.setColorAt(0.65, clear);
    sweep.setColorAt(1, color);
    const QPainterPath edge = roundedRect(rect.adjusted(1, 1, -1, -1), radius);
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setOpacity(p.opacity() * 0.35);
    p.strokePath(edge, QPen(QBrush(sweep), 7)); // soft halo standing in for the mask blur
    p.setOpacity(1);
    p.strokePath(edge, QPen(QBrush(sweep), 2));
    p.restore();
}

// ---- card ------------------------------------------------------------------------------------------

GlassCard::GlassCard(Level level, QWidget* parent)
    : QWidget(parent)
    , m_level(level)
{
}

void GlassCard::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    paintSurface(p, this, QRectF(rect()), Theme::radiusCard, m_level);
}

} // namespace Glass

} // namespace glass
