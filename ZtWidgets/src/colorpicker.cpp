/*
 * Copyright (c) 2013-2021 Victor Wåhlström
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

#include "colordisplay_p.h"
#include "colorhexedit_p.h"
#include "colorpickerpopup_p.h"
#include "huesaturationwheel_p.h"
#include <ZtWidgets/colorpicker.h>
#include <ZtWidgets/slideredit.h>

#include <QFontDatabase>
#include <QHBoxLayout>

//! @cond Doxygen_Suppress

class ColorPickerPrivate
{
    Q_DISABLE_COPY(ColorPickerPrivate)

  public:
    explicit ColorPickerPrivate();

    ColorHexEdit* m_Hex;
    ColorDisplay* m_Display;
    ColorPickerPopup* m_Popup;
    QColor m_Color;
    ColorPicker::EditType m_EditType;
    bool m_DisplayAlpha : 1;
};

ColorPickerPrivate::ColorPickerPrivate()
    : m_Hex(nullptr)
    , m_Display(nullptr)
    , m_Popup(nullptr)
    , m_Color(Qt::white)
    , m_EditType(ColorPicker::Float)
    , m_DisplayAlpha(true)
{}

//! @endcond

ColorPicker::ColorPicker(QWidget* parent)
    : QWidget(parent)
    , m_Impl(new ColorPickerPrivate())
{
    QHBoxLayout* layout = new QHBoxLayout;
    m_Impl->m_Hex       = new ColorHexEdit;
    layout->setContentsMargins(0, 0, 0, 0);

    m_Impl->m_Display = new ColorDisplay;

    layout->addWidget(m_Impl->m_Display);
    layout->addWidget(m_Impl->m_Hex);

    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setStyleHint(QFont::TypeWriter);
    font.setWeight(QFont::ExtraBold);
    font.setStyleStrategy(QFont::ForceOutline);
    setFont(font);

    auto on_display_clicked = [this]()
    {
        if (!m_Impl->m_Popup)
        {
            m_Impl->m_Popup = new ColorPickerPopup;
            m_Impl->m_Popup->setMinimumSize(185, 290);
            m_Impl->m_Popup->setMaximumSize(185, 290);
            m_Impl->m_Popup->setDisplayAlpha(m_Impl->m_DisplayAlpha);
            m_Impl->m_Popup->setEditType(m_Impl->m_EditType);
            m_Impl->m_Popup->setFont(this->font());
            m_Impl->m_Popup->setColor(m_Impl->m_Color);

            connect(m_Impl->m_Popup, &ColorPickerPopup::colorChanged, this, &ColorPicker::updateColor);
            connect(m_Impl->m_Popup, &ColorPickerPopup::colorChanged, this, &ColorPicker::colorChanged);
            connect(m_Impl->m_Popup, &ColorPickerPopup::colorChanging, this, &ColorPicker::updateColor);
            connect(m_Impl->m_Popup, &ColorPickerPopup::colorChanging, this, &ColorPicker::colorChanging);
        }

        m_Impl->m_Popup->move(mapToGlobal(rect().topLeft()));
        m_Impl->m_Popup->show();
    };

    connect(m_Impl->m_Display, &ColorDisplay::clicked, this, on_display_clicked);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::updateColor);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::colorChanged);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::updateColor);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::colorChanging);

    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPicker::updateColor);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPicker::colorChanged);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPicker::updateColor);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPicker::colorChanging);

    // set default color and sync child widgets
    setColor(QColor(255, 255, 255, 255));
}

ColorPicker::~ColorPicker()
{
    if (m_Impl->m_Popup)
    {
        delete m_Impl->m_Popup;
    }

    delete m_Impl;
}

void ColorPicker::updateColor(const QColor& color)
{
    m_Impl->m_Hex->updateColor(color);
    m_Impl->m_Display->updateColor(color);
    if (m_Impl->m_Popup)
        m_Impl->m_Popup->updateColor(color);

    if (m_Impl->m_Color != color)
    {
        m_Impl->m_Color = color;
        update();
    }
}

void ColorPicker::setColor(const QColor& color)
{
    if (m_Impl->m_Color != color)
    {
        updateColor(color);
        Q_EMIT colorChanged(m_Impl->m_Color);
    }
}

void ColorPicker::setDisplayAlpha(bool visible)
{
    m_Impl->m_DisplayAlpha = visible;
    if (m_Impl->m_Popup)
    {
        m_Impl->m_Popup->setDisplayAlpha(visible);
    }

    m_Impl->m_Hex->setDisplayAlpha(visible);
}

bool ColorPicker::displayAlpha()
{
    return m_Impl->m_DisplayAlpha;
}

void ColorPicker::setEditType(ColorPicker::EditType type)
{
    m_Impl->m_EditType = type;
    if (m_Impl->m_Popup)
    {
        m_Impl->m_Popup->setEditType(type);
    }
}

ColorPicker::EditType ColorPicker::editType()
{
    return m_Impl->m_EditType;
}
