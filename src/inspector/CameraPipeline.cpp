#include "inspector/CameraPipeline.hpp"

#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include <QUrl>

#include <algorithm>

namespace {

constexpr guint PlayFlagVideo = 1 << 0; ///< GST_PLAY_FLAG_VIDEO: playbin decodes the video only
constexpr int BusInterval = 20; ///< ms between two looks at the bus
constexpr GstClockTime PrerollTimeout = 5 * GST_SECOND;

/// Converts the pictures to 32-bit BGRx (QImage::Format_RGB32 in memory) for the appsink.
const char* const SinkBin = "videoconvert ! video/x-raw,format=BGRx ! appsink name=sink";

bool hasElement(const char* name)
{
    GstElementFactory* factory = gst_element_factory_find(name);
    if (!factory) {
        return false;
    }
    gst_object_unref(factory);
    return true;
}

} // namespace

/// appsink callbacks, on the streaming thread.
struct CameraPipelineCallbacks {
    static GstFlowReturn preroll(GstAppSink* sink, gpointer camera)
    {
        static_cast<CameraPipeline*>(camera)->deliver(gst_app_sink_pull_preroll(sink));
        return GST_FLOW_OK;
    }
    static GstFlowReturn sample(GstAppSink* sink, gpointer camera)
    {
        static_cast<CameraPipeline*>(camera)->deliver(gst_app_sink_pull_sample(sink));
        return GST_FLOW_OK;
    }
};

void CameraPipeline::initialize(int& argc, char**& argv)
{
    gst_init(&argc, &argv);
}

QString CameraPipeline::version()
{
    gchar* text = gst_version_string();
    const QString version = QString::fromUtf8(text);
    g_free(text);
    return version;
}

QStringList CameraPipeline::missingElements()
{
    QStringList missing;
    for (const char* name : { "playbin", "appsink", "videoconvert", "videotestsrc", "qtdemux", "avdec_h264" }) {
        if (!hasElement(name)) {
            missing << QString::fromLatin1(name);
        }
    }
    return missing;
}

QString CameraPipeline::timecode(qint64 time)
{
    const qint64 ms = std::max<qint64>(0, time) / GST_MSECOND;
    const qint64 h = ms / 3'600'000, m = ms / 60'000 % 60, s = ms / 1000 % 60, rest = ms % 1000;
    const QChar zero(u'0');
    const QString text = QStringLiteral("%1:%2.%3").arg(m, 2, 10, zero).arg(s, 2, 10, zero).arg(rest, 3, 10, zero);
    return h > 0 ? QStringLiteral("%1:%2").arg(h).arg(text) : text;
}

CameraPipeline::CameraPipeline(const CameraSource& source, QObject* parent)
    : QObject(parent)
    , m_source(source)
{
    if (m_source.kind == CameraSource::Kind::Test) {
        m_duration = qint64(CameraSource::TestSeconds) * GST_SECOND;
    }
    m_busTimer.setInterval(BusInterval);
    connect(&m_busTimer, &QTimer::timeout, this, &CameraPipeline::pollBus);
    if (build()) {
        m_busTimer.start();
    }
}

CameraPipeline::~CameraPipeline()
{
    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL); // joins the streaming threads
    }
    if (m_sink) {
        gst_object_unref(m_sink);
    }
    if (m_bus) {
        gst_object_unref(m_bus);
    }
    if (m_pipeline) {
        gst_object_unref(m_pipeline);
    }
}

