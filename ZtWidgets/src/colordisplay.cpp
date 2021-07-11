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

#include "colordisplay_p.h"

#include "color_utils_p.h"

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QSizePolicy>

//! @cond Doxygen_Suppress
class ColorDisplayPrivate
{
    Q_DISABLE_COPY(ColorDisplayPrivate)
    Q_DECLARE_PUBLIC(ColorDisplay)

  private:
    explicit ColorDisplayPrivate(ColorDisplay*);

    ColorDisplay* const q_ptr;

    QColor m_Color;
};

ColorDisplayPrivate::ColorDisplayPrivate(ColorDisplay* display)
    : q_ptr(display)
    , m_Color(Qt::white)
{}
//! @endcond

ColorDisplay::ColorDisplay(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new ColorDisplayPrivate(this))
{
    setMinimumSize(15, 15);
    QSizePolicy size_policy;
    size_policy.setHorizontalPolicy(QSizePolicy::Minimum);
    setSizePolicy(size_policy);
}

ColorDisplay::~ColorDisplay()
{
    delete d_ptr;
}

void ColorDisplay::updateColor(const QColor& color)
{
    Q_D(ColorDisplay);
    if (d->m_Color != color)
    {
        d->m_Color = color;
        update();
    }
}

void ColorDisplay::mouseReleaseEvent(QMouseEvent*)
{
    Q_EMIT clicked();
}

void ColorDisplay::paintEvent(QPaintEvent*)
{
    Q_D(ColorDisplay);
    QPainter painter(this);
    painter.save();

    painter.setClipRect(rect());
    drawCheckerboard(painter, rect(), 5);
    painter.fillRect(rect(), d->m_Color);

    painter.restore();
}
