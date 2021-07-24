/*
 * Copyright (c) 2013-2021 Victor Wåhlström
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 */

#include <ZtWidgets/slideredit.h>

#include <QKeyEvent>
#include <QPainter>
#include <QStyleOption>
#include <QTime>
#include <QTimer>
#include <QtMath>

static constexpr const int S_DRAW_PADDING = 2;

static QString toString(qreal val, quint16 precision)
{
    return QString("%1").arg(val, 0, 'f', precision, '0');
}

static qreal snapToPrecision(qreal value, quint32 precision)
{
    const qreal p = qPow(10, precision);
    return qRound64(value * p) / p;
}

static qreal mapToPosition(SliderEdit::ValueMapping map_type,
                           qreal val,
                           qreal min_val,
                           qreal max_val,
                           qreal min_pos,
                           qreal max_pos)
{
    qreal pos = 0;
    switch (map_type)
    {
        case SliderEdit::ValueMapping::LinearScale:
        {
            const qreal scale = (val - min_val) / (max_val - min_val);
            pos               = min_pos + scale * (max_pos - min_pos);
        }
        break;
        case SliderEdit::ValueMapping::LogarithmicScale:
        {
            const qreal offset  = min_val < 1 ? 1 - min_val : 0;
            const qreal log_min = qLn(min_val + offset);
            const qreal log_max = qLn(max_val + offset);
            const qreal scale   = (log_max - log_min) / (max_pos - min_pos);
            pos                 = (qLn(val + offset) - log_min) / scale + min_pos;
        }
        break;
    }

    return pos;
}

static qreal mapFromPosition(SliderEdit::ValueMapping map_type,
                             qreal pos,
                             qreal min_val,
                             qreal max_val,
                             qreal min_pos,
                             qreal max_pos)
{
    qreal val = 0;

    switch (map_type)
    {
        case SliderEdit::ValueMapping::LinearScale:
            val = pos / (max_pos - min_pos) * (max_val - min_val) + min_val;
            break;
        case SliderEdit::ValueMapping::LogarithmicScale:
        {
            const qreal offset  = min_val < 1 ? 1 - min_val : 0;
            const qreal log_min = qLn(min_val + offset);
            const qreal log_max = qLn(max_val + offset);
            const qreal scale   = (log_max - log_min) / (max_pos - min_pos);
            val                 = qExp(log_min + scale * (pos - min_pos)) - offset;
        }
        break;
    }

    return val;
}

//! @cond Doxygen_Suppress
class SliderEditPrivate
{
    Q_DISABLE_COPY(SliderEditPrivate)

  public:
    explicit SliderEditPrivate(SliderEdit*);

    void beginEdit();
    void endEdit();
    void cancelEdit();
    bool isEditing() const;
    quint32 toEditCursorPos(int pos) const;
    qreal valueFromMousePos(const QPointF& pos) const;

    QString m_Label;
    QString m_Unit;
    QString m_EditText;
    QString m_Text;
    QTimer m_AnimEditCursorActivationTimer;
    QTimer m_AnimEditCursorBlinkTimer;
    QPoint m_MousePressPos;
    quint32 m_EditTextCurPos;
    qint32 m_EditTextSelOffset;
    qreal m_Value;
    qreal m_Min;
    qreal m_Max;
    qreal m_SingleStep;
    qreal m_PageStep;
    quint32 m_Precision;
    Qt::Alignment m_Alignment;
    Qt::Orientation m_Orientation;
    SliderEdit::SliderComponents m_SliderComponents;
    SliderEdit::SliderBehavior m_SliderBehavior;
    SliderEdit::ValueMapping m_ValueMapping;
    bool m_Editable : 1;
    bool m_AnimEditCursor : 1;
    bool m_AnimEditCursorVisible : 1;

  private:
    SliderEdit* m_SliderEdit;
};

