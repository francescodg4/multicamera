#include "themes/metro/Theme.hpp"

#include <QStringList>

namespace metro::Theme {

const Palette& palette(bool dark)
{
    static const Palette light {
        false,
        QColor(0xf2, 0xf2, 0xf2), // canvas
        QColor(0xff, 0xff, 0xff), // surface
        QColor(0x1d, 0x1d, 0x1d), // text
        QColor(0x5d, 0x5d, 0x5d),
        QColor(0x8a, 0x8a, 0x8a), // border
        QColor(0xd4, 0xd4, 0xd4), // line
        QColor(0xc6, 0xc6, 0xc6), // track
    };
    static const Palette night {
        true,
        QColor(0x1d, 0x1d, 0x1d),
        QColor(0x2b, 0x2b, 0x2b),
        QColor(0xff, 0xff, 0xff),
        QColor(0xa6, 0xa6, 0xa6),
        QColor(0x80, 0x80, 0x80),
        QColor(0x3d, 0x3d, 0x3d),
        QColor(0x4d, 0x4d, 0x4d),
    };
    return dark ? night : light;
}

QFont font(qreal pointSize, int weight)
{
    QFont f(QStringList { QStringLiteral("Segoe UI"), QStringLiteral("Selawik"), QStringLiteral("Open Sans"), QStringLiteral("Inter"), QStringLiteral("DejaVu Sans") });
    f.setPointSizeF(pointSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

} // namespace metro::Theme
