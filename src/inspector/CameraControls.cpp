#include "inspector/CameraControls.hpp"

#include "Icons.hpp"
#include "WidgetStyle.hpp"
#include "inspector/CameraPipeline.hpp"

#include <QEvent>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpression>
#include <QSlider>

namespace {

constexpr qint64 Millisecond = 1'000'000; ///< ns

int toMs(qint64 time)
{
    return int(std::clamp<qint64>(time / Millisecond, 0, std::numeric_limits<int>::max()));
}

QFont readoutFont(int pixelSize, bool bold)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(pixelSize);
    font.setBold(bold);
    return font;
}

/// Round transport key: as high as the design's controls, and as wide.
QPushButton* transportButton(const QString& tip)
{
    auto* button = new QPushButton;
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus); // keys stay with the window's shortcuts (Space, E, Q...)
    button->setIconSize(QSize(16, 16));
    return button;
}

} // namespace

// ---- readout ------------------------------------------------------------------------------------

TimeReadout::TimeReadout(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void TimeReadout::setText(const QString& time, const QString& detail)
{
    if (m_time != time || m_detail != detail) {
        const bool longer = time.size() != m_time.size() || detail.size() != m_detail.size();
        m_time = time;
        m_detail = detail;
        if (longer) {
            updateGeometry();
        }
        update();
    }
}

QSize TimeReadout::sizeHint() const
{
    // as wide as its longest line with every digit at its widest: stable while the numbers change
    const auto widest = [](QString text) { return text.replace(QRegularExpression(QStringLiteral("\\d")), QStringLiteral("0")); };
    const QFontMetrics time(readoutFont(12, true));
    const QFontMetrics detail(readoutFont(11, false));
    const int width = std::max({ time.horizontalAdvance(QStringLiteral("00:00.000")), time.horizontalAdvance(widest(m_time)), detail.horizontalAdvance(widest(m_detail)) });
    return { width + 20, time.height() + detail.height() + 6 };
}

void TimeReadout::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    QColor ink = palette().color(QPalette::Text);
    if (style) {
        style->display(p, rect());
        ink = style->displayText();
    }
    const QRect text = rect().adjusted(10, 2, -10, -2);
    p.setPen(ink);
    p.setFont(readoutFont(12, true));
    p.drawText(text.adjusted(0, 0, 0, -text.height() / 2), Qt::AlignLeft | Qt::AlignBottom, m_time);
    ink.setAlphaF(ink.alphaF() * 0.75f);
    p.setPen(ink);
    p.setFont(readoutFont(11, false));
    p.drawText(text.adjusted(0, text.height() / 2, 0, 0), Qt::AlignLeft | Qt::AlignTop, m_detail);
}

// ---- controls -----------------------------------------------------------------------------------

CameraControls::CameraControls(CameraPipeline* camera, QWidget* parent)
    : QWidget(parent)
    , m_camera(camera)
{
    m_previous = transportButton(tr("Previous frame (Q)"));
    connect(m_previous, &QPushButton::clicked, camera, &CameraPipeline::stepBackward);
    m_play = transportButton(tr("Play / pause (Space)"));
    connect(m_play, &QPushButton::clicked, camera, &CameraPipeline::togglePlay);
    m_stop = transportButton(tr("Stop (S)"));
    connect(m_stop, &QPushButton::clicked, camera, &CameraPipeline::stop);
    m_next = transportButton(tr("Next frame (E)"));
    connect(m_next, &QPushButton::clicked, camera, &CameraPipeline::stepForward);

    m_timeBar = new QSlider(Qt::Horizontal);
    m_timeBar->setRange(0, 0);
    m_timeBar->setPageStep(5000);
    m_timeBar->setMinimumWidth(60);
    m_timeBar->setFocusPolicy(Qt::ClickFocus);
    m_timeBar->setToolTip(tr("Time bar: drag or click to move along the recording"));
    connect(m_timeBar, &QSlider::sliderPressed, this, &CameraControls::beginScrub);
    connect(m_timeBar, &QSlider::valueChanged, this, &CameraControls::scrub);
    connect(m_timeBar, &QSlider::sliderReleased, this, &CameraControls::endScrub);

    m_readout = new TimeReadout;

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);
    row->addWidget(m_previous);
    row->addWidget(m_play);
    row->addWidget(m_stop);
    row->addWidget(m_next);
    row->addSpacing(4);
    row->addWidget(m_timeBar, 1);
    row->addSpacing(4);
    row->addWidget(m_readout);

    m_seekTimer.setSingleShot(true);
    m_seekTimer.setInterval(40);
    connect(&m_seekTimer, &QTimer::timeout, this, [this] { m_camera->seek(m_seekTarget * Millisecond); });

    connect(camera, &CameraPipeline::frameReady, this, [this] {
        if (isVisible()) {
            updatePosition();
        }
    });
    connect(camera, &CameraPipeline::stateChanged, this, &CameraControls::updateTransport);
    connect(camera, &CameraPipeline::durationChanged, this, &CameraControls::updateDuration);
    tintIcons();
}