SliderEditPrivate::SliderEditPrivate(SliderEdit* slider_edit)
    : m_EditTextCurPos(0)
    , m_EditTextSelOffset(0)
    , m_Value(0.0)
    , m_Min(0.0)
    , m_Max(1.0)
    , m_SingleStep(1.0)
    , m_PageStep(10.0)
    , m_Precision(3)
    , m_Alignment(Qt::AlignCenter)
    , m_Orientation(Qt::Horizontal)
    , m_SliderComponents(SliderEdit::SliderComponent::Text | SliderEdit::SliderComponent::Gauge)
    , m_SliderBehavior()
    , m_ValueMapping(SliderEdit::ValueMapping::LinearScale)
    , m_Editable(true)
    , m_AnimEditCursor(true)
    , m_AnimEditCursorVisible(true)
    , m_SliderEdit(slider_edit)
{
    QObject::connect(&m_AnimEditCursorActivationTimer,
                     &QTimer::timeout,
                     [this]()
                     {
                         m_AnimEditCursor = true;
                         m_SliderEdit->update();
                     });

    QObject::connect(&m_AnimEditCursorBlinkTimer,
                     &QTimer::timeout,
                     [this]()
                     {
                         m_AnimEditCursorVisible = !m_AnimEditCursorVisible;
                         m_SliderEdit->update();
                     });
}

void SliderEditPrivate::beginEdit()
{
    // editing not supported (yet) when oriented vertically
    bool editable = m_Editable && !(m_Orientation == Qt::Vertical);
    if (!editable)
        return;

    m_EditText          = toString(m_Value, m_Precision);
    m_Text              = m_EditText;
    m_EditTextCurPos    = m_EditText.size();
    m_EditTextSelOffset = -m_EditText.size();
    m_AnimEditCursorActivationTimer.start(1000);
    m_AnimEditCursorBlinkTimer.start(500);
    m_SliderEdit->setCursor(Qt::IBeamCursor);
}

void SliderEditPrivate::endEdit()
{
    if (m_Text != m_EditText)
    {
        bool valid;

        qreal val = m_EditText.toDouble(&valid);
        if (!valid)
        {
            cancelEdit();
            return;
        }

        if (m_SliderBehavior & SliderEdit::SliderBehaviorFlag::SnapToPrecision)
            val = snapToPrecision(val, m_Precision);
        if (!(m_SliderBehavior & SliderEdit::SliderBehaviorFlag::AllowValueUnderflow))
            val = qMax(m_Min, val);
        if (!(m_SliderBehavior & SliderEdit::SliderBehaviorFlag::AllowValueOverflow))
            val = qMin(m_Max, val);

        bool changed = m_Value != val;
        m_Value      = val;

        if (changed)
        {
            Q_EMIT m_SliderEdit->valueChanged(m_Value);
        }
    }

    cancelEdit();
}

void SliderEditPrivate::cancelEdit()
{
    m_EditText          = QString();
    m_EditTextCurPos    = 0;
    m_EditTextSelOffset = 0;
    m_AnimEditCursorActivationTimer.stop();
    m_AnimEditCursorBlinkTimer.stop();
    m_SliderEdit->unsetCursor();
    m_SliderEdit->update();
}

bool SliderEditPrivate::isEditing() const
{
    return !m_EditText.isNull();
}

quint32 SliderEditPrivate::toEditCursorPos(int pos) const
{
    QFontMetrics fm(m_SliderEdit->font());
    int cw         = fm.averageCharWidth();
    int cursor_pos = (pos + (int)(cw * 0.5f)) / cw;
    return static_cast<quint32>(qBound(0, cursor_pos, m_EditText.size()));
}

qreal SliderEditPrivate::valueFromMousePos(const QPointF& pos) const
{
    const QRectF& r = m_SliderEdit->rect();

    qreal min_pos = 0;
    qreal max_pos = 0;
    qreal p       = 0;

    if (m_Orientation == Qt::Horizontal)
    {
        min_pos = r.x();
        max_pos = r.x() + r.width();
        p       = pos.x();
    }
    else
    {
        min_pos = r.y();
        max_pos = r.y() + r.height();
        p       = r.height() - pos.y();
    }
    p = qBound(min_pos, p, max_pos);

    return mapFromPosition(m_ValueMapping, p, m_Min, m_Max, min_pos, max_pos);
}
//! @endcond

