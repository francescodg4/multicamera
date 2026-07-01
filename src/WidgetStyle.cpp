#include "WidgetStyle.hpp"

#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHeaderView>
#include <QLCDNumber>
#include <QLineEdit>
#include <QMenuBar>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QSlider>
#include <QStyleOption>
#include <QTabBar>

#include <cmath>

namespace {

WidgetStyle::Look lookOf(const QStyleOption* option)
{
    WidgetStyle::Look look;
    look.enabled = option->state & QStyle::State_Enabled;
    look.hover = look.enabled && (option->state & QStyle::State_MouseOver);
    look.pressed = look.enabled && (option->state & QStyle::State_Sunken);
    look.focus = look.enabled && (option->state & QStyle::State_HasFocus);
    look.focusVisible = look.focus && (option->state & QStyle::State_KeyboardFocusChange);
    look.checked = option->state & QStyle::State_On;
    return look;
}

/// Square of @p size centred in @p area.
QRect centred(const QRect& area, int size)
{
    QRect r(0, 0, size, size);
    r.moveCenter(area.center());
    return r;
}

/// Handle of a scroll bar: its length is proportional to the visible page.
QRect scrollHandle(const QStyleOptionSlider* s, int minLength)
{
    const bool horizontal = s->orientation == Qt::Horizontal;
    const int length = horizontal ? s->rect.width() : s->rect.height();
    const qint64 range = qint64(s->maximum) - s->minimum;
    int handle = range > 0 ? int(qint64(s->pageStep) * length / (range + s->pageStep)) : length;
    handle = std::clamp(handle, std::min(minLength, length), length);
    const int pos = QStyle::sliderPositionFromValue(s->minimum, s->maximum, s->sliderPosition, length - handle, s->upsideDown);
    return horizontal ? QRect(s->rect.left() + pos, s->rect.top(), handle, s->rect.height())
                      : QRect(s->rect.left(), s->rect.top() + pos, s->rect.width(), handle);
}

int arrowWidth(const QRect& rect)
{
    return std::min(rect.height(), 28);
}

/// The list of a combo box's popup (it lies in a container whose parent is the combo box).
bool isComboPopup(const QAbstractItemView* view)
{
    return view->parentWidget() && qobject_cast<const QComboBox*>(view->parentWidget()->parentWidget());
}

} // namespace

WidgetStyle::WidgetStyle(const Metrics& metrics)
    : QProxyStyle(QStringLiteral("Fusion"))
    , m(metrics)
{
}

QColor WidgetStyle::buttonText(Button, const Look& look, const QPalette& palette) const
{
    return palette.color(look.enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
}

void WidgetStyle::spinButton(QPainter& p, const QRect& rect, Qt::ArrowType type, const Look& look) const
{
    arrow(p, centred(rect, m.arrow), type, look);
}

QColor WidgetStyle::menuBarText(bool) const
{
    return standardPalette().color(QPalette::WindowText);
}

void WidgetStyle::tile(QPainter& p, const QWidget* widget, const QRect& rect, const QRect& titleRect, const QString& title, int) const
{
    card(p, widget, rect, titleRect, title);
}

QColor WidgetStyle::tileText() const
{
    return standardPalette().color(QPalette::ButtonText);
}

void WidgetStyle::focusFrame(QPainter& p, const QRect& rect) const
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(standardPalette().color(QPalette::Highlight), 2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(QRectF(rect).adjusted(1, 1, -1, -1), 6, 6);
    p.restore();
}

void WidgetStyle::lamp(QPainter& p, const QRect& rect, const QColor& color, bool lit) const
{
    p.save();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(color.darker(170), 1));
    p.setBrush(lit ? color : color.darker(260));
    p.drawEllipse(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));
    p.restore();
}

QPointF WidgetStyle::dialPoint(const QRectF& rect, qreal value, qreal radius)
{
    const qreal angle = (225.0 - 270.0 * std::clamp(value, 0.0, 1.0)) * M_PI / 180.0;
    return rect.center() + QPointF(std::cos(angle), -std::sin(angle)) * radius;
}

// ---- primitives ---------------------------------------------------------------------------------

void WidgetStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* p, const QWidget* widget) const
{
    const Look look = lookOf(option);
    switch (element) {
    case PE_PanelButtonCommand: {
        Button kind = Button::Normal;
        if (const auto* b = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            if (b->features & QStyleOptionButton::Flat) {
                kind = Button::Flat;
            } else if (b->features & QStyleOptionButton::DefaultButton) {
                kind = Button::Default;
            }
        }
        button(*p, option->rect, kind, look);
        return;
    }
    case PE_PanelButtonTool:
    case PE_PanelButtonBevel:
        button(*p, option->rect, Button::Normal, look);
        return;
    case PE_PanelLineEdit:
        // the line edit of a spin box or an editable combo box lies on that control's field
        if (widget && (qobject_cast<const QAbstractSpinBox*>(widget->parentWidget()) || qobject_cast<const QComboBox*>(widget->parentWidget()))) {
            return;
        }
        field(*p, option->rect, look);
        return;
    case PE_Frame:
        view(*p, option->rect, look);
        return;
    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck:
        check(*p, option->rect, option->state & State_NoChange ? Qt::PartiallyChecked : option->state & State_On ? Qt::Checked : Qt::Unchecked, look);
        return;
    case PE_IndicatorRadioButton:
        radio(*p, option->rect, option->state & State_On, look);
        return;
    case PE_IndicatorArrowUp:
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinPlus:
        arrow(*p, centred(option->rect, std::min(m.arrow, std::min(option->rect.width(), option->rect.height()))), Qt::UpArrow, look);
        return;
    case PE_IndicatorArrowDown:
    case PE_IndicatorSpinDown:
    case PE_IndicatorSpinMinus:
        arrow(*p, centred(option->rect, std::min(m.arrow, std::min(option->rect.width(), option->rect.height()))), Qt::DownArrow, look);
        return;
    case PE_IndicatorArrowLeft:
        arrow(*p, centred(option->rect, std::min(m.arrow, std::min(option->rect.width(), option->rect.height()))), Qt::LeftArrow, look);
        return;
    case PE_IndicatorArrowRight:
        arrow(*p, centred(option->rect, std::min(m.arrow, std::min(option->rect.width(), option->rect.height()))), Qt::RightArrow, look);
        return;
    case PE_FrameTabWidget:
        tabPane(*p, widget, option->rect);
        return;
    case PE_PanelItemViewItem: {
        const auto* item = qstyleoption_cast<const QStyleOptionViewItem*>(option);
        if (item && (item->features & QStyleOptionViewItem::Alternate)) {
            p->fillRect(option->rect, option->palette.brush(QPalette::AlternateBase));
        }
        if (!(option->state & (State_Selected | State_MouseOver)) || !look.enabled) {
            return;
        }
        // cells of one row share a single selection shape: open the sides that continue
        // (tree views say where the cell is; for tables selecting rows it follows from the column)
        auto position = item ? item->viewItemPosition : QStyleOptionViewItem::Invalid;
        const auto* view = qobject_cast<const QAbstractItemView*>(widget);
        if (position == QStyleOptionViewItem::Invalid && view && view->selectionBehavior() == QAbstractItemView::SelectRows && item->index.isValid()) {
            const int last = item->index.model()->columnCount(item->index.parent()) - 1;
            const int column = item->index.column();
            position = last == 0 ? QStyleOptionViewItem::OnlyOne
                : column == 0    ? QStyleOptionViewItem::Beginning
                : column == last ? QStyleOptionViewItem::End
                                 : QStyleOptionViewItem::Middle;
        }
        QRect shape = option->rect;
        if (position == QStyleOptionViewItem::Middle || position == QStyleOptionViewItem::End) {
            shape.setLeft(shape.left() - 32);
        }
        if (position == QStyleOptionViewItem::Middle || position == QStyleOptionViewItem::Beginning) {
            shape.setRight(shape.right() + 32);
        }
        Look row = look;
        row.checked = option->state & State_Selected;
        p->save();
        p->setClipRect(option->rect);
        selection(*p, shape, row);
        p->restore();
        return;
    }
    case PE_PanelMenu:
        menu(*p, option->rect);
        return;
    case PE_PanelTipLabel:
        tooltip(*p, option->rect);
        return;
    // drawn as part of their control, or not at all
    case PE_FrameDefaultButton:
    case PE_FrameFocusRect:
    case PE_FrameLineEdit:
    case PE_FrameGroupBox:
    case PE_FrameTabBarBase:
    case PE_FrameStatusBarItem:
    case PE_FrameMenu:
    case PE_FrameButtonTool:
    case PE_IndicatorButtonDropDown:
    case PE_PanelScrollAreaCorner:
    case PE_PanelMenuBar:
        return;
    default:
        break;
    }
    QProxyStyle::drawPrimitive(element, option, p, widget);
}

