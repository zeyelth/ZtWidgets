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

#include "colorpickerpopup_p.h"

#include "colorhexedit_p.h"
#include "colordisplay_p.h"
#include "huesaturationwheel_p.h"

#include "color_utils_p.h"

#include <ZtWidgets/colorpicker.h>
#include <ZtWidgets/slideredit.h>

#include <QtMath>
#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QPainter>
#include <QResizeEvent>
#include <QLabel>
#include <QVBoxLayout>

enum class ColorChannel
{
    Red,
    Blue,
    Green,
    Alpha,
    HsvValue
};

static bool isBright(const QColor& c)
{
    return qSqrt(qPow(c.redF(), 2) * 0.299f +
                 qPow(c.greenF(), 2) * 0.587f +
                 qPow(c.blueF(), 2) * 0.114f ) > 0.6f;
}

static void valueToColor(QColor& color, ColorPicker::EditType t, ColorChannel channel, qreal val)
{
    switch(channel)
    {
        case ColorChannel::Red:
            t == ColorPicker::Float ? color.setRedF(val) : color.setRed(qRound(val));
            break;
        case ColorChannel::Green:
            t == ColorPicker::Float ? color.setGreenF(val) : color.setGreen(qRound(val));
            break;
        case ColorChannel::Blue:
            t == ColorPicker::Float ? color.setBlueF(val) : color.setBlue(qRound(val));
            break;
        case ColorChannel::Alpha:
            t == ColorPicker::Float ? color.setAlphaF(val) : color.setAlpha(qRound(val));
            break;
        case ColorChannel::HsvValue:
            t == ColorPicker::Float ? color.setHsvF(color.hsvHueF(), color.hsvSaturationF(), val, color.alphaF()) : color.setHsv(color.hsvHue(), color.hsvSaturation(), qRound(val), color.alpha());
            break;
    }
}

//! @cond Doxygen_Suppress
class ColorSliderEdit : public SliderEdit
{
public:
    explicit ColorSliderEdit(const QColor& c0, const QColor& c1)
        : SliderEdit()
        , m_Color0(c0)
        , m_Color1(c1)
        , m_TextColor(isBright(c1) ? Qt::black : Qt::white)
    {
        setSliderComponents(SliderEdit::SliderComponent::Marker | SliderEdit::SliderComponent::Text);
        setAlignment(Qt::AlignRight);
    }

protected:
    void resizeEvent(QResizeEvent* event)
    {
        QPalette p = palette();

        QImage img(event->size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(0);

        QRect r = img.rect();
        QPainter painter(&img);

        if (m_Color0.alpha() < 255 || m_Color1.alpha() < 255)
        {
            quint32 size = qMin(r.width(), r.height()) / 2;
            drawCheckerboard(painter, r, size);
        }

        QLinearGradient g;
        if(orientation() == Qt::Horizontal)
            g = QLinearGradient(QPointF(0.0f, 0.0f), QPointF((float)event->size().width(), 0.0f));
        else
            g = QLinearGradient(QPointF(0.0f, 0.0f), QPointF(0.0f, (float)event->size().height()));
        g.setColorAt(0, m_Color0);
        g.setColorAt(1, m_Color1);

        painter.fillRect(r, g);

        p.setBrush(QPalette::Base, img);
        p.setBrush(QPalette::Text, m_TextColor);
        setPalette(p);
    }
private:
    const QColor m_Color0;
    const QColor m_Color1;
    const QColor m_TextColor;
};

class ColorPickerPopupPrivate
{
    Q_DISABLE_COPY(ColorPickerPopupPrivate)
    Q_DECLARE_PUBLIC(ColorPickerPopup)

private:
    explicit ColorPickerPopupPrivate(ColorPickerPopup*);

    ColorPickerPopup* const q_ptr;

