#include "ThemeRegistry.hpp"

#include "WidgetStyle.hpp"

#include <QApplication>
#include <QFont>
#include <QSettings>

// each theme registers itself through one factory function
namespace glass {
ThemeEntry themeEntry();
}
namespace emerald {
ThemeEntry themeEntry();
}
namespace winamp {
ThemeEntry themeEntry();
}

namespace Themes {

const QList<ThemeEntry>& all()
{
    static const QList<ThemeEntry> themes = {
        glass::themeEntry(),
        emerald::themeEntry(),
        winamp::themeEntry(),
    };
    return themes;
}

const ThemeEntry* find(const QString& id)
{
    for (const ThemeEntry& theme : all()) {
        if (theme.id == id) {
            return &theme;
        }
    }
    return nullptr;
}

bool apply(const QString& id)
{
    const ThemeEntry* theme = find(id);
    if (!theme) {
        return false;
    }
    WidgetStyle* style = theme->widgetStyle();
    QApplication::setStyle(style); // the application owns it (and deletes the previous one)
    QApplication::setPalette(style->standardPalette());
    QApplication::setFont(style->font());
    return true;
}

QString saved()
{
    const QString id = QSettings().value(QStringLiteral("theme")).toString();
    return find(id) ? id : all().first().id;
}

void save(const QString& id)
{
    QSettings().setValue(QStringLiteral("theme"), id);
}

} // namespace Themes
