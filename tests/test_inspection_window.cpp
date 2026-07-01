#include "ThemeRegistry.hpp"
#include "inspector/CameraControls.hpp"
#include "inspector/CameraGrid.hpp"
#include "inspector/CameraPipeline.hpp"
#include "inspector/CameraTile.hpp"
#include "inspector/InspectionWindow.hpp"
#include "test_support.hpp"

#include <QPushButton>
#include <QSlider>
#include <QWindow>

#include <catch2/catch_test_macros.hpp>

// The inspection workflow as a user drives it: click a camera, step it with E (and Q), move along
// its time bar, maximise it, change the interface design.

using namespace test;

namespace {

/// The inspector on three test cameras, shown in the first design.
struct Inspector {
    Inspector()
        : window(CameraSource::discover(QString(), 3))
    {
        window.setTheme(Themes::all().first().id, true);
        window.show();
        QApplication::setActiveWindow(&window);
        REQUIRE(eventually([&] { return window.windowHandle() && window.windowHandle()->isExposed(); }));
    }

    ~Inspector()
    {
        window.setMarkColor(QColor()); // the mark colour is shared by every window
    }

    CameraTile* tile(int index) const { return window.grid()->tiles()[index]; }
    CameraPipeline* camera(int index) const { return window.cameras()[index]; }
    QColor pixel(int index, int x, int y) const { return tile(index)->grab().toImage().pixelColor(x, y); }

    InspectionWindow window;
};

} // namespace

TEST_CASE_METHOD(Inspector, "A click selects a camera")
{
    REQUIRE(window.current() == nullptr);
    click(tile(1), tile(1)->rect().center());
    CHECK(window.current() == camera(1));
    CHECK(tile(1)->isSelected());
    CHECK_FALSE(tile(0)->isSelected());
}

TEST_CASE_METHOD(Inspector, "E steps one picture forward, Q one back")
{
    window.select(0);
    CameraPipeline* current = window.current();
    REQUIRE(press(&window, Qt::Key_E)); // a stopped camera pauses on its first picture
    REQUIRE(eventually([&] { return !current->frame().isNull(); }));
    CHECK(current->state() == CameraPipeline::State::Paused);
    CHECK(current->frameNumber() == 0);
    for (int n = 1; n <= 3; ++n) {
        REQUIRE(press(&window, Qt::Key_E));
        REQUIRE(eventually([&] { return current->frameNumber() == n; }));
    }
    REQUIRE(press(&window, Qt::Key_Q));
    REQUIRE(eventually([&] { return current->frameNumber() == 2; }));
}

TEST_CASE_METHOD(Inspector, "E freezes a playing camera")
{
    window.select(0);
    CameraPipeline* current = window.current();
    REQUIRE(press(&window, Qt::Key_Space));
    REQUIRE(current->state() == CameraPipeline::State::Playing);
    REQUIRE(eventually([&] { return current->frameNumber() > 3; }));
    REQUIRE(press(&window, Qt::Key_E));
    CHECK(current->state() == CameraPipeline::State::Paused);
    wait(100);
    const qint64 frozen = current->frameNumber();
    REQUIRE(press(&window, Qt::Key_E));
    REQUIRE(eventually([&] { return current->frameNumber() == frozen + 1; }));
}

TEST_CASE_METHOD(Inspector, "The time bar seeks")
{
    window.select(2);
    QSlider* timeBar = tile(2)->controls()->timeBar();
    REQUIRE(timeBar->isVisible());
    CHECK(timeBar->maximum() == CameraSource::TestSeconds * 1000);
    timeBar->setValue(2000); // as the keyboard or a click on the bar does
    REQUIRE(eventually([&] { return camera(2)->position() == 2000 * Ms; }));
    CHECK(camera(2)->frameNumber() == 2 * CameraSource::TestFps);
}

