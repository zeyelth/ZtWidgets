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

#ifndef VERTICALCOLORCOMPONENTSLIDER_H
#define VERTICALCOLORCOMPONENTSLIDER_H

#include "abstractcolorcomponentslider.h"


/**
 * @brief A vertically oriented color component slider
 */
class CVerticalColorComponentSlider : public CAbstractColorComponentSlider
{
    Q_OBJECT
public:
    /**
     * @brief Construct an instance of CVerticalColorComponentSlider
     * @param components Set of color components controlled by this widget. Typically one but can be more.
     * @param parent Parent widget
     */
    CVerticalColorComponentSlider(Components components, QWidget* parent = Q_NULLPTR);

    /**
     * @brief Construct an instance of CVerticalColorComponentSlider
     * @param components Set of color components controlled by this widget. Typically one but can be more.
     * @param width Width of the widget in pixels
     * @param color0 Start color of gradient
     * @param color1 End color of gradient
     * @param parent Parent widget
     */
    CVerticalColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent = Q_NULLPTR);

    void paintEvent(QPaintEvent*) override;

private:
    void updateColor(const QPointF& pos) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
};

#endif // VERTICALCOLORCOMPONENTSLIDER_H
