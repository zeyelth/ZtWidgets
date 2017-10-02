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

#ifndef COLORPICKERPOPUP_H
#define COLORPICKERPOPUP_H

#include <QWidget>

#include <ZtWidgets/colorpicker.h>

class ColorPickerPopupPrivate;

/**
 * @brief Internal ColorPicker Popup class
 */
class ColorPickerPopup : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(ColorPickerPopup)
    Q_DECLARE_PRIVATE(ColorPickerPopup)

public:
    /**
     * @brief Constructor
     * @param parent parent widget
     */
    explicit ColorPickerPopup(QWidget* parent = Q_NULLPTR);
    virtual ~ColorPickerPopup();

    /**
     * @brief Show or hide the alpha channel
     * @param visible true if alpha channel should be visible
     */
    void setDisplayAlpha(bool visible);

    /**
     * @brief Get the display status of the alpha channel
     * @return true if alpha channel is displayed in the widget
     */
    bool displayAlpha() const;

    /**
     * @brief Set the type used when directly editing values
     * @param type The type used by the widget
     */
    void setEditType(ColorPicker::EditType type);

    /**
     * @brief Get the type used when directly editing values
     * @return The type used when directly editing values
     */
    ColorPicker::EditType editType() const;

    /**
     * @brief Set color
     * @param color The new color
     */
    void setColor(const QColor& color);

    /**
    * @brief Update color
    * @param color The new color
    *
    * @note Does not emit a signal
    *
    * Update the color this widget represents.
    */
    void updateColor(const QColor& color);

Q_SIGNALS:
    /**
     * @brief Emitted when the color has changed
     * @param color new color
     */
    void colorChanged(const QColor& color);
    /**
     * @brief Emitted when the color is changing
     * @param color new color
     */
    void colorChanging(const QColor& color);

protected:
    /**
     * @brief Overridden from QWidget
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief Overridden from QWidget
     */
    void changeEvent(QEvent* event) override;

private:
    ColorPickerPopupPrivate* const d_ptr;
};

#endif // COLORPICKERPOPUP_H
