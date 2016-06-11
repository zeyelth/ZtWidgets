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

#ifndef HORIZONTALCOLORCOMPONENTSLIDER_H
#define HORIZONTALCOLORCOMPONENTSLIDER_H

#include "abstractcolorcomponentslider.h"
#include <QtCore/QTimer>

/**
 * @brief A horizontally oriented color component slider
 */
class CHorizontalColorComponentSlider : public CAbstractColorComponentSlider
{
    Q_OBJECT
public:
    /**
     * @brief Construct an instance of CHorizontalColorComponentSlider
     * @param components Set of color components controlled by this widget. Typically one but can be more.
     * @param parent Parent widget
     */
    CHorizontalColorComponentSlider(Components components, QWidget* parent = Q_NULLPTR);

    /**
     * @brief Construct an instance of CHorizontalColorComponentSlider
     * @param components Set of color components controlled by this widget. Typically one but can be more.
     * @param width Width of the widget in pixels
     * @param color0 Start color of gradient
     * @param color1 End color of gradient
     * @param parent Parent widget
     */
    CHorizontalColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent = Q_NULLPTR);

    /**
     * @brief Enables keyboard input, used to manually set the components' value
     */
    void enableKeyInput();
    /**
     * @brief Disables keyboard input
     */
    void disableKeyInput();
    /**
     * @brief Keyboard input status
     * @return Returns true if keyboard input is enabled, false otherwise
     */
    bool keyInputEnabled() const;

    /**
     * @brief Show or hide a text overlay of the components' value over the slider
     * @param enable set to true if the text overlay should be enabled, false otherwise
     */
    void displayText(bool enable);

    /**
     * @brief Reimplemented from QWidget::paintEvent()
     */
    void paintEvent(QPaintEvent*) override;

    /**
     * @brief Reimplemented from QWidget::mouseDoubleClickEvent()
     */
    void mouseDoubleClickEvent(QMouseEvent*) override;

private:
    bool isEditing() const;
    quint32 toEditCursorPos(int pos) const;
    void beginEdit();
    void endEdit();
    void cancelEdit();

    void updateColor(const QPointF& pos) override;

    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void focusInEvent(QFocusEvent*) override;
    void focusOutEvent(QFocusEvent*) override;

    QString m_EditText;
    QString m_Text;
    QFont m_Font;
    QTimer m_Timer;
    quint32 m_EditTextCurPos;
    qint32 m_EditTextSelOffset;
    bool m_AnimEditCursor;
    bool m_DisplayText;
    bool m_KeyInputEnabled;
};

#endif // HORIZONTALCOLORCOMPONENTSLIDER_H
