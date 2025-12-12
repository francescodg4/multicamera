#include "inspector/CameraPipeline.hpp"

#include <QDir>
#include <QSignalSpy>
#include <QTest>

namespace {

constexpr qint64 Ms = 1'000'000; ///< ns
constexpr qint64 TestFrame = 1'000'000'000 / CameraSource::TestFps;

} // namespace

/// The GStreamer backbone: state changes, accurate seeks and frame stepping, on a synthetic
/// test camera (exact frame times) and on a recording from the videos folder.
class TestCameraPipeline : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        int argc = 1;
        char name[] = "tst_camerapipeline";
        char* args[] = { name, nullptr };
        char** argv = args;
        CameraPipeline::initialize(argc, argv);
        QVERIFY2(CameraPipeline::missingElements().isEmpty(), qPrintable(CameraPipeline::missingElements().join(u", ")));
    }

    void timecode()
    {
        QCOMPARE(CameraPipeline::timecode(0), QStringLiteral("00:00.000"));
        QCOMPARE(CameraPipeline::timecode(83'480 * Ms), QStringLiteral("01:23.480"));
        QCOMPARE(CameraPipeline::timecode(3'600'000 * Ms), QStringLiteral("1:00:00.000"));
        QCOMPARE(CameraPipeline::timecode(-5 * Ms), QStringLiteral("00:00.000"));
    }

    void discover()
    {
        const QList<CameraSource> sources = CameraSource::discover(QString(), 3);
        QCOMPARE(sources.size(), 3);
        QCOMPARE(sources[0].name, QStringLiteral("CAM 01"));
        QCOMPARE(sources[2].name, QStringLiteral("CAM 03"));
        QCOMPARE(sources[0].kind, CameraSource::Kind::Test);
    }

    void stoppedCameraHasNoPicture()
    {
        CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
        QCOMPARE(camera.state(), CameraPipeline::State::Stopped);
        QVERIFY(camera.frame().isNull());
        QCOMPARE(camera.duration(), qint64(CameraSource::TestSeconds) * 1000 * Ms);
    }

    void stepForwardShowsTheNextPicture()
    {
        CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
        camera.stepForward(); // a stopped camera pauses on its first picture
        QCOMPARE(camera.state(), CameraPipeline::State::Paused);
        QTRY_VERIFY(!camera.frame().isNull());
        QCOMPARE(camera.frame().size(), QSize(CameraSource::TestWidth, CameraSource::TestHeight));
        QCOMPARE(camera.position(), qint64(0));
        QCOMPARE(camera.frameDuration(), TestFrame);

        for (int n = 1; n <= 5; ++n) {
            camera.stepForward();
            QTRY_COMPARE(camera.position(), n * TestFrame);
            QCOMPARE(camera.frameNumber(), qint64(n));
        }
        QCOMPARE(camera.state(), CameraPipeline::State::Paused);
    }

    void stepBackwardShowsThePreviousPicture()
    {
        CameraPipeline camera(CameraSource::test(QStringLiteral("smpte")));
        camera.seek(10 * TestFrame);
        QTRY_COMPARE(camera.position(), 10 * TestFrame);
        camera.stepBackward();
        QTRY_COMPARE(camera.position(), 9 * TestFrame);
        camera.stepBackward();
        QTRY_COMPARE(camera.position(), 8 * TestFrame);
        camera.stepForward();
        QTRY_COMPARE(camera.position(), 9 * TestFrame);
    }

    void seekIsAccurate()
    {
        CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
        camera.seek(1000 * Ms);
        QTRY_COMPARE(camera.position(), 1000 * Ms);
        QCOMPARE(camera.frameNumber(), qint64(CameraSource::TestFps));
        camera.seek(2 * TestFrame + TestFrame / 2); // inside a picture: that picture
        QTRY_COMPARE(camera.position(), 2 * TestFrame);
        camera.seek(-1);
        QTRY_COMPARE(camera.position(), qint64(0));
    }

    void playPauseStop()
    {
        CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
        QSignalSpy states(&camera, &CameraPipeline::stateChanged);
        camera.play();
        QCOMPARE(camera.state(), CameraPipeline::State::Playing);
        QTRY_VERIFY(camera.position() > 200 * Ms);
        camera.pause();
        QCOMPARE(camera.state(), CameraPipeline::State::Paused);
        QTest::qWait(100);
        const qint64 frozen = camera.position();
        QTest::qWait(300);
        QCOMPARE(camera.position(), frozen);
        camera.stop();
        QCOMPARE(camera.state(), CameraPipeline::State::Stopped);
        QVERIFY(camera.frame().isNull());
        QCOMPARE(states.count(), 3);
    }

    void recording()
    {
        const QDir videos(QStringLiteral(INSPECTOR_VIDEOS_DIR));
        const QStringList files = videos.entryList({ QStringLiteral("*.mp4") }, QDir::Files, QDir::Name);
        if (files.isEmpty()) {
            QSKIP("no recordings in the videos folder");
        }
        CameraPipeline camera(CameraSource::file(videos.filePath(files.first())));
        camera.stepForward();
        QTRY_VERIFY(!camera.frame().isNull());
        QTRY_VERIFY(camera.duration() > 0);
        const qint64 frame = camera.frameDuration();
        QVERIFY(frame > 0);

        // steps: one picture each (container timestamps are rounded to its timescale)
        qint64 previous = camera.position();
        for (int n = 1; n <= 3; ++n) {
            camera.stepForward();
            QTRY_COMPARE(camera.frameNumber(), qint64(n));
            QVERIFY(std::abs(camera.position() - previous - frame) <= Ms);
            previous = camera.position();
        }

        // accurate seek into the middle, then one picture back
        const qint64 middle = camera.duration() / 2;
        camera.seek(middle);
        QTRY_VERIFY(std::abs(camera.position() - middle) <= frame);
        const qint64 there = camera.position();
        QCOMPARE(camera.frameNumber(), camera.frameAt(middle));
        const qint64 index = camera.frameNumber();
        camera.stepBackward();
        QTRY_COMPARE(camera.frameNumber(), index - 1);
        camera.stepBackward();
        QTRY_COMPARE(camera.frameNumber(), index - 2);
        camera.stepForward();
        QTRY_COMPARE(camera.frameNumber(), index - 1);
        QVERIFY2(std::abs(there - camera.position() - frame) <= Ms,
            qPrintable(QStringLiteral("from %1 back to %2, frame %3").arg(there).arg(camera.position()).arg(frame)));

        camera.play();
        QTRY_VERIFY(camera.position() > there + 200 * Ms);
    }
};

QTEST_GUILESS_MAIN(TestCameraPipeline)
#include "tst_CameraPipeline.moc"