TEST_CASE_METHOD(Inspector, "The control buttons drive their own camera")
{
    window.select(1);
    const QList<QPushButton*> buttons = tile(1)->controls()->findChildren<QPushButton*>();
    REQUIRE(buttons.size() == 4); // previous, play, stop, next
    buttons[3]->click(); // next: a stopped camera pauses on its first picture
    REQUIRE(eventually([&] { return !camera(1)->frame().isNull(); }));
    buttons[3]->click();
    REQUIRE(eventually([&] { return camera(1)->frameNumber() == 1; }));
    buttons[1]->click();
    CHECK(camera(1)->state() == CameraPipeline::State::Playing);
    buttons[2]->click();
    CHECK(camera(1)->state() == CameraPipeline::State::Stopped);
    CHECK(camera(0)->state() == CameraPipeline::State::Stopped); // only its own camera
}

TEST_CASE_METHOD(Inspector, "The controls follow the selection")
{
    for (CameraTile* each : window.grid()->tiles()) {
        CHECK_FALSE(each->controls()->isVisible());
    }
    click(tile(0), tile(0)->rect().center());
    CHECK(tile(0)->controls()->isVisible());
    CHECK_FALSE(tile(1)->controls()->isVisible());
    click(tile(1), tile(1)->rect().center());
    CHECK_FALSE(tile(0)->controls()->isVisible());
    CHECK(tile(1)->controls()->isVisible());
    REQUIRE(press(&window, Qt::Key_Escape));
    CHECK_FALSE(tile(1)->controls()->isVisible());
}

TEST_CASE_METHOD(Inspector, "The picture makes room for the controls")
{
    const QRect whole = tile(0)->screenRect();
    window.select(0);
    const QRect smaller = tile(0)->screenRect();
    const QRect controls = tile(0)->controls()->geometry();
    CHECK(smaller.height() < whole.height()); // the picture shrinks
    CHECK(smaller.top() == whole.top());
    CHECK_FALSE(smaller.intersects(controls)); // the controls never cover the picture
    CHECK(whole.intersects(controls)); // they take the bottom of the card
    CHECK(tile(0)->rect().contains(controls));
    window.select(-1);
    CHECK(tile(0)->screenRect() == whole);
}

TEST_CASE_METHOD(Inspector, "No camera, no transport")
{
    CHECK_FALSE(press(&window, Qt::Key_E)); // nothing to step: the key does nothing
    for (CameraPipeline* each : window.cameras()) {
        CHECK(each->state() == CameraPipeline::State::Stopped);
    }
}

TEST_CASE_METHOD(Inspector, "A double click maximises a camera")
{
    doubleClick(tile(2), tile(2)->rect().center());
    CHECK(window.grid()->maximized() == 2);
    CHECK(window.current() == camera(2));
    CHECK_FALSE(tile(0)->isVisible());
    REQUIRE(press(&window, Qt::Key_Escape));
    CHECK(window.grid()->maximized() == -1);
    CHECK(tile(0)->isVisible());
}

TEST_CASE_METHOD(Inspector, "A single camera hides the control room")
{
    auto* controlRoom = window.findChild<QWidget*>(QStringLiteral("controlRoom"));
    REQUIRE(controlRoom);
    REQUIRE(controlRoom->isVisible());
    doubleClick(tile(1), tile(1)->rect().center());
    CHECK_FALSE(controlRoom->isVisible()); // one camera: no controls for all of them
    CHECK(tile(1)->controls()->isVisible()); // its own controls stay
    window.select(2); // the maximised view follows the selection
    CHECK_FALSE(controlRoom->isVisible());
    REQUIRE(press(&window, Qt::Key_F));
    CHECK(controlRoom->isVisible());
    REQUIRE(press(&window, Qt::Key_F));
    CHECK_FALSE(controlRoom->isVisible());
    REQUIRE(press(&window, Qt::Key_Escape));
    CHECK(controlRoom->isVisible());
}

TEST_CASE_METHOD(Inspector, "Play all, stop all")
{
    window.playAll();
    for (CameraPipeline* each : window.cameras()) {
        CHECK(each->state() == CameraPipeline::State::Playing);
        REQUIRE(eventually([&] { return !each->frame().isNull(); }));
    }
    window.stopAll();
    for (CameraPipeline* each : window.cameras()) {
        CHECK(each->state() == CameraPipeline::State::Stopped);
    }
}

