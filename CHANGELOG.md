# Changelog

## 1.0.0 — 2026-07-07

The first release of the multicamera inspection system.

### Inspection
- A board of cameras, each a GStreamer pipeline: the recordings of a folder plus synthetic test
  cameras that burn their time into the picture.
- Click a camera to inspect it: previous / next picture, play / pause, stop, a time bar and the
  time and number of the picture on screen, on the camera's own card.
- Accurate seeks and frame steps (forward with **E**, back with **Q**), exact on the frame grid.
- Double click (or **F**) shows a camera alone: the control room steps aside and the camera keeps
  its controls, without the selection mark.
- Play, pause or stop every camera at once; a board of 1–4 columns or automatic; looping recordings.

### Interface designs
- Five designs, chosen in the control room (**Ctrl+1** … **Ctrl+5**) and remembered: Liquid
  Glass, Revolut (light and dark), Metro (dark and light), Flat (a slate operations console) and
  Winamp.
- **Highlight**: the colour of the mark round the camera under inspection, picked by the user or
  left to each design.

### Packages
- Debian package and tarball for Linux (`cmake --build build --target package`), with the
  inspector, its menu entry and this documentation.

### Development
- Dev container with Qt 6, GStreamer 1.24 and Catch2; tests on Catch2 for the pipelines and the
  inspection workflow, run headless.
