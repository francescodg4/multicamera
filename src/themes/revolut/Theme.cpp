#include "themes/revolut/Theme.hpp"

#include <QStringList>

namespace revolut::Theme {

namespace {

    QColor hsl(int h, int s, int l, int a = 255)
    {
        QColor c = QColor::fromHslF(h / 360.0f, s / 100.0f, l / 100.0f);
        c.setAlpha(a);
        return c;
    }

} // namespace

const Palette& palette(bool dark)
{
    static const Palette light {
        false,
        QColor(0xfd, 0xfb, 0xff), // canvas
        hsl(266, 60, 97), // cards
        QColor(0xff, 0xff, 0xff), // widgets
        hsl(266, 34, 91), // chip
        hsl(266, 34, 86),
        QColor(0x1a, 0x0f, 0x33), // text
        QColor(0x4a, 0x41, 0x66),
        QColor(0x6f, 0x66, 0x8a),
        QColor(154, 81, 248), // accent
        hsl(266, 80, 50),
        hsl(266, 100, 96), // tint
        QColor(0x1a, 0x0f, 0x33, 26), // line
        hsl(266, 30, 88), // track
    };
    static const Palette night {
        true,
        QColor(0x07, 0x06, 0x0d),
        QColor(0x13, 0x11, 0x1f),
        QColor(0x1c, 0x19, 0x30),
        QColor(255, 255, 255, 31), // rgba(255, 255, 255, 0.12)
        QColor(255, 255, 255, 48),
        QColor(0xf1, 0xed, 0xf9),
        QColor(0xa9, 0xa2, 0xbf),
        QColor(0x8a, 0x83, 0xa1),
        hsl(266, 100, 68),
        hsl(266, 100, 76),
        hsl(266, 40, 14),
        QColor(255, 255, 255, 20),
        QColor(255, 255, 255, 36),
    };
    return dark ? night : light;
}

QFont font(qreal pointSize, int weight)
{
    QFont f(QStringList { QStringLiteral("Inter"), QStringLiteral("Segoe UI"), QStringLiteral("DejaVu Sans") });
    f.setPointSizeF(pointSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

QFont eyebrowFont()
{
    QFont f = font(8, QFont::Bold);
    f.setCapitalization(QFont::AllUppercase);
    f.setLetterSpacing(QFont::AbsoluteSpacing, 0.9);
    return f;
}

} // namespace revolut::Theme
