#pragma once

#include "inspector/CameraSource.hpp"

#include <QMainWindow>

class CameraGrid;
class CameraPipeline;
class QAction;
class QActionGroup;
class QCheckBox;
class QComboBox;
class QPushButton;

/// The multicamera inspection system: a control room (all cameras at once, board layout,
/// interface design and mode) over the board of cameras. The camera clicked is inspected: its
/// player controls show over the bottom of its card, and the keyboard drives it (E steps one
/// picture forward, Q one back).
class InspectionWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit InspectionWindow(const QList<CameraSource>& sources, QWidget* parent = nullptr);

    const QList<CameraPipeline*>& cameras() const { return m_cameras; }
    CameraGrid* grid() const { return m_grid; }

    QString theme() const { return m_theme; }
    bool isDark() const { return m_dark; }
    /// Draws the whole application with the design of theme @p id, in its dark or light mode if
    /// it has two.
    void setTheme(const QString& id, bool dark);

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
    QWidget* controlBox();
    void chooseTheme(const QString& id); ///< user choice: applied and remembered
    void chooseDark(bool dark); ///< user choice: applied and remembered
    void toggleMaximized(int index);
    void tintIcons(); ///< button icons follow the text colour of the interface

    QList<CameraPipeline*> m_cameras;
    CameraGrid* m_grid = nullptr;
    int m_current = -1;
    QString m_theme;
    bool m_dark = true;

    // control room (hidden while a single camera is maximised)
    QWidget* m_controlRoom = nullptr;
    QComboBox* m_themeSelector = nullptr;
    QCheckBox* m_darkBox = nullptr;
    QComboBox* m_layout = nullptr;
    QCheckBox* m_loop = nullptr;
    QPushButton* m_playAllButton = nullptr;
    QPushButton* m_pauseAllButton = nullptr;
    QPushButton* m_stopAllButton = nullptr;
    QActionGroup* m_themeActions = nullptr;
    QAction* m_darkAction = nullptr;

    // actions on the camera under inspection
    QAction* m_playAction = nullptr;
    QAction* m_stopAction = nullptr;
    QAction* m_previousAction = nullptr;
    QAction* m_nextAction = nullptr;
    QAction* m_startAction = nullptr;
    QAction* m_maximizeAction = nullptr;
    QAction* m_playAllAction = nullptr;
    QAction* m_pauseAllAction = nullptr;
    QAction* m_stopAllAction = nullptr;
};
