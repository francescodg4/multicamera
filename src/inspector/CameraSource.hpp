#pragma once

#include <QList>
#include <QString>

/// Where the pictures of a camera come from.
struct CameraSource {
    enum class Kind {
        File, ///< a recording, demuxed and decoded by playbin
        Test, ///< a synthetic GStreamer test camera (videotestsrc)
    };

    // format of the test cameras
    static constexpr int TestWidth = 640;
    static constexpr int TestHeight = 360;
    static constexpr int TestFps = 25;
    static constexpr int TestSeconds = 120; ///< nominal length: the time bar of a test camera spans it

    Kind kind = Kind::Test;
    QString name; ///< "CAM 01"
    QString label; ///< what it shows: the recording's name or the test pattern
    QString location; ///< file path, or videotestsrc pattern
    QString properties; ///< extra videotestsrc properties ("horizontal-speed=2")

    static CameraSource file(const QString& path);
    static CameraSource test(const QString& pattern, const QString& properties = {});

    /// The recordings in @p videoDir (sorted by name) followed by @p testCameras synthetic
    /// cameras, numbered CAM 01, CAM 02...
    static QList<CameraSource> discover(const QString& videoDir, int testCameras);
};
