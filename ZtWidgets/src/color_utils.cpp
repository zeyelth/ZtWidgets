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

#include "color_utils_p.h"

#include <QPainter>
#include <QColor>
#include <QRect>

void drawCheckerboard(QPainter& painter, const QRect& rect, unsigned int size)
{
    QColor color1(153, 153, 152);
    QColor color2(102, 102, 102);

    painter.save();
    painter.fillRect(rect, color1);
    QRect square(0, 0, size, size);
    quint32 step_x(size * 2);
    quint32 step_y(size);
    bool odd = true;
    while(square.top() < rect.bottom())
    {
        while(square.left() < rect.right())
        {
            painter.fillRect(square, color2);
            square.moveLeft(square.left() + step_x);
        }

        square.moveLeft(0);
        if(odd)
        {
            square.moveLeft(square.left() + step_x * 0.5);
        }

        square.moveTop(square.top() + step_y);
        odd = !odd;
    }

    painter.restore();
}
