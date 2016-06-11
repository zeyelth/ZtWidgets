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

#ifndef COLORDISPLAY_H
#define COLORDISPLAY_H

#include "colorwidgetbase.h"

/**
 * @brief A simple color display widget
 *
 * A simple color display widget, including alpha values
 */
class CColorDisplay : public CColorWidgetBase
{
    Q_OBJECT
public:
    /**
     * @brief Construct an instance of CColorDisplay
     * @param parent Parent widget
     */
    CColorDisplay(QWidget* parent = Q_NULLPTR);

    /**
     * @brief Reimplemented from QWidget::mouseReleaseEvent()
     */
    void mouseReleaseEvent(QMouseEvent*) override;

    /**
     * @brief Reimplemented from QWidget::paintEvent()
     */
    void paintEvent(QPaintEvent*) override;
signals:
    /**
     * Emitted when the widget is clicked
     */
    void clicked();
};

#endif // COLORDISPLAY_H
