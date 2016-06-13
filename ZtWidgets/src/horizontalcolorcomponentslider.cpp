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

#include <ZtWidgets/horizontalcolorcomponentslider.h>

#include <QtGlobal>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtCore/QTime>


static QString toString(qreal val)
{
    return QString("%1").arg(val, 6, 'f', 4, '0');
}

static QString toString(int val)
{
    return QString("%1").arg(val);
}

CHorizontalColorComponentSlider::CHorizontalColorComponentSlider(Components components, QWidget *parent)
    : CHorizontalColorComponentSlider(components, 20, Qt::white, Qt::black, parent)
{

}

CHorizontalColorComponentSlider::CHorizontalColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent)
    : CAbstractColorComponentSlider(components, width, color0, color1, parent),
      m_Font("Monospace", 9),
      m_EditTextCurPos(0),
      m_EditTextSelOffset(0),
      m_EditType(EditType::Float),
      m_AnimEditCursor(true),
      m_DisplayText(true),
      m_KeyInputEnabled(true)
{
    m_Font.setStyleHint(QFont::TypeWriter);
    m_Font.setStyleStrategy(QFont::ForceOutline);

    setMinimumWidth(0);
    setMaximumWidth(QWIDGETSIZE_MAX);
    setMinimumHeight(m_Width);
    setMaximumHeight(m_Width);

    connect(&m_Timer, &QTimer::timeout, this, [this](){ m_AnimEditCursor = true; update(); });
}

void CHorizontalColorComponentSlider::enableKeyInput()
{
    m_KeyInputEnabled = true;
}

void CHorizontalColorComponentSlider::disableKeyInput()
{
    m_KeyInputEnabled = false;
}

bool CHorizontalColorComponentSlider::keyInputEnabled() const
{
    return m_KeyInputEnabled;
}

void CHorizontalColorComponentSlider::displayText(bool show)
{
    m_DisplayText = show;
}

void CHorizontalColorComponentSlider::updateColor(const QPointF& pos)
{
    const QRectF& r = rect();
    qreal value;
    value = qBound(r.left(), pos.x(), r.right()) / (r.right() - r.left());

    updateActiveComponents(value);
}

void CHorizontalColorComponentSlider::beginEdit()
{
    if (!keyInputEnabled())
    {
        return;
    }

    m_EditText = m_EditType == EditType::Float ? toString(componentsValueF()) : toString(componentsValue());
    m_Text = m_EditText;
    m_EditTextCurPos = m_EditText.size();
    m_EditTextSelOffset = -m_EditText.size();
    m_Timer.start(1000);
}

void CHorizontalColorComponentSlider::cancelEdit()
{
    m_EditText = QString();
    m_EditTextCurPos = 0;
    m_EditTextSelOffset = 0;
    m_Timer.stop();
    update();
}

void CHorizontalColorComponentSlider::endEdit()
{
    if (m_Text != m_EditText)
    {
        bool valid;
        switch (m_EditType)
        {
            case EditType::Float:
                {
                    qreal val = qBound(0.0, m_EditText.toDouble(&valid), 1.0);
                    if (valid)
                    {
                        updateActiveComponents(val);
                        emit colorChanged(m_Color);
                    }
                }
                break;
            case EditType::Int:
                {
                    int val = qBound(0, m_EditText.toInt(&valid), 255);
                    if (valid)
                    {
                        updateActiveComponents(val);
                        emit colorChanged(m_Color);
                    }
                }
                break;
        }
    }

    cancelEdit();
}

bool CHorizontalColorComponentSlider::isEditing() const
{
    return keyInputEnabled() && !m_EditText.isNull();
}

quint32 CHorizontalColorComponentSlider::toEditCursorPos(int pos) const
{
    QFontMetrics fm(m_Font);
    QRect font_rect = fm.tightBoundingRect(m_EditText);
    int cw = fm.averageCharWidth();
    int cursor_pos = (pos + font_rect.width() - rect().width() + (int)(cw * 0.5f)) / cw;
    return static_cast<quint32>(qBound(0, cursor_pos, m_EditText.size()));
}

