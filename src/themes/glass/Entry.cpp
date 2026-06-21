#include "ThemeRegistry.hpp"
#include "themes/glass/Style.hpp"

#include <QCoreApplication>

namespace glass {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("glass"),
        QCoreApplication::translate("Themes", "Liquid Glass"),
        QCoreApplication::translate("Themes", "Frosted cards over a live ambient canvas, recessed glass fields, clay controls and radiant highlights."),
        [](bool) -> WidgetStyle* { return new Style; },
    };
}

} // namespace glass
