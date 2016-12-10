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

#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "ztwidgets_global.h"

#include <QWidget>

class ColorPickerPrivate;

/**
 * @brief A compact color picker widget
 *
 * A compact color picker widget with a hue and saturation wheel.
 * Additional sliders for value, red, green, blue and alpha channels.
 */
class ZTWIDGETS_EXPORT ColorPicker : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(ColorPicker)
    Q_DECLARE_PRIVATE(ColorPicker)

    /**
     * @brief Show or hide the alpha channel
     */
    Q_PROPERTY(bool displayAlpha READ displayAlpha WRITE setDisplayAlpha)

    /**
     * @brief Select the type used by the UI when directly editing values
     */
    Q_PROPERTY(EditType editType READ editType WRITE setEditType)

public:

    /**
     * @brief Supported edit types. These are used for display and UI.
     */
    enum EditType
    {
        Int        = 0,
        Float      = 1,
    };

    Q_ENUM(EditType)

    /**
     * @brief Construct an instance of ColorPicker
     * @param parent Parent widget
     */
    ColorPicker(QWidget* parent = Q_NULLPTR);

    virtual ~ColorPicker();

     /**
     * @brief Update color
     * @param color The new color
     *
     * @note Does not emit a signal
     *
     * Update the color this widget represents.
     */
    void updateColor(const QColor& color);

    /**
     * @brief Set color
     * @param color The new color
     *
     * Set the color this widget represents. Will emit a colorChanged signal if the color changes.
     */
    void setColor(const QColor& color);

    /**
     * @brief Get the display status of the alpha channel
     * @return true if alpha channel is displayed in the widget
     */
    bool displayAlpha();

    /**
     * @brief Get the type used when directly editing values
     * @return The type used when directly editing values
     */
    EditType editType();

    /**
     * @brief Show or hide the alpha channel
     * @param visible true if alpha channel should be visible
     */
    void setDisplayAlpha(bool visible);

    /**
     * @brief Set the type used when directly editing values
     * @param type The type used by the widget
     */
    void setEditType(EditType type);

signals:
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

private:
    ColorPickerPrivate* const d_ptr;
};

#endif // COLORPICKER_H