void CHorizontalColorComponentSlider::mousePressEvent(QMouseEvent* event)
{
    if (isEditing())
    {
        quint32 p = toEditCursorPos(event->pos().x());
        m_EditTextCurPos = p;
        m_EditTextSelOffset = 0;
        update();
    }
    else
    {
        updateColor(event->pos());
        emit colorChanging(m_Color);
    }
}

void CHorizontalColorComponentSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (isEditing())
    {
        m_AnimEditCursor = false;
    }
    mousePressEvent(event);
}

void CHorizontalColorComponentSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (!isEditing())
    {
        updateColor(event->pos());
        emit colorChanged(m_Color);
    }
    update();
}

void CHorizontalColorComponentSlider::mouseDoubleClickEvent(QMouseEvent*)
{
    cancelEdit();
    beginEdit();
}

void CHorizontalColorComponentSlider::focusInEvent(QFocusEvent* event)
{
    int reason = event->reason();
    if(reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason)
    {
        cancelEdit();
        beginEdit();
    }
}

void CHorizontalColorComponentSlider::focusOutEvent(QFocusEvent* event)
{
    int reason = event->reason();
    if(reason == Qt::TabFocusReason || reason == Qt::BacktabFocusReason)
    {
        endEdit();
    }
    else
    {
        cancelEdit();
    }
}

void CHorizontalColorComponentSlider::keyPressEvent(QKeyEvent* event)
{
    if (isEditing())
    {
        int key = event->key();
        if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            endEdit();
        }
        else if (key == Qt::Key_Left)
        {
            if (event->modifiers() & Qt::ShiftModifier)
            {
                m_EditTextSelOffset += m_EditTextCurPos > 0 ? 1 : 0;
                m_EditTextCurPos < 1 ? 0 : --m_EditTextCurPos;
            }
            else
            {
                m_EditTextSelOffset = 0;
                m_EditTextCurPos < 1 ? 0 : --m_EditTextCurPos;
            }

            m_AnimEditCursor = false;
            update();
            return;
        }
        else if (key == Qt::Key_Right)
        {
            quint32 eol = m_EditText.size();
            if (event->modifiers() & Qt::ShiftModifier)
            {
                m_EditTextSelOffset -= m_EditTextCurPos < eol ? 1 : 0;
                m_EditTextCurPos >= eol ? eol : ++m_EditTextCurPos;
            }
            else
            {
                m_EditTextSelOffset = 0;
                m_EditTextCurPos >= eol ? eol : ++m_EditTextCurPos;
            }

            m_AnimEditCursor = false;
            update();
            return;
        }

        // check for modifying input
        if (!((key >= Qt::Key_0 && key <= Qt::Key_9) ||
             key == Qt::Key_Comma || key == Qt::Key_Period ||
             key == Qt::Key_Backspace || key == Qt::Key_Delete))
        {
            return;
        }

        if (m_EditTextSelOffset)
        {
            // delete selection
            int pos = qMin(m_EditTextCurPos, m_EditTextCurPos + m_EditTextSelOffset);
            int n = qAbs(m_EditTextSelOffset);
            m_EditText = m_EditText.replace(pos, n, "");
            m_EditTextCurPos = pos;
            m_EditTextSelOffset = 0;

            // insert value of key if a valid input key was pressed
            if(key != Qt::Key_Delete && key != Qt::Key_Backspace)
            {
                m_EditText.insert(m_EditTextCurPos++, event->text()[0]);
            }
        }
        else if (key == Qt::Key_Delete)
        {
            m_EditText = m_EditText.replace(m_EditTextCurPos, 1, "");
        }
        else if (key == Qt::Key_Backspace)
        {
            if (m_EditTextCurPos > 0)
            {
                m_EditText = m_EditText.replace(--m_EditTextCurPos, 1, "");
            }
        }
        else
        {
            m_EditText.insert(m_EditTextCurPos++, event->text()[0]);
        }

        update();
    }
}