bool CameraPipeline::build()
{
    GError* error = nullptr;
    if (m_source.kind == CameraSource::Kind::File) {
        m_pipeline = gst_element_factory_make("playbin", nullptr);
        GstElement* sinkBin = gst_parse_bin_from_description(SinkBin, TRUE, &error);
        if (!m_pipeline || !sinkBin) {
            fail(error ? QString::fromUtf8(error->message) : tr("GStreamer's playbin is not installed"));
            g_clear_error(&error);
            if (sinkBin) {
                gst_object_unref(sinkBin);
            }
            return false;
        }
        const QByteArray uri = QUrl::fromLocalFile(m_source.location).toEncoded();
        g_object_set(m_pipeline, "uri", uri.constData(), "flags", PlayFlagVideo, "video-sink", sinkBin, nullptr);
        m_sink = gst_bin_get_by_name(GST_BIN(sinkBin), "sink");
    } else {
        QString description = QStringLiteral("videotestsrc pattern=%1 %2 ! video/x-raw,width=%3,height=%4,framerate=%5/1")
                                  .arg(m_source.location, m_source.properties)
                                  .arg(CameraSource::TestWidth)
                                  .arg(CameraSource::TestHeight)
                                  .arg(CameraSource::TestFps);
        if (hasElement("timeoverlay")) {
            // burn in the buffer time: each step and seek can be checked on the picture itself
            description += QStringLiteral(" ! timeoverlay time-mode=buffer-time halignment=right valignment=bottom font-desc=\"Monospace Bold 30\" shaded-background=true");
        }
        description += QStringLiteral(" ! ") + QString::fromLatin1(SinkBin);
        m_pipeline = gst_parse_launch(description.toUtf8().constData(), &error);
        if (!m_pipeline) {
            fail(error ? QString::fromUtf8(error->message) : tr("Cannot build the test camera"));
            g_clear_error(&error);
            return false;
        }
        g_clear_error(&error); // a recoverable parse warning
        m_sink = gst_bin_get_by_name(GST_BIN(m_pipeline), "sink");
    }
    if (!m_sink) {
        fail(tr("The pipeline has no appsink"));
        return false;
    }

    // keep the clock (sync), never queue more than a couple of pictures behind a busy GUI
    g_object_set(m_sink, "sync", TRUE, "max-buffers", 2u, "drop", TRUE, "enable-last-sample", FALSE, nullptr);
    GstAppSinkCallbacks callbacks {};
    callbacks.new_preroll = &CameraPipelineCallbacks::preroll;
    callbacks.new_sample = &CameraPipelineCallbacks::sample;
    gst_app_sink_set_callbacks(GST_APP_SINK(m_sink), &callbacks, this, nullptr);

    m_bus = gst_element_get_bus(m_pipeline);
    gst_element_set_state(m_pipeline, GST_STATE_READY);
    return true;
}

// ---- pictures -----------------------------------------------------------------------------------

void CameraPipeline::deliver(GstSample* sample)
{
    if (!sample) {
        return;
    }
    GstVideoInfo info;
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);
    if (!buffer || !caps || !gst_video_info_from_caps(&info, caps)) {
        gst_sample_unref(sample);
        return;
    }

    GstVideoFrame video;
    if (!gst_video_frame_map(&video, &info, buffer, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return;
    }
    const QImage image = QImage(static_cast<const uchar*>(GST_VIDEO_FRAME_PLANE_DATA(&video, 0)),
        GST_VIDEO_FRAME_WIDTH(&video),
        GST_VIDEO_FRAME_HEIGHT(&video),
        GST_VIDEO_FRAME_PLANE_STRIDE(&video, 0),
        QImage::Format_RGB32)
                             .copy(); // the buffer goes back to GStreamer
    gst_video_frame_unmap(&video);

    // stream time of the picture (after a seek the segment maps running time back to it)
    qint64 position = 0;
    const GstClockTime pts = GST_BUFFER_PTS(buffer);
    if (GST_CLOCK_TIME_IS_VALID(pts)) {
        guint64 streamTime = pts;
        const GstSegment* segment = gst_sample_get_segment(sample);
        if (segment && segment->format == GST_FORMAT_TIME) {
            if (gst_segment_to_stream_time_full(segment, GST_FORMAT_TIME, pts, &streamTime) != 1) {
                streamTime = 0;
            }
        }
        position = qint64(streamTime);
    }
    qint64 frameDuration = 0;
    qint64 index = 0;
    if (info.fps_n > 0 && info.fps_d > 0) {
        // constant frame rate: the picture's place on the frame grid. After an accurate seek the
        // decoder clips the timestamp of the picture overlapping the target to the target itself
        const guint64 grid = guint64(info.fps_d) * GST_SECOND;
        frameDuration = qint64(gst_util_uint64_scale_round(GST_SECOND, info.fps_d, info.fps_n));
        index = qint64(gst_util_uint64_scale_round(guint64(position), info.fps_n, grid));
        position = qint64(gst_util_uint64_scale_round(guint64(index), grid, info.fps_n));
    } else if (GST_BUFFER_DURATION_IS_VALID(buffer)) {
        frameDuration = qint64(GST_BUFFER_DURATION(buffer));
        index = frameDuration > 0 ? (position + frameDuration / 2) / frameDuration : 0;
    }
    gst_sample_unref(sample);

    {
        const QMutexLocker lock(&m_mutex);
        m_frame = image;
        m_position = position;
        m_frameDuration = frameDuration;
        m_frameIndex = index;
        m_fpsN = info.fps_n;
        m_fpsD = info.fps_d;
    }
    // one pending notification is enough: the GUI always takes the latest picture
    if (!m_notifying.exchange(true)) {
        QMetaObject::invokeMethod(
            this, [this] {
                m_notifying = false;
                if (m_source.kind == CameraSource::Kind::Test && m_state == State::Playing && this->position() >= m_duration) {
                    reachedEnd(); // test cameras never end by themselves
                }
                emit frameReady();
            },
            Qt::QueuedConnection);
    }
}

