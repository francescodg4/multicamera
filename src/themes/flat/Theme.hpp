#pragma once

#include <QColor>
#include <QFont>

namespace flat {

/// Tokens of the flat dark operations console: navy hairline panels on a black canvas, muted labels
/// and light values, one signal colour (detection green). A single, low-glare dark look.
namespace Theme {

// canvas & surfaces
inline const QColor canvas { 0x00, 0x00, 0x00 }; ///< --flat-canvas: window, gutters
inline const QColor panel { 0x0a, 0x11, 0x1b }; ///< --flat-panel
inline const QColor sunken { 0x08, 0x0b, 0x12 }; ///< --flat-panel-sunken: console, fields, screens
inline const QColor panelBorder { 0x16, 0x20, 0x2c }; ///< --flat-panel-border: 1px round each panel
inline const QColor divider { 0x12, 0x1c, 0x27 }; ///< --flat-divider: rows, title separators
inline const QColor rowHover { 0x0f, 0x1a, 0x26 }; ///< --flat-row-hover
inline const QColor selected { 0x14, 0x22, 0x31 }; ///< --flat-selected: selected tab, row
inline const QColor selectedLine { 0x5e, 0x7c, 0x96 }; ///< --flat-selected-line: tab indicator, focus

// buttons
inline const QColor button { 0x10, 0x1a, 0x26 };
inline const QColor buttonPressed { 0x1a, 0x2c, 0x40 };
inline const QColor buttonDisabledBorder { 0x10, 0x18, 0x21 };
inline const QColor fieldHover { 0x24, 0x32, 0x4a };

// text
inline const QColor textTitle { 0xc8, 0xd0, 0xda }; ///< --flat-text-title
inline const QColor text { 0xc0, 0xc4, 0xca }; ///< --flat-text: values, body
inline const QColor textMuted { 0x88, 0x8f, 0x99 }; ///< --flat-text-muted: headers, keys, legends
inline const QColor textTab { 0x82, 0x89, 0x91 }; ///< unselected tabs
inline const QColor textSelected { 0xa8, 0xbc, 0xcf }; ///< --flat-text-selected
inline const QColor textDisabled { 0x4a, 0x52, 0x60 }; ///< --flat-text-disabled
inline const QColor textOverlay { 0xe9, 0xe5, 0xe4 }; ///< --flat-text-overlay: timestamps on pictures
inline const QColor textChip { 0xe8, 0xe8, 0xe8 }; ///< overlay caption chips, tooltips

// the signal: tracked objects and the primary action, never decoration
inline const QColor detect { 0x5c, 0xd6, 0x6b }; ///< --flat-detect: bounding box, badge border
inline const QColor detectFill { 0x3f, 0x8f, 0x55 }; ///< --flat-detect-fill: badge, primary action
inline const QColor detectHover { 0x4a, 0x9d, 0x61 };
inline const QColor detectPressed { 0x35, 0x7a, 0x48 };
inline const QColor detectDisabled { 0x1c, 0x2a, 0x22 };
inline const QColor detectText { 0xe3, 0xfd, 0xee }; ///< --flat-detect-text

// geometry
inline constexpr qreal radius = 2; ///< --flat-radius: panels, chips, buttons
inline constexpr int gutter = 6; ///< --flat-gutter: canvas between panels

/// --flat-font-ui (Segoe UI, Noto Sans, DejaVu Sans), in pixels.
QFont font(int pixelSize = 13, int weight = QFont::Normal);
/// --flat-font-mono (Consolas, DejaVu Sans Mono), in pixels.
QFont mono(int pixelSize = 12);

} // namespace Theme

} // namespace flat
