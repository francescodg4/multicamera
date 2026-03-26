#pragma once

#include "inspector/CameraSource.hpp"

#include <QMainWindow>
#include <QTimer>

class CameraGrid;
class CameraPipeline;
class QAction;
class QActionGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLCDNumber;
class QLabel;
class QPushButton;
class QSlider;

/// The multicamera inspection system: a control room (all cameras at once, board layout,
/// interface design), the board of cameras, and an inspection panel for the camera clicked on
/// (transport, time bar, picture readouts; E steps one picture forward, Q one back).
class InspectionWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit InspectionWindow(const QList<CameraSource>& sources, QWidget* parent = nullptr);

    const QList<CameraPipeline*>& cameras() const { return m_cameras; }
    CameraGrid* grid() const { return m_grid; }

    QString theme() const { return m_theme; }
    /// Draws the whole application with the design of theme @p id.
    void setTheme(const QString& id);

    /// The camera under inspection (null if none is selected).
    CameraPipeline* current() const;
    /// Inspects camera @p index (-1: none).
    void select(int index);

public slots:
    void playAll();
    void pauseAll();
    void stopAll();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void createActions();
    void createMenus();
    QGroupBox* controlBox();
    QGroupBox* inspectionBox();
    void chooseTheme(const QString& id); ///< user choice: applied and remembered
    void toggleMaximized(int index);
    void tintIcons(); ///< button icons follow the text colour of the interface

    // inspection panel
    void updateTransport(); ///< buttons for the state of the current camera
    void updateDuration(); ///< time bar range
    void updatePosition(); ///< time bar and readouts for the picture shown
    void beginScrub();
    void scrub(int ms);
    void endScrub();

    QList<CameraPipeline*> m_cameras;
    CameraGrid* m_grid = nullptr;
    int m_current = -1;
    QList<QMetaObject::Connection> m_currentConnections;
    QString m_theme;

    // control room
    QComboBox* m_themeSelector = nullptr;
    QComboBox* m_layout = nullptr;
    QCheckBox* m_loop = nullptr;
    QPushButton* m_playAllButton = nullptr;
    QPushButton* m_pauseAllButton = nullptr;
    QPushButton* m_stopAllButton = nullptr;
    QActionGroup* m_themeActions = nullptr;

    // inspection panel
    QGroupBox* m_inspection = nullptr;
    QPushButton* m_previousButton = nullptr;
    QPushButton* m_playButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QPushButton* m_nextButton = nullptr;
    QSlider* m_timeBar = nullptr;
    QLCDNumber* m_timeReadout = nullptr;
    QLCDNumber* m_frameReadout = nullptr;
    QLabel* m_durationLabel = nullptr;

    // actions on the current camera
    QAction* m_playAction = nullptr;
    QAction* m_stopAction = nullptr;
    QAction* m_previousAction = nullptr;
    QAction* m_nextAction = nullptr;
    QAction* m_startAction = nullptr;
    QAction* m_maximizeAction = nullptr;
    QAction* m_playAllAction = nullptr;
    QAction* m_pauseAllAction = nullptr;
    QAction* m_stopAllAction = nullptr;

    // scrubbing: seeks are coalesced while the time bar is dragged
    QTimer m_seekTimer;
    int m_seekTarget = 0; ///< ms
    bool m_resumeAfterScrub = false;
};
