#pragma once

#include <QList>
#include <QString>

class WidgetStyle;

/// One selectable design of the interface.
struct ThemeEntry {
    QString id; ///< used on the command line and in the settings
    QString name;
    QString description;
    WidgetStyle* (*widgetStyle)(bool dark); ///< the standard widgets in this design (in its dark or light mode)
    bool modes = false; ///< has a light and a dark mode; the others have a single look
};

namespace Themes {

const QList<ThemeEntry>& all();
const ThemeEntry* find(const QString& id);

/// Makes theme @p id, in its dark or light mode, the application's style, palette and font;
/// false if there is no such theme.
bool apply(const QString& id, bool dark);

/// The theme chosen last time (or the first one).
QString saved();
void save(const QString& id);

/// The mode chosen last time for the themes that have two (dark at first).
bool savedDark();
void saveDark(bool dark);

/// A theme in one mode.
struct Look {
    QString id;
    bool dark = true;
    QString name() const; ///< "glass", or "revolut-dark" for a theme with two modes
};
/// The looks of theme @p id ("all": of every theme): both modes of a theme with two when
/// @p bothModes, else the @p dark one.
QList<Look> looks(const QString& id, bool dark, bool bothModes);

} // namespace Themes
