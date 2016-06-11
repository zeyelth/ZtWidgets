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

class QFrame;
class QShowEvent;
class CHorizontalColorComponentSlider;
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

public:
    /**
     * @brief Construct an instance of CColorPicker
     * @param parent Parent widget
     */
    CColorPicker(QWidget* parent = Q_NULLPTR);

    void updateColor(const QColor& color) override;

private slots:
    void onDisplayClicked();

private:
    class CPopup : public CColorWidgetBase
    {

    public:
        CPopup(QWidget* parent = Q_NULLPTR);

        void updateColor(const QColor& color) override;
        void showEvent(QShowEvent* event) override;

    private:
        QFrame* m_frame;
        CColorHexEdit* m_hex;
        CColorDisplay* m_display;
        CHueSaturationWheel* m_wheel;
        CVerticalColorComponentSlider* m_valueSlider;
        CHorizontalColorComponentSlider* m_redSlider;
        CHorizontalColorComponentSlider* m_greenSlider;
        CHorizontalColorComponentSlider* m_blueSlider;
        CHorizontalColorComponentSlider* m_alphaSlider;
    };

    CColorHexEdit* m_hex;
    CColorDisplay* m_display;
    CPopup* m_popup;
};

#endif // COLORPICKER_H
