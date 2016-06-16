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

#ifndef ABSTRACTCOLORCOMPONENTSLIDER_H
#define ABSTRACTCOLORCOMPONENTSLIDER_H

#include "colorwidgetbase.h"


/**
 * @brief A custom slider for manipulating color components.
 */
class AbstractColorComponentSlider : public ColorWidgetBase
{
    Q_OBJECT

public:
    /**
     * @brief Supported components
     */
    enum class Component
    {
        Hue        = 1 << 0,
        Value      = 1 << 1,
        Saturation = 1 << 2,
        HSV        = Hue | Value | Saturation,
        Red        = 1 << 3,
        Green      = 1 << 4,
        Blue       = 1 << 5,
        RGB        = Red | Green | Blue,
        Alpha      = 1 << 6,
        RGBA       = RGB | Alpha,
    };
    Q_DECLARE_FLAGS(Components, Component)

    /**
     * @brief Abstract base class for color component sliders
     * @param components Set of color components controlled by this widget. Typically one but can be more.
     * @param width Width of the widget in pixels
     * @param color0 Start color of gradient
     * @param color1 End color of gradient
     * @param parent Parent widget
     */
    AbstractColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent = Q_NULLPTR);

    /**
     * @brief Choose components affected by this widget. Can be more than one.
     * @param components The components affected by this widget. See Component for supported components.
     */
    void setActiveComponents(Components components);

    /**
     * @brief Set the start and end gradient of the slider, for visual purposes
     * @param color0 start of gradient
     * @param color1 end of gradient
     */
    void setGradient(const QColor& color0, const QColor& color1);

    /**
     * @brief Get a string representation of the active components, e.g. "Red", "RB"
     * @return A string representing the active components
     */
    QString activeComponentsName() const;

    /**
     * @brief Reimplement this from QWidget
     */
    void paintEvent(QPaintEvent*) override = 0;

    /**
     * @brief Get a string representation of a given component
     * @param component The component
     * @param abbreviate true if the string should be abreviated, e.g. "R" instead of "Red", false otherwise
     * @return A string representing the given component
     */
    static QString componentName(Component component, bool abbreviate);

protected:
    /**
     * @brief Get the value of components represented by this widget.
     *
     * Get the value of components represented by this widget. If the widget represents more than one component, an average is returned.
     *
     * @return The value
     */
    qreal componentsValueF() const;

    /**
     * @brief Get the value of components represented by this widget.
     *
     * Get the value of components represented by this widget. If the widget represents more than one component, an average is returned.
     *
     * @return The value
     */
    int componentsValue() const;

    /**
     * @brief Get the number of components represented by this widget
     * @return Number of components represented by this widget
     */
    quint32 componentCount() const;

    /**
     * @brief Update all components represented by this widget with a given value
     * @param value Value to update all components with
     */
    void updateActiveComponents(qreal value);

    /**
     * @brief Update all components represented by this widget with a given value
     * @param value Value to update all components with
     */
    void updateActiveComponents(int value);

    /**
     * @brief Update the QColor of this component based on user interaction
     * @param pos Widget cursor position
     */
    virtual void updateColor(const QPointF& pos) = 0;

    /// Start color of gradient
    QColor m_GradientColor0;
    /// End color of gradient
    QColor m_GradientColor1;
    /// Components represented by this widget
    Components m_Components;
    /// Width of widget in pixels
    quint32 m_Width;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AbstractColorComponentSlider::Components)

#endif // ABSTRACTCOLORCOMPONENTSLIDER_H
