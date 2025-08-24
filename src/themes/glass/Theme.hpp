#pragma once

#include <QColor>
#include <QFont>

namespace glass {

/// Design tokens.
namespace Theme {

// ambient canvas
inline const QColor canvas { 0x06, 0x18, 0x2c };
inline const QColor ambient[] = {
    // brand colours drifting under the glass
    { 0x1f, 0x6f, 0xd6 },
    { 0x12, 0xb5, 0xc9 },
    { 0x5a, 0x3f, 0xd0 },
    { 0x0e, 0x8f, 0x7a },
};

// glass
inline const QColor glassTint { 15, 15, 15 }; ///< dark tint, alpha adapts around 0.4
inline const QColor glassBorder { 255, 255, 255, 20 }; ///< 1px rgba(255,255,255,0.08)
inline const QColor glassHighlight { 255, 255, 255, 38 }; ///< 1px rgba(255,255,255,0.15), top/left
inline const QColor solidSurface { 0x10, 0x22, 0x38 }; ///< reduced-transparency fallback

// clay
inline const QColor clay { 0x3d, 0x6f, 0x9e };
inline const QColor clayRecessed { 0x22, 0x40, 0x60 }; ///< empty slots
inline const QColor clayBadge { 0xf6, 0xa6, 0x23 };

// radiance
inline const QColor accent { 0x3b, 0x9b, 0xff };
inline const QColor glowAccent { 0x3b, 0x9b, 0xff, 200 };
inline const QColor glowSoft { 0x7f, 0xe6, 0xff, 130 }; ///< hover
inline const QColor glowBadge { 0xf6, 0xa6, 0x23, 150 };

// content
inline const QColor text { 0xee, 0xf6, 0xff };
inline const QColor textDim { 0xa8, 0xc4, 0xde };
inline const QColor iconMuted { 255, 255, 255, 120 };
inline const QColor cyan { 0x7f, 0xe6, 0xff };
inline const QColor gold { 0xf3, 0xc7, 0x42 };
inline const QColor green { 0x5e, 0xf0, 0xb4 };
inline const QColor glowGreen { 0x5e, 0xf0, 0xb4, 190 };

// geometry
inline constexpr qreal radiusCard = 20;
inline constexpr qreal radiusControl = 16;
inline constexpr int gap = 16;
inline constexpr qreal pressedScale = 0.98;

/// Uppercase, letter-spaced font used by section titles.
QFont titleFont();

} // namespace Theme

} // namespace glass
