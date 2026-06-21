#include "ThemeRegistry.hpp"
#include "inspector/CameraGrid.hpp"
#include "inspector/CameraPipeline.hpp"
#include "inspector/CameraTile.hpp"
#include "inspector/InspectionWindow.hpp"

#include <QApplication>
#include <QSlider>
#include <QTest>

namespace {

constexpr qint64 Ms = 1'000'000; ///< ns

} // namespace

/// The inspection workflow as a user drives it: click a camera, step it with E (and Q), move
/// along its time bar, maximise it, change the interface design.
class TestInspectionWindow : public QObject {
    Q_OBJECT
private slots:
    void initTestCase()
    {
        int argc = 1;
        char name[] = "tst_inspectionwindow";
        char* args[] = { name, nullptr };
        char** argv = args;
        CameraPipeline::initialize(argc, argv);
    }

    void init()
    {
        m_window = std::make_unique<InspectionWindow>(CameraSource::discover(QString(), 3));
        m_window->setTheme(Themes::all().first().id, true);
        m_window->show();
        QApplication::setActiveWindow(m_window.get());
        QVERIFY(QTest::qWaitForWindowExposed(m_window.get()));
    }

    void cleanup()
    {
        m_window.reset();
    }

    void clickSelectsACamera()
    {
        QCOMPARE(m_window->current(), nullptr);
        CameraTile* tile = m_window->grid()->tiles()[1];
        QTest::mouseClick(tile, Qt::LeftButton, {}, tile->rect().center());
        QCOMPARE(m_window->current(), m_window->cameras()[1]);
        QVERIFY(tile->isSelected());
        QVERIFY(!m_window->grid()->tiles()[0]->isSelected());
    }

    void keyEStepsOneFrame()
    {
        m_window->select(0);
        CameraPipeline* camera = m_window->current();
        QTest::keyClick(m_window.get(), Qt::Key_E); // a stopped camera pauses on its first picture
        QTRY_VERIFY(!camera->frame().isNull());
        QCOMPARE(camera->state(), CameraPipeline::State::Paused);
        QCOMPARE(camera->frameNumber(), qint64(0));
        for (int n = 1; n <= 3; ++n) {
            QTest::keyClick(m_window.get(), Qt::Key_E);
            QTRY_COMPARE(camera->frameNumber(), qint64(n));
        }
        QTest::keyClick(m_window.get(), Qt::Key_Q);
        QTRY_COMPARE(camera->frameNumber(), qint64(2));
    }

    void keyEFreezesAPlayingCamera()
    {
        m_window->select(0);
        CameraPipeline* camera = m_window->current();
        QTest::keyClick(m_window.get(), Qt::Key_Space);
        QCOMPARE(camera->state(), CameraPipeline::State::Playing);
        QTRY_VERIFY(camera->frameNumber() > 3);
        QTest::keyClick(m_window.get(), Qt::Key_E);
        QCOMPARE(camera->state(), CameraPipeline::State::Paused);
        QTest::qWait(100);
        const qint64 frozen = camera->frameNumber();
        QTest::keyClick(m_window.get(), Qt::Key_E);
        QTRY_COMPARE(camera->frameNumber(), frozen + 1);
    }

    void timeBarSeeks()
    {
        m_window->select(2);
        CameraPipeline* camera = m_window->current();
        auto* timeBar = m_window->findChild<QSlider*>();
        QVERIFY(timeBar);
        QVERIFY(timeBar->isEnabled());
        QCOMPARE(timeBar->maximum(), CameraSource::TestSeconds * 1000);
        timeBar->setValue(2000); // as the keyboard or a click on the bar does
        QTRY_COMPARE(camera->position(), 2000 * Ms);
        QCOMPARE(camera->frameNumber(), qint64(2 * CameraSource::TestFps));
    }

    void noCameraNoTransport()
    {
        auto* timeBar = m_window->findChild<QSlider*>();
        QVERIFY(!timeBar->isEnabled());
        QTest::keyClick(m_window.get(), Qt::Key_E); // nothing to step: nothing happens
        for (CameraPipeline* camera : m_window->cameras()) {
            QCOMPARE(camera->state(), CameraPipeline::State::Stopped);
        }
    }

    void doubleClickMaximizes()
    {
        CameraTile* tile = m_window->grid()->tiles()[2];
        QTest::mouseDClick(tile, Qt::LeftButton, {}, tile->rect().center());
        QCOMPARE(m_window->grid()->maximized(), 2);
        QCOMPARE(m_window->current(), m_window->cameras()[2]);
        QVERIFY(!m_window->grid()->tiles()[0]->isVisible());
        QTest::keyClick(m_window.get(), Qt::Key_Escape);
        QCOMPARE(m_window->grid()->maximized(), -1);
        QVERIFY(m_window->grid()->tiles()[0]->isVisible());
    }

    void playAllStopAll()
    {
        m_window->playAll();
        for (CameraPipeline* camera : m_window->cameras()) {
            QCOMPARE(camera->state(), CameraPipeline::State::Playing);
            QTRY_VERIFY(!camera->frame().isNull());
        }
        m_window->stopAll();
        for (CameraPipeline* camera : m_window->cameras()) {
            QCOMPARE(camera->state(), CameraPipeline::State::Stopped);
        }
    }

    void everyTheme()
    {
        m_window->select(0);
        for (const Themes::Look& look : Themes::looks(QStringLiteral("all"), true, true)) {
            m_window->setTheme(look.id, look.dark);
            QCOMPARE(m_window->theme(), look.id);
            QCOMPARE(m_window->isDark(), look.dark);
            QVERIFY(!m_window->grab().isNull());
        }
    }

    void revolutHasTwoModes()
    {
        QVERIFY(!Themes::find(QStringLiteral("emerald")));
        const ThemeEntry* revolut = Themes::find(QStringLiteral("revolut"));
        QVERIFY(revolut && revolut->modes);
        m_window->setTheme(revolut->id, true);
        const QColor dark = QApplication::palette().color(QPalette::Window);
        m_window->setTheme(revolut->id, false);
        const QColor light = QApplication::palette().color(QPalette::Window);
        QVERIFY(dark.lightness() < 40);
        QVERIFY(light.lightness() > 220);
        QCOMPARE(Themes::looks(revolut->id, true, true).size(), 2);
        QCOMPARE(Themes::looks(QStringLiteral("glass"), true, true).size(), 1);
    }

private:
    std::unique_ptr<InspectionWindow> m_window;
};

QTEST_MAIN(TestInspectionWindow)
#include "tst_InspectionWindow.moc"
