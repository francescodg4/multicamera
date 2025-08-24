#include "ThemeRegistry.hpp"
#include "themes/emerald/Style.hpp"

#include <QCoreApplication>

namespace emerald {

ThemeEntry themeEntry()
{
    return {
        QStringLiteral("emerald"),
        QCoreApplication::translate("Themes", "Emerald"),
        QCoreApplication::translate("Themes", "GBA-era storage box: pixel grid, box banners, capsule keys, lined-paper fields and an HP-bar progress."),
        []() -> WidgetStyle* { return new Style; },
    };
}

} // namespace emerald
