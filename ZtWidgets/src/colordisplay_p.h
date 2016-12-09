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

#include <QWidget>

class ColorDisplayPrivate;

/**
 * @brief A simple color display widget
 *
 * A simple color display widget, including alpha values
 */
class ColorDisplay : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(ColorDisplay)
    Q_DECLARE_PRIVATE(ColorDisplay)

public:
    /**
     * @brief Construct an instance of ColorDisplay
     * @param parent Parent widget
     */
    explicit ColorDisplay(QWidget* parent = Q_NULLPTR);

    virtual ~ColorDisplay();

    /**
     * @brief Update color
     * @param color The new color
     *
     * @note Does not emit a signal
     *
     * Update the color this widget represents.
     */
    void updateColor(const QColor& color);

signals:
    /**
     * Emitted when the widget is clicked
     */
    void clicked();

    /**
     * @param color The new color
     *
     * Emitted when the color has changed.
     */
    void colorChanged(const QColor& color);

    /**
     * @param color The new color
     *
     * Emitted while the color is being changed.
     */
    void colorChanging(const QColor& color);

protected:
    /**
     * @brief Reimplemented from QWidget::mouseReleaseEvent()
     */
    void mouseReleaseEvent(QMouseEvent*) override;

    /**
     * @brief Reimplemented from QWidget::paintEvent()
     */
    void paintEvent(QPaintEvent*) override;

private:
    ColorDisplayPrivate* const d_ptr;
};

#endif // COLORDISPLAY_H
