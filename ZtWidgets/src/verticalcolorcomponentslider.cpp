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

#include <ZtWidgets/verticalcolorcomponentslider.h>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

CVerticalColorComponentSlider::CVerticalColorComponentSlider(Components components, QWidget *parent)
    : CVerticalColorComponentSlider(components, 20, Qt::white, Qt::black, parent)
{

}

CVerticalColorComponentSlider::CVerticalColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent)
    : CAbstractColorComponentSlider(components, width, color0, color1, parent)
{
    setMinimumHeight(0);
    setMaximumHeight(QWIDGETSIZE_MAX);
    setMinimumWidth(m_Width);
    setMaximumWidth(m_Width);
}

void CVerticalColorComponentSlider::updateColor(const QPointF& pos)
{
    const QRectF& r = rect();
    qreal value;
    value = 1 - qBound(r.top(), pos.y(), r.bottom()) / (r.bottom() - r.top());

    updateActiveComponents(value);
}

void CVerticalColorComponentSlider::mousePressEvent(QMouseEvent* event)
{
    updateColor(event->pos());
    emit colorChanging(m_Color);
}

void CVerticalColorComponentSlider::mouseMoveEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}

void CVerticalColorComponentSlider::mouseReleaseEvent(QMouseEvent* event)
{
    updateColor(event->pos());
    emit colorChanged(m_Color);
    update();
}

void CVerticalColorComponentSlider::paintEvent(QPaintEvent*)
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

    QLinearGradient gradient = QLinearGradient(r.bottomLeft(), r.topLeft());
    gradient.setColorAt(0, m_GradientColor0);
    gradient.setColorAt(1, m_GradientColor1);
    painter.fillRect(r, gradient);

    qreal val = componentsValue();
    int pos;
    QLineF line0;
    QLineF line1;
    QLineF line2;

    pos = r.height() - val * (r.bottom() - r.top());
    line1 = QLineF(r.left(), pos, r.right(), pos);
    line0 = line1.translated(0, -1);
    line2 = line1.translated(0, 1);

    painter.setRenderHint(QPainter::Antialiasing, false);

    pen.setWidth(1);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.drawLine(line0);
    painter.drawLine(line2);
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawLine(line1);

    painter.restore();
}