SliderEdit::SliderEdit(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_Impl(new SliderEditPrivate(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setOrientation(orientation());
}

SliderEdit::~SliderEdit()
{
    delete m_Impl;
}

QSize SliderEdit::sizeHint() const
{
    QFontMetrics fm(font());

    QString t_min = toString(m_Impl->m_Min, m_Impl->m_Precision);
    QString t_max = toString(m_Impl->m_Max, m_Impl->m_Precision);

    int w = qMax(fm.horizontalAdvance(t_min), fm.horizontalAdvance(t_max)) + S_DRAW_PADDING * 2;
    int h = fm.height() + S_DRAW_PADDING * 2;

    return m_Impl->m_Orientation == Qt::Horizontal ? QSize(w, h) : QSize(h, w);
}

void SliderEdit::updateValue(qreal value)
{
    if (m_Impl->m_SliderBehavior & SliderEdit::SliderBehaviorFlag::SnapToPrecision)
    {
        value = snapToPrecision(value, m_Impl->m_Precision);
    }

    value           = qBound(m_Impl->m_Min, value, m_Impl->m_Max);
    m_Impl->m_Value = value;

    update();
}

void SliderEdit::setValue(qreal value)
{
    updateValue(value);
    Q_EMIT valueChanged(m_Impl->m_Value);

    update();
}

qreal SliderEdit::value() const
{
    return m_Impl->m_Value;
}

void SliderEdit::setMinimum(qreal minimum)
{
    m_Impl->m_Min = minimum;
    if (m_Impl->m_Min > m_Impl->m_Max)
    {
        m_Impl->m_Max = m_Impl->m_Min;
    }

    setValue(m_Impl->m_Value);

    update();
}

qreal SliderEdit::minimum() const
{
    return m_Impl->m_Min;
}

void SliderEdit::setMaximum(qreal maximum)
{
    m_Impl->m_Max = maximum;
    if (m_Impl->m_Max < m_Impl->m_Min)
    {
        m_Impl->m_Min = m_Impl->m_Max;
    }

    setValue(m_Impl->m_Value);

    update();
}

qreal SliderEdit::maximum() const
{
    return m_Impl->m_Max;
}

void SliderEdit::setRange(qreal minimum, qreal maximum)
{
    setMinimum(minimum);
    setMaximum(maximum);
}

bool SliderEdit::editable() const
{
    return m_Impl->m_Editable;
}

void SliderEdit::setEditable(bool editable)
{
    m_Impl->m_Editable = editable;
}

void SliderEdit::setSingleStep(qreal step)
{
    m_Impl->m_SingleStep = step;
    update();
}

qreal SliderEdit::singleStep() const
{
    return m_Impl->m_SingleStep;
}

void SliderEdit::setPageStep(qreal step)
{
    m_Impl->m_PageStep = step;
    update();
}

qreal SliderEdit::pageStep() const
{
    return m_Impl->m_PageStep;
}

void SliderEdit::setPrecision(quint32 precision)
{
    m_Impl->m_Precision = precision;
    update();
}

quint32 SliderEdit::precision() const
{
    return m_Impl->m_Precision;
}

void SliderEdit::setSliderComponents(SliderComponents components)
{
    m_Impl->m_SliderComponents = components;
    update();
}

SliderEdit::SliderComponents SliderEdit::sliderComponents() const
{
    return m_Impl->m_SliderComponents;
}

void SliderEdit::setSliderBehavior(SliderBehavior behavior)
{
    bool changed             = m_Impl->m_SliderBehavior != behavior;
    m_Impl->m_SliderBehavior = behavior;
    if (changed &&
        ((m_Impl->m_SliderBehavior & (SliderBehaviorFlag::AllowValueUnderflow | SliderBehaviorFlag::AllowValueOverflow |
                                      SliderBehaviorFlag::SnapToPrecision))))
    {
        setValue(m_Impl->m_Value);
    }

    update();
}

SliderEdit::SliderBehavior SliderEdit::sliderBehavior() const
{
    return m_Impl->m_SliderBehavior;
}

void SliderEdit::setValueMapping(ValueMapping mapping)
{
    m_Impl->m_ValueMapping = mapping;

    update();
}

SliderEdit::ValueMapping SliderEdit::valueMapping() const
{
    return m_Impl->m_ValueMapping;
}

void SliderEdit::setAlignment(Qt::Alignment alignment)
{
    m_Impl->m_Alignment = alignment;
    update();
}

Qt::Alignment SliderEdit::alignment() const
{
    return m_Impl->m_Alignment;
}

void SliderEdit::setOrientation(Qt::Orientation orientation)
{
    m_Impl->m_Orientation = orientation;
    if (m_Impl->m_Orientation == Qt::Horizontal)
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    else
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    update();
}

Qt::Orientation SliderEdit::orientation() const
{
    return m_Impl->m_Orientation;
}

void SliderEdit::setUnit(const QString& unit)
{
    m_Impl->m_Unit = unit;
    update();
}

const QString& SliderEdit::unit() const
{
    return m_Impl->m_Unit;
}

void SliderEdit::setLabel(const QString& label)
{
    m_Impl->m_Label = label;
    update();
}

const QString& SliderEdit::label() const
{
    return m_Impl->m_Label;
}

void SliderEdit::mousePressEvent(QMouseEvent* event)
{
    m_Impl->m_MousePressPos = event->pos();
    if (m_Impl->isEditing())
    {
        m_Impl->m_EditTextCurPos    = m_Impl->toEditCursorPos(event->pos().x());
        m_Impl->m_EditTextSelOffset = 0;
    }
}

void SliderEdit::mouseMoveEvent(QMouseEvent* event)
{
    if (m_Impl->isEditing())
    {
        m_Impl->m_AnimEditCursor = false;
        m_Impl->m_AnimEditCursorActivationTimer.stop();
        m_Impl->m_EditTextCurPos    = m_Impl->toEditCursorPos(event->pos().x());
        m_Impl->m_EditTextSelOffset = m_Impl->toEditCursorPos(m_Impl->m_MousePressPos.x()) - m_Impl->m_EditTextCurPos;
    }
    else
    {
        m_Impl->m_Value = m_Impl->valueFromMousePos(event->pos());
        Q_EMIT valueChanging(m_Impl->m_Value);
    }
    update();
}

void SliderEdit::mouseReleaseEvent(QMouseEvent* event)
{
    if (!m_Impl->isEditing())
    {
        if (event->pos() == m_Impl->m_MousePressPos)
        {
            m_Impl->cancelEdit();
            m_Impl->beginEdit();
        }
        else
        {
            setValue(m_Impl->valueFromMousePos(event->pos()));
        }
    }
    else
    {
        m_Impl->m_AnimEditCursorActivationTimer.start(500);
    }
    update();
}

void SliderEdit::mouseDoubleClickEvent(QMouseEvent*)
{
    if (!m_Impl->isEditing())
    {
        m_Impl->beginEdit();
    }

    m_Impl->m_EditTextCurPos    = m_Impl->m_EditText.size();
    m_Impl->m_EditTextSelOffset = -m_Impl->m_EditText.size();
}

void SliderEdit::focusInEvent(QFocusEvent* event)
{
    int reason = event->reason();
    if (reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason)
    {
        m_Impl->cancelEdit();
        m_Impl->beginEdit();
    }
}

void SliderEdit::focusOutEvent(QFocusEvent* event)
{
    int reason = event->reason();
    switch (reason)
    {
        case Qt::MouseFocusReason:
        case Qt::TabFocusReason:
        case Qt::BacktabFocusReason:
            m_Impl->endEdit();
            break;
        default:
            m_Impl->cancelEdit();
            break;
    }
}

void SliderEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();

    bool is_input_key = ((key >= Qt::Key_0 && key <= Qt::Key_9) || key == Qt::Key_Comma || key == Qt::Key_Period ||
                         key == Qt::Key_Backspace || key == Qt::Key_Delete || key == Qt::Key_Minus);

    // directly enter edit mode if an input key is pressed
    if (!m_Impl->isEditing() && is_input_key)
    {
        m_Impl->beginEdit();
    }

    if (m_Impl->isEditing())
    {
        if (key == Qt::Key_Escape)
        {
            m_Impl->cancelEdit();
        }
        else if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            m_Impl->endEdit();
        }
        else if (key == Qt::Key_Left)
        {
            if (event->modifiers() & Qt::ShiftModifier)
            {
                m_Impl->m_EditTextSelOffset += m_Impl->m_EditTextCurPos > 0 ? 1 : 0;
                m_Impl->m_EditTextCurPos < 1 ? 0 : --m_Impl->m_EditTextCurPos;
            }
            else
            {
                m_Impl->m_EditTextSelOffset = 0;
                m_Impl->m_EditTextCurPos < 1 ? 0 : --m_Impl->m_EditTextCurPos;
            }

            m_Impl->m_AnimEditCursor = false;
            update();
            return;
        }
        else if (key == Qt::Key_Right)
        {
            quint32 eol = m_Impl->m_EditText.size();
            if (event->modifiers() & Qt::ShiftModifier)
            {
                m_Impl->m_EditTextSelOffset -= m_Impl->m_EditTextCurPos < eol ? 1 : 0;
                m_Impl->m_EditTextCurPos >= eol ? eol : ++m_Impl->m_EditTextCurPos;
            }
            else
            {
                m_Impl->m_EditTextSelOffset = 0;
                m_Impl->m_EditTextCurPos >= eol ? eol : ++m_Impl->m_EditTextCurPos;
            }

            m_Impl->m_AnimEditCursor = false;
            update();
            return;
        }

        if (!is_input_key)
        {
            return;
        }

        if (m_Impl->m_EditTextSelOffset)
        {
            // delete selection
            int pos            = qMin(m_Impl->m_EditTextCurPos, m_Impl->m_EditTextCurPos + m_Impl->m_EditTextSelOffset);
            int n              = qAbs(m_Impl->m_EditTextSelOffset);
            m_Impl->m_EditText = m_Impl->m_EditText.replace(pos, n, "");
            m_Impl->m_EditTextCurPos    = pos;
            m_Impl->m_EditTextSelOffset = 0;

            // insert value of key if a valid input key was pressed
            if (key != Qt::Key_Delete && key != Qt::Key_Backspace)
            {
                m_Impl->m_EditText.insert(m_Impl->m_EditTextCurPos++, event->text()[0]);
            }
        }
        else if (key == Qt::Key_Delete)
        {
            m_Impl->m_EditText = m_Impl->m_EditText.replace(m_Impl->m_EditTextCurPos, 1, "");
        }
        else if (key == Qt::Key_Backspace)
        {
            if (m_Impl->m_EditTextCurPos > 0)
            {
                m_Impl->m_EditText = m_Impl->m_EditText.replace(--m_Impl->m_EditTextCurPos, 1, "");
            }
        }
        else
        {
            m_Impl->m_EditText.insert(m_Impl->m_EditTextCurPos++, event->text()[0]);
        }
    }
    else
    {
        if (key == Qt::Key_Left || key == Qt::Key_Down)
        {
            setValue(m_Impl->m_Value - m_Impl->m_SingleStep);
        }

        if (key == Qt::Key_Right || key == Qt::Key_Up)
        {
            setValue(m_Impl->m_Value + m_Impl->m_SingleStep);
        }

        if (key == Qt::Key_PageDown)
        {
            setValue(m_Impl->m_Value - m_Impl->m_PageStep);
        }

        if (key == Qt::Key_PageUp)
        {
            setValue(m_Impl->m_Value + m_Impl->m_PageStep);
        }
    }
    update();
}

