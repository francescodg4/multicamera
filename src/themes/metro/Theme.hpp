#pragma once

#include <QColor>
#include <QFont>

namespace metro {

/// Tokens of the Windows 8 "Metro" / Modern UI: flat accent tiles on a dark or light canvas, no
/// shadows, no rounded corners, Segoe UI.
namespace Theme {

/// Colours of one mode.
struct Palette {
    bool dark = false;
    QColor canvas; ///< --metro-bg-dark / --metro-bg-light
    QColor surface; ///< panels, views and menus: the one step of framing Metro allows
    QColor text;
    QColor text2; ///< secondary text
    QColor border; ///< stroke of fields, check boxes, buttons at rest
    QColor line; ///< hairlines
    QColor track; ///< empty part of sliders and progress
};

const Palette& palette(bool dark);

// same in both modes
inline const QColor tileText { 0xff, 0xff, 0xff }; ///< --metro-tile-fg
inline const QColor accent { 0x00, 0xa4, 0xef }; ///< teal / cyan: primary accent, default focus
inline const QColor charms { 0x11, 0x11, 0x11 }; ///< the charms bar black: screens and displays
inline const QColor displayText { 0x00, 0xb7, 0xc3 }; ///< --metro-teal

/// Live tile colours, one per tile in turn.
inline const QColor tiles[] = {
    { 0x00, 0xb7, 0xc3 }, // teal
    { 0xd8, 0x00, 0x73 }, // magenta
    { 0x7e, 0x38, 0xb7 }, // purple
    { 0xe3, 0xa2, 0x1a }, // orange
    { 0x10, 0x7c, 0x41 }, // green
    { 0xe5, 0x14, 0x00 }, // red
    { 0x1b, 0xa1, 0xe2 }, // blue
};

/// --metro-font-family (Segoe UI, with open look-alikes behind it).
QFont font(qreal pointSize = 10, int weight = QFont::Normal);

} // namespace Theme

} // namespace metro
