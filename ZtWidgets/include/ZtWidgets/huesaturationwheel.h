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

#ifndef HUESATURATIONWHEEL_H
#define HUESATURATIONWHEEL_H

#include "colorwidgetbase.h"

class HueSaturationWheelPrivate;

/**
 * @brief A color wheel for controlling hue and saturation
 */
class HueSaturationWheel : public ColorWidgetBase
{
    Q_OBJECT

    Q_DISABLE_COPY(HueSaturationWheel)
    Q_DECLARE_PRIVATE(HueSaturationWheel)

public:
    /**
     * @brief Construct an instance of HueSaturationWheel
     * @param parent Parent widget
     */
    HueSaturationWheel(QWidget* parent = Q_NULLPTR);

    virtual ~HueSaturationWheel();

    /**
     * @brief Reimplemented from ColorWidgetBase::updateColor()
     */
    void updateColor(const QColor&) override;

    /**
     * @brief Reimplemented from QWidget::updateColor()
     */
    void resizeEvent(QResizeEvent*) override;

    /**
     * @brief Reimplemented from QWidget::mousePressEvent()
     */
    void mousePressEvent(QMouseEvent*) override;

    /**
     * @brief Reimplemented from QWidget::mouseMoveEvent()
     */
    void mouseMoveEvent(QMouseEvent*) override;

    /**
     * @brief Reimplemented from QWidget::mouseReleaseEvent()
     */
    void mouseReleaseEvent(QMouseEvent*) override;

    /**
     * @brief Reimplemented from QWidget::paintEvent()
     */
    void paintEvent(QPaintEvent*) override;

private:
    HueSaturationWheelPrivate* const d_ptr;
};

#endif // HUESATURATIONWHEEL_H
