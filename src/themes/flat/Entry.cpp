#include "ThemeRegistry.hpp"
#include "themes/flat/Style.hpp"

#include <QCoreApplication>

namespace flat {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("flat"),
        QCoreApplication::translate("Themes", "Flat"),
        QCoreApplication::translate("Themes", "Flat dark operations console: navy hairline panels on a black canvas, muted labels and light values, detection green for the camera under inspection."),
        [](bool) -> WidgetStyle* { return new Style; },
    };
}

} // namespace flat
