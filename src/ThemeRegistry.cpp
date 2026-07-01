#include "ThemeRegistry.hpp"

#include "WidgetStyle.hpp"

#include <QApplication>
#include <QFont>
#include <QSettings>

// each theme registers itself through one factory function
namespace glass {
ThemeEntry themeEntry();
}
namespace metro {
ThemeEntry themeEntry();
}
namespace revolut {
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
        revolut::themeEntry(),
        winamp::themeEntry(),
        metro::themeEntry(),
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

bool apply(const QString& id, bool dark)
{
    const ThemeEntry* theme = find(id);
    if (!theme) {
        return false;
    }
    WidgetStyle* style = theme->widgetStyle(dark);
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

bool savedDark()
{
    return QSettings().value(QStringLiteral("dark"), true).toBool();
}

void saveDark(bool dark)
{
    QSettings().setValue(QStringLiteral("dark"), dark);
}

QString Look::name() const
{
    const ThemeEntry* theme = find(id);
    return theme && theme->modes ? QStringLiteral("%1-%2").arg(id, dark ? QStringLiteral("dark") : QStringLiteral("light")) : id;
}

QList<Look> looks(const QString& id, bool dark, bool bothModes)
{
    QList<Look> result;
    for (const ThemeEntry& theme : all()) {
        if (id != QLatin1String("all") && theme.id != id) {
            continue;
        }
        if (theme.modes && bothModes) {
            result << Look { theme.id, true } << Look { theme.id, false };
        } else {
            result << Look { theme.id, dark };
        }
    }
    return result;
}

} // namespace Themes