// ---- controls -----------------------------------------------------------------------------------

void WidgetStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* p, const QWidget* widget) const
{
    const Look look = lookOf(option);
    switch (element) {
    case CE_PushButtonBevel:
        proxy()->drawPrimitive(PE_PanelButtonCommand, option, p, widget);
        return;
    case CE_PushButtonLabel:
        if (const auto* b = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            const Button kind = b->features & QStyleOptionButton::Flat ? Button::Flat
                : b->features & QStyleOptionButton::DefaultButton  ? Button::Default
                                                                    : Button::Normal;
            QStyleOptionButton label(*b);
            label.palette.setColor(QPalette::ButtonText, buttonText(kind, look, b->palette));
            QProxyStyle::drawControl(element, &label, p, widget);
            return;
        }
        break;
    case CE_ProgressBarGroove:
        return; // drawn with the contents
    case CE_ProgressBarContents:
        if (const auto* bar = qstyleoption_cast<const QStyleOptionProgressBar*>(option)) {
            const bool horizontal = bar->state & State_Horizontal;
            const qint64 range = qint64(bar->maximum) - bar->minimum;
            const qreal fraction = range > 0 ? std::clamp(qreal(bar->progress - bar->minimum) / range, 0.0, 1.0) : 0.0;
            const QRect r = option->rect;
            QRect filled = r;
            if (horizontal) {
                const int w = int(std::lround(r.width() * fraction));
                filled = bar->invertedAppearance ? QRect(r.right() - w + 1, r.top(), w, r.height()) : QRect(r.left(), r.top(), w, r.height());
            } else {
                const int h = int(std::lround(r.height() * fraction));
                filled = bar->invertedAppearance ? QRect(r.left(), r.top(), r.width(), h) : QRect(r.left(), r.bottom() - h + 1, r.width(), h);
            }
            progress(*p, r, filled, horizontal ? Qt::Horizontal : Qt::Vertical);
            return;
        }
        break;
    case CE_TabBarTabShape:
        tab(*p, option->rect, option->state & State_Selected, look);
        return;
    case CE_HeaderSection:
        header(*p, option->rect, look);
        return;
    case CE_HeaderEmptyArea:
        header(*p, option->rect, Look {});
        return;
    case CE_MenuBarEmptyArea:
        menuBar(*p, widget, widget ? widget->rect() : option->rect);
        return;
    case CE_MenuBarItem:
        if (const auto* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            p->save();
            p->setClipRect(option->rect);
            menuBar(*p, widget, widget ? widget->rect() : option->rect);
            p->restore();
            const bool active = look.enabled && (option->state & State_Selected);
            if (active) {
                highlight(*p, option->rect, true);
            }
            int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextSingleLine;
            if (!proxy()->styleHint(SH_UnderlineShortcut, item, widget)) {
                flags |= Qt::TextHideMnemonic;
            }
            p->save();
            QColor text = menuBarText(active);
            if (!look.enabled) {
                text.setAlphaF(0.45f);
            }
            p->setPen(text);
            p->drawText(option->rect, flags, item->text);
            p->restore();
            return;
        }
        break;
    case CE_MenuItem:
        if (const auto* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option)) {
            if (item->menuItemType == QStyleOptionMenuItem::Separator) {
                QColor line = option->palette.color(QPalette::WindowText);
                line.setAlphaF(0.25f);
                p->fillRect(QRect(option->rect.left() + 8, option->rect.center().y(), option->rect.width() - 16, 1), line);
                return;
            }
            // menus are drawn like views, so their items take the view's text colour
            QStyleOptionMenuItem copy(*item);
            const bool active = look.enabled && (option->state & State_Selected);
            if (active) {
                highlight(*p, option->rect, false);
                copy.state &= ~State_Selected;
            }
            const QColor text = item->palette.color(QPalette::Active, active ? QPalette::HighlightedText : QPalette::Text);
            for (const QPalette::ColorRole role : { QPalette::Text, QPalette::WindowText, QPalette::ButtonText }) {
                copy.palette.setColor(QPalette::Active, role, text);
                copy.palette.setColor(QPalette::Inactive, role, text);
            }
            QProxyStyle::drawControl(element, &copy, p, widget);
            return;
        }
        break;
    case CE_MenuEmptyArea:
        return;
    case CE_ShapedFrame:
        if (qobject_cast<const QLCDNumber*>(widget)) {
            display(*p, option->rect);
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawControl(element, option, p, widget);
}

// ---- complex controls ---------------------------------------------------------------------------

void WidgetStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* p, const QWidget* widget) const
{
    const Look look = lookOf(option);
    switch (control) {
    case CC_Slider:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            const bool horizontal = s->orientation == Qt::Horizontal;
            const QRect knob = proxy()->subControlRect(CC_Slider, s, SC_SliderHandle, widget);
            const QPoint c = knob.center();
            // the track runs between the handle's centre at the minimum and at the maximum
            QRect track = horizontal ? QRect(s->rect.left() + knob.width() / 2, c.y() - m.groove / 2, s->rect.width() - knob.width(), m.groove)
                                     : QRect(c.x() - m.groove / 2, s->rect.top() + knob.height() / 2, m.groove, s->rect.height() - knob.height());
            QRect filled = track;
            if (horizontal) {
                s->upsideDown ? filled.setLeft(c.x()) : filled.setRight(c.x());
            } else {
                s->upsideDown ? filled.setTop(c.y()) : filled.setBottom(c.y());
            }

            if (s->tickPosition != QSlider::NoTicks) {
                QColor tick = s->palette.color(QPalette::WindowText);
                tick.setAlphaF(0.35f);
                const int interval = s->tickInterval > 0 ? s->tickInterval : std::max(1, s->pageStep);
                const int span = horizontal ? track.width() : track.height();
                for (int v = s->minimum; v <= s->maximum; v += interval) {
                    const int pos = sliderPositionFromValue(s->minimum, s->maximum, v, span, s->upsideDown);
                    if (horizontal) {
                        const int x = track.left() + pos;
                        if (s->tickPosition & QSlider::TicksAbove) {
                            p->fillRect(QRect(x, knob.top() + 1, 1, track.top() - knob.top() - 3), tick);
                        }
                        if (s->tickPosition & QSlider::TicksBelow) {
                            p->fillRect(QRect(x, track.bottom() + 3, 1, knob.bottom() - track.bottom() - 3), tick);
                        }
                    } else {
                        const int y = track.top() + pos;
                        if (s->tickPosition & QSlider::TicksLeft) {
                            p->fillRect(QRect(knob.left() + 1, y, track.left() - knob.left() - 3, 1), tick);
                        }
                        if (s->tickPosition & QSlider::TicksRight) {
                            p->fillRect(QRect(track.right() + 3, y, knob.right() - track.right() - 3, 1), tick);
                        }
                    }
                }
            }
            groove(*p, track, filled, s->orientation, look);
            Look h = look;
            h.hover = look.hover && (s->activeSubControls & SC_SliderHandle);
            h.pressed = look.enabled && (s->state & State_Sunken) && (s->activeSubControls & SC_SliderHandle);
            handle(*p, knob, s->orientation, h);
            return;
        }
        break;
    case CC_ScrollBar:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            Look h = look;
            h.hover = look.hover && (s->activeSubControls & SC_ScrollBarSlider);
            h.pressed = look.enabled && (s->state & State_Sunken) && (s->activeSubControls & SC_ScrollBarSlider);
            scrollBar(*p, s->rect, scrollHandle(s, pixelMetric(PM_ScrollBarSliderMin)), s->orientation, h);
            return;
        }
        break;
    case CC_ComboBox:
        if (const auto* c = qstyleoption_cast<const QStyleOptionComboBox*>(option)) {
            const QRect a = proxy()->subControlRect(CC_ComboBox, c, SC_ComboBoxArrow, widget);
            if (c->editable) {
                // a field with a drop-down part, like a spin box's step buttons
                field(*p, c->rect, look);
                Look part = look;
                part.hover = look.hover && (c->activeSubControls & SC_ComboBoxArrow);
                part.pressed = look.pressed && (c->activeSubControls & SC_ComboBoxArrow);
                spinButton(*p, a.adjusted(0, m.frame, 0, -m.frame), Qt::DownArrow, part);
            } else {
                button(*p, c->rect, Button::Normal, look);
                arrow(*p, centred(a, m.arrow), Qt::DownArrow, look);
            }
            return;
        }
        break;
    case CC_SpinBox:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSpinBox*>(option)) {
            if (s->frame) {
                field(*p, s->rect, look);
            }
            const auto part = [&](SubControl sc, QAbstractSpinBox::StepEnabledFlag step) {
                Look l = look;
                l.enabled = look.enabled && (s->stepEnabled & step);
                l.hover = l.enabled && look.hover && (s->activeSubControls & sc);
                l.pressed = l.enabled && (s->state & State_Sunken) && (s->activeSubControls & sc);
                return l;
            };
            spinButton(*p, proxy()->subControlRect(CC_SpinBox, s, SC_SpinBoxUp, widget), Qt::UpArrow, part(SC_SpinBoxUp, QAbstractSpinBox::StepUpEnabled));
            spinButton(*p, proxy()->subControlRect(CC_SpinBox, s, SC_SpinBoxDown, widget), Qt::DownArrow, part(SC_SpinBoxDown, QAbstractSpinBox::StepDownEnabled));
            return;
        }
        break;
    case CC_Dial:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            const qint64 range = qint64(s->maximum) - s->minimum;
            const qreal value = range > 0 ? qreal(s->sliderPosition - s->minimum) / range : 0.0;
            dial(*p, centred(s->rect, std::min(s->rect.width(), s->rect.height())), value, look);
            return;
        }
        break;
    case CC_ToolButton:
        if (const auto* t = qstyleoption_cast<const QStyleOptionToolButton*>(option)) {
            const QRect face = proxy()->subControlRect(CC_ToolButton, t, SC_ToolButton, widget);
            const QRect menuPart = proxy()->subControlRect(CC_ToolButton, t, SC_ToolButtonMenu, widget);
            const bool split = t->features & QStyleOptionToolButton::MenuButtonPopup;
            Look l = look;
            l.pressed = look.enabled && (t->state & State_Sunken) && (!split || (t->activeSubControls & SC_ToolButton));
            l.pressed = l.pressed || (look.enabled && split && (t->activeSubControls & SC_ToolButtonMenu) && (t->state & State_Sunken));
            if (!(t->state & State_AutoRaise) || l.hover || l.pressed || l.checked) {
                button(*p, t->rect, Button::Normal, l);
            }
            if (split) {
                QColor line = buttonText(Button::Normal, l, t->palette);
                line.setAlphaF(0.3f);
                p->fillRect(QRect(menuPart.left(), menuPart.top() + menuPart.height() / 4, 1, menuPart.height() / 2), line);
                arrow(*p, centred(menuPart, m.arrow), Qt::DownArrow, l);
            } else if (t->features & QStyleOptionToolButton::HasMenu) {
                arrow(*p, QRect(t->rect.right() - m.arrow - 4, t->rect.bottom() - m.arrow - 4, m.arrow, m.arrow), Qt::DownArrow, l);
            }
            QStyleOptionToolButton label(*t);
            label.rect = face.adjusted(m.frame, m.frame, -m.frame, -m.frame);
            label.palette.setColor(QPalette::ButtonText, buttonText(Button::Normal, l, t->palette));
            if (l.pressed && m.pressShift) {
                label.rect.translate(m.pressShift, m.pressShift);
            }
            proxy()->drawControl(CE_ToolButtonLabel, &label, p, widget);
            return;
        }
        break;
    case CC_GroupBox:
        if (const auto* g = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
            card(*p, widget, g->rect, proxy()->subControlRect(CC_GroupBox, g, SC_GroupBoxLabel, widget), g->text);
            return;
        }
        break;
    default:
        break;
    }
    QProxyStyle::drawComplexControl(control, option, p, widget);
}

