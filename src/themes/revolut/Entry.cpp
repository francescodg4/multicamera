#include "ThemeRegistry.hpp"
#include "themes/revolut/Style.hpp"

#include <QCoreApplication>

namespace revolut {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("revolut"),
        QCoreApplication::translate("Themes", "Revolut"),
        QCoreApplication::translate("Themes", "Revolut.com in the Idetica identity: flat tinted cards, violet pill buttons, app widgets, in a light and a dark mode."),
        [](bool dark) -> WidgetStyle* { return new Style(dark); },
        true,
    };
}

} // namespace revolut
