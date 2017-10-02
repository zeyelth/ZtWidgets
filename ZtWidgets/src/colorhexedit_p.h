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

#ifndef COLORHEXEDIT_H
#define COLORHEXEDIT_H

#include <QWidget>

class ColorHexEditPrivate;

/**
 * @brief A line edit widget showing a hex value of the color it represents
 *
 * A line edit widget, configured to manipulate color values on the form of \#AARRGGBB
 * where AA, RR, GG and BB represents the alpha, red, green, and blue channels respectively.
 */
class ColorHexEdit : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(ColorHexEdit)
    Q_DECLARE_PRIVATE(ColorHexEdit)

    /**
     * @brief Show or hide the alpha channel
     */
    Q_PROPERTY(bool displayAlpha READ displayAlpha WRITE setDisplayAlpha)

public:
    /**
     * @brief Construct an instance of ColorHexEdit
     * @param parent Parent widget
     */
    explicit ColorHexEdit(QWidget* parent = Q_NULLPTR);

    virtual ~ColorHexEdit();

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
     * @brief Show or hide the alpha channel
     * @param visible true if alpha channel should be visible
     */
    void setDisplayAlpha(bool visible);

protected:
    /**
     * @brief Overridden from QWidget
     */
    void showEvent(QShowEvent* event) override;

Q_SIGNALS:
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
    ColorHexEditPrivate* const d_ptr;
};

#endif // COLORHEXEDIT_H
