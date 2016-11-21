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

#include <ZtWidgets/huesaturationwheel.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

static QRect fittedSquare(const QRect& rect)
{
    QRect square(rect);

    int w = square.width();
    int h = square.height();
    if (w > h)
    {
        int offset = (w - h) / 2;
        square.setWidth(h);
        square.moveLeft(square.left() + offset);
    }
    else
    {
        int offset = (h - w) / 2;
        square.setHeight(w);
        square.moveTop(square.top() + offset);
    }
    return square;
}

//! @cond Doxygen_Suppress
class HueSaturationWheelPrivate
{
    Q_DISABLE_COPY(HueSaturationWheelPrivate)
    Q_DECLARE_PUBLIC(HueSaturationWheel)

private:
    explicit HueSaturationWheelPrivate(HueSaturationWheel*);

    HueSaturationWheel* const q_ptr;

    void updateColor(const QPointF& pos);
    void updateMarkerPos();
    void rebuildColorWheel();

    QImage m_wheelImg;
    QPointF m_markerPos;
};

HueSaturationWheelPrivate::HueSaturationWheelPrivate(HueSaturationWheel* hs_wheel)
    : q_ptr(hs_wheel)
{}

void HueSaturationWheelPrivate::updateMarkerPos()
{
    Q_Q(HueSaturationWheel);
    QRect square = fittedSquare(q->rect());
    qreal radius = square.width() * 0.5;
    qreal h = q->m_Color.hsvHueF();
    qreal s = q->m_Color.hsvSaturationF();

    qreal distance = s * radius;

    QPoint center = square.center();
    QLineF line(center.x(), center.y(), center.x(), center.y() + distance);
    line.setAngle(360.0 - h * 360.0 - 90.0);
    m_markerPos = line.p2();
}

void HueSaturationWheelPrivate::rebuildColorWheel()
{
    Q_Q(HueSaturationWheel);
    QRect square = fittedSquare(q->rect());
    m_wheelImg = QImage(square.size(), QImage::Format_ARGB32_Premultiplied);
    m_wheelImg.fill(0);

    QRect r = m_wheelImg.rect();
    QPainter painter(&m_wheelImg);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(Qt::transparent);
    painter.setPen(pen);
    painter.setBrush(Qt::transparent);
    painter.drawRect(r);

    QPainterPath path;
    path.addEllipse(r);

    painter.setClipPath(path);

    // Calculate hue, value
    QConicalGradient hue(r.center(), -90.0);
    QColor color;
    qreal step = 0.0;
    qreal val = q->m_Color.valueF();

    while (step < 1.0)
    {
        color.setHsvF(1.0 - step, 1.0, val);
        hue.setColorAt(step, color);
        step += 0.1;
    }
    painter.fillPath(path, hue);

    // Calculate saturation. May not be pixel perfect
    qreal radius = r.width() * 0.5;
    QRadialGradient sat(r.center(), radius);
    color.setRgbF(val, val, val, 1.0);
    sat.setColorAt(0, color);
    color.setRgbF(val, val, val, 0.0);
    sat.setColorAt(1, color);
    painter.fillPath(path, sat);
    painter.restore();

    q->update();
}

void HueSaturationWheelPrivate::updateColor(const QPointF &pos)
{
    Q_Q(HueSaturationWheel);
    QRect square = fittedSquare(q->rect());

    qreal radius = square.width() * 0.5;
    QLineF line(square.center(), pos);
    qreal distance = qBound(0.0, line.length(), radius);
    line.setAngle(line.angle() + 90.0);
    qreal h = (360.0 - line.angle()) / 360.0;
    qreal s = distance / radius;
    qreal v = q->m_Color.valueF();
    qreal a = q->m_Color.alphaF();
    q->m_Color.setHsvF(h, s, v, a);
}
//! @endcond

HueSaturationWheel::HueSaturationWheel(QWidget* parent)
    : ColorWidgetBase(parent)
    , d_ptr(new HueSaturationWheelPrivate(this))
{
    Q_D(HueSaturationWheel);
    setFocusPolicy(Qt::ClickFocus);

    m_Color = Qt::white;
    d->m_markerPos = QPointF(0, 0);
    setMinimumSize(10, 10);
}

HueSaturationWheel::~HueSaturationWheel()
{
    delete d_ptr;
}

void HueSaturationWheel::updateColor(const QColor& color)
{
    Q_D(HueSaturationWheel);
    if (color.rgb() == m_Color.rgb())
    {
        // alpha may have changed
        ColorWidgetBase::updateColor(color);
        return;
    }

    int old_value = m_Color.value();

    QRect square = fittedSquare(rect());
    qreal radius = square.width() * 0.5;
    qreal h = color.hsvHueF();
    qreal s = color.hsvSaturationF();
    m_Color.setRgba(color.rgba());

    qreal distance = s * radius;
    QPoint center = square.center();

    QLineF line(center.x(), center.y(), center.x(), center.y() + distance);
    line.setAngle(360.0 - h * 360.0 - 90.0);

    d->updateColor(line.p2());
    d->updateMarkerPos();
    if (old_value != color.value())
    {
        d->rebuildColorWheel();
    }

    ColorWidgetBase::updateColor(color);
}

void HueSaturationWheel::resizeEvent(QResizeEvent* event)
{
    Q_D(HueSaturationWheel);
    d->rebuildColorWheel();
    d->updateMarkerPos();
    ColorWidgetBase::resizeEvent(event);
}

void HueSaturationWheel::mousePressEvent(QMouseEvent* event)
{
    Q_D(HueSaturationWheel);
    d->updateColor(event->pos());
    d->updateMarkerPos();
    emit colorChanging(m_Color);
}

void HueSaturationWheel::mouseMoveEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}

void HueSaturationWheel::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(HueSaturationWheel);
    d->updateColor(event->pos());
    d->updateMarkerPos();
    update();
    emit colorChanged(m_Color);
}

void HueSaturationWheel::paintEvent(QPaintEvent*)
{
    Q_D(HueSaturationWheel);
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(rect());

    QRect square = fittedSquare(rect());
    painter.drawImage(square, d->m_wheelImg);

    QPen pen;
    QColor marker_color = m_Color.valueF() > 0.5 ? Qt::black : Qt::white;
    pen.setColor(marker_color);

    painter.setPen(pen);
    QRectF marker(d->m_markerPos.x() - 2, d->m_markerPos.y() - 2, 5, 5);
    // arcs are specified in 1/16 degrees; draw a full circle
    painter.drawArc(marker, 0, 360 * 16);

    painter.restore();
}
