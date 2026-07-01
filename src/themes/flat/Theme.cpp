#include "themes/flat/Theme.hpp"

#include <QStringList>

namespace flat::Theme {

const Palette& palette()
{
    static const Palette slate {
        QColor(0x1b, 0x22, 0x2c), // canvas
        QColor(0x27, 0x31, 0x3f), // panel
        QColor(0x1e, 0x26, 0x31), // sunken
        QColor(0x3b, 0x48, 0x5a), // panel border
        QColor(0x35, 0x41, 0x52), // divider
        QColor(0x2e, 0x3a, 0x4b), // row hover
        QColor(0x34, 0x47, 0x5f), // selected
        QColor(0x86, 0xa8, 0xc8), // selected line
        QColor(0x2f, 0x3b, 0x4c), // button
        QColor(0x3d, 0x50, 0x69), // button pressed
        QColor(0x2c, 0x36, 0x44), // button disabled border
        QColor(0x55, 0x66, 0x7e), // field hover
        QColor(0xf0, 0xf3, 0xf7), // text title
        QColor(0xe3, 0xe7, 0xec), // text
        QColor(0xb0, 0xb9, 0xc5), // text muted
        QColor(0xd2, 0xe2, 0xf2), // text selected
        QColor(0x6b, 0x77, 0x88), // text disabled
        QColor(0xf3, 0xf0, 0xef), // text overlay
        QColor(0xf2, 0xf2, 0xf2), // text chip
        QColor(0x5c, 0xb4, 0xf5), // signal
        QColor(0x2f, 0x6f, 0xa3), // signal fill
        QColor(0x38, 0x7e, 0xb6), // signal hover
        QColor(0x29, 0x61, 0x8e), // signal pressed
        QColor(0x2a, 0x36, 0x46), // signal disabled
        QColor(0xee, 0xf7, 0xff), // signal text
    };
    return slate;
}

QFont font(int pixelSize, int weight)
{
    QFont f(QStringList { QStringLiteral("Segoe UI"), QStringLiteral("Noto Sans"), QStringLiteral("DejaVu Sans") });
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

} // namespace flat::Theme
