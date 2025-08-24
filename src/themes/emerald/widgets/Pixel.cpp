#include "Pixel.hpp"

#include "themes/emerald/Theme.hpp"

#include <QFontMetrics>
#include <QHash>
#include <QWidget>

#include <cmath>

namespace emerald {

namespace Pixel {

namespace {

    void px(QPainter& p, int x, int y, const QColor& c) { p.fillRect(x, y, 1, 1, c); }

    /// Fills a shape row by row: cut(d) = columns removed at distance d from the top/bottom edge.
    template <typename Cut>
    void rows(QPainter& p, const QRect& r, const QColor& fill, const QColor& border, Cut cut)
    {
        const int x = r.left(), y = r.top(), right = r.right(), bottom = r.bottom();
        for (int yy = y; yy <= bottom; ++yy) {
            const int d = std::min(yy - y, bottom - yy);
            const int c = cut(d);
            if (d == 0) {
                p.fillRect(x + c, yy, right - x - 2 * c + 1, 1, border);
                continue;
            }
            // edge pixels: keep the outline continuous where the cut steps by more than one pixel
            const int prev = cut(d - 1);
            p.fillRect(x + c, yy, std::max(1, prev - c + 1), 1, border);
            p.fillRect(right - c - std::max(0, prev - c), yy, std::max(1, prev - c + 1), 1, border);
            const int inner0 = x + c + std::max(1, prev - c + 1);
            const int inner1 = right - c - std::max(1, prev - c + 1);
            if (inner1 >= inner0) {
                p.fillRect(inner0, yy, inner1 - inner0 + 1, 1, fill);
            }
        }
    }

    void pill(QPainter& p, const QRect& r, const QColor& fill, const QColor& border)
    {
        const double radius = r.height() / 2.0;
        rows(p, r, fill, border, [radius](int d) {
            const double dy = radius - d - 0.5;
            return d >= radius ? 0 : int(std::lround(radius - std::sqrt(std::max(0.0, radius * radius - dy * dy))));
        });
    }

    // ---- wallpapers ------------------------------------------------------------------------------

    struct Paper {
        QColor base, pattern, sky, ground;
    };