QImage CameraPipeline::frame() const
{
    const QMutexLocker lock(&m_mutex);
    return m_frame;
}

qint64 CameraPipeline::position() const
{
    const QMutexLocker lock(&m_mutex);
    return m_position;
}

qint64 CameraPipeline::frameDuration() const
{
    const QMutexLocker lock(&m_mutex);
    return m_frameDuration;
}

qint64 CameraPipeline::frameNumber() const
{
    const QMutexLocker lock(&m_mutex);
    return m_frameIndex;
}

qint64 CameraPipeline::frameTime(qint64 index) const
{
    const QMutexLocker lock(&m_mutex);
    if (m_fpsN > 0 && m_fpsD > 0) {
        return qint64(gst_util_uint64_scale_round(guint64(std::max<qint64>(0, index)), guint64(m_fpsD) * GST_SECOND, guint64(m_fpsN)));
    }
    return index * m_frameDuration;
}

qint64 CameraPipeline::frameAt(qint64 time) const
{
    const QMutexLocker lock(&m_mutex);
    const guint64 t = guint64(std::max<qint64>(0, time)) + GST_USECOND; // absorbs the rounding of frameTime()
    if (m_fpsN > 0 && m_fpsD > 0) {
        return qint64(gst_util_uint64_scale(t, guint64(m_fpsN), guint64(m_fpsD) * GST_SECOND));
    }
    return m_frameDuration > 0 ? qint64(t) / m_frameDuration : 0;
}

// ---- control ------------------------------------------------------------------------------------

void CameraPipeline::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(state);
    }
}

void CameraPipeline::fail(const QString& message)
{
    m_error = message;
    setState(State::Error);
    emit errorOccurred(message);
}