TEST_CASE_METHOD(Inspector, "Every design and mode draws the window")
{
    window.select(0);
    for (const Themes::Look& look : Themes::looks(QStringLiteral("all"), true, true)) {
        INFO(look.name().toStdString());
        window.setTheme(look.id, look.dark);
        CHECK(window.theme() == look.id);
        CHECK(window.isDark() == look.dark);
        CHECK_FALSE(window.grab().isNull());
    }
}

TEST_CASE_METHOD(Inspector, "Metro: each camera a live tile in its own colour")
{
    const ThemeEntry* metro = Themes::find(QStringLiteral("metro"));
    REQUIRE(metro);
    REQUIRE(metro->modes);
    for (const bool dark : { true, false }) {
        INFO((dark ? "dark" : "light"));
        window.setTheme(metro->id, dark);
        CHECK(QApplication::palette().color(QPalette::Window) == (dark ? QColor(0x1d, 0x1d, 0x1d) : QColor(0xf2, 0xf2, 0xf2)));
        CHECK(pixel(0, 2, 2) == QColor(0x00, 0xb7, 0xc3));
        CHECK(pixel(1, 2, 2) == QColor(0xd8, 0x00, 0x73));
    }
}

TEST_CASE_METHOD(Inspector, "Flat: slate panels with gutters, an azure box round the camera")
{
    const ThemeEntry* flat = Themes::find(QStringLiteral("flat"));
    REQUIRE(flat);
    CHECK_FALSE(flat->modes);
    window.setTheme(flat->id, true);
    CHECK(QApplication::palette().color(QPalette::Window) == QColor(0x1b, 0x22, 0x2c));
    window.select(1);
    CHECK(pixel(0, 1, 1) == QColor(0x1b, 0x22, 0x2c)); // a gutter of canvas round each camera
    CHECK(pixel(0, 3, 3) == QColor(0x3b, 0x48, 0x5a)); // the panel's hairline
    CHECK(pixel(0, 5, 5) == QColor(0x27, 0x31, 0x3f)); // the slate panel
    CHECK(pixel(1, 4, 4) == QColor(0x5c, 0xb4, 0xf5)); // the camera under inspection
}

TEST_CASE_METHOD(Inspector, "A camera alone is not marked")
{
    window.setTheme(QStringLiteral("flat"), true);
    window.select(1);
    CHECK(pixel(1, 4, 4) == QColor(0x5c, 0xb4, 0xf5));
    window.grid()->setMaximized(1);
    CHECK(tile(1)->isSelected()); // still inspected, with its controls
    CHECK(pixel(1, 4, 4) == QColor(0x27, 0x31, 0x3f)); // but not marked
    window.grid()->setMaximized(-1);
    CHECK(pixel(1, 4, 4) == QColor(0x5c, 0xb4, 0xf5));
}

TEST_CASE_METHOD(Inspector, "The highlight colour is the user's")
{
    window.select(1);
    const QColor orange(0xff, 0x80, 0x00);
    for (const QString& id : { QStringLiteral("flat"), QStringLiteral("metro") }) {
        INFO(id.toStdString());
        window.setTheme(id, true);
        window.setMarkColor(orange);
        CHECK(pixel(1, 4, 4) == orange);
        window.setMarkColor(QColor()); // back to the design's own
        CHECK(pixel(1, 4, 4) != orange);
    }
    CHECK(pixel(1, 4, 4) == QColor(0xff, 0xff, 0xff)); // Metro's white
}

TEST_CASE_METHOD(Inspector, "Revolut has two modes")
{
    CHECK_FALSE(Themes::find(QStringLiteral("emerald")));
    const ThemeEntry* revolut = Themes::find(QStringLiteral("revolut"));
    REQUIRE(revolut);
    REQUIRE(revolut->modes);
    window.setTheme(revolut->id, true);
    const QColor dark = QApplication::palette().color(QPalette::Window);
    window.setTheme(revolut->id, false);
    const QColor light = QApplication::palette().color(QPalette::Window);
    CHECK(dark.lightness() < 40);
    CHECK(light.lightness() > 220);
    CHECK(Themes::looks(revolut->id, true, true).size() == 2);
    CHECK(Themes::looks(QStringLiteral("glass"), true, true).size() == 1);
}