    Paper paper(Wallpaper w)
    {
        switch (w) {
        case Wallpaper::Forest: return { { 0x88, 0xc8, 0x60 }, { 0x68, 0xa8, 0x48 }, { 0xa8, 0xd8, 0xf8 }, { 0x48, 0x98, 0x40 } };
        case Wallpaper::City: return { { 0xc0, 0xb8, 0xc8 }, { 0xa8, 0xa0, 0xb8 }, { 0xc8, 0xd8, 0xf0 }, { 0x78, 0x78, 0x90 } };
        case Wallpaper::Desert: return { { 0xd8, 0xb8, 0x78 }, { 0xc8, 0xa0, 0x60 }, { 0xf0, 0xe0, 0xb0 }, { 0xc0, 0x98, 0x58 } };
        case Wallpaper::Savannah: return { { 0xa8, 0xc8, 0x58 }, { 0x90, 0xb0, 0x40 }, { 0xb8, 0xe0, 0xf8 }, { 0x88, 0xb0, 0x38 } };
        case Wallpaper::Crag: return { { 0xa8, 0xa8, 0xa8 }, { 0x90, 0x90, 0x90 }, { 0xc8, 0xd0, 0xd8 }, { 0x70, 0x70, 0x78 } };
        case Wallpaper::Volcano: return { { 0xe0, 0x78, 0x50 }, { 0xc8, 0x58, 0x38 }, { 0xf0, 0xb0, 0x90 }, { 0x98, 0x40, 0x30 } };
        case Wallpaper::Snow: return { { 0xe8, 0xf0, 0xf8 }, { 0xc0, 0xd8, 0xf0 }, { 0xd0, 0xe8, 0xf8 }, { 0xf8, 0xf8, 0xf8 } };
        case Wallpaper::Cave: return { { 0xa8, 0x88, 0x58 }, { 0x88, 0x68, 0x38 }, { 0x68, 0x58, 0x48 }, { 0x88, 0x68, 0x40 } };
        case Wallpaper::Beach: return { { 0xf0, 0xd8, 0x88 }, { 0xe0, 0xc0, 0x68 }, { 0x88, 0xd0, 0xf0 }, { 0xf0, 0xd8, 0x88 } };
        case Wallpaper::Seafloor: return { { 0x38, 0x68, 0xc0 }, { 0x60, 0x90, 0xe0 }, { 0x20, 0x40, 0x88 }, { 0xc0, 0x40, 0x48 } };
        case Wallpaper::River: return { { 0x88, 0xd8, 0xd0 }, { 0x68, 0xc0, 0xb8 }, { 0x70, 0xc8, 0x58 }, { 0x58, 0xa8, 0xe0 } };
        case Wallpaper::Sky: return { { 0x78, 0xb0, 0xf0 }, { 0xa8, 0xd0, 0xf8 }, { 0x58, 0x98, 0xe8 }, { 0xf8, 0xf8, 0xf8 } };
        case Wallpaper::PolkaDot: return { { 0xb8, 0xe0, 0xf8 }, { 0x90, 0xc8, 0xf0 }, { 0xb8, 0xe0, 0xf8 }, { 0x90, 0xc8, 0xf0 } };
        case Wallpaper::Stripes: return { { 0xf0, 0xa8, 0x90 }, { 0xe8, 0x90, 0x78 }, { 0xf0, 0xa8, 0x90 }, { 0xf8, 0xf8, 0xf8 } };
        case Wallpaper::Metallic: return { { 0xb8, 0xb8, 0xc0 }, { 0xa0, 0xa0, 0xa8 }, { 0xd0, 0xd0, 0xd8 }, { 0x88, 0x88, 0x98 } };
        case Wallpaper::Simple: return { { 0xe8, 0xe8, 0xe8 }, { 0xd8, 0xd8, 0xd8 }, { 0xf0, 0xf0, 0xf0 }, { 0xd0, 0xd0, 0xd0 } };
        }
        return { Qt::gray, Qt::darkGray, Qt::lightGray, Qt::gray };
    }

    /// One 16×16 low-contrast tile of the wallpaper pattern at (x, y).
    void tile(QPainter& p, int x, int y, Wallpaper w, const QColor& c)
    {
        switch (w) {
        case Wallpaper::Forest:
        case Wallpaper::Savannah: // grass tufts
            for (const QPoint o : { QPoint(3, 5), QPoint(11, 12) }) {
                px(p, x + o.x() - 1, y + o.y(), c);
                px(p, x + o.x(), y + o.y() - 1, c);
                px(p, x + o.x() + 1, y + o.y(), c);
            }
            break;
        case Wallpaper::City: // herringbone
            for (int i = 0; i < 8; ++i) {
                px(p, x + i, y + i, c);
                px(p, x + 8 + i, y + 7 - i, c);
            }
            break;
        case Wallpaper::Desert:
        case Wallpaper::Beach:
        case Wallpaper::River: // waves
            for (int i = 0; i < 16; ++i) {
                px(p, x + i, y + 6 + int(std::lround(1.5 * std::sin(i * M_PI / 8))), c);
            }
            break;
        case Wallpaper::Crag:
        case Wallpaper::Cave: // pebbles
            p.fillRect(x + 2, y + 3, 3, 2, c);
            p.fillRect(x + 10, y + 10, 4, 2, c);
            p.fillRect(x + 11, y + 2, 2, 2, c);
            break;
        case Wallpaper::Volcano:
        case Wallpaper::Snow:
        case Wallpaper::PolkaDot: // dots
            p.fillRect(x + 3, y + 3, 2, 2, c);
            p.fillRect(x + 11, y + 11, 2, 2, c);
            break;
        case Wallpaper::Seafloor: // bubbles
            p.fillRect(x + 4, y + 3, 2, 1, c);
            p.fillRect(x + 3, y + 4, 1, 2, c);
            p.fillRect(x + 6, y + 4, 1, 2, c);
            p.fillRect(x + 4, y + 6, 2, 1, c);
            p.fillRect(x + 12, y + 11, 2, 2, c);
            break;
        case Wallpaper::Sky: // cloud puffs
            p.fillRect(x + 3, y + 5, 6, 2, c);
            p.fillRect(x + 5, y + 4, 3, 1, c);
            break;
        case Wallpaper::Stripes:
            p.fillRect(x + 4, y, 3, 16, c);
            break;
        case Wallpaper::Metallic: // rivets and brushed lines
            px(p, x + 2, y + 2, c);
            px(p, x + 13, y + 13, c);
            p.fillRect(x, y + 8, 16, 1, c);
            break;
        case Wallpaper::Simple: // faint grid
            p.fillRect(x, y + 15, 16, 1, c);
            p.fillRect(x + 15, y, 1, 16, c);
            break;
        }
    }

