# Multicamera inspection: C++ / Qt 6 on GStreamer

A multicamera inspection system: an array of cameras, each one a GStreamer pipeline, on a board
drawn in the interface design of your choice. Click a camera to inspect it: play or pause it, move
along its time bar, and step through it picture by picture with **E**.

The cameras are the recordings in [videos/](videos/) plus synthetic GStreamer test cameras
(`videotestsrc`), which burn their buffer time into the picture so every seek and step can be
checked by eye.

## Using it

- **Control room**: play, pause or stop every camera at once, choose the board layout (automatic or
  1–4 columns), loop recordings at their end, and pick the interface **Design**.
- **Board**: each camera shows a status lamp (play / pause / stop), the time and number of the picture
  on screen. **Click** a camera to inspect it; **double click** maximises it.
- **Inspection panel** (the camera clicked): previous frame, play/pause, stop, next frame, the time bar
  (drag or click to move), the time and frame readouts, the length and frame rate.

| Key | |
|---|---|
| **1** … **9** | inspect a camera (or click it) |
| **Esc** | inspect none, show every camera |
| **Space** | play / pause |
| **E** | next frame (hold to repeat); on a playing camera it first freezes the picture |
| **Q** | previous frame |
| **Home** | go to the start |
| **S** | stop |
| **F** | maximise the camera |
| **Ctrl+P** / **Ctrl+Shift+P** / **Ctrl+.** | play / pause / stop all |
| **Ctrl+1** … **Ctrl+3** | interface design |

## Interface designs

Each design draws every widget, and the camera cards, through its own `QStyle`, following
its design rules. The choice is remembered.

- **Liquid Glass** (`glass`): frosted cards over an ambient canvas, clay buttons, a radiant glow round
  the camera under inspection.
- **Emerald** (`emerald`): the GBA storage-box system: pixel-grid cards under box banners, CRT
  screens, and the glove cursor pointing at the selected box.
- **Winamp** (`winamp`): brushed metal, cobalt LCD glass, LED status lamps, the selected module lit
  in LCD blue.

## GStreamer backbone

Every camera is a `CameraPipeline`:

```
recording:  playbin uri=file://…  (video only)  →  videoconvert ! video/x-raw,format=BGRx ! appsink
test:       videotestsrc pattern=… ! 640x360@25 ! timeoverlay ! videoconvert ! BGRx ! appsink
```

- Pictures are copied into a `QImage` on the streaming thread and announced to the GUI with a queued
  `frameReady()` (at most one pending: the GUI always takes the latest picture).
- **Play / pause / stop** map to PLAYING / PAUSED / READY; the bus is polled from the Qt event loop.
- **Seeking** is flushing and accurate, aimed at a picture of the frame grid.
- **Next frame** is a GStreamer step event (1 buffer) on the paused pipeline. **Previous frame** is an
  accurate seek to the picture before (GStreamer cannot step backwards).
- Picture times are put back on the frame grid (accurate seeks clip the timestamp of the picture
  overlapping the target), so the frame numbers stay exact.

## Build & run

The development environment is the Docker image of [.devcontainer/](.devcontainer/): Qt 6.4 and
GStreamer 1.24 with the MP4 demuxer (plugins-good) and the H.264 decoder (libav). Open the folder in
the dev container, or build the image yourself:

```bash
docker build -t qt6-gstreamer:v1.0 .devcontainer
docker run --rm -it -v "$PWD:/work" -w /work qt6-gstreamer:v1.0 bash
```

Then:

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/inspector                       # the recordings in ./videos and 4 test cameras
./build/inspector --theme winamp --select 1
```

| `inspector` option | |
|---|---|
| `--theme <id>` | design: `glass`, `emerald`, `winamp` (default: the last one selected) |
| `--videos <dir>` | folder of the recordings (default: `./videos`) |
| `--test-cameras <n>` | test cameras after the recordings (default 4) |
| `--columns <n>` | columns of the board (default: automatic) |
| `--select <n>` | camera to inspect at start (1 is the first) |
| `--paused` | do not start the cameras |
| `--screenshot <dir>` | save `inspector-<theme>.png` and quit; `--theme all` saves every design; works with `-platform offscreen` |

## Code

```
src/
  inspector/
    CameraSource        what a camera shows: a recording or a test pattern; discovery of videos/
    CameraPipeline      the GStreamer pipeline of one camera and its player controls
    CameraTile          one camera on the board, drawn by the design
    CameraGrid          the board: layout, selection, maximised camera
    InspectionWindow    control room, board, inspection panel, menus and shortcuts
    main.cpp            command line, screenshot mode
  WidgetStyle           base QStyle of the designs and their drawing vocabulary
  ThemeRegistry         the list of designs, applying one, the remembered choice
  Icons                 vector icons (transport glyphs included)
  themes/<id>/          one folder per design (Entry, Style, Theme tokens, drawing helpers)
tests/
  tst_CameraPipeline    states, accurate seeks, frame steps on test cameras and a recording
  tst_InspectionWindow  the workflow: click a camera, E / Q, time bar, maximise, designs
```