// ---- geometry -----------------------------------------------------------------------------------

QRect WidgetStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl sc, const QWidget* widget) const
{
    const QRect r = option->rect;
    switch (control) {
    case CC_Slider:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            const QSize knob = handleSize(s->orientation);
            if (sc == SC_SliderHandle) {
                if (s->orientation == Qt::Horizontal) {
                    const int pos = sliderPositionFromValue(s->minimum, s->maximum, s->sliderPosition, r.width() - knob.width(), s->upsideDown);
                    return QRect(r.left() + pos, r.center().y() - knob.height() / 2, knob.width(), knob.height());
                }
                const int pos = sliderPositionFromValue(s->minimum, s->maximum, s->sliderPosition, r.height() - knob.height(), s->upsideDown);
                return QRect(r.center().x() - knob.width() / 2, r.top() + pos, knob.width(), knob.height());
            }
            if (sc == SC_SliderGroove) {
                return r; // the whole travel of the handle
            }
        }
        break;
    case CC_ScrollBar:
        if (const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
            // no step buttons: the groove is the whole bar
            const QRect knob = scrollHandle(s, pixelMetric(PM_ScrollBarSliderMin));
            const bool horizontal = s->orientation == Qt::Horizontal;
            switch (sc) {
            case SC_ScrollBarGroove:
                return r;
            case SC_ScrollBarSlider:
                return knob;
            case SC_ScrollBarSubPage:
                return horizontal ? QRect(r.left(), r.top(), knob.left() - r.left(), r.height()) : QRect(r.left(), r.top(), r.width(), knob.top() - r.top());
            case SC_ScrollBarAddPage:
                return horizontal ? QRect(knob.right() + 1, r.top(), r.right() - knob.right(), r.height()) : QRect(r.left(), knob.bottom() + 1, r.width(), r.bottom() - knob.bottom());
            case SC_ScrollBarAddLine:
            case SC_ScrollBarSubLine:
            case SC_ScrollBarFirst:
            case SC_ScrollBarLast:
                return {};
            default:
                break;
            }
        }
        break;
    case CC_ComboBox: {
        const int aw = arrowWidth(r);
        switch (sc) {
        case SC_ComboBoxFrame:
        case SC_ComboBoxListBoxPopup:
            return r;
        case SC_ComboBoxArrow:
            return QRect(r.right() - aw - m.frame + 1, r.top(), aw, r.height());
        case SC_ComboBoxEditField:
            return QRect(r.left() + m.padding, r.top() + m.frame, r.width() - m.padding - aw - m.frame, r.height() - 2 * m.frame);
        default:
            break;
        }
        break;
    }
    case CC_SpinBox: {
        const int bw = arrowWidth(r);
        const int inner = r.height() - 2 * m.frame;
        switch (sc) {
        case SC_SpinBoxFrame:
            return r;
        case SC_SpinBoxUp:
            return QRect(r.right() - bw - m.frame + 1, r.top() + m.frame, bw, inner / 2);
        case SC_SpinBoxDown:
            return QRect(r.right() - bw - m.frame + 1, r.top() + m.frame + inner / 2, bw, inner - inner / 2);
        case SC_SpinBoxEditField:
            return QRect(r.left() + m.padding, r.top() + m.frame, r.width() - m.padding - bw - m.frame, inner);
        default:
            break;
        }
        break;
    }
    case CC_GroupBox:
        if (const auto* g = qstyleoption_cast<const QStyleOptionGroupBox*>(option)) {
            const int title = g->text.isEmpty() ? 0 : m.title;
            switch (sc) {
            case SC_GroupBoxFrame:
                return r;
            case SC_GroupBoxLabel:
                return QRect(r.left(), r.top(), r.width(), title);
            case SC_GroupBoxContents:
                return r.adjusted(0, title, 0, 0);
            case SC_GroupBoxCheckBox:
                return {};
            default:
                break;
            }
        }
        break;
    default:
        break;
    }
    return QProxyStyle::subControlRect(control, option, sc, widget);
}