void CHorizontalColorComponentSlider::setEditType(CHorizontalColorComponentSlider::EditType type)
{
    m_EditType = type;
    update();
}

CHorizontalColorComponentSlider::EditType CHorizontalColorComponentSlider::editType()
{
    return m_EditType;
}

void CHorizontalColorComponentSlider::paintEvent(QPaintEvent*)
{
    const QRect& r = rect();
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(r);

    QPen pen;

    if (m_GradientColor0.alpha() < 255 || m_GradientColor1.alpha() < 255)
    {
        quint32 size = qMin(r.width(), r.height()) / 2;
        drawCheckerboard(painter, r, size);
    }

    QLinearGradient gradient = QLinearGradient(r.topLeft(), r.topRight());
    gradient.setColorAt(0, m_GradientColor0);
    gradient.setColorAt(1, m_GradientColor1);
    painter.fillRect(r, gradient);

    qreal val = componentsValueF();
    int pos;
    QLineF line0;
    QLineF line1;
    QLineF line2;
    pos = val * (r.right() - r.left());
    line1 = QLineF(pos, r.top(), pos, r.bottom());
    line0 = line1.translated(-1, 0);
    line2 = line1.translated(1, 0);

    if (m_DisplayText)
    {
        QString text = isEditing() ? m_EditText : (m_EditType == EditType::Float ? toString(val) : toString(componentsValue()));

        QFontMetrics fm(m_Font);
        int cursor_width = 3;
        int font_width = fm.width(text);
        int text_cur_pos = 0;
        int text_sel_pos = 0;

        if (isEditing())
        {
            text_cur_pos = r.width() - cursor_width - font_width + fm.width(text.mid(0, m_EditTextCurPos));
            text_sel_pos = r.width() - cursor_width - font_width + fm.width(text.mid(0, m_EditTextCurPos + m_EditTextSelOffset));
        }

        if (isEditing() && m_EditTextSelOffset != 0)
        {
            QRectF font_rect = fm.tightBoundingRect(text);
            font_rect.moveLeft(qMin(text_cur_pos, text_sel_pos));
            font_rect.setRight(qMax(text_cur_pos, text_sel_pos));
            font_rect.setTop(r.top());
            font_rect.setBottom(r.bottom() + 1);
            painter.setPen(pen);
            painter.fillRect(font_rect, QColor(50, 50, 255));
        }

        pen.setWidthF(0.5);
        pen.setColor(QColor(0, 0, 0, 255));
        painter.setPen(pen);
        painter.setBrush(QColor(255, 255, 255, 224));
        QPainterPath text_path;
        qreal y_pos = r.height() - fm.descent() / 2;
        text_path.addText(r.width() - font_width - cursor_width, y_pos, m_Font, text);
        painter.drawPath(text_path);

        if (isEditing() && (!m_AnimEditCursor || (QTime::currentTime().second() % 2) == 0))
        {
            painter.setRenderHint(QPainter::Antialiasing, false);
            QLineF cursor(qBound(0, text_cur_pos, r.right() - r.left()), r.top(), text_cur_pos, r.bottom());
            QLineF cursorl = cursor.translated(-1, 0);
            QLineF cursorr = cursor.translated(1, 0);
            pen.setWidth(1);
            pen.setColor(Qt::black);
            painter.setPen(pen);
            painter.drawLine(cursorl);
            painter.drawLine(cursorr);
            pen.setColor(Qt::white);
            painter.setPen(pen);
            painter.drawLine(cursor);
        }
    }

    if(!isEditing())
    {
        painter.setRenderHint(QPainter::Antialiasing, false);

        pen.setWidth(1);
        pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawLine(line0);
        painter.drawLine(line2);
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawLine(line1);
    }

    painter.restore();
}
