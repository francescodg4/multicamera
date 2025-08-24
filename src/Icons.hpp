#pragma once

#include <QColor>
#include <QIcon>
#include <QPainterPath>
#include <QPixmap>
#include <QStringView>

class QPainter;

/// Vector icons drawn with QPainter (no image assets needed).
enum class Icon {
    // tab bar
    Person,
    Wrench,
    Pin,
    Image,
    Doc,
    Book,
    // interface glyphs
    Eye,
    EyeOff,
    Play,
    Pause,
    Stop,
    FramePrevious,
    FrameNext,
    ChevronRight,
    ChevronLeft,
    Close,
    Plus,
    Trash,
    LogDoc,
    // items
    Titanium,
    Ingot,
    Mesh,
    Glass,
    Bottle,
    Drop,
    Fiber,
    Aerogel,
    Scanner,
    Flashlight,
    Repair,
    Knife,
    Battery,
    Fabricator,
    // equipment
    Diver,
    Mask,
    Chip,
    Tank,
    Gloves,
    Fins,
    Compass,
    // pings
    Lifepod,
    Seamoth,
    Signal,
    Beacon,
    // theme selection
    Palette,
};

namespace Icons {

/// Paints @p icon centred in @p rect, keeping its aspect ratio. @p color is used by the
/// monochrome icons and as the accent of the coloured ones (e.g. the liquid of a bottle).
void paint(QPainter& painter, Icon icon, const QRectF& rect, const QColor& color = Qt::white);

QPixmap pixmap(Icon icon, const QSize& size, const QColor& color = Qt::white);
QIcon icon(Icon icon, const QColor& color = Qt::white);

/// Parses SVG path data (M L H V C S Q T Z, absolute and relative; no arcs).
QPainterPath svgPath(QStringView data);

} // namespace Icons
