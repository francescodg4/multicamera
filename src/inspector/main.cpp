#include "ThemeRegistry.hpp"
#include "inspector/CameraGrid.hpp"
#include "inspector/CameraPipeline.hpp"
#include "inspector/InspectionWindow.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QElapsedTimer>
#include <QTimer>

namespace {

/// Runs the event loop for @p ms, as the application would live.
void settle(int ms)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < ms) {
        QApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

/// The videos folder: the working directory's, the one next to the executable, or the source tree's.
QString findVideos()
{
    const QString app = QApplication::applicationDirPath();
    for (const QString& dir : { QDir::current().filePath(QStringLiteral("videos")), app + QStringLiteral("/videos"), app + QStringLiteral("/../videos"), QStringLiteral(INSPECTOR_VIDEOS_DIR) }) {
        if (QDir(dir).exists()) {
            return QDir(dir).absolutePath();
        }
    }
    return {};
}

} // namespace

int main(int argc, char* argv[])
{
    CameraPipeline::initialize(argc, argv); // takes the --gst-* options
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("sparapinz"));
    QApplication::setApplicationName(QStringLiteral("MulticamInspection"));
    QApplication::setApplicationVersion(QStringLiteral(INSPECTOR_VERSION));

    QStringList ids;
    for (const ThemeEntry& theme : Themes::all()) {
        ids << theme.id;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Multicamera inspection system on a GStreamer backbone"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption themeOption("theme", QStringLiteral("Interface to use: %1 (default: the last one selected).").arg(ids.join(QStringLiteral(", "))), "id");
    const QCommandLineOption videosOption("videos", "Folder of the recordings shown as cameras (default: ./videos).", "dir");
    const QCommandLineOption testOption("test-cameras", "Number of GStreamer test cameras added after the recordings (default: 4).", "n", "4");
    const QCommandLineOption columnsOption("columns", "Columns of the camera board (default: automatic).", "n", "0");
    const QCommandLineOption selectOption("select", "Camera to inspect at start (1 is the first).", "n");
    const QCommandLineOption pausedOption("paused", "Do not start the cameras.");
    const QCommandLineOption screenshotOption("screenshot", "Save the window as inspector-<theme>.png in <dir> and quit; --theme all saves every interface.", "dir");
    parser.addOptions({ themeOption, videosOption, testOption, columnsOption, selectOption, pausedOption, screenshotOption });
    parser.process(app);

    const QString requested = parser.value(themeOption);
    if (!requested.isEmpty() && requested != QLatin1String("all") && !Themes::find(requested)) {
        qWarning("Unknown theme '%s' (available: %s)", qPrintable(requested), qPrintable(ids.join(QStringLiteral(", "))));
        return 1;
    }
    const QStringList themes = requested == QLatin1String("all") ? ids : QStringList { requested.isEmpty() ? Themes::saved() : requested };

    const QStringList missing = CameraPipeline::missingElements();
    if (!missing.isEmpty()) {
        qWarning("GStreamer elements not installed: %s", qPrintable(missing.join(QStringLiteral(", "))));
    }

    const QString videos = parser.isSet(videosOption) ? parser.value(videosOption) : findVideos();
    const QList<CameraSource> sources = CameraSource::discover(videos, std::max(0, parser.value(testOption).toInt()));
    if (sources.isEmpty()) {
        qWarning("No cameras: no recordings in '%s' and no test cameras", qPrintable(videos));
        return 1;
    }

    InspectionWindow window(sources);
    window.setTheme(themes.first());
    window.grid()->setColumns(parser.value(columnsOption).toInt());
    window.show();
    if (!parser.isSet(pausedOption)) {
        window.playAll();
    }
    if (parser.isSet(selectOption)) {
        window.select(parser.value(selectOption).toInt() - 1);
    }

    if (parser.isSet(screenshotOption)) {
        QTimer::singleShot(0, &window, [&] {
            settle(2500); // let every camera preroll and play a little
            const QDir dir(parser.value(screenshotOption));
            dir.mkpath(QStringLiteral("."));
            for (const QString& id : themes) {
                window.setTheme(id);
                settle(500);
                window.grab().save(dir.filePath(QStringLiteral("inspector-%1.png").arg(id)));
            }
            QApplication::quit();
        });
    }
    return app.exec();
}
