#include "inspector/CameraTile.hpp"

#include "WidgetStyle.hpp"
#include "inspector/CameraPipeline.hpp"

#include <QFontDatabase>
#include <QMouseEvent>
#include <QPainter>

namespace {

constexpr int Spill = 6; ///< room round the card for the selection mark
constexpr int LampSize = 10;

QFont overlayFont(int pixelSize)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(pixelSize);
    font.setBold(true);
    return font;
}

QColor stateColor(CameraPipeline::State state)
{
    switch (state) {
    case CameraPipeline::State::Playing:
        return { 0x3d, 0xdc, 0x84 };
    case CameraPipeline::State::Paused:
        return { 0xff, 0xb0, 0x20 };
    case CameraPipeline::State::Stopped:
    case CameraPipeline::State::Error:
        break;
    }
    return { 0xf0, 0x48, 0x3c };
}

QString stateName(CameraPipeline::State state)
{
    switch (state) {
    case CameraPipeline::State::Playing:
        return CameraTile::tr("PLAY");
    case CameraPipeline::State::Paused:
        return CameraTile::tr("PAUSE");
    case CameraPipeline::State::Stopped:
        return CameraTile::tr("STOP");
    case CameraPipeline::State::Error:
        break;
    }
    return CameraTile::tr("ERROR");
}

/// Dark translucent chip that keeps overlay text legible on any picture.
void chip(QPainter& p, const QRect& rect)
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 150));
    p.drawRoundedRect(QRectF(rect), 4, 4);
    p.restore();
}

} // namespace

CameraTile::CameraTile(CameraPipeline* camera, QWidget* parent)
    : QWidget(parent)
    , m_camera(camera)
{
    setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::PointingHandCursor);
    setToolTip(tr("%1 — %2\nClick to inspect, double click to maximise").arg(camera->source().name, camera->source().label));
    // a new picture only repaints the screen; a new state also the lamp
    connect(camera, &CameraPipeline::frameReady, this, [this] { update(screenRect()); });
    connect(camera, &CameraPipeline::stateChanged, this, [this] { update(); });
}

void CameraTile::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;
        m_chrome = QPixmap();
        update();
    }
}

QSize CameraTile::sizeHint() const
{
    return { 400, 300 };
}

QSize CameraTile::minimumSizeHint() const
{
    return { 200, 160 };
}

QRect CameraTile::cardRect() const
{
    return rect().adjusted(Spill, Spill, -Spill, -Spill);
}

QRect CameraTile::screenRect() const
{
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    const WidgetStyle::Metrics m = style ? style->metrics() : WidgetStyle::Metrics {};
    return cardRect().adjusted(m.margin, m.title + m.spacing / 2, -m.margin, -m.margin);
}

void CameraTile::paintChrome()
{
    const qreal dpr = devicePixelRatioF();
    m_chrome = QPixmap(size() * dpr);
    m_chrome.setDevicePixelRatio(dpr);
    m_chrome.fill(Qt::transparent);
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    if (!style) {
        return;
    }
    QPainter p(&m_chrome);
    const QRect card = cardRect();
    const QRect title(card.topLeft(), QSize(card.width(), style->metrics().title));
    const QString caption = fontMetrics().elidedText(QStringLiteral("%1 · %2").arg(m_camera->source().name, m_camera->source().label), Qt::ElideRight, card.width() - 72);
    style->card(p, this, card, title, caption);
    style->display(p, screenRect());
    if (m_selected) {
        style->focusFrame(p, card);
    }
}

void CameraTile::paintEvent(QPaintEvent*)
{
    if (m_chrome.isNull() || m_chrome.size() != size() * devicePixelRatioF()) {
        paintChrome();
    }
    QPainter p(this);
    p.drawPixmap(0, 0, m_chrome);

    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    const int inset = (style ? style->metrics().frame : 2) + 2;
    const QRect screen = screenRect().adjusted(inset, inset, -inset, -inset);
    if (screen.width() < 8 || screen.height() < 8) {
        return;
    }
    p.setClipRect(screen);

    const QImage frame = m_camera->frame();
    if (!frame.isNull()) {
        QRect target(QPoint(), frame.size().scaled(screen.size(), Qt::KeepAspectRatio));
        target.moveCenter(screen.center());
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.drawImage(target, frame);
    } else {
        p.setFont(overlayFont(std::clamp(screen.height() / 10, 11, 22)));
        p.setPen(style ? style->displayText() : palette().color(QPalette::Text));
        const bool failed = m_camera->state() == CameraPipeline::State::Error;
        p.drawText(screen, Qt::AlignCenter, failed ? tr("SIGNAL LOST") : tr("NO SIGNAL"));
    }
    paintOverlay(p, screen);
}

void CameraTile::paintOverlay(QPainter& p, const QRect& screen)
{
    const auto* style = qobject_cast<const WidgetStyle*>(this->style());
    const CameraPipeline::State state = m_camera->state();
    p.setFont(overlayFont(11));
    const QFontMetrics fm(p.font());
    const int pad = 6, h = fm.height() + 4;

    // state: lamp and name, top left
    const QString name = stateName(state);
    const QRect top(screen.left() + pad, screen.top() + pad, LampSize + 6 + fm.horizontalAdvance(name) + 12, h);
    chip(p, top);
    const QRect lampRect(top.left() + 6, top.center().y() - LampSize / 2 + 1, LampSize, LampSize);
    const bool lit = state != CameraPipeline::State::Stopped;
    if (style) {
        style->lamp(p, lampRect, stateColor(state), lit);
    }
    p.setPen(Qt::white);
    p.drawText(top.adjusted(LampSize + 12, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, name);

    // time and picture number, bottom left
    if (state == CameraPipeline::State::Stopped) {
        return;
    }
    const QString time = QStringLiteral("%1  #%2").arg(CameraPipeline::timecode(m_camera->position())).arg(m_camera->frameNumber());
    const QRect bottom(screen.left() + pad, screen.bottom() - pad - h + 1, fm.horizontalAdvance(time) + 12, h);
    chip(p, bottom);
    p.drawText(bottom, Qt::AlignCenter, time);
}

void CameraTile::resizeEvent(QResizeEvent* event)
{
    m_chrome = QPixmap();
    QWidget::resizeEvent(event);
}

void CameraTile::moveEvent(QMoveEvent* event)
{
    m_chrome = QPixmap(); // glass samples the backdrop under the card
    QWidget::moveEvent(event);
}

void CameraTile::changeEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::StyleChange:
    case QEvent::PaletteChange:
    case QEvent::FontChange:
        m_chrome = QPixmap();
        update();
        break;
    default:
        break;
    }
    QWidget::changeEvent(event);
}

void CameraTile::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
    QWidget::mousePressEvent(event);
}

void CameraTile::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QWidget::mouseDoubleClickEvent(event);
}
