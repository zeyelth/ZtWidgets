/*
 * Copyright (c) 2016 Victor Wåhlström
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

#ifndef SLIDEREDIT_H
#define SLIDEREDIT_H

#include "ztwidgets_global.h"

#include <QWidget>
#include <QString>

class SliderEditPrivate;

/**
 * @brief The SliderEdit class provides a horizontal slider with editing capabilities
 */
class ZTWIDGETS_EXPORT SliderEdit : public QWidget
{
    Q_OBJECT

    Q_DISABLE_COPY(SliderEdit)
    Q_DECLARE_PRIVATE(SliderEdit)

    /**
     * @brief This property holds the slider's minimum value
     *
     * @note Setting this property may modify the properties minimum and value, to ensure they are valid and in range
     */
    Q_PROPERTY(qreal minimum READ minimum WRITE setMinimum)

    /**
     * @brief This property holds the slider's maximum value
     *
     * @note Changing this may modify the properties minimum and value, to ensure they are valid and in range
     */
    Q_PROPERTY(qreal maximum READ maximum WRITE setMaximum)

    /**
     * @brief This property holds whether the slider can be edited via keyboard input
     */
    Q_PROPERTY(bool editable READ editable WRITE setEditable)

    /**
     * @brief This property controls the small incremental stepping behavior
     */
    Q_PROPERTY(qreal singleStep READ singleStep WRITE setSingleStep)

    /**
     * @brief This property controls the big incremental stepping behavior
     */
    Q_PROPERTY(qreal pageStep READ pageStep WRITE setPageStep)

    /**
     * @brief This property holds the slider's value
     */
    Q_PROPERTY(qreal value READ value WRITE setValue USER true)

    /**
     * @brief This property holds the displayed label
     */
    Q_PROPERTY(QString label READ label WRITE setLabel)

    /**
     * @brief This property holds the displayed unit
     */
    Q_PROPERTY(QString unit READ unit WRITE setUnit)

    /**
     * @brief This property holds the number of displayed decimals of the value
     *
     * Changing this does not affect the stored value
     */
    Q_PROPERTY(quint32 precision READ precision WRITE setPrecision)

    /**
     * @brief This property controls snapping behavior. If active, values will snap to the specified precision
     *
     * @note Changing this may alter the current value
     *
     * See also precision()
     */
    Q_PROPERTY(bool snapToPrecision READ isSnappingToPrecision WRITE setSnapToPrecision)

    /**
     * @brief This property holds the alignment of any displayed text in the widget
     */
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

    /**
     * @brief This property holds the orientation of the widget
     */
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

    /**
     * @brief This property holds the active slider components used by the widget
     */
    Q_PROPERTY(SliderComponents sliderComponents READ sliderComponents WRITE setSliderComponents)

public:

    /**
     * @brief Supported components
     */
    enum SliderComponent
    {
        Text   = 1 << 0,
        Marker = 1 << 1,
        Gauge  = 1 << 2,
    };

    Q_DECLARE_FLAGS(SliderComponents, SliderComponent)
    Q_FLAG(SliderComponents)