QRect WidgetStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget) const
{
    switch (element) {
    case SE_LineEditContents: {
        const int side = m.padding / 2 + m.frame;
        return option->rect.adjusted(side, m.frame, -side, -m.frame);
    }
    default:
        break;
    }
    return QProxyStyle::subElementRect(element, option, widget);
}

QSize WidgetStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    switch (type) {
    case CT_PushButton: {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        return QSize(std::max(s.width(), 2 * m.control), std::max(s.height(), m.control));
    }
    case CT_ToolButton: {
        const QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        return QSize(s.width() + m.padding, std::max(s.height(), m.control));
    }
    case CT_LineEdit:
        return QSize(size.width() + m.padding + 2 * m.frame, std::max(size.height() + 2 * m.frame, m.control));
    case CT_ComboBox:
    case CT_SpinBox:
        return QSize(size.width() + m.padding + arrowWidth(QRect(0, 0, m.control, m.control)) + 2 * m.frame + 4, std::max(size.height() + 2 * m.frame, m.control));
    case CT_TabBarTab: {
        const QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        return QSize(s.width(), std::max(s.height(), m.tab));
    }
    case CT_ItemViewItem:
    case CT_HeaderSection: {
        const QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        return QSize(s.width(), std::max(s.height(), m.row));
    }
    case CT_MenuBarItem:
        return QProxyStyle::sizeFromContents(type, option, size, widget) + QSize(8, 6);
    case CT_MenuItem: {
        const QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
        const auto* item = qstyleoption_cast<const QStyleOptionMenuItem*>(option);
        if (item && item->menuItemType == QStyleOptionMenuItem::Separator) {
            return s;
        }
        return QSize(s.width() + 8, std::max(s.height(), m.row + 2));
    }
    default:
        break;
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int WidgetStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric) {
    case PM_ButtonMargin:
        return m.padding;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        return m.pressShift;
    case PM_DefaultFrameWidth:
    case PM_SpinBoxFrameWidth:
    case PM_ComboBoxFrameWidth:
    case PM_MenuPanelWidth:
        return m.frame;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return m.indicator;
    case PM_CheckBoxLabelSpacing:
    case PM_RadioButtonLabelSpacing:
        return 8;
    case PM_SliderLength:
    case PM_SliderThickness:
    case PM_SliderControlThickness: {
        const auto* s = qstyleoption_cast<const QStyleOptionSlider*>(option);
        const Qt::Orientation o = s ? s->orientation : Qt::Horizontal;
        const QSize knob = handleSize(o);
        const bool along = metric == PM_SliderLength;
        return (o == Qt::Horizontal) == along ? knob.width() : knob.height();
    }
    case PM_ScrollBarExtent:
        return m.scroll;
    case PM_ScrollBarSliderMin:
        return 32;
    case PM_MenuButtonIndicator:
        return m.control * 2 / 3;
    case PM_TabBarTabHSpace:
        return 2 * m.padding;
    case PM_TabBarBaseOverlap:
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_TabBarBaseHeight:
    case PM_MenuBarPanelWidth:
        return 0;
    case PM_TabBarTabOverlap:
        return 0;
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return m.margin;
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return m.spacing;
    case PM_MenuBarVMargin:
        return 2;
    case PM_MenuBarHMargin:
        return 6;
    case PM_MenuBarItemSpacing:
        return 4;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        return 4;
    case PM_ToolTipLabelFrameWidth:
        return 6;
    default:
        break;
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

int WidgetStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const
{
    switch (hint) {
    case SH_Table_GridLineColor: {
        QColor grid = (option ? option->palette : standardPalette()).color(QPalette::Text);
        grid.setAlphaF(0.12f);
        return int(grid.rgba());
    }
    case SH_Slider_AbsoluteSetButtons:
        return Qt::LeftButton;
    case SH_ScrollBar_LeftClickAbsolutePosition:
    case SH_ItemView_ShowDecorationSelected:
        return true;
    case SH_ScrollBar_Transient:
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return false;
    default:
        break;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

void WidgetStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<QAbstractButton*>(widget) || qobject_cast<QComboBox*>(widget) || qobject_cast<QAbstractSpinBox*>(widget)
        || qobject_cast<QAbstractSlider*>(widget) || qobject_cast<QTabBar*>(widget) || qobject_cast<QLineEdit*>(widget)
        || qobject_cast<QHeaderView*>(widget) || qobject_cast<QMenuBar*>(widget)) {
        widget->setAttribute(Qt::WA_Hover);
    }
    if (auto* view = qobject_cast<QAbstractItemView*>(widget)) {
        view->viewport()->setAttribute(Qt::WA_Hover);
        if (isComboPopup(view)) {
            // its delegate fills every row with Window: let the popup's menu panel show through
            QPalette palette = view->palette();
            palette.setColor(QPalette::Window, Qt::transparent);
            view->setPalette(palette);
        }
    }
    if (auto* lcd = qobject_cast<QLCDNumber*>(widget)) {
        lcd->setSegmentStyle(QLCDNumber::Flat);
        QPalette palette = lcd->palette();
        palette.setColor(QPalette::WindowText, displayText());
        lcd->setPalette(palette);
    }
}

void WidgetStyle::unpolish(QWidget* widget)
{
    const auto* view = qobject_cast<QAbstractItemView*>(widget);
    if (qobject_cast<QLCDNumber*>(widget) || (view && isComboPopup(view))) {
        widget->setPalette(QPalette());
    }
    QProxyStyle::unpolish(widget);
}
