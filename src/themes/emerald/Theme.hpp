#pragma once

#include <QColor>

namespace emerald {

/// Indexed palette of the GBA "storage box" interface (Pokémon Emerald box system).
namespace Theme {

// skill tokens
inline const QColor primaryGreen { 0x38, 0x98, 0x88 };
inline const QColor darkGreen { 0x18, 0x50, 0x48 };
inline const QColor cream { 0xf8, 0xf8, 0xe0 };
inline const QColor borderDark { 0x28, 0x28, 0x28 };
inline const QColor lcdBlue { 0x78, 0xa8, 0xc0 };
inline const QColor kbdUpperBlue { 0x80, 0xc0, 0xd8 };
inline const QColor kbdLowerOrange { 0xd8, 0x98, 0x78 };
inline const QColor kbdNumGreen { 0x90, 0xd0, 0x88 };

// supporting palette entries
inline const QColor white { 0xf8, 0xf8, 0xf8 };
inline const QColor text { 0x40, 0x40, 0x40 }; ///< dark text on light surfaces
inline const QColor textShadow { 0xd0, 0xd0, 0xc8 }; ///< its light drop shadow
inline const QColor lightText { 0xf8, 0xf8, 0xf8 }; ///< text on dark surfaces
inline const QColor lightTextShadow { 0x50, 0x50, 0x58 };
inline const QColor dimText { 0x90, 0x90, 0x88 };
inline const QColor paperLine { 0xe8, 0xe0, 0xb8 };
inline const QColor highlight { 0xf8, 0xf0, 0x90 }; ///< highlighted selection tile
inline const QColor metal { 0xa8, 0xa8, 0xb8 };
inline const QColor metalLight { 0xd8, 0xd8, 0xe0 };
inline const QColor metalDark { 0x68, 0x68, 0x78 };
inline const QColor crtDark { 0x38, 0x68, 0x88 };
inline const QColor keyYellow { 0xf0, 0xd8, 0x60 };
inline const QColor keyGray { 0xc0, 0xc0, 0xc8 };
inline const QColor badge { 0xe0, 0x50, 0x40 };

} // namespace Theme

} // namespace emerald
