#pragma once

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QKeySequence>
#include <QMouseEvent>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <catch2/catch_tostring.hpp>

#include <functional>

/// Helpers for tests that drive GStreamer pipelines and widgets through the Qt event loop.
namespace test {

constexpr qint64 Ms = 1'000'000; ///< ns

/// Runs the event loop for @p ms.
inline void wait(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

/// Runs the event loop until @p done holds; false if it still does not after @p timeoutMs.
inline bool eventually(const std::function<bool()>& done, int timeoutMs = 5000)
{
    QElapsedTimer timer;
    timer.start();
    while (!done()) {
        if (timer.elapsed() > timeoutMs) {
            return false;
        }
        wait(10);
    }
    return true;
}

/// Sends a left-button mouse event at @p pos of @p widget.
inline void mouse(QWidget* widget, QEvent::Type type, const QPoint& pos)
{
    const Qt::MouseButtons held = type == QEvent::MouseButtonRelease ? Qt::NoButton : Qt::LeftButton;
    QMouseEvent event(type, QPointF(pos), QPointF(widget->mapToGlobal(pos)), Qt::LeftButton, held, Qt::NoModifier);
    QApplication::sendEvent(widget, &event);
}

inline void click(QWidget* widget, const QPoint& pos)
{
    mouse(widget, QEvent::MouseButtonPress, pos);
    mouse(widget, QEvent::MouseButtonRelease, pos);
}

inline void doubleClick(QWidget* widget, const QPoint& pos)
{
    click(widget, pos);
    mouse(widget, QEvent::MouseButtonDblClick, pos);
    mouse(widget, QEvent::MouseButtonRelease, pos);
}

/// Presses @p keys in @p window: triggers the enabled action they are a shortcut of. False if
/// no enabled action answers them.
inline bool press(QWidget* window, const QKeySequence& keys)
{
    for (QAction* action : window->findChildren<QAction*>()) {
        if (action->isEnabled() && action->shortcuts().contains(keys)) {
            action->trigger();
            return true;
        }
    }
    return false;
}

} // namespace test

template <>
struct Catch::StringMaker<QString> {
    static std::string convert(const QString& value) { return '"' + value.toStdString() + '"'; }
};

template <>
struct Catch::StringMaker<QColor> {
    static std::string convert(const QColor& value) { return value.name(QColor::HexArgb).toStdString(); }
};
