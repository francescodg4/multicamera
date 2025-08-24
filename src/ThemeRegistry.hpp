#pragma once

#include <QList>
#include <QString>

class WidgetStyle;

/// One selectable design of the interface.
struct ThemeEntry {
    QString id; ///< used on the command line and in the settings
    QString name;
    QString description;
    WidgetStyle* (*widgetStyle)(); ///< the standard widgets in this design
};

namespace Themes {

const QList<ThemeEntry>& all();
const ThemeEntry* find(const QString& id);

/// Makes theme @p id the application's style, palette and font; false if there is no such theme.
bool apply(const QString& id);

/// The theme chosen last time (or the first one).
QString saved();
void save(const QString& id);

} // namespace Themes