    QFrame* m_Frame;
    ColorHexEdit* m_Hex;
    ColorDisplay* m_Display;
    HueSaturationWheel* m_Wheel;
    ColorSliderEdit* m_ValueSlider;
    ColorSliderEdit* m_RedSlider;
    ColorSliderEdit* m_GreenSlider;
    ColorSliderEdit* m_BlueSlider;
    ColorSliderEdit* m_AlphaSlider;
    QLabel* m_AlphaLabel;
    QColor m_Color;
    ColorPicker::EditType m_EditType;
};

ColorPickerPopupPrivate::ColorPickerPopupPrivate(ColorPickerPopup* popup)
    : q_ptr(popup)
    , m_Frame(nullptr)
    , m_Hex(nullptr)
    , m_Display(nullptr)
    , m_Wheel(nullptr)
    , m_ValueSlider(nullptr)
    , m_RedSlider(nullptr)
    , m_GreenSlider(nullptr)
    , m_BlueSlider(nullptr)
    , m_AlphaSlider(nullptr)
    , m_AlphaLabel(nullptr)
    , m_Color(Qt::white)
    , m_EditType(ColorPicker::EditType::Float)
{}
//! @endcond

ColorPickerPopup::ColorPickerPopup(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ColorPickerPopupPrivate(this))
{
    Q_D(ColorPickerPopup);
    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    QVBoxLayout* main_layout = new QVBoxLayout;
    d->m_Frame = new QFrame;
    d->m_Frame->setFrameStyle(QFrame::Panel);
    d->m_Frame->setFrameShadow(QFrame::Raised);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(d->m_Frame);
    setLayout(main_layout);

    QVBoxLayout* layout = new QVBoxLayout;
    d->m_Hex = new ColorHexEdit;

    d->m_Display = new ColorDisplay;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->addWidget(d->m_Display);
    top_layout->addWidget(d->m_Hex);
    top_layout->addStretch(100);

    d->m_Wheel = new HueSaturationWheel;
    QSizePolicy size_policy;
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    d->m_Wheel->setSizePolicy(size_policy);

    d->m_ValueSlider = new ColorSliderEdit(Qt::white, Qt::black);
    d->m_ValueSlider->setOrientation(Qt::Vertical);

    QPalette p = palette();
    p.setBrush(QPalette::Text, Qt::white);
    d->m_RedSlider = new ColorSliderEdit(Qt::black, Qt::red);
    d->m_GreenSlider = new ColorSliderEdit(Qt::black, Qt::green);
    d->m_BlueSlider = new ColorSliderEdit(Qt::black, Qt::blue);
    d->m_AlphaSlider = new ColorSliderEdit(QColor(255, 255, 255, 0), Qt::white);

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(d->m_Wheel);
    mid_layout->addWidget(d->m_ValueSlider);
    mid_layout->setContentsMargins(5, 0, 5, 0);

    QVBoxLayout* bottom_layout = new QVBoxLayout;
    bottom_layout->setSpacing(0);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("R"));
    slayout->addWidget(d->m_RedSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("G"));
    slayout->addWidget(d->m_GreenSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("B"));
    slayout->addWidget(d->m_BlueSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    d->m_AlphaLabel = new QLabel("A");
    slayout->addWidget(d->m_AlphaLabel);
    slayout->addWidget(d->m_AlphaSlider);
    bottom_layout->addLayout(slayout);

    layout->addLayout(top_layout);
    layout->addLayout(mid_layout);
    layout->addLayout(bottom_layout);

    layout->setContentsMargins(2, 2, 2, 2);
    d->m_Frame->setLayout(layout);

    connect(d->m_Wheel, &HueSaturationWheel::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(d->m_Wheel, &HueSaturationWheel::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(d->m_Wheel, &HueSaturationWheel::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(d->m_Wheel, &HueSaturationWheel::colorChanging, this, &ColorPickerPopup::colorChanging);

    connect(d->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(d->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(d->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(d->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPickerPopup::colorChanging);

    connect(d->m_Display, &ColorDisplay::clicked, this, &ColorPickerPopup::hide);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPickerPopup::colorChanging);

    auto svchanging = [this](qreal val, ColorChannel channel)
    {
        Q_D(ColorPickerPopup);
        valueToColor(d->m_Color, d->m_EditType, channel, val);

        updateColor(d->m_Color);
        emit colorChanging(d->m_Color);
    };

    auto svchanged = [this](qreal val, ColorChannel channel)
    {
        Q_D(ColorPickerPopup);
        valueToColor(d->m_Color, d->m_EditType, channel, val);

        updateColor(d->m_Color);
        emit colorChanged(d->m_Color);
    };

    connect(d->m_ValueSlider, &SliderEdit::valueChanging, [this, svchanging](qreal val) { svchanging(val, ColorChannel::HsvValue); });
    connect(d->m_ValueSlider, &SliderEdit::valueChanged, [this, svchanged](qreal val) { svchanged(val, ColorChannel::HsvValue); });

    connect(d->m_RedSlider, &SliderEdit::valueChanging, [this, svchanging](qreal val) { svchanging(val, ColorChannel::Red); });
    connect(d->m_RedSlider, &SliderEdit::valueChanged, [this, svchanged](qreal val) { svchanged(val, ColorChannel::Red); });

    connect(d->m_GreenSlider, &SliderEdit::valueChanging, [this, svchanging](qreal val) { svchanging(val, ColorChannel::Green); });
    connect(d->m_GreenSlider, &SliderEdit::valueChanged, [this, svchanged](qreal val) { svchanged(val, ColorChannel::Green); });

    connect(d->m_BlueSlider, &SliderEdit::valueChanging, [this, svchanging](qreal val) { svchanging(val, ColorChannel::Blue); });
    connect(d->m_BlueSlider, &SliderEdit::valueChanged, [this, svchanged](qreal val) { svchanged(val, ColorChannel::Blue); });

    connect(d->m_AlphaSlider, &SliderEdit::valueChanging, [this, svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(d->m_AlphaSlider, &SliderEdit::valueChanged, [this, svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    // sync color of all child widgets
    d->m_Wheel->setColor(d->m_Color);

    QWidget::setTabOrder(d->m_Hex, d->m_RedSlider);
    QWidget::setTabOrder(d->m_RedSlider, d->m_GreenSlider);
    QWidget::setTabOrder(d->m_GreenSlider, d->m_BlueSlider);
    QWidget::setTabOrder(d->m_BlueSlider, d->m_AlphaSlider);
    QWidget::setTabOrder(d->m_AlphaSlider, d->m_Hex);
}

ColorPickerPopup::~ColorPickerPopup()
{
    delete d_ptr;
}

void ColorPickerPopup::updateColor(const QColor& color)
{
    Q_D(ColorPickerPopup);
    d->m_Wheel->updateColor(color);
    d->m_Hex->updateColor(color);
    d->m_Display->updateColor(color);

    if(d->m_EditType == ColorPicker::Float)
    {
        d->m_RedSlider->updateValue(color.redF());
        d->m_GreenSlider->updateValue(color.greenF());
        d->m_BlueSlider->updateValue(color.blueF());
        d->m_AlphaSlider->updateValue(color.alphaF());
        d->m_ValueSlider->updateValue(color.valueF());
    }
    else
    {
        d->m_RedSlider->updateValue(color.red());
        d->m_GreenSlider->updateValue(color.green());
        d->m_BlueSlider->updateValue(color.blue());
        d->m_AlphaSlider->updateValue(color.alpha());
        d->m_ValueSlider->updateValue(color.value());
    }

    if(d->m_Color != color)
    {
        d->m_Color = color;
        update();
    }
}

void ColorPickerPopup::setColor(const QColor& color)
{
    Q_D(ColorPickerPopup);
    if (d->m_Color != color)
    {
        updateColor(color);
        emit colorChanged(d->m_Color);
    }
}

void ColorPickerPopup::showEvent(QShowEvent* event)
{
    Q_D(ColorPickerPopup);
    QPoint p(pos());
    int fw = d->m_Frame->lineWidth();
    QMargins margins = d->m_Frame->layout()->contentsMargins();
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

    QWidget::showEvent(event);
    activateWindow();
}

void ColorPickerPopup::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ActivationChange && !isActiveWindow())
    {
        hide();
    }
    QWidget::changeEvent(event);
}

bool ColorPickerPopup::displayAlpha() const
{
    Q_D(const ColorPickerPopup);
    return d->m_Hex->displayAlpha();
}

void ColorPickerPopup::setDisplayAlpha(bool visible)
{
    Q_D(ColorPickerPopup);
    d->m_AlphaLabel->setVisible(visible);
    d->m_AlphaSlider->setVisible(visible);
    d->m_Hex->setDisplayAlpha(visible);
}

ColorPicker::EditType ColorPickerPopup::editType() const
{
    Q_D(const ColorPickerPopup);
    return d->m_EditType;
}

void ColorPickerPopup::setEditType(ColorPicker::EditType type)
{
    Q_D(ColorPickerPopup);
    d->m_EditType = type;

    auto update_slider = [&](SliderEdit* w)
    {
        switch(type)
        {
            case ColorPicker::EditType::Int:
                w->setRange(0, 255);
                w->setPrecision(0);
                break;
            case ColorPicker::EditType::Float:
                w->setRange(0.0f, 1.0f);
                w->setPrecision(3);
                break;
            default:
                Q_ASSERT("Unhandled edit type!");
                break;
        }
    };

    update_slider(d->m_RedSlider);
    update_slider(d->m_GreenSlider);
    update_slider(d->m_BlueSlider);
    update_slider(d->m_AlphaSlider);
    update_slider(d->m_ValueSlider);
}
