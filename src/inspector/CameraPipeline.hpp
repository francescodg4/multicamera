#pragma once

#include "inspector/CameraSource.hpp"

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QTimer>

#include <atomic>

// GStreamer stays out of the headers (its GLib headers and Qt's keywords do not mix well)
typedef struct _GstElement GstElement;
typedef struct _GstBus GstBus;
typedef struct _GstSample GstSample;

/// One camera of the system: a GStreamer pipeline that decodes its source into an appsink,
/// and the controls of an inspection player on top of it (play, pause, stop, accurate seek,
/// frame stepping).
///
/// Recordings play through playbin; test cameras are videotestsrc with the buffer time burnt
/// in. Pictures arrive on GStreamer's streaming thread, are converted to QImage there and
/// announced on the object's thread with frameReady(). Times are in nanoseconds, like GStreamer.
class CameraPipeline : public QObject {
    Q_OBJECT
public:
    enum class State {
        Stopped, ///< READY: nothing decoded, no picture
        Paused, ///< PAUSED: one picture shown, can be stepped
        Playing,
        Error,
    };
    Q_ENUM(State)

    /// Initialises GStreamer (call once, before creating any camera).
    static void initialize(int& argc, char**& argv);
    static QString version();
    /// GStreamer elements the system needs that are not installed.
    static QStringList missingElements();
    /// @p time as mm:ss.mmm (h:mm:ss.mmm from an hour on).
    static QString timecode(qint64 time);

    explicit CameraPipeline(const CameraSource& source, QObject* parent = nullptr);
    ~CameraPipeline() override;

    const CameraSource& source() const { return m_source; }
    State state() const { return m_state; }
    QString errorString() const { return m_error; }

    /// Picture shown (null while stopped).
    QImage frame() const;
    /// Stream time of the picture shown.
    qint64 position() const;
    /// Length of the source (0 while unknown).
    qint64 duration() const { return m_duration; }
    /// Time between two pictures (0 until the first one).
    qint64 frameDuration() const;
    /// Index of the picture shown, from 0.
    qint64 frameNumber() const;
    /// Start time of picture @p index, and index of the picture shown at @p time (once the
    /// frame rate is known from the first picture).
    qint64 frameTime(qint64 index) const;
    qint64 frameAt(qint64 time) const;

    /// At the end, start again (true) or pause on the last picture.
    bool loops() const { return m_loop; }
    void setLoop(bool loop) { m_loop = loop; }

public slots:
    void play();
    void pause();
    void stop();
    void togglePlay();
    /// Shows the picture at @p position: flushing, accurate seek (a stopped camera is paused first).
    void seek(qint64 position);
    /// Shows the next picture (GStreamer step event). A playing camera just pauses, freezing
    /// on the picture it reached.
    void stepForward();
    /// Shows the previous picture (accurate seek into it).
    void stepBackward();

signals:
    void frameReady();
    void stateChanged(CameraPipeline::State state);
    void durationChanged(qint64 duration);
    void errorOccurred(const QString& message);

private:
    friend struct CameraPipelineCallbacks;

    bool build();
    void setState(State state);
    void fail(const QString& message);
    /// Goes to PAUSED and waits for the first picture (preroll); false if it failed.
    bool pauseAndWait();
    void pollBus();
    void queryDuration();
    void reachedEnd();
    void deliver(GstSample* sample); ///< streaming thread

    CameraSource m_source;
    GstElement* m_pipeline = nullptr;
    GstElement* m_sink = nullptr;
    GstBus* m_bus = nullptr;
    QTimer m_busTimer;

    State m_state = State::Stopped;
    QString m_error;
    qint64 m_duration = 0;
    bool m_loop = true;

    mutable QMutex m_mutex; ///< guards the picture, written by the streaming thread
    QImage m_frame;
    qint64 m_position = 0;
    qint64 m_frameDuration = 0;
    qint64 m_frameIndex = 0;
    int m_fpsN = 0; ///< frame rate of the stream (0: variable or unknown)
    int m_fpsD = 1;
    std::atomic_bool m_notifying { false }; ///< a frameReady() is queued
};
