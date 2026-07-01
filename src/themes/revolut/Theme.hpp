#pragma once

#include <QColor>
#include <QFont>

namespace revolut {

/// Tokens of the Revolut.com interface in the Idetica identity (violet palette), light and dark.
namespace Theme {

/// Colours of one mode.
struct Palette {
    bool dark = false;
    QColor canvas; ///< page background
    QColor card; ///< neutral bento cards
    QColor widget; ///< app widgets: fields, views, displays, menus
    QColor chip; ///< secondary buttons: violet-grey on light, translucent white on dark
    QColor chipHover;
    QColor text; ///< primary / secondary / tertiary text
    QColor text2;
    QColor text3;
    QColor accent; ///< brand violet: primary buttons, active states
    QColor accentHover;
    QColor tint; ///< the accent's soft tint
    QColor line; ///< hairlines
    QColor track; ///< empty part of sliders
};

const Palette& palette(bool dark);

// same in both modes
inline const QColor focus { 0xf5, 0x9e, 0x0b }; ///< focus ring: amber
inline const QColor onAccent { 0xff, 0xff, 0xff }; ///< text on violet

// geometry (--rui-radius-*)
inline constexpr qreal radiusCard = 24; ///< r24: cards, popups
inline constexpr qreal radiusWidget = 16; ///< r16: widgets
inline constexpr qreal radiusField = 12; ///< r12
inline constexpr qreal radiusItem = 8; ///< r8

/// Inter (the brand font), with the system sans behind it.
QFont font(qreal pointSize = 10, int weight = QFont::Normal);
/// Emphasis 4: the small uppercase eyebrow over a card.
QFont eyebrowFont();

} // namespace Theme

} // namespace revolut
