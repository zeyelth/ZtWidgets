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

#include "colorwidgetbase.h"

class QLineEdit;

/**
 * @brief A line edit widget showing a hex value of the color it represents
 *
 * A line edit widget, configured to manipulate color values on the form of \#AARRGGBB
 * where AA, RR, GG and BB represents the alpha, red, green, and blue channels respectively.
 */
class CColorHexEdit : public CColorWidgetBase
{
    Q_OBJECT

public:
    /**
     * @brief Construct an instance of CColorHexEdit
     * @param parent Parent widget
     */
    CColorHexEdit(QWidget* parent = Q_NULLPTR);

    /**
     * @brief Update the text string
     * @param color The new color
     *
     * Update the text string representing the color of this widget.
     */
    void updateColor(const QColor& color) override;
private slots:
    void onTextEdited(const QString& text);
private:
    /**
     * @brief Returns the inner width of this widget
     * @return The inner width of this widget
     */
    int editWidth() const;

    QLineEdit* m_LineEdit;
    bool m_Modified;
};

#endif // COLORHEXEDIT_H
