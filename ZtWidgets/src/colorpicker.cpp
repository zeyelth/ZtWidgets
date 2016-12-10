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

#include <ZtWidgets/colorpicker.h>
#include <ZtWidgets/slideredit.h>
#include "colorhexedit_p.h"
#include "colordisplay_p.h"
#include "huesaturationwheel_p.h"
#include "colorpickerpopup_p.h"

#include <QHBoxLayout>
#include <QFontDatabase>

//! @cond Doxygen_Suppress

class ColorPickerPrivate
{
    Q_DISABLE_COPY(ColorPickerPrivate)
    Q_DECLARE_PUBLIC(ColorPicker)

private:
    explicit ColorPickerPrivate(ColorPicker*);

    ColorPicker* const q_ptr;

    ColorHexEdit* m_Hex;
    ColorDisplay* m_Display;
    ColorPickerPopup* m_Popup;
    QColor m_Color;
    ColorPicker::EditType m_EditType;
    bool m_DisplayAlpha : 1;
};

ColorPickerPrivate::ColorPickerPrivate(ColorPicker* colorpicker)
    : q_ptr(colorpicker)
    , m_Hex(nullptr)
    , m_Display(nullptr)
    , m_Popup(nullptr)
    , m_Color(Qt::white)
    , m_EditType(ColorPicker::Float)
    , m_DisplayAlpha(true)
{}

//! @endcond

ColorPicker::ColorPicker(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ColorPickerPrivate(this))
{
    Q_D(ColorPicker);
    QHBoxLayout* layout = new QHBoxLayout;
    d->m_Hex = new ColorHexEdit;
    layout->setContentsMargins(0, 0, 0, 0);

    d->m_Display = new ColorDisplay;

    layout->addWidget(d->m_Display);
    layout->addWidget(d->m_Hex);

    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setStyleHint(QFont::TypeWriter);
    font.setWeight(QFont::ExtraBold);
    font.setStyleStrategy(QFont::ForceOutline);
    setFont(font);

    auto on_display_clicked = [this]()
    {
        Q_D(ColorPicker);
        if(!d->m_Popup)
        {
            d->m_Popup = new ColorPickerPopup;
            d->m_Popup->setMinimumSize(185, 290);
            d->m_Popup->setMaximumSize(185, 290);
            d->m_Popup->setDisplayAlpha(d->m_DisplayAlpha);
            d->m_Popup->setEditType(d->m_EditType);
            d->m_Popup->setFont(this->font());
            d->m_Popup->setColor(d->m_Color);

            connect(d->m_Popup, &ColorPickerPopup::colorChanged, this, &ColorPicker::updateColor);
            connect(d->m_Popup, &ColorPickerPopup::colorChanged, this, &ColorPicker::colorChanged);
            connect(d->m_Popup, &ColorPickerPopup::colorChanging, this, &ColorPicker::updateColor);
            connect(d->m_Popup, &ColorPickerPopup::colorChanging, this, &ColorPicker::colorChanging);
        }

        d->m_Popup->move(mapToGlobal(rect().topLeft()));
        d->m_Popup->show();
    };

    connect(d->m_Display, &ColorDisplay::clicked, this, on_display_clicked);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::colorChanged);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::colorChanging);

    connect(d->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPicker::updateColor);
    connect(d->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPicker::colorChanged);
    connect(d->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPicker::updateColor);
    connect(d->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPicker::colorChanging);

    // set default color and sync child widgets
    setColor(QColor(255, 255, 255, 255));
}

ColorPicker::~ColorPicker()
{
    Q_D(ColorPicker);
    if(d->m_Popup)
    {
        delete d->m_Popup;
    }

    delete d_ptr;
}

void ColorPicker::updateColor(const QColor& color)
{
    Q_D(ColorPicker);

    d->m_Hex->updateColor(color);
    d->m_Display->updateColor(color);
    if(d->m_Popup)
        d->m_Popup->updateColor(color);

    if (d->m_Color != color)
    {
        d->m_Color = color;
        update();
    }
}

void ColorPicker::setColor(const QColor& color)
{
    Q_D(ColorPicker);
    if (d->m_Color != color)
    {
        updateColor(color);
        emit colorChanged(d->m_Color);
    }
}

void ColorPicker::setDisplayAlpha(bool visible)
{
    Q_D(ColorPicker);
    d->m_DisplayAlpha = visible;
    if(d->m_Popup)
    {
        d->m_Popup->setDisplayAlpha(visible);
    }

    d->m_Hex->setDisplayAlpha(visible);
}

bool ColorPicker::displayAlpha()
{
    Q_D(ColorPicker);
    return d->m_DisplayAlpha;
}

void ColorPicker::setEditType(ColorPicker::EditType type)
{
    Q_D(ColorPicker);
    d->m_EditType = type;
    if(d->m_Popup)
    {
        d->m_Popup->setEditType(type);
    }
}

ColorPicker::EditType ColorPicker::editType()
{
    Q_D(ColorPicker);
    return d->m_EditType;
}
