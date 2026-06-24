#pragma once

#include <QTimer>
#include <QWidget>

class CameraPipeline;
class QPushButton;
class QSlider;

/// Time and picture readout of the controls, on a display drawn by the interface design.
class TimeReadout : public QWidget {
    Q_OBJECT
public:
    explicit TimeReadout(QWidget* parent = nullptr);

    void setText(const QString& time, const QString& detail);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_time; ///< "00:02.900"
    QString m_detail; ///< "#87 / 855": picture number / pictures in all
};

/// The player controls of one camera, shown over the bottom of its card while it is inspected:
/// previous frame, play / pause, stop, next frame, the time bar and the time and picture readout.
/// Dragging the time bar seeks (coalesced) with the camera paused, and resumes it after.
class CameraControls : public QWidget {
    Q_OBJECT
public:
    explicit CameraControls(CameraPipeline* camera, QWidget* parent = nullptr);

    QSlider* timeBar() const { return m_timeBar; }

protected:
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void updateTransport(); ///< play / pause button for the camera's state
    void updateDuration(); ///< time bar range
    void updatePosition(); ///< time bar and readout for the picture shown
    void tintIcons(); ///< keys follow the design: its control height and text colour
    void beginScrub();
    void scrub(int ms);
    void endScrub();

    CameraPipeline* m_camera;
    QPushButton* m_previous = nullptr;
    QPushButton* m_play = nullptr;
    QPushButton* m_stop = nullptr;
    QPushButton* m_next = nullptr;
    QSlider* m_timeBar = nullptr;
    TimeReadout* m_readout = nullptr;

    QTimer m_seekTimer;
    int m_seekTarget = 0; ///< ms
    bool m_resumeAfterScrub = false;
};
