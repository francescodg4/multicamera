#include "themes/flat/Theme.hpp"

#include <QStringList>

namespace flat::Theme {

QFont font(int pixelSize, int weight)
{
    QFont f(QStringList { QStringLiteral("Segoe UI"), QStringLiteral("Noto Sans"), QStringLiteral("DejaVu Sans") });
    f.setPixelSize(pixelSize);
    f.setWeight(QFont::Weight(weight));
    return f;
}

QFont mono(int pixelSize)
{
    QFont f(QStringList { QStringLiteral("Consolas"), QStringLiteral("DejaVu Sans Mono") });
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(pixelSize);
    return f;
}

} // namespace flat::Theme
