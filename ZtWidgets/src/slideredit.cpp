/*
 * Copyright (c) 2013-2016 Victor Wåhlström
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

#include <QtMath>
#include <QKeyEvent>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QStyleOption>

static constexpr const int S_DRAW_PADDING = 2;

static QString toString(qreal val, quint16 precision)
{
    return QString("%1").arg(val, 0, 'f', precision, '0');
}

//! @cond Doxygen_Suppress
class SliderEditPrivate
{
    Q_DISABLE_COPY(SliderEditPrivate)
    Q_DECLARE_PUBLIC(SliderEdit)

private:
    explicit SliderEditPrivate(SliderEdit*);

    void beginEdit();
    void endEdit();
    void cancelEdit();
    bool isEditing() const;
    quint32 toEditCursorPos(int pos) const;
    qreal valueFromMousePos(const QPointF& pos) const;

    SliderEdit* const q_ptr;

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
    bool m_Editable : 1;
    bool m_AnimEditCursor : 1;
    bool m_AnimEditCursorVisible : 1;
    bool m_SnappingEnabled : 1;
};

SliderEditPrivate::SliderEditPrivate(SliderEdit* slider_edit)
    : q_ptr(slider_edit)
    , m_EditTextCurPos(0)
    , m_EditTextSelOffset(0)
    , m_Min(0.0)
    , m_Max(1.0)
    , m_SingleStep(1.0)
    , m_PageStep(10.0)
    , m_Precision(3)
    , m_Alignment(Qt::AlignCenter)
    , m_Orientation(Qt::Horizontal)
    , m_SliderComponents(SliderEdit::SliderComponent::Text | SliderEdit::SliderComponent::Gauge)
    , m_Editable(true)
    , m_AnimEditCursor(true)
    , m_AnimEditCursorVisible(true)
    , m_SnappingEnabled(false)
{
    QObject::connect(&m_AnimEditCursorActivationTimer, &QTimer::timeout, [this]()
    {
        Q_Q(SliderEdit);
        m_AnimEditCursor = true;
        q->update();
    });

    QObject::connect(&m_AnimEditCursorBlinkTimer, &QTimer::timeout, [this]()
    {
        Q_Q(SliderEdit);
        m_AnimEditCursorVisible = !m_AnimEditCursorVisible;
        q->update();
    });
}

void SliderEditPrivate::beginEdit()
{
    Q_Q(SliderEdit);
    // editing not supported (yet) when oriented vertically
    bool editable = m_Editable && !(m_Orientation == Qt::Vertical);
    if(!editable)
        return;

    m_EditText = toString(m_Value, m_Precision);
    m_Text = m_EditText;
    m_EditTextCurPos = m_EditText.size();
    m_EditTextSelOffset = -m_EditText.size();
    m_AnimEditCursorActivationTimer.start(1000);
    m_AnimEditCursorBlinkTimer.start(500);
    q->setCursor(Qt::IBeamCursor);
}

void SliderEditPrivate::endEdit()
{
    Q_Q(SliderEdit);
    if (m_Text != m_EditText)
    {
        bool valid;

        qreal val = m_EditText.toDouble(&valid);
        if(!valid)
        {
            cancelEdit();
            return;
        }

        val = qBound(m_Min, val, m_Max);

        bool changed = m_Value != val;
        m_Value = val;

        if (changed)
        {
            Q_EMIT q->valueChanged(m_Value);
        }
    }

    cancelEdit();
}

void SliderEditPrivate::cancelEdit()
{
    Q_Q(SliderEdit);
    m_EditText = QString();
    m_EditTextCurPos = 0;
    m_EditTextSelOffset = 0;
    m_AnimEditCursorActivationTimer.stop();
    m_AnimEditCursorBlinkTimer.stop();
    q->unsetCursor();
    q->update();
}

bool SliderEditPrivate::isEditing() const
{
    return !m_EditText.isNull();
}

quint32 SliderEditPrivate::toEditCursorPos(int pos) const
{
    Q_Q(const SliderEdit);
    QFontMetrics fm(q->font());
    int cw = fm.averageCharWidth();
    int cursor_pos = (pos + (int)(cw * 0.5f)) / cw;
    return static_cast<quint32>(qBound(0, cursor_pos, m_EditText.size()));
}

qreal SliderEditPrivate::valueFromMousePos(const QPointF& pos) const
{
    Q_Q(const SliderEdit);
    const QRectF& r = q->rect();
    qreal value;

    if(m_Orientation == Qt::Horizontal)
        value = qBound(r.x(), pos.x(), r.x() + r.width()) / r.width();
    else
        value = qBound(r.y(), r.height() - pos.y(), r.y() + r.height()) / r.height();

    return value * (m_Max - m_Min) + m_Min;
}
//! @endcond

SliderEdit::SliderEdit(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
    , d_ptr(new SliderEditPrivate(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setOrientation(orientation());
}

SliderEdit::~SliderEdit()
{
    delete d_ptr;
}

QSize SliderEdit::sizeHint() const
{
    Q_D(const SliderEdit);
    QFontMetrics fm(font());

    QString t_min = toString(d->m_Min, d->m_Precision);
    QString t_max = toString(d->m_Max, d->m_Precision);

    int w = qMax(fm.width(t_min), fm.width(t_max)) + S_DRAW_PADDING * 2;
    int h = fm.height() + S_DRAW_PADDING * 2;

    return d->m_Orientation == Qt::Horizontal ? QSize(w, h) : QSize(h, w);
}

void SliderEdit::updateValue(qreal value)
{
    Q_D(SliderEdit);

    if(d->m_SnappingEnabled)
    {
        qreal p = qPow(10, d->m_Precision);
        value = qRound64(value * p) / p;
    }

    value = qBound(d->m_Min, value, d->m_Max);
    d->m_Value = value;

    update();
}

void SliderEdit::setValue(qreal value)
{
    Q_D(SliderEdit);
    updateValue(value);
    Q_EMIT valueChanged(d->m_Value);

    update();
}

qreal SliderEdit::value() const
{
    Q_D(const SliderEdit);
    return d->m_Value;
}

void SliderEdit::setMinimum(qreal minimum)
{
    Q_D(SliderEdit);
    d->m_Min = minimum;
    if(d->m_Min > d->m_Max)
    {
        d->m_Max = d->m_Min;
    }

    setValue(d->m_Value);

    update();
}

qreal SliderEdit::minimum() const
{
    Q_D(const SliderEdit);
    return d->m_Min;
}

void SliderEdit::setMaximum(qreal maximum)
{
    Q_D(SliderEdit);
    d->m_Max = maximum;
    if(d->m_Max < d->m_Min)
    {
        d->m_Min = d->m_Max;
    }

    setValue(d->m_Value);

    update();
}

qreal SliderEdit::maximum() const
{
    Q_D(const SliderEdit);
    return d->m_Max;
}

void SliderEdit::setRange(qreal minimum, qreal maximum)
{
    setMinimum(minimum);
    setMaximum(maximum);
}

bool SliderEdit::editable() const
{
    Q_D(const SliderEdit);
    return d->m_Editable;
}

void SliderEdit::setEditable(bool editable)
{
    Q_D(SliderEdit);
    d->m_Editable = editable;
}

void SliderEdit::setSingleStep(qreal step)
{
    Q_D(SliderEdit);
    d->m_SingleStep = step;
    update();
}

qreal SliderEdit::singleStep() const
{
    Q_D(const SliderEdit);
    return d->m_SingleStep;
}

void SliderEdit::setPageStep(qreal step)
{
    Q_D(SliderEdit);
    d->m_PageStep = step;
    update();
}

qreal SliderEdit::pageStep() const
{
    Q_D(const SliderEdit);
    return d->m_PageStep;
}

void SliderEdit::setPrecision(quint32 precision)
{
    Q_D(SliderEdit);
    d->m_Precision = precision;
    update();
}

void SliderEdit::setSnapToPrecision(bool snap)
{
    Q_D(SliderEdit);
    bool changed = d->m_SnappingEnabled != snap;
    d->m_SnappingEnabled = snap;
    if(changed)
    {
        setValue(d->m_Value);
    }
    update();
}

bool SliderEdit::isSnappingToPrecision() const
{
    Q_D(const SliderEdit);
    return d->m_SnappingEnabled;
}

quint32 SliderEdit::precision() const
{
    Q_D(const SliderEdit);
    return d->m_Precision;
}

void SliderEdit::setSliderComponents(SliderComponents components)
{
    Q_D(SliderEdit);
    d->m_SliderComponents = components;
    update();
}

SliderEdit::SliderComponents SliderEdit::sliderComponents() const
{
    Q_D(const SliderEdit);
    return d->m_SliderComponents;
}

void SliderEdit::setAlignment(Qt::Alignment alignment)
{
    Q_D(SliderEdit);
    d->m_Alignment = alignment;
    update();
}

Qt::Alignment SliderEdit::alignment() const
{
    Q_D(const SliderEdit);
    return d->m_Alignment;
}

void SliderEdit::setOrientation(Qt::Orientation orientation)
{
    Q_D(SliderEdit);
    d->m_Orientation = orientation;
    if(d->m_Orientation == Qt::Horizontal)
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    else
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    update();
}

Qt::Orientation SliderEdit::orientation() const
{
    Q_D(const SliderEdit);
    return d->m_Orientation;
}

void SliderEdit::setUnit(const QString& unit)
{
    Q_D(SliderEdit);
    d->m_Unit = unit;
    update();
}

const QString& SliderEdit::unit() const
{
    Q_D(const SliderEdit);
    return d->m_Unit;
}

void SliderEdit::setLabel(const QString& label)
{
    Q_D(SliderEdit);
    d->m_Label = label;
    update();
}

const QString& SliderEdit::label() const
{
    Q_D(const SliderEdit);
    return d->m_Label;
}

void SliderEdit::mousePressEvent(QMouseEvent* event)
{
    Q_D(SliderEdit);
    d->m_MousePressPos = event->pos();
    if(d->isEditing())
    {
        d->m_EditTextCurPos = d->toEditCursorPos(event->pos().x());
        d->m_EditTextSelOffset = 0;
    }
}

void SliderEdit::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(SliderEdit);
    if(d->isEditing())
    {
        d->m_AnimEditCursor = false;
        d->m_AnimEditCursorActivationTimer.stop();
        d->m_EditTextCurPos = d->toEditCursorPos(event->pos().x());
        d->m_EditTextSelOffset = d->toEditCursorPos(d->m_MousePressPos.x()) - d->m_EditTextCurPos;
    }
    else
    {
        d->m_Value = d->valueFromMousePos(event->pos());
        Q_EMIT valueChanging(d->m_Value);
    }
    update();
}

void SliderEdit::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(SliderEdit);
    if(!d->isEditing())
    {
        if(event->pos() == d->m_MousePressPos)
        {
            d->cancelEdit();
            d->beginEdit();
        }
        else
        {
            setValue(d->valueFromMousePos(event->pos()));
        }
    }
    else
    {
        d->m_AnimEditCursorActivationTimer.start(500);
    }
    update();
}

void SliderEdit::mouseDoubleClickEvent(QMouseEvent*)
{
    Q_D(SliderEdit);
    if(!d->isEditing())
    {
        d->beginEdit();
    }

    d->m_EditTextCurPos = d->m_EditText.size();
    d->m_EditTextSelOffset = -d->m_EditText.size();
}

void SliderEdit::focusInEvent(QFocusEvent* event)
{
    Q_D(SliderEdit);
    int reason = event->reason();
    if(reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason)
    {
        d->cancelEdit();
        d->beginEdit();
    }
}

void SliderEdit::focusOutEvent(QFocusEvent* event)
{
    Q_D(SliderEdit);
    int reason = event->reason();
    if(reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason)
    {
        d->endEdit();
    }
    else
    {
        d->cancelEdit();
    }
}

void SliderEdit::keyPressEvent(QKeyEvent* event)
{
    Q_D(SliderEdit);

    int key = event->key();

    bool is_input_key = ((key >= Qt::Key_0 && key <= Qt::Key_9) ||
                          key == Qt::Key_Comma || key == Qt::Key_Period ||
                          key == Qt::Key_Backspace || key == Qt::Key_Delete);

    // directly enter edit mode if an input key is pressed
    if(!d->isEditing() && is_input_key)
    {
        d->beginEdit();
    }

    if(d->isEditing())
    {
        if(key == Qt::Key_Escape)
        {
            d->cancelEdit();
        }
        else if(key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            d->endEdit();
        }
        else if(key == Qt::Key_Left)
        {
            if(event->modifiers() & Qt::ShiftModifier)
            {
                d->m_EditTextSelOffset += d->m_EditTextCurPos > 0 ? 1 : 0;
                d->m_EditTextCurPos < 1 ? 0 : --d->m_EditTextCurPos;
            }
            else
            {
                d->m_EditTextSelOffset = 0;
                d->m_EditTextCurPos < 1 ? 0 : --d->m_EditTextCurPos;
            }

            d->m_AnimEditCursor = false;
            update();
            return;
        }
        else if(key == Qt::Key_Right)
        {
            quint32 eol = d->m_EditText.size();
            if(event->modifiers() & Qt::ShiftModifier)
            {
                d->m_EditTextSelOffset -= d->m_EditTextCurPos < eol ? 1 : 0;
                d->m_EditTextCurPos >= eol ? eol : ++d->m_EditTextCurPos;
            }
            else
            {
                d->m_EditTextSelOffset = 0;
                d->m_EditTextCurPos >= eol ? eol : ++d->m_EditTextCurPos;
            }

            d->m_AnimEditCursor = false;
            update();
            return;
        }

        if(!is_input_key)
        {
            return;
        }

        if(d->m_EditTextSelOffset)
        {
            // delete selection
            int pos = qMin(d->m_EditTextCurPos, d->m_EditTextCurPos + d->m_EditTextSelOffset);
            int n = qAbs(d->m_EditTextSelOffset);
            d->m_EditText = d->m_EditText.replace(pos, n, "");
            d->m_EditTextCurPos = pos;
            d->m_EditTextSelOffset = 0;

            // insert value of key if a valid input key was pressed
            if(key != Qt::Key_Delete && key != Qt::Key_Backspace)
            {
                d->m_EditText.insert(d->m_EditTextCurPos++, event->text()[0]);
            }
        }
        else if(key == Qt::Key_Delete)
        {
            d->m_EditText = d->m_EditText.replace(d->m_EditTextCurPos, 1, "");
        }
        else if(key == Qt::Key_Backspace)
        {
            if(d->m_EditTextCurPos > 0)
            {
                d->m_EditText = d->m_EditText.replace(--d->m_EditTextCurPos, 1, "");
            }
        }
        else
        {
            d->m_EditText.insert(d->m_EditTextCurPos++, event->text()[0]);
        }
    }
    else
    {
        if(key == Qt::Key_Left || key == Qt::Key_Down)
        {
            setValue( d->m_Value - d->m_SingleStep);
        }

        if(key == Qt::Key_Right || key == Qt::Key_Up)
        {
            setValue( d->m_Value + d->m_SingleStep);
        }

        if(key == Qt::Key_PageDown)
        {
            setValue( d->m_Value - d->m_PageStep);
        }

        if(key == Qt::Key_PageUp)
        {
            setValue( d->m_Value + d->m_PageStep);
        }
    }
    update();
}


void SliderEdit::paintEvent(QPaintEvent*)
{
    Q_D(SliderEdit);

    const QRect& r = rect().adjusted(S_DRAW_PADDING, S_DRAW_PADDING, -S_DRAW_PADDING, -S_DRAW_PADDING);
    const QFont& fnt = font();

    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(r);

    painter.setFont(fnt);

    painter.fillRect(r, palette().base());

    if(d->isEditing())
    {
        QFontMetrics fm(fnt);
        QRect font_rect = fm.tightBoundingRect(d->m_EditText);
        const int text_offset = d->m_Alignment & Qt::AlignRight ? r.width() - font_rect.width() - 1 : 2;
        const int cursor_width = 1;
        const QRect text_rect = r.adjusted(text_offset, 0, 0, 0);

        const int text_cur_pos = S_DRAW_PADDING + text_offset - cursor_width + fm.width(d->m_EditText.mid(0, d->m_EditTextCurPos));
        const int text_sel_pos = S_DRAW_PADDING + text_offset - cursor_width + fm.width(d->m_EditText.mid(0, d->m_EditTextCurPos + d->m_EditTextSelOffset));

        if(d->m_EditTextSelOffset != 0)
        {
            font_rect.moveLeft(qMin(text_cur_pos, text_sel_pos));
            font_rect.setRight(qMax(text_cur_pos, text_sel_pos));
            font_rect.setY(r.y());
            font_rect.setHeight(r.height());
            painter.fillRect(font_rect, palette().highlight());

            // paint text inside the highlighted area
            painter.setClipRect(font_rect);
            painter.setPen(palette().highlightedText().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, d->m_EditText);

            // paint text outside the highlighted area
            painter.setClipRegion(QRegion(rect()).subtracted(font_rect));
            painter.setPen(palette().text().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, d->m_EditText);
            painter.setClipRect(rect());
        }
        else
        {
            painter.setPen(palette().text().color());
            painter.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, d->m_EditText);
        }

        if(!d->m_AnimEditCursor || d->m_AnimEditCursorVisible)
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
        qreal rel_pos = (d->m_Value - d->m_Min) / (d->m_Max - d->m_Min);
        int rect_pos;
        QRect filled_rect;
        if(d->m_Orientation == Qt::Horizontal)
        {
            rect_pos = rel_pos * r.width();
            filled_rect = QRect(r.x(), r.y(), rect_pos, r.height());
        }
        else
        {
            rect_pos = rel_pos * r.height();
            filled_rect = QRect(r.x(), r.y() + r.height() - rect_pos, r.width(), rect_pos);
        }

        if(d->m_SliderComponents & SliderComponent::Gauge)
            painter.fillRect(filled_rect, palette().highlight());

        auto draw_text = [&]()
        {
            bool can_draw_text = (d->m_SliderComponents & SliderComponent::Text) && d->m_Orientation == Qt::Horizontal;
            if(!can_draw_text)
                return;

            QString text = toString(d->m_Value, d->m_Precision) + (d->m_Unit.isEmpty() ? "" : " " + d->m_Unit);
            if(d->m_Alignment & Qt::AlignJustify)
            {
                if(!d->m_Label.isEmpty())
                    painter.drawText(r, Qt::AlignLeft, d->m_Label + ":");

                painter.drawText(r, Qt::AlignRight, text);
            }
            else
            {
                painter.drawText(r, d->m_Alignment, QString("%1%2").arg(d->m_Label.isEmpty() ? "" : d->m_Label + ": ").arg(text));
            }
        };

        if(d->m_SliderComponents & SliderComponent::Gauge)
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

        if(d->m_SliderComponents & SliderComponent::Marker)
        {
            painter.setRenderHint(QPainter::Antialiasing, false);
            int marker_pos;
            QRect marker_rect;
            if(d->m_Orientation == Qt::Horizontal)
            {
                marker_pos = qBound(0, r.x() + rect_pos, r.width());
                marker_rect = QRect(marker_pos - 1, r.y(), 2, r.height() - 1);
            }
            else
            {
                marker_pos = qBound(0, r.y() + rect_pos, r.height());
                marker_rect = QRect(r.x(), r.height() - marker_pos + 1, r.width() - 1, 2);
            }

            painter.setBrush(Qt::white);
            painter.setPen(Qt::black);
            painter.drawRect(marker_rect);
            painter.setRenderHint(QPainter::Antialiasing);
        }
    }

    if(hasFocus())
    {
        painter.setClipRect(rect(), Qt::NoClip);
        QStyleOptionFocusRect opts;
        opts.initFrom(this);
        opts.rect = rect().adjusted(1, 1, -1, -1);
        opts.backgroundColor = palette().window().color();
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &opts, &painter, this);
    }

    painter.restore();
}
