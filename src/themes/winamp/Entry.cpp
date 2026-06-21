#include "ThemeRegistry.hpp"
#include "themes/winamp/Style.hpp"

#include <QCoreApplication>

namespace winamp {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("winamp"),
        QCoreApplication::translate("Themes", "Winamp"),
        QCoreApplication::translate("Themes", "Winamp Modern skin: brushed metal, capsule buttons, cobalt LCD glass with glowing text, LEDs and VU segments."),
        [](bool) -> WidgetStyle* { return new Style; },
    };
}

} // namespace winamp
