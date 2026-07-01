#pragma once

#include <QColor>
#include <QIcon>

/// The transport glyphs of the player controls.
enum class Icon {
    Play,
    Pause,
    Stop,
    FramePrevious,
    FrameNext,
};

namespace Icons {

/// Vector icon drawn with QPainter in @p color (no image assets), dimmed when disabled.
QIcon icon(Icon icon, const QColor& color = Qt::white);

} // namespace Icons