void SliderEdit::paintEvent(QPaintEvent*)
{
    const QRect& r   = rect().adjusted(S_DRAW_PADDING, S_DRAW_PADDING, -S_DRAW_PADDING, -S_DRAW_PADDING);
    const QFont& fnt = font();

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(r);

    painter.setFont(fnt);

    painter.fillRect(r, palette().base());

    if (m_Impl->isEditing())
    {
        QFontMetrics fm(fnt);
        QRect font_rect        = fm.tightBoundingRect(m_Impl->m_EditText);
        const int text_offset  = m_Impl->m_Alignment & Qt::AlignRight ? r.width() - font_rect.width() - 1 : 2;
        const int cursor_width = 1;
        const QRect text_rect  = r.adjusted(text_offset, 0, 0, 0);

        const int text_cur_pos = S_DRAW_PADDING + text_offset - cursor_width +
                                 fm.horizontalAdvance(m_Impl->m_EditText.mid(0, m_Impl->m_EditTextCurPos));
        const int text_sel_pos =
            S_DRAW_PADDING + text_offset - cursor_width +
            fm.horizontalAdvance(m_Impl->m_EditText.mid(0, m_Impl->m_EditTextCurPos + m_Impl->m_EditTextSelOffset));

        if (m_Impl->m_EditTextSelOffset != 0)
        {
            font_rect.moveLeft(qMin(text_cur_pos, text_sel_pos));
            font_rect.setRight(qMax(text_cur_pos, text_sel_pos));
            font_rect.setY(r.y());
            font_rect.setHeight(r.height());
            painter.fillRect(font_rect, palette().highlight());

            // paint text inside the highlighted area
            painter.setClipRect(font_rect);
            painter.setPen(palette().highlightedText().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, m_Impl->m_EditText);

            // paint text outside the highlighted area
            painter.setClipRegion(QRegion(rect()).subtracted(font_rect));
            painter.setPen(palette().text().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, m_Impl->m_EditText);
            painter.setClipRect(rect());
        }
        else
        {
            painter.setPen(palette().text().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, m_Impl->m_EditText);
        }

        if (!m_Impl->m_AnimEditCursor || m_Impl->m_AnimEditCursorVisible)
        {
            QLine cursor(qBound(0, text_cur_pos, r.width()), r.y(), text_cur_pos, r.y() + r.height());

            QPen pen(palette().text().color());
            pen.setWidth(1);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.setPen(pen);
            painter.drawLine(cursor);
            painter.setRenderHint(QPainter::Antialiasing);
        }
    }
    else
    {
        int rect_pos;
        QRect filled_rect;
        // use a clamped value in case SliderBehaviorFlag::AllowValueUnderflow or SliderBehaviorFlag::AllowValueOverflow
        // is set
        const qreal clamped_val = qBound(m_Impl->m_Min, m_Impl->m_Value, m_Impl->m_Max);
        if (m_Impl->m_Orientation == Qt::Horizontal)
        {
            rect_pos = mapToPosition(m_Impl->m_ValueMapping, clamped_val, m_Impl->m_Min, m_Impl->m_Max, 0, r.width());
            filled_rect = QRect(r.x(), r.y(), rect_pos, r.height());
        }
        else
        {
            rect_pos = mapToPosition(m_Impl->m_ValueMapping, clamped_val, m_Impl->m_Min, m_Impl->m_Max, 0, r.height());
            filled_rect = QRect(r.x(), r.y() + r.height() - rect_pos, r.width(), rect_pos);
        }

        if (m_Impl->m_SliderComponents & SliderComponent::Gauge)
            painter.fillRect(filled_rect, palette().highlight());

        auto draw_text = [&]()
        {
            bool can_draw_text =
                (m_Impl->m_SliderComponents & SliderComponent::Text) && m_Impl->m_Orientation == Qt::Horizontal;
            if (!can_draw_text)
                return;

            QString text =
                toString(m_Impl->m_Value, m_Impl->m_Precision) + (m_Impl->m_Unit.isEmpty() ? "" : " " + m_Impl->m_Unit);
            if (m_Impl->m_Alignment & Qt::AlignJustify)
            {
                if (!m_Impl->m_Label.isEmpty())
                    painter.drawText(r, Qt::AlignLeft, m_Impl->m_Label + ":");

                painter.drawText(r, Qt::AlignRight, text);
            }
            else
            {
                painter.drawText(
                    r,
                    m_Impl->m_Alignment,
                    QString("%1%2").arg(m_Impl->m_Label.isEmpty() ? "" : m_Impl->m_Label + ": ").arg(text));
            }
        };

        if (m_Impl->m_SliderComponents & SliderComponent::Gauge)
        {
            painter.setClipRect(filled_rect);
            painter.setPen(palette().highlightedText().color());
            draw_text();

            QRect empty_rect(r.x() + rect_pos, r.y(), r.width() - rect_pos, r.height());
            painter.setClipRect(empty_rect);
        }

        painter.setPen(palette().text().color());
        draw_text();
        painter.setClipRect(rect());

        if (m_Impl->m_SliderComponents & SliderComponent::Marker)
        {
            painter.setRenderHint(QPainter::Antialiasing, false);
            int marker_pos;
            QRect marker_rect;
            if (m_Impl->m_Orientation == Qt::Horizontal)
            {
                marker_pos  = qBound(0, r.x() + rect_pos, r.width());
                marker_rect = QRect(marker_pos - 1, r.y(), 2, r.height() - 1);
            }
            else
            {
                marker_pos  = qBound(0, r.y() + rect_pos, r.height());
                marker_rect = QRect(r.x(), r.height() - marker_pos + 1, r.width() - 1, 2);
            }

            painter.setBrush(Qt::white);
            painter.setPen(Qt::black);
            painter.drawRect(marker_rect);
            painter.setRenderHint(QPainter::Antialiasing);
        }
    }

    if (hasFocus())
    {
        painter.setClipRect(rect(), Qt::NoClip);
        QStyleOptionFocusRect opts;
        opts.initFrom(this);
        opts.rect            = rect().adjusted(1, 1, -1, -1);
        opts.backgroundColor = palette().window().color();
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opts, &painter, this);
    }

    painter.restore();
}