    /**
     * @brief Construct an instance of SliderEdit
     * @param parent Parent widget
     * @param f Widget window flags
     */
    explicit SliderEdit(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~SliderEdit();

    /**
     * @brief Overridden from QWidget
     */
    QSize sizeHint() const override;

    /**
     * @brief Get the minimum allowed value
     */
    qreal minimum() const;

    /**
     * @brief Set the minimum allowed value
     * @param minimum The new minimum
     *
     * @note This may modify the maximum allowed value and adjust the value to the new limits
     */
    void setMinimum(qreal minimum);

    /**
     * @brief Get the maximum allowed value
     * @return The current maximum allowed value
     */
    qreal maximum() const;

    /**
     * @brief Set the maximum allowed value
     * @param maximum The new maximum
     *
     * @note This may modify the minimum allowed value and adjust the value to the new limits
     */
    void setMaximum(qreal maximum);

    /**
     * @brief Convenience function to set minimum and maximum values at the same time
     * @param minimum The new minimum
     * @param maximum The new maximum
     *
     * @note Calling this is equivalent to setMinimum(minimum); setMaximum(maximum);
     */
    void setRange(qreal minimum, qreal maximum);

    /**
     * @brief Get the edit status of the widget
     * @return true if the slider widget can be edited with a keyboard, false otherwise
     */
    bool editable() const;

    /**
     * @brief Set the edit status of the widget
     * @param editable true if it should be possible to edit this widget's value with a keyboard
     */
    void setEditable(bool editable);

    /**
     * @brief Get the small step value, e.g. when using arrow keys
     */
    qreal singleStep() const;

    /**
     * @brief Set the value the slider should increment or decrement with each small step
     * @param step The new step
     */
    void setSingleStep(qreal step);

    /**
     * @brief Get the big step value, e.g. when using PageUp/PageDown
     */
    qreal pageStep() const;

    /**
     * @brief Set the value the slider should increment or decrement with each big step
     * @param step The new step
     */
    void setPageStep(qreal step);

    /**
     * @brief Get the precision snapping status of the widget
     */
    bool isSnappingToPrecision() const;

    /**
     * @brief Enable or disable precision snapping. If set, values will snap to the precision of the widget, as decided by precision().
     * @param enable true if snapping should be enabled
     */
    void setSnapToPrecision(bool enable);

    /**
     * @brief Get the value of the slider
     */
    qreal value() const;

    /**
     * @brief Set the value of the slider
     * @param value new value
     */
    void setValue(qreal value);

    /**
     * @brief Update the value without emitting any signals
     * @param value new value
     */
    void updateValue(qreal value);

    /**
     * @brief Set the display unit of the slider
     * @param unit unit to be displayed after the value
     */
    void setUnit(const QString& unit);

    /**
     * @brief Get the display unit of the slider
     * @return A string representing the display unit of this slider
     */
    const QString& unit() const;

    /**
     * @brief Set the displayed label of the slider
     * @param label A label which is to be displayed on the slider
     */
    void setLabel(const QString& label);

    /**
     * @brief Get the displayed label of the slider
     * @return  A string representing the displayed label of this slider
     */
    const QString& label() const;

    /**
     * @brief Set the precision of the displayed value
     * @param precision number of decimal points in the displayed value
     */
    void setPrecision(quint32 precision);

    /**
     * @brief Get the precision of the displayed value
     * @return The precision of the displayed value
     */
    quint32 precision() const;

    /**
     * @brief This property holds the active slider components used by the widget
     * @param components this slider widget should use
     */
    void setSliderComponents(SliderComponents components);

    /**
     * @brief Active slider components
     * @return Active slider components
     */
    SliderComponents sliderComponents() const;

    /**
     * @brief Set the alignment of any text displayed in the widget
     * @param alignment Alignment of the text. This includes the label, value, and unit.
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief Alignment of any text displayed in the widget
     * @return Alignment of displayed text
     */
    Qt::Alignment alignment() const;

    /**
     * @brief Set the orientation of the widget
     * @param orientation Orientation of the widget
     */
    void setOrientation(Qt::Orientation orientation);

    /**
     * @brief Current orientation of the widget
     * @return Current orientation of the widget
     */
    Qt::Orientation orientation() const;

Q_SIGNALS:
    /**
     * @param value The new value
     *
     * Emitted when the value has changed
     */
    void valueChanged(const qreal& value);

    /**
     * @param value The new value
     *
     * Emitted while the value is being changed
     */
    void valueChanging(const qreal& value);

protected:
    /**
     * @brief Overridden from QWidget
     */
    void paintEvent(QPaintEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void keyPressEvent(QKeyEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void mousePressEvent(QMouseEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void mouseMoveEvent(QMouseEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void mouseReleaseEvent(QMouseEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void mouseDoubleClickEvent(QMouseEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void focusInEvent(QFocusEvent*) override;

    /**
     * @brief Overridden from QWidget
     */
    void focusOutEvent(QFocusEvent*) override;

private:
    SliderEditPrivate* const d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SliderEdit::SliderComponents)

#endif // SLIDEREDIT_H
