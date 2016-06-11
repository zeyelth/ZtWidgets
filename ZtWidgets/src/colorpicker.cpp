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


CColorPicker::CPopup::CPopup(QWidget *parent)
    : CColorWidgetBase(parent)
{
    setWindowFlags(Qt::Popup);
    QVBoxLayout* main_layout = new QVBoxLayout;
    m_frame = new QFrame;
    m_frame->setFrameStyle(QFrame::Panel);
    m_frame->setFrameShadow(QFrame::Raised);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_frame);
    setLayout(main_layout);

    QVBoxLayout* layout = new QVBoxLayout;
    m_hex = new CColorHexEdit;

    m_display = new CColorDisplay;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->addWidget(m_display);
    top_layout->addWidget(m_hex);
    top_layout->addStretch(100);

    m_wheel = new CHueSaturationWheel;
    QSizePolicy size_policy;
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_wheel->setSizePolicy(size_policy);

    int size = 15;

    m_valueSlider = new CVerticalColorComponentSlider(CAbstractColorComponentSlider::Component::Value, size, Qt::black, Qt::white);

    m_redSlider = new CHorizontalColorComponentSlider(CAbstractColorComponentSlider::Component::Red, size, Qt::black, Qt::red);
    m_greenSlider = new CHorizontalColorComponentSlider(CAbstractColorComponentSlider::Component::Green, size, Qt::black, Qt::green);
    m_blueSlider = new CHorizontalColorComponentSlider(CAbstractColorComponentSlider::Component::Blue, size, Qt::black, Qt::blue);
    m_alphaSlider = new CHorizontalColorComponentSlider(CAbstractColorComponentSlider::Component::Alpha, size, QColor(255, 255, 255, 0), QColor(255, 255, 255, 255));

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(m_wheel);
    mid_layout->addWidget(m_valueSlider);
    mid_layout->setContentsMargins(5, 0, 5, 0);

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);

    bool abbreviate = true;

    auto labelfunc = [&](CAbstractColorComponentSlider::Component component)
    {
        QLabel* label = new QLabel(CAbstractColorComponentSlider::componentName(component, abbreviate));
        label->setFont(font);
        label->setMaximumHeight(size);
        label->setMinimumWidth(size);
        label->setMaximumWidth(size);
        return label;
    };

    QVBoxLayout* bottom_layout = new QVBoxLayout;
    bottom_layout->setSpacing(2);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(CAbstractColorComponentSlider::Component::Red));
    slayout->addWidget(m_redSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(CAbstractColorComponentSlider::Component::Green));
    slayout->addWidget(m_greenSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(CAbstractColorComponentSlider::Component::Blue));
    slayout->addWidget(m_blueSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(labelfunc(CAbstractColorComponentSlider::Component::Alpha));
    slayout->addWidget(m_alphaSlider);
    bottom_layout->addLayout(slayout);

    layout->addLayout(top_layout);
    layout->addLayout(mid_layout);
    layout->addLayout(bottom_layout);

    layout->setContentsMargins(2, 2, 2, 2);
    m_frame->setLayout(layout);

    connect(m_display, &CColorDisplay::clicked, this, &CPopup::hide);

    auto connectfunc = [this](CColorWidgetBase* w)
    {
        connect(w, &CColorWidgetBase::colorChanged, this, &CPopup::updateColor);
        connect(w, &CColorWidgetBase::colorChanged, this, &CPopup::colorChanged);
        connect(w, &CColorWidgetBase::colorChanging, this, &CPopup::updateColor);
        connect(w, &CColorWidgetBase::colorChanging, this, &CPopup::colorChanging);
    };

    connectfunc(m_wheel);
    connectfunc(m_hex);
    connectfunc(m_display);
    connectfunc(m_valueSlider);
    connectfunc(m_redSlider);
    connectfunc(m_greenSlider);
    connectfunc(m_blueSlider);
    connectfunc(m_alphaSlider);

    // sync color of all child widgets
    m_wheel->setColor(m_Color);

    QWidget::setTabOrder(m_hex, m_redSlider);
    QWidget::setTabOrder(m_redSlider, m_greenSlider);
    QWidget::setTabOrder(m_greenSlider, m_blueSlider);
    QWidget::setTabOrder(m_blueSlider, m_alphaSlider);
    QWidget::setTabOrder(m_alphaSlider, m_hex);
}

void CColorPicker::CPopup::updateColor(const QColor& color)
{
    auto forward = [&](CColorWidgetBase* w)
    {
        if (w != sender())
        {
            w->updateColor(color);
        }
    };

    forward(m_wheel);
    forward(m_hex);
    forward(m_display);
    forward(m_valueSlider);
    forward(m_redSlider);
    forward(m_greenSlider);
    forward(m_blueSlider);
    forward(m_alphaSlider);

    CColorWidgetBase::updateColor(color);
}

void CColorPicker::CPopup::showEvent(QShowEvent* event)
{
    QPoint p(pos());
    int fw = m_frame->lineWidth();
    QMargins margins = m_frame->layout()->contentsMargins();
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

    CColorWidgetBase::showEvent(event);
}

CColorPicker::CColorPicker(QWidget *parent)
    : CColorWidgetBase(parent)
{
    QHBoxLayout* layout = new QHBoxLayout;
    m_hex = new CColorHexEdit;
    layout->setContentsMargins(0, 0, 0, 0);

    m_display = new CColorDisplay;

    layout->addWidget(m_display);
    layout->addWidget(m_hex);

    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_popup = new CPopup(this);
    m_popup->setMinimumSize(185, 290);
    m_popup->setMaximumSize(185, 290);

    connect(m_display, &CColorDisplay::clicked, this, &CColorPicker::onDisplayClicked);

    auto connectfunc = [this](CColorWidgetBase* w)
    {
        connect(w, &CColorWidgetBase::colorChanged, this, &CColorPicker::updateColor);
        connect(w, &CColorWidgetBase::colorChanged, this, &CColorPicker::colorChanged);
        connect(w, &CColorWidgetBase::colorChanging, this, &CColorPicker::updateColor);
        connect(w, &CColorWidgetBase::colorChanging, this, &CColorPicker::colorChanging);
    };

    connectfunc(m_hex);
    connectfunc(m_display);
    connectfunc(m_popup);

    // set default color and sync child widgets
    setColor(QColor(255, 255, 255, 255));
}

void CColorPicker::updateColor(const QColor& color)
{
    auto forward = [&](CColorWidgetBase* w)
    {
        if (w != sender())
        {
            w->updateColor(color);
        }
    };

    forward(m_hex);
    forward(m_display);
    forward(m_popup);

    CColorWidgetBase::updateColor(color);
}


void CColorPicker::onDisplayClicked()
{
    m_popup->move(mapToGlobal(rect().topLeft()));
    m_popup->show();
}
