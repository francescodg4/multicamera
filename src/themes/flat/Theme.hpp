#pragma once

#include <QColor>
#include <QFont>

namespace flat {

/// Tokens of the flat slate operations console: lighter navy hairline panels on a slate canvas,
/// muted labels and light values, one signal colour (azure) in a cool family.
namespace Theme {

/// The colours of the design.
struct Palette {
    // canvas & surfaces
    QColor canvas; ///< --flat-canvas: window, gutters
    QColor panel; ///< --flat-panel
    QColor sunken; ///< --flat-panel-sunken: console, fields, screens
    QColor panelBorder; ///< --flat-panel-border: 1px round each panel
    QColor divider; ///< --flat-divider: rows, title separators
    QColor rowHover; ///< --flat-row-hover
    QColor selected; ///< --flat-selected: selected tab, row
    QColor selectedLine; ///< --flat-selected-line: tab indicator, focus

    // buttons
    QColor button;
    QColor buttonPressed;
    QColor buttonDisabledBorder;
    QColor fieldHover;

    // text
    QColor textTitle; ///< --flat-text-title
    QColor text; ///< --flat-text: values, body
    QColor textMuted; ///< --flat-text-muted: headers, keys, legends
    QColor textSelected; ///< --flat-text-selected
    QColor textDisabled; ///< --flat-text-disabled
    QColor textOverlay; ///< --flat-text-overlay: timestamps on pictures
    QColor textChip; ///< overlay caption chips, tooltips

    // the signal: tracked objects and the primary action, never decoration
    QColor signal; ///< --flat-signal: bounding box, badge border, check marks
    QColor signalFill; ///< --flat-signal-fill: badge, primary action
    QColor signalHover;
    QColor signalPressed;
    QColor signalDisabled;
    QColor signalText; ///< --flat-signal-text
};

const Palette& palette();

// geometry
inline constexpr qreal radius = 2; ///< --flat-radius: panels, chips, buttons
inline constexpr int gutter = 6; ///< --flat-gutter: canvas between panels

/// --flat-font-ui (Segoe UI, Noto Sans, DejaVu Sans), in pixels.
QFont font(int pixelSize = 13, int weight = QFont::Normal);

} // namespace Theme

} // namespace flat