void CameraControls::showEvent(QShowEvent* event)
{
    // hidden controls do not follow the camera: catch up
    updateDuration();
    updateTransport();
    updatePosition();
    QWidget::showEvent(event);
}

void CameraControls::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::StyleChange || event->type() == QEvent::PaletteChange) {
        tintIcons();
    }
    QWidget::changeEvent(event);
}

QColor CameraControls::ink() const
{
    // the controls lie on the camera's card: glyphs in the colour the design gives tiles
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    return style ? style->tileText() : palette().color(QPalette::ButtonText);
}

void CameraControls::tintIcons()
{
    // round keys, as wide as the design's controls are high
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    const int size = style ? style->metrics().control : 30;
    for (QPushButton* button : { m_previous, m_play, m_stop, m_next }) {
        button->setFixedSize(size, size);
    }
    const QColor ink = this->ink();
    m_previous->setIcon(Icons::icon(Icon::FramePrevious, ink));
    m_stop->setIcon(Icons::icon(Icon::Stop, ink));
    m_next->setIcon(Icons::icon(Icon::FrameNext, ink));
    updateTransport();
}

void CameraControls::updateTransport()
{
    const bool playing = m_camera->state() == CameraPipeline::State::Playing;
    m_play->setIcon(Icons::icon(playing ? Icon::Pause : Icon::Play, ink()));
    m_play->setToolTip(playing ? tr("Pause (Space)") : tr("Play (Space)"));
}

void CameraControls::updateDuration()
{
    const QSignalBlocker block(m_timeBar);
    m_timeBar->setRange(0, toMs(m_camera->duration()));
    const qint64 frame = m_camera->frameDuration();
    m_timeBar->setSingleStep(frame > 0 ? std::max(1, toMs(frame)) : 40);
}

void CameraControls::updatePosition()
{
    const qint64 position = m_camera->position();
    const qint64 frame = m_camera->frameDuration();
    if (m_timeBar->maximum() == 0 && m_camera->duration() > 0) {
        updateDuration();
    }
    if (frame > 0 && m_timeBar->singleStep() != std::max(1, toMs(frame))) {
        updateDuration(); // the frame rate is known from the first picture
    }
    // while the user moves the time bar it shows where they are going, not the last picture
    if (!m_timeBar->isSliderDown() && !m_seekTimer.isActive()) {
        const QSignalBlocker block(m_timeBar);
        m_timeBar->setValue(toMs(position));
    }
    // compact, so the time bar keeps the room: the time, then picture number / pictures in all
    const bool stopped = m_camera->state() == CameraPipeline::State::Stopped;
    const qint64 frames = frame > 0 ? m_camera->frameAt(m_camera->duration()) : 0;
    const QString total = frames > 0 ? QString::number(frames) : QStringLiteral("-");
    const QString time = stopped ? QStringLiteral("--:--.---") : CameraPipeline::timecode(position);
    const QString picture = stopped ? QStringLiteral("-") : QString::number(m_camera->frameNumber());
    m_readout->setText(time, QStringLiteral("#%1 / %2").arg(picture, total));
    QString tip = tr("%1 of %2").arg(time, CameraPipeline::timecode(m_camera->duration()));
    if (frame > 0) {
        tip += tr("\npicture %1 of %2 · %3 fps").arg(stopped ? QStringLiteral("-") : QString::number(m_camera->frameNumber()), total).arg(1e9 / double(frame), 0, 'f', 2);
    }
    m_readout->setToolTip(tip);
}

void CameraControls::beginScrub()
{
    m_resumeAfterScrub = m_camera->state() == CameraPipeline::State::Playing;
    m_camera->pause();
}

void CameraControls::scrub(int ms)
{
    m_seekTarget = ms;
    if (!m_seekTimer.isActive()) {
        m_seekTimer.start();
    }
}

void CameraControls::endScrub()
{
    m_seekTimer.stop();
    m_camera->seek(qint64(m_timeBar->value()) * Millisecond);
    if (m_resumeAfterScrub) {
        m_camera->play();
    }
    m_resumeAfterScrub = false;
}
