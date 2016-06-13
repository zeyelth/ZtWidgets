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

#include "colorwidgetbase.h"
#include "horizontalcolorcomponentslider.h"

class QFrame;
class QLabel;
class QShowEvent;
class CVerticalColorComponentSlider;
class CColorHexEdit;
class CColorDisplay;
class CHueSaturationWheel;


/**
 * @brief A compact color picker widget
 *
 * A compact color picker widget with a hue and saturation wheel.
 * Additional sliders for value, red, green, blue and alpha channels.
 */
class CColorPicker : public CColorWidgetBase
{
    Q_OBJECT

    /**
     * @brief Show or hide the alpha channel
     */
    Q_PROPERTY(bool displayAlpha READ displayAlpha WRITE setDisplayAlpha)

    /**
     * @brief Select the type used by the UI when directly editing values
     */
    Q_PROPERTY(CHorizontalColorComponentSlider::EditType editType READ editType WRITE setEditType)

public:

    /**
     * @brief Construct an instance of CColorPicker
     * @param parent Parent widget
     */
    CColorPicker(QWidget* parent = Q_NULLPTR);

    void updateColor(const QColor& color) override;

    /**
     * @brief Get the display status of the alpha channel
     * @return true if alpha channel is displayed in the widget
     */
    bool displayAlpha();

    /**
     * @brief Get the type used when directly editing values
     * @return The type used when directly editing values
     */
    CHorizontalColorComponentSlider::EditType editType();

public slots:
    /**
     * @brief Show or hide the alpha channel
     * @param visible true if alpha channel should be visible
     */
    void setDisplayAlpha(bool visible);

    /**
     * @brief Set the type used when directly editing values
     * @param type The type used by the widget
     */
    void setEditType(CHorizontalColorComponentSlider::EditType type);

private slots:
    void onDisplayClicked();

private:
    class CPopup : public CColorWidgetBase
    {

    public:
        CPopup(QWidget* parent = Q_NULLPTR);

        void updateColor(const QColor& color) override;
        void showEvent(QShowEvent* event) override;

        bool displayAlpha();
        void setDisplayAlpha(bool visible);

        CHorizontalColorComponentSlider::EditType editType();
        void setEditType(CHorizontalColorComponentSlider::EditType editType);

    private:
        QFrame* m_Frame;
        CColorHexEdit* m_Hex;
        CColorDisplay* m_Display;
        CHueSaturationWheel* m_Wheel;
        CVerticalColorComponentSlider* m_ValueSlider;
        CHorizontalColorComponentSlider* m_RedSlider;
        CHorizontalColorComponentSlider* m_GreenSlider;
        CHorizontalColorComponentSlider* m_BlueSlider;
        CHorizontalColorComponentSlider* m_AlphaSlider;
        QLabel* m_AlphaLabel;
    };

    CColorHexEdit* m_Hex;
    CColorDisplay* m_Display;
    CPopup* m_Popup;
    bool m_DisplayAlpha;
};

#endif // COLORPICKER_H