void CameraPipeline::play()
{
    if (!m_pipeline) {
        return;
    }
    if (m_state == State::Error) {
        gst_element_set_state(m_pipeline, GST_STATE_READY); // start over
        m_error.clear();
    }
    if (gst_element_set_state(m_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        fail(tr("%1 cannot play").arg(m_source.name));
        return;
    }
    setState(State::Playing);
}

void CameraPipeline::pause()
{
    if (!m_pipeline || m_state == State::Paused) {
        return;
    }
    if (gst_element_set_state(m_pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE) {
        fail(tr("%1 cannot pause").arg(m_source.name));
        return;
    }
    setState(State::Paused);
}

void CameraPipeline::stop()
{
    if (!m_pipeline) {
        return;
    }
    gst_element_set_state(m_pipeline, GST_STATE_READY); // returns once streaming has stopped
    {
        const QMutexLocker lock(&m_mutex);
        m_frame = QImage();
        m_position = 0;
        m_frameIndex = 0;
    }
    m_error.clear();
    setState(State::Stopped);
    emit frameReady();
}

void CameraPipeline::togglePlay()
{
    if (m_state == State::Playing) {
        pause();
    } else {
        play();
    }
}

bool CameraPipeline::pauseAndWait()
{
    if (!m_pipeline) {
        return false;
    }
    if (m_state == State::Error) {
        gst_element_set_state(m_pipeline, GST_STATE_READY);
        m_error.clear();
    }
    if (gst_element_set_state(m_pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE) {
        fail(tr("%1 cannot pause").arg(m_source.name));
        return false;
    }
    GstState current = GST_STATE_VOID_PENDING;
    if (gst_element_get_state(m_pipeline, &current, nullptr, PrerollTimeout) == GST_STATE_CHANGE_FAILURE) {
        pollBus(); // reports the error
        return false;
    }
    setState(State::Paused);
    return current == GST_STATE_PAUSED;
}

void CameraPipeline::seek(qint64 position)
{
    if (!m_pipeline) {
        return;
    }
    if ((m_state == State::Stopped || m_state == State::Error) && !pauseAndWait()) {
        return;
    }
    if (m_duration > 0) {
        // seeking at or past the end would leave no picture to show
        const qint64 last = std::max<qint64>(0, m_duration - std::max<qint64>(frameDuration(), 1));
        position = std::min(position, last);
    }
    position = std::max<qint64>(0, position);
    const qint64 frame = frameDuration();
    if (frame > 0) {
        position = frameTime(frameAt(position)); // aim at the picture containing the position
        if (m_source.kind == CameraSource::Kind::File) {
            // a quarter into it: container timestamps are rounded to their timescale, and the
            // accurate seek shows the picture overlapping the target
            position += frame / 4;
        }
        // (videotestsrc stamps its pictures from the seek position: they stay on the grid)
    }
    gst_element_seek_simple(m_pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), position);
}

void CameraPipeline::stepForward()
{
    if (!m_pipeline) {
        return;
    }
    if (m_state != State::Paused) {
        pauseAndWait(); // the pipeline prerolls the picture after the last one shown
        return;
    }
    // one buffer forward, at normal rate; flush: a new step replaces one still running
    gst_element_send_event(m_pipeline, gst_event_new_step(GST_FORMAT_BUFFERS, 1, 1.0, TRUE, FALSE));
}

void CameraPipeline::stepBackward()
{
    if (!m_pipeline) {
        return;
    }
    if (m_state != State::Paused && !pauseAndWait()) {
        return;
    }
    // GStreamer cannot step backwards: an accurate seek to the previous picture decodes from the
    // key frame before it and shows exactly that picture
    const qint64 index = frameNumber();
    if (index > 0) {
        seek(frameTime(index - 1));
    }
}

// ---- bus ----------------------------------------------------------------------------------------

void CameraPipeline::queryDuration()
{
    if (m_source.kind != CameraSource::Kind::File) {
        return;
    }
    gint64 duration = 0;
    if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &duration) && duration > 0 && duration != m_duration) {
        m_duration = duration;
        emit durationChanged(duration);
    }
}

void CameraPipeline::reachedEnd()
{
    if (m_loop) {
        gst_element_seek_simple(m_pipeline, GST_FORMAT_TIME, GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), 0);
    } else {
        pause();
    }
}

void CameraPipeline::pollBus()
{
    if (!m_bus) {
        return;
    }
    while (GstMessage* message = gst_bus_pop(m_bus)) {
        switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError* error = nullptr;
            gchar* debug = nullptr;
            gst_message_parse_error(message, &error, &debug);
            const QString text = QStringLiteral("%1: %2").arg(m_source.name, error ? QString::fromUtf8(error->message) : tr("stream error"));
            qWarning("%s (%s)", qPrintable(text), debug ? debug : "");
            g_clear_error(&error);
            g_free(debug);
            gst_element_set_state(m_pipeline, GST_STATE_READY);
            fail(text);
            break;
        }
        case GST_MESSAGE_EOS:
            // a stepped or paused camera stays on its last picture
            if (m_state == State::Playing) {
                reachedEnd();
            }
            break;
        case GST_MESSAGE_DURATION_CHANGED:
        case GST_MESSAGE_ASYNC_DONE:
            queryDuration();
            break;
        default:
            break;
        }
        gst_message_unref(message);
    }
}