    // ---- glove cursor ------------------------------------------------------------------------------

    const char* const GloveDown[] = {
        ".KKKKKKKK..",
        ".KYYYYYYK..",
        ".KYYYYYYK..",
        "KKKKKKKKKK.",
        "KWWKWWKWWK.",
        "KWWKWWKWWK.",
        "KWWWWWWWWK.",
        ".KWWWWWWKG.",
        "..KWWWWKG..",
        "...KWWKG...",
        "...KWWKG...",
        "...KWWK....",
        "....KK.....",
    };

    struct Glove {
        QImage image;
        QPoint tip;
    };

    const Glove& glove(Direction d)
    {
        static const QHash<int, Glove> gloves = [] {
            QImage down(11, 13, QImage::Format_ARGB32_Premultiplied);
            down.fill(Qt::transparent);
            for (int y = 0; y < 13; ++y) {
                for (int x = 0; x < 11; ++x) {
                    switch (GloveDown[y][x]) {
                    case 'K': down.setPixelColor(x, y, Theme::borderDark); break;
                    case 'W': down.setPixelColor(x, y, Theme::white); break;
                    case 'Y': down.setPixelColor(x, y, Theme::keyYellow); break;
                    case 'G': down.setPixelColor(x, y, QColor(0x88, 0x88, 0x88)); break;
                    default: break;
                    }
                }
            }
            const QImage right = down.transformed(QTransform().rotate(-90));
            QHash<int, Glove> map;
            map.insert(int(Direction::Down), { down, { 4, 12 } });
            map.insert(int(Direction::Up), { down.mirrored(false, true), { 4, 0 } });
            map.insert(int(Direction::Right), { right, { right.width() - 1, 6 } });
            map.insert(int(Direction::Left), { right.mirrored(true, false), { 0, 6 } });
            return map;
        }();
        return *gloves.constFind(int(d));
    }

} // namespace

// ---- canvas & text -------------------------------------------------------------------------------

QFont font(bool heading)
{
    QFont f(QStringLiteral("DejaVu Sans Mono"));
    f.setPixelSize(heading ? 10 : 9);
    f.setBold(heading);
    f.setStyleStrategy(QFont::NoAntialias);
    f.setHintingPreference(QFont::PreferFullHinting);
    return f;
}

Canvas::Canvas(QWidget* widget)
    : m_widget(widget)
    , m_image((widget->width() + Scale - 1) / Scale, (widget->height() + Scale - 1) / Scale, QImage::Format_ARGB32_Premultiplied)
{
    m_image.fill(Qt::transparent);
    m_painter.begin(&m_image);
    m_painter.setRenderHint(QPainter::Antialiasing, false);
    m_painter.setFont(font());
}

Canvas::Canvas(QPainter& target, const QRect& deviceRect)
    : m_target(&target)
    , m_origin(deviceRect.topLeft())
    , m_image(std::max(1, deviceRect.width() / Scale), std::max(1, deviceRect.height() / Scale), QImage::Format_ARGB32_Premultiplied)
{
    m_image.fill(Qt::transparent);
    m_painter.begin(&m_image);
    m_painter.setRenderHint(QPainter::Antialiasing, false);
    m_painter.setFont(font());
}

Canvas::~Canvas()
{
    m_painter.end();
    const QRect target(m_origin, m_image.size() * Scale);
    if (m_target) {
        m_target->save();
        m_target->setRenderHint(QPainter::SmoothPixmapTransform, false);
        m_target->drawImage(target, m_image);
        m_target->restore();
    } else {
        QPainter out(m_widget);
        out.drawImage(target, m_image); // nearest-neighbour: image-rendering: pixelated
    }
}

void text(QPainter& p, const QRect& rect, int flags, const QString& str, const QColor& color, const QColor& shadow)
{
    p.save();
    p.setPen(shadow);
    p.drawText(rect.translated(1, 0), flags, str);
    p.drawText(rect.translated(0, 1), flags, str);
    p.drawText(rect.translated(1, 1), flags, str);
    p.setPen(color);
    p.drawText(rect, flags, str);
    p.restore();
}

int textHeight(const QString& str, int width, bool heading)
{
    return QFontMetrics(font(heading)).boundingRect(QRect(0, 0, std::max(1, width), 100000), Qt::TextWordWrap, str).height() + 1;
}

// ---- surfaces ------------------------------------------------------------------------------------

void chamfer(QPainter& p, const QRect& rect, const QColor& fill, const QColor& border, int corner)
{
    rows(p, rect, fill, border, [corner](int d) { return std::max(0, corner - d); });
}

void wallpaper(QPainter& p, const QRect& rect, Wallpaper theme)
{
    const Paper w = paper(theme);
    chamfer(p, rect, w.base, w.pattern.darker(220), 3);
    chamfer(p, rect.adjusted(1, 1, -1, -1), w.base, w.base.lighter(125), 2); // bi-colour frame
    const QRect inside = rect.adjusted(2, 2, -2, -2);
    p.save();
    p.setClipRect(inside);
    for (int y = inside.top(); y <= inside.bottom(); y += 16) {
        for (int x = inside.left(); x <= inside.right(); x += 16) {
            tile(p, x, y, theme, w.pattern);
        }
    }
    p.restore();
}

void banner(QPainter& p, const QRect& rect, Wallpaper theme, const QString& caption)
{
    const Paper w = paper(theme);
    pill(p, rect, w.sky, Theme::borderDark);

    // landscape art inside the pill, masked by the pill shape
    QImage art(rect.size(), QImage::Format_ARGB32_Premultiplied);
    art.fill(Qt::transparent);
    {
        QPainter a(&art);
        const int h = art.height(), wd = art.width();
        a.fillRect(0, 0, wd, h, w.sky);
        a.fillRect(0, h * 2 / 3, wd, h, w.ground);
        for (int x = 0; x < wd; x += 12) { // horizon bumps: trees, dunes, rocks or clouds
            const int bump = 2 + (x / 12) % 3;
            a.fillRect(x + 2, h * 2 / 3 - bump, 8, bump, w.ground.darker(115));
        }
        a.fillRect(0, h * 2 / 3, wd, 1, w.ground.lighter(120));
        a.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        pill(a, art.rect(), Qt::black, Qt::black);
    }
    p.drawImage(rect.topLeft(), art);
    pill(p, rect, Qt::transparent, Theme::borderDark);

    if (!caption.isEmpty()) {
        p.save();
        p.setFont(font(true));
        const int tw = QFontMetrics(p.font()).horizontalAdvance(caption) + 12;
        const QRect plate(rect.center().x() - tw / 2, rect.top() + 2, tw, rect.height() - 4);
        chamfer(p, plate, Theme::cream, Theme::borderDark, 2);
        text(p, plate.adjusted(0, 0, 0, -1), Qt::AlignCenter, caption, Theme::text, Theme::textShadow);
        p.restore();
    }
}

QRect dataWindow(QPainter& p, const QRect& rect, const QString& title)
{
    chamfer(p, rect, Theme::metal, Theme::borderDark, 3);
    p.fillRect(rect.left() + 2, rect.top() + 1, rect.width() - 4, 1, Theme::metalLight); // bevel
    p.fillRect(rect.left() + 2, rect.bottom() - 1, rect.width() - 4, 1, Theme::metalDark);

    const QRect plate(rect.left() + 4, rect.top() + 3, rect.width() - 8, 11);
    chamfer(p, plate, Theme::metalDark, Theme::borderDark, 2);
    p.save();
    p.setFont(font(true));
    text(p, plate.adjusted(4, 0, 0, 0), Qt::AlignVCenter | Qt::AlignLeft, title, Theme::lightText, Theme::borderDark);
    p.restore();

    const QRect screen = rect.adjusted(6, 18, -6, -6);
    crtFrame(p, screen);
    return screen;
}

void crtFrame(QPainter& p, const QRect& screen)
{
    chamfer(p, screen.adjusted(-2, -2, 2, 2), Theme::lcdBlue, Theme::borderDark, 2);
    p.fillRect(screen, Theme::crtDark);
}

void scanlines(QPainter& p, const QRect& screen)
{
    for (int y = screen.top() + 1; y <= screen.bottom(); y += 2) {
        p.fillRect(screen.left(), y, screen.width(), 1, QColor(0, 0, 32, 56));
    }
}

void textBox(QPainter& p, const QRect& rect)
{
    chamfer(p, rect, Theme::cream, Theme::borderDark, 2);
    chamfer(p, rect.adjusted(2, 2, -2, -2), Theme::cream, Theme::lcdBlue, 1); // double-line border
    for (int y = rect.top() + 10; y < rect.bottom() - 3; y += 8) {
        p.fillRect(rect.left() + 4, y, rect.width() - 8, 1, Theme::paperLine); // lined paper
    }
}

void deck(QPainter& p, const QRect& rect)
{
    chamfer(p, rect, Theme::primaryGreen, Theme::borderDark, 3);
    p.fillRect(rect.left() + 3, rect.top() + 1, rect.width() - 6, 1, Theme::primaryGreen.lighter(130));
    p.fillRect(rect.left() + 1, rect.top() + 3, 1, rect.height() - 6, Theme::primaryGreen.lighter(130));
}

void deckSlot(QPainter& p, const QRect& rect, bool filled, bool selected)
{
    const QColor fill = selected ? Theme::highlight : filled ? Theme::darkGreen : Theme::primaryGreen.darker(125);
    chamfer(p, rect, fill, selected ? Theme::borderDark : Theme::darkGreen.darker(160), 2);
    if (!selected) {
        p.fillRect(rect.left() + 2, rect.top() + 1, rect.width() - 4, 1, fill.lighter(140));
    }
}

void highlightTile(QPainter& p, const QRect& rect)
{
    chamfer(p, rect, Theme::highlight, QColor(0xd0, 0xb0, 0x40), 2);
}

void key(QPainter& p, const QRect& rect, const QColor& face, const QString& label, bool pressed, bool enabled)
{
    const QColor f = enabled ? face : Theme::keyGray;
    const QRect body = rect.adjusted(0, 0, -1, -1);
    if (!pressed) {
        pill(p, body.translated(1, 1), Theme::borderDark, Theme::borderDark); // hard pixel shadow
    }
    const QRect top = pressed ? body.translated(1, 1) : body;
    pill(p, top, f, Theme::borderDark);
    if (!pressed) {
        p.fillRect(top.left() + top.height() / 2, top.top() + 2, top.width() - top.height(), 1, f.lighter(135)); // inner highlight line
        p.fillRect(top.left() + top.height() / 2, top.bottom() - 2, top.width() - top.height(), 1, f.darker(125));
    }
    text(p, top.adjusted(0, 0, 0, -1), Qt::AlignCenter, label, enabled ? Theme::text : Theme::dimText, enabled ? f.lighter(140) : f);
}

int keyWidth(const QString& label)
{
    return QFontMetrics(font()).horizontalAdvance(label) + 16;
}

// ---- sprites -------------------------------------------------------------------------------------

void hand(QPainter& p, const QPoint& tip, Direction direction)
{
    const Glove& g = glove(direction);
    p.drawImage(tip - g.tip, g.image);
}

void arrow(QPainter& p, const QRect& rect, Direction direction, const QColor& fill, const QColor& shade)
{
    // drawn pointing right, then turned
    const bool vertical = direction == Direction::Up || direction == Direction::Down;
    const int w = vertical ? rect.height() : rect.width();
    const int h = vertical ? rect.width() : rect.height();
    QImage img(w, h, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    const double mid = (h - 1) / 2.0;
    for (int x = 0; x < w; ++x) {
        const double half = mid * (1.0 - double(x) / w);
        const int y0 = int(std::ceil(mid - half)), y1 = int(std::floor(mid + half));
        for (int y = y0; y <= y1; ++y) {
            const bool edge = y == y0 || y == y1 || x == 0;
            img.setPixelColor(x, y, edge ? Theme::borderDark : (y <= mid ? fill : shade));
        }
    }
    QTransform t;
    if (direction == Direction::Left) {
        img = img.mirrored(true, false);
    } else if (direction == Direction::Down) {
        img = img.transformed(t.rotate(90));
    } else if (direction == Direction::Up) {
        img = img.transformed(t.rotate(-90));
    }
    p.drawImage(rect.topLeft(), img);
}

void arrow(QPainter& p, const QRect& rect, Direction direction)
{
    arrow(p, rect, direction, Theme::white, Theme::kbdUpperBlue);
}

QImage sprite(Icon icon, int size, const QColor& accent)
{
    static QHash<QString, QImage> cache;
    const QString key = QStringLiteral("%1/%2/%3").arg(int(icon)).arg(size).arg(accent.rgba(), 0, 16);
    if (auto it = cache.constFind(key); it != cache.constEnd()) {
        return *it;
    }
    QImage img(size, size, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    {
        QPainter p(&img);
        Icons::paint(p, icon, QRectF(1, 1, size - 2, size - 2), accent);
    }
    // hard alpha, GBA 15-bit colour
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            const QRgb c = img.pixel(x, y);
            img.setPixel(x, y, qAlpha(c) < 110 ? 0 : qRgba(qRed(c) & 0xf8, qGreen(c) & 0xf8, qBlue(c) & 0xf8, 255));
        }
    }
    // 1px dark outline around the sprite
    QImage out = img.copy();
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (qAlpha(img.pixel(x, y))) {
                continue;
            }
            const auto solid = [&](int xx, int yy) { return xx >= 0 && yy >= 0 && xx < size && yy < size && qAlpha(img.pixel(xx, yy)); };
            if (solid(x - 1, y) || solid(x + 1, y) || solid(x, y - 1) || solid(x, y + 1)) {
                out.setPixelColor(x, y, Theme::borderDark);
            }
        }
    }
    cache.insert(key, out);
    return out;
}

QImage pixelate(const QImage& image, const QSize& lowRes)
{
    QImage small = image.scaled(lowRes, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < small.height(); ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(small.scanLine(y));
        for (int x = 0; x < small.width(); ++x) {
            line[x] = qRgb(qRed(line[x]) & 0xf8, qGreen(line[x]) & 0xf8, qBlue(line[x]) & 0xf8);
        }
    }
    return small;
}

} // namespace Pixel

} // namespace emerald
