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
#include <ZtWidgets/horizontalcolorcomponentslider.h>
#include <ZtWidgets/verticalcolorcomponentslider.h>
#include <ZtWidgets/colorhexedit.h>
#include <ZtWidgets/colordisplay.h>
#include <ZtWidgets/huesaturationwheel.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>


ColorPicker::Popup::Popup(QWidget *parent)
    : ColorWidgetBase(parent)
{
    setWindowFlags(Qt::Popup);
    QVBoxLayout* main_layout = new QVBoxLayout;
    m_Frame = new QFrame;
    m_Frame->setFrameStyle(QFrame::Panel);
    m_Frame->setFrameShadow(QFrame::Raised);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_Frame);
    setLayout(main_layout);

    QVBoxLayout* layout = new QVBoxLayout;
    m_Hex = new ColorHexEdit;

    m_Display = new ColorDisplay;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->addWidget(m_Display);
    top_layout->addWidget(m_Hex);
    top_layout->addStretch(100);

    m_Wheel = new HueSaturationWheel;
    QSizePolicy size_policy;
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_Wheel->setSizePolicy(size_policy);

    int size = 15;

    m_ValueSlider = new VerticalColorComponentSlider(AbstractColorComponentSlider::Component::Value, size, Qt::black, Qt::white);

    m_RedSlider = new HorizontalColorComponentSlider(AbstractColorComponentSlider::Component::Red, size, Qt::black, Qt::red);
    m_GreenSlider = new HorizontalColorComponentSlider(AbstractColorComponentSlider::Component::Green, size, Qt::black, Qt::green);
    m_BlueSlider = new HorizontalColorComponentSlider(AbstractColorComponentSlider::Component::Blue, size, Qt::black, Qt::blue);
    m_AlphaSlider = new HorizontalColorComponentSlider(AbstractColorComponentSlider::Component::Alpha, size, QColor(255, 255, 255, 0), QColor(255, 255, 255, 255));

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(m_Wheel);
    mid_layout->addWidget(m_ValueSlider);
    mid_layout->setContentsMargins(5, 0, 5, 0);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);

    bool abbreviate = true;

    auto labelfunc = [&](AbstractColorComponentSlider::Component component)
    {
        QLabel* label = new QLabel(AbstractColorComponentSlider::componentName(component, abbreviate));
        label->setFont(font);
        label->setMaximumHeight(size);
        label->setMinimumWidth(size);
        label->setMaximumWidth(size);
        return label;
    };

    QVBoxLayout* bottom_layout = new QVBoxLayout;
    bottom_layout->setSpacing(2);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(AbstractColorComponentSlider::Component::Red));
    slayout->addWidget(m_RedSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(AbstractColorComponentSlider::Component::Green));
    slayout->addWidget(m_GreenSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(AbstractColorComponentSlider::Component::Blue));
    slayout->addWidget(m_BlueSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    m_AlphaLabel = labelfunc(AbstractColorComponentSlider::Component::Alpha);
    slayout->addWidget(m_AlphaLabel);
    slayout->addWidget(m_AlphaSlider);
    bottom_layout->addLayout(slayout);

    layout->addLayout(top_layout);
    layout->addLayout(mid_layout);
    layout->addLayout(bottom_layout);

    layout->setContentsMargins(2, 2, 2, 2);
    m_Frame->setLayout(layout);

    connect(m_Display, &ColorDisplay::clicked, this, &Popup::hide);

    auto connectfunc = [this](ColorWidgetBase* w)
    {
        connect(w, &ColorWidgetBase::colorChanged, this, &Popup::updateColor);
        connect(w, &ColorWidgetBase::colorChanged, this, &Popup::colorChanged);
        connect(w, &ColorWidgetBase::colorChanging, this, &Popup::updateColor);
        connect(w, &ColorWidgetBase::colorChanging, this, &Popup::colorChanging);
    };

    connectfunc(m_Wheel);
    connectfunc(m_Hex);
    connectfunc(m_Display);
    connectfunc(m_ValueSlider);
    connectfunc(m_RedSlider);
    connectfunc(m_GreenSlider);
    connectfunc(m_BlueSlider);
    connectfunc(m_AlphaSlider);

    // sync color of all child widgets
    m_Wheel->setColor(m_Color);

    QWidget::setTabOrder(m_Hex, m_RedSlider);
    QWidget::setTabOrder(m_RedSlider, m_GreenSlider);
    QWidget::setTabOrder(m_GreenSlider, m_BlueSlider);
    QWidget::setTabOrder(m_BlueSlider, m_AlphaSlider);
    QWidget::setTabOrder(m_AlphaSlider, m_Hex);
}

