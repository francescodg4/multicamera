#include "inspector/CameraSource.hpp"

#include <QDir>
#include <QFileInfo>

CameraSource CameraSource::file(const QString& path)
{
    CameraSource source;
    source.kind = Kind::File;
    source.label = QFileInfo(path).completeBaseName();
    source.location = QFileInfo(path).absoluteFilePath();
    return source;
}

CameraSource CameraSource::test(const QString& pattern, const QString& properties)
{
    CameraSource source;
    source.kind = Kind::Test;
    source.label = QStringLiteral("Test · %1").arg(pattern);
    source.location = pattern;
    source.properties = properties;
    return source;
}

QList<CameraSource> CameraSource::discover(const QString& videoDir, int testCameras)
{
    QList<CameraSource> sources;
    if (!videoDir.isEmpty()) {
        const QDir dir(videoDir);
        const QStringList filters = { QStringLiteral("*.mp4"), QStringLiteral("*.mkv"), QStringLiteral("*.mov"), QStringLiteral("*.avi"), QStringLiteral("*.webm") };
        for (const QString& name : dir.entryList(filters, QDir::Files, QDir::Name)) {
            sources << file(dir.filePath(name));
        }
    }

    // test patterns that move, so playing, seeking and stepping show on screen
    static const struct {
        const char* pattern;
        const char* properties;
    } patterns[] = {
        { "ball", "motion=wavy" },
        { "smpte", "horizontal-speed=2" },
        { "zone-plate", "kx2=20 ky2=20 kt=1" },
        { "pinwheel", "horizontal-speed=3" },
        { "snow", "" },
        { "circular", "horizontal-speed=1" },
    };
    for (int i = 0; i < testCameras; ++i) {
        const auto& p = patterns[i % std::size(patterns)];
        sources << test(QString::fromLatin1(p.pattern), QString::fromLatin1(p.properties));
    }

    for (int i = 0; i < sources.size(); ++i) {
        sources[i].name = QStringLiteral("CAM %1").arg(i + 1, 2, 10, QLatin1Char('0'));
    }
    return sources;
}
