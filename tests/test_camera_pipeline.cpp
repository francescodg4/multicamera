#include "inspector/CameraPipeline.hpp"
#include "test_support.hpp"

#include <QDir>

#include <catch2/catch_test_macros.hpp>

// The GStreamer backbone: state changes, accurate seeks and frame stepping, on a synthetic test
// camera (exact frame times) and on a recording from the videos folder.

using namespace test;

namespace {

constexpr qint64 TestFrame = 1'000'000'000 / CameraSource::TestFps;

} // namespace

TEST_CASE("GStreamer has every element of the pipelines")
{
    INFO(CameraPipeline::missingElements().join(u", ").toStdString());
    REQUIRE(CameraPipeline::missingElements().isEmpty());
}

TEST_CASE("Timecodes")
{
    CHECK(CameraPipeline::timecode(0) == QStringLiteral("00:00.000"));
    CHECK(CameraPipeline::timecode(83'480 * Ms) == QStringLiteral("01:23.480"));
    CHECK(CameraPipeline::timecode(3'600'000 * Ms) == QStringLiteral("1:00:00.000"));
    CHECK(CameraPipeline::timecode(-5 * Ms) == QStringLiteral("00:00.000"));
}

TEST_CASE("Discovery numbers the cameras")
{
    const QList<CameraSource> sources = CameraSource::discover(QString(), 3);
    REQUIRE(sources.size() == 3);
    CHECK(sources[0].name == QStringLiteral("CAM 01"));
    CHECK(sources[2].name == QStringLiteral("CAM 03"));
    CHECK(sources[0].kind == CameraSource::Kind::Test);
}

TEST_CASE("A stopped camera has no picture")
{
    CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
    CHECK(camera.state() == CameraPipeline::State::Stopped);
    CHECK(camera.frame().isNull());
    CHECK(camera.duration() == qint64(CameraSource::TestSeconds) * 1000 * Ms);
}

TEST_CASE("Stepping forward shows the next picture")
{
    CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
    camera.stepForward(); // a stopped camera pauses on its first picture
    REQUIRE(camera.state() == CameraPipeline::State::Paused);
    REQUIRE(eventually([&] { return !camera.frame().isNull(); }));
    CHECK(camera.frame().size() == QSize(CameraSource::TestWidth, CameraSource::TestHeight));
    CHECK(camera.position() == 0);
    CHECK(camera.frameDuration() == TestFrame);

    for (int n = 1; n <= 5; ++n) {
        camera.stepForward();
        REQUIRE(eventually([&] { return camera.position() == n * TestFrame; }));
        CHECK(camera.frameNumber() == n);
    }
    CHECK(camera.state() == CameraPipeline::State::Paused);
}

TEST_CASE("Stepping backward shows the previous picture")
{
    CameraPipeline camera(CameraSource::test(QStringLiteral("smpte")));
    camera.seek(10 * TestFrame);
    REQUIRE(eventually([&] { return camera.position() == 10 * TestFrame; }));
    camera.stepBackward();
    REQUIRE(eventually([&] { return camera.position() == 9 * TestFrame; }));
    camera.stepBackward();
    REQUIRE(eventually([&] { return camera.position() == 8 * TestFrame; }));
    camera.stepForward();
    REQUIRE(eventually([&] { return camera.position() == 9 * TestFrame; }));
}

TEST_CASE("Seeks are accurate")
{
    CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
    camera.seek(1000 * Ms);
    REQUIRE(eventually([&] { return camera.position() == 1000 * Ms; }));
    CHECK(camera.frameNumber() == CameraSource::TestFps);
    camera.seek(2 * TestFrame + TestFrame / 2); // inside a picture: that picture
    REQUIRE(eventually([&] { return camera.position() == 2 * TestFrame; }));
    camera.seek(-1);
    REQUIRE(eventually([&] { return camera.position() == 0; }));
}

TEST_CASE("Play, pause and stop")
{
    CameraPipeline camera(CameraSource::test(QStringLiteral("ball")));
    int changes = 0;
    QObject::connect(&camera, &CameraPipeline::stateChanged, [&] { ++changes; });
    camera.play();
    REQUIRE(camera.state() == CameraPipeline::State::Playing);
    REQUIRE(eventually([&] { return camera.position() > 200 * Ms; }));
    camera.pause();
    REQUIRE(camera.state() == CameraPipeline::State::Paused);
    wait(100);
    const qint64 frozen = camera.position();
    wait(300);
    CHECK(camera.position() == frozen);
    camera.stop();
    CHECK(camera.state() == CameraPipeline::State::Stopped);
    CHECK(camera.frame().isNull());
    CHECK(changes == 3);
}

TEST_CASE("A recording steps and seeks picture by picture")
{
    const QDir videos(QStringLiteral(INSPECTOR_VIDEOS_DIR));
    const QStringList files = videos.entryList({ QStringLiteral("*.mp4") }, QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        SKIP("no recordings in the videos folder");
    }
    CameraPipeline camera(CameraSource::file(videos.filePath(files.first())));
    camera.stepForward();
    REQUIRE(eventually([&] { return !camera.frame().isNull(); }));
    REQUIRE(eventually([&] { return camera.duration() > 0; }));
    const qint64 frame = camera.frameDuration();
    REQUIRE(frame > 0);

    // steps: one picture each (container timestamps are rounded to its timescale)
    qint64 previous = camera.position();
    for (int n = 1; n <= 3; ++n) {
        camera.stepForward();
        REQUIRE(eventually([&] { return camera.frameNumber() == n; }));
        CHECK(std::abs(camera.position() - previous - frame) <= Ms);
        previous = camera.position();
    }

    // accurate seek into the middle, then one picture back
    const qint64 middle = camera.duration() / 2;
    camera.seek(middle);
    REQUIRE(eventually([&] { return std::abs(camera.position() - middle) <= frame; }));
    const qint64 there = camera.position();
    CHECK(camera.frameNumber() == camera.frameAt(middle));
    const qint64 index = camera.frameNumber();
    camera.stepBackward();
    REQUIRE(eventually([&] { return camera.frameNumber() == index - 1; }));
    camera.stepBackward();
    REQUIRE(eventually([&] { return camera.frameNumber() == index - 2; }));
    camera.stepForward();
    REQUIRE(eventually([&] { return camera.frameNumber() == index - 1; }));
    INFO("from " << there << " back to " << camera.position() << ", frame " << frame);
    CHECK(std::abs(there - camera.position() - frame) <= Ms);

    camera.play();
    REQUIRE(eventually([&] { return camera.position() > there + 200 * Ms; }));
}
