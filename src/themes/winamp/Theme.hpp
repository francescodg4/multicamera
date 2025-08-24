#pragma once

#include <QColor>

namespace winamp {

/// Tokens of the Winamp Modern look: brushed metal frame, cobalt LCD glass, glowing telemetry.
namespace Theme {

// skill tokens
inline const QColor frameTop { 0xd8, 0xde, 0xe8 }; ///< --winamp-frame-bg (0%)
inline const QColor frameBottom { 0xa0, 0xaa, 0xb8 }; ///< --winamp-frame-bg (100%)
inline const QColor bevelLight { 0xff, 0xff, 0xff }; ///< --winamp-metal-bevel: inset 1px 1px 0 #ffffff
inline const QColor bevelDark { 0x40, 0x48, 0x58 }; ///< --winamp-metal-bevel: inset -1px -1px 0 #404858
inline const QColor lcdBg { 0x09, 0x1a, 0x38 }; ///< --winamp-lcd-bg
inline const QColor lcdGlow { 0x70, 0xb0, 0xff }; ///< --winamp-lcd-glow
inline const QColor titleTop { 0x18, 0x48, 0x88 }; ///< --winamp-titlebar-bg (0%)
inline const QColor titleBottom { 0x08, 0x18, 0x30 }; ///< --winamp-titlebar-bg (100%)
inline constexpr qreal radiusButton = 12; ///< --border-radius-button
inline constexpr qreal radiusPanel = 6; ///< --border-radius-panel

// supporting colours
inline const QColor lcdDim { 0x1e, 0x3d, 0x6e }; ///< unlit segments, grid lines
inline const QColor lcdRow { 0x14, 0x2e, 0x5c }; ///< highlighted playlist row
inline const QColor accent { 0x2f, 0x6f, 0xd8 }; ///< blue active state (Play)
inline const QColor text { 0x1c, 0x24, 0x33 }; ///< UI text on metal
inline const QColor textDim { 0x5a, 0x64, 0x78 };
inline const QColor outline { 0x6a, 0x74, 0x86 };

} // namespace Theme

} // namespace winamp
