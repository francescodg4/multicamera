#include "ThemeRegistry.hpp"
#include "themes/metro/Style.hpp"

#include <QCoreApplication>

namespace metro {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("metro"),
        QCoreApplication::translate("Themes", "Metro"),
        QCoreApplication::translate("Themes", "Windows 8 Modern UI: flat live tiles in accent colours, Segoe typography, square stroke controls, no shadows, on a dark or light canvas."),
        [](bool dark) -> WidgetStyle* { return new Style(dark); },
        true,
    };
}

} // namespace metro