void ColorPicker::Popup::updateColor(const QColor& color)
{
    auto forward = [&](ColorWidgetBase* w)
    {
        if (w != sender())
        {
            w->updateColor(color);
        }
    };

    forward(m_Wheel);
    forward(m_Hex);
    forward(m_Display);
    forward(m_ValueSlider);
    forward(m_RedSlider);
    forward(m_GreenSlider);
    forward(m_BlueSlider);
    forward(m_AlphaSlider);

    ColorWidgetBase::updateColor(color);
}

void ColorPicker::Popup::showEvent(QShowEvent* event)
{
    QPoint p(pos());
    int fw = m_Frame->lineWidth();
    QMargins margins = m_Frame->layout()->contentsMargins();
    p.setX(p.x() - margins.left() - fw);
    p.setY(p.y() - margins.top() -fw);
    QRect r = qApp->desktop()->screenGeometry(this);

    if (p.x() + width() > r.x() + r.width())
    {
        p.setX(r.x() + r.width() - width());
    }
    if (p.y() + height() > r.y() + r.height())
    {
        p.setY(r.y() + r.height() - height());
    }

    move(p);

    ColorWidgetBase::showEvent(event);
}

void ColorPicker::Popup::setDisplayAlpha(bool visible)
{
    m_AlphaLabel->setVisible(visible);
    m_AlphaSlider->setVisible(visible);
    m_Hex->setDisplayAlpha(visible);
}

bool ColorPicker::Popup::displayAlpha()
{
    return m_Hex->displayAlpha();
}

void ColorPicker::Popup::setEditType(HorizontalColorComponentSlider::EditType editType)
{
    m_RedSlider->setEditType(editType);
    m_GreenSlider->setEditType(editType);
    m_BlueSlider->setEditType(editType);
}

HorizontalColorComponentSlider::EditType ColorPicker::Popup::editType()
{
    return m_RedSlider->editType();
}

ColorPicker::ColorPicker(QWidget *parent)
    : ColorWidgetBase(parent)
{
    QHBoxLayout* layout = new QHBoxLayout;
    m_Hex = new ColorHexEdit;
    layout->setContentsMargins(0, 0, 0, 0);

    m_Display = new ColorDisplay;

    layout->addWidget(m_Display);
    layout->addWidget(m_Hex);

    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_Popup = new Popup(this);
    m_Popup->setMinimumSize(185, 290);
    m_Popup->setMaximumSize(185, 290);

    connect(m_Display, &ColorDisplay::clicked, this, &ColorPicker::onDisplayClicked);

    auto connectfunc = [this](ColorWidgetBase* w)
    {
        connect(w, &ColorWidgetBase::colorChanged, this, &ColorPicker::updateColor);
        connect(w, &ColorWidgetBase::colorChanged, this, &ColorPicker::colorChanged);
        connect(w, &ColorWidgetBase::colorChanging, this, &ColorPicker::updateColor);
        connect(w, &ColorWidgetBase::colorChanging, this, &ColorPicker::colorChanging);
    };

    connectfunc(m_Hex);
    connectfunc(m_Display);
    connectfunc(m_Popup);

    // set default color and sync child widgets
    setColor(QColor(255, 255, 255, 255));
}

void ColorPicker::updateColor(const QColor& color)
{
    auto forward = [&](ColorWidgetBase* w)
    {
        if (w != sender())
        {
            w->updateColor(color);
        }
    };

    forward(m_Hex);
    forward(m_Display);
    forward(m_Popup);

    ColorWidgetBase::updateColor(color);
}

void ColorPicker::setDisplayAlpha(bool visible)
{
    m_Popup->setDisplayAlpha(visible);
    m_Hex->setDisplayAlpha(visible);
}

bool ColorPicker::displayAlpha()
{
    return m_Popup->displayAlpha();
}

void ColorPicker::setEditType(HorizontalColorComponentSlider::EditType type)
{
    m_Popup->setEditType(type);
}

HorizontalColorComponentSlider::EditType ColorPicker::editType()
{
    return m_Popup->editType();
}

void ColorPicker::onDisplayClicked()
{
    m_Popup->move(mapToGlobal(rect().topLeft()));
    m_Popup->show();
}
