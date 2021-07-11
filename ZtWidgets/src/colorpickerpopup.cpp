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

#include "colordisplay_p.h"
#include "colorhexedit_p.h"
#include "huesaturationwheel_p.h"

#include "color_utils_p.h"

#include <ZtWidgets/colorpicker.h>
#include <ZtWidgets/slideredit.h>

#include <QApplication>
#include <QButtonGroup>
#include <QDesktopWidget>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QtMath>

enum class ColorChannel
{
    Red,
    Blue,
    Green,
    Alpha,
    HslHue,
    HslSaturation,
    Lightness,
    HsvHue,
    HsvSaturation,
    Value,
};

static bool isBright(const QColor& c)
{
    return qSqrt(qPow(c.redF(), 2) * 0.299f + qPow(c.greenF(), 2) * 0.587f + qPow(c.blueF(), 2) * 0.114f) > 0.6f;
}

static void valueToColor(QColor& color, ColorPicker::EditType t, ColorChannel channel, qreal val)
{
    switch (channel)
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
        case ColorChannel::HsvHue:
            t == ColorPicker::Float ? color.setHsvF(val, color.hsvSaturationF(), color.valueF(), color.alphaF())
                                    : color.setHsv(qRound(val), color.hsvSaturation(), color.value(), color.alpha());
            break;
        case ColorChannel::HsvSaturation:
            t == ColorPicker::Float ? color.setHsvF(color.hsvHueF(), val, color.valueF(), color.alphaF())
                                    : color.setHsv(color.hsvHue(), qRound(val), color.value(), color.alpha());
            break;
        case ColorChannel::Value:
            t == ColorPicker::Float ? color.setHsvF(color.hsvHueF(), color.hsvSaturationF(), val, color.alphaF())
                                    : color.setHsv(color.hsvHue(), color.hsvSaturation(), qRound(val), color.alpha());
            break;
        case ColorChannel::HslHue:
            t == ColorPicker::Float
                ? color.setHslF(val, color.hslSaturationF(), color.lightnessF(), color.alphaF())
                : color.setHsl(qRound(val), color.hslSaturation(), color.lightness(), color.alpha());
            break;
        case ColorChannel::HslSaturation:
            t == ColorPicker::Float ? color.setHslF(color.hslHueF(), val, color.lightnessF(), color.alphaF())
                                    : color.setHsl(color.hslHue(), qRound(val), color.lightness(), color.alpha());
            break;
        case ColorChannel::Lightness:
            t == ColorPicker::Float ? color.setHslF(color.hslHueF(), color.hslSaturationF(), val, color.alphaF())
                                    : color.setHsl(color.hslHue(), color.hslSaturation(), qRound(val), color.alpha());
            break;
    }
}

//! @cond Doxygen_Suppress
class ColorSliderEdit : public SliderEdit
{
  public:
    explicit ColorSliderEdit(const QMap<qreal, QColor>& gradients)
        : SliderEdit()
        , m_Gradients(gradients)
        , m_TextColor(isBright(gradients.last()) ? Qt::black : Qt::white)
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

        quint32 size = qMin(r.width(), r.height()) / 2;
        drawCheckerboard(painter, r, size);

        QLinearGradient g;
        if (orientation() == Qt::Horizontal)
            g = QLinearGradient(QPointF(0.0f, 0.0f), QPointF((float)event->size().width(), 0.0f));
        else
            g = QLinearGradient(QPointF(0.0f, (float)event->size().height()), QPointF(0.0f, 0.0f));

        auto i = m_Gradients.begin();
        while (i != m_Gradients.end())
        {
            g.setColorAt(i.key(), i.value());
            ++i;
        }

        painter.fillRect(r, g);

        p.setBrush(QPalette::Base, img);
        p.setBrush(QPalette::Text, m_TextColor);
        setPalette(p);
    }

  private:
    const QMap<qreal, QColor> m_Gradients;
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
    QButtonGroup* m_ButtonGroup;
    QStackedWidget* m_SliderStack;
    ColorSliderEdit* m_ValueSlider;
    ColorSliderEdit* m_RedSlider;
    ColorSliderEdit* m_GreenSlider;
    ColorSliderEdit* m_BlueSlider;
    ColorSliderEdit* m_RgbAlphaSlider;
    ColorSliderEdit* m_HslHueSlider;
    ColorSliderEdit* m_HslSaturationSlider;
    ColorSliderEdit* m_LightnessSlider;
    ColorSliderEdit* m_HslAlphaSlider;
    ColorSliderEdit* m_HsvHueSlider;
    ColorSliderEdit* m_HsvSaturationSlider;
    ColorSliderEdit* m_HsvValueSlider;
    ColorSliderEdit* m_HsvAlphaSlider;
    QLabel* m_RgbAlphaLabel;
    QLabel* m_HslAlphaLabel;
    QLabel* m_HsvAlphaLabel;
    QColor m_Color;
    ColorPicker::EditType m_EditType;
};

ColorPickerPopupPrivate::ColorPickerPopupPrivate(ColorPickerPopup* popup)
    : q_ptr(popup)
    , m_Frame(nullptr)
    , m_Hex(nullptr)
    , m_Display(nullptr)
    , m_Wheel(nullptr)
    , m_ButtonGroup(nullptr)
    , m_SliderStack(nullptr)
    , m_ValueSlider(nullptr)
    , m_RedSlider(nullptr)
    , m_GreenSlider(nullptr)
    , m_BlueSlider(nullptr)
    , m_RgbAlphaSlider(nullptr)
    , m_HslHueSlider(nullptr)
    , m_HslSaturationSlider(nullptr)
    , m_LightnessSlider(nullptr)
    , m_HslAlphaSlider(nullptr)
    , m_HsvHueSlider(nullptr)
    , m_HsvSaturationSlider(nullptr)
    , m_HsvValueSlider(nullptr)
    , m_HsvAlphaSlider(nullptr)
    , m_RgbAlphaLabel(nullptr)
    , m_HslAlphaLabel(nullptr)
    , m_HsvAlphaLabel(nullptr)
    , m_Color(Qt::white)
    , m_EditType(ColorPicker::EditType::Float)
{}
//! @endcond

ColorPickerPopup::ColorPickerPopup(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new ColorPickerPopupPrivate(this))
{
    Q_D(ColorPickerPopup);
    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    QVBoxLayout* main_layout = new QVBoxLayout;
    d->m_Frame               = new QFrame;
    d->m_Frame->setFrameStyle(QFrame::Panel);
    d->m_Frame->setFrameShadow(QFrame::Raised);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(d->m_Frame);
    setLayout(main_layout);

    QVBoxLayout* layout = new QVBoxLayout;
    d->m_Hex            = new ColorHexEdit;

    d->m_Display = new ColorDisplay;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->addWidget(d->m_Display);
    top_layout->addWidget(d->m_Hex);
    top_layout->addStretch(100);

    d->m_Wheel = new HueSaturationWheel;
    d->m_Wheel->setToolTip(tr("Hue/Saturation"));
    QSizePolicy size_policy;
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    d->m_Wheel->setSizePolicy(size_policy);

    QMap<qreal, QColor> bw_gradient;
    bw_gradient[0] = Qt::black;
    bw_gradient[1] = Qt::white;
    QMap<qreal, QColor> red_gradient;
    red_gradient[0] = Qt::black;
    red_gradient[1] = Qt::red;
    QMap<qreal, QColor> green_gradient;
    green_gradient[0] = Qt::black;
    green_gradient[1] = Qt::green;
    QMap<qreal, QColor> blue_gradient;
    blue_gradient[0] = Qt::black;
    blue_gradient[1] = Qt::blue;
    QMap<qreal, QColor> alpha_gradient;
    alpha_gradient[0] = QColor(255, 255, 255, 0);
    alpha_gradient[1] = Qt::white;

    QMap<qreal, QColor> hue_gradient;
    QColor hue_color;

    qreal step = 0.0;
    while (step < 1.0)
    {
        hue_color.setHsvF(step, 1.0, 1.0);
        hue_gradient[step] = hue_color;
        step += 0.1;
    }

    blue_gradient[0] = Qt::black;
    blue_gradient[1] = Qt::blue;

    d->m_ValueSlider = new ColorSliderEdit(bw_gradient);
    d->m_ValueSlider->setToolTip(tr("Value"));
    d->m_ValueSlider->setOrientation(Qt::Vertical);

    QPalette p = palette();
    p.setBrush(QPalette::Text, Qt::white);

    d->m_RedSlider = new ColorSliderEdit(red_gradient);
    d->m_RedSlider->setToolTip(tr("Red"));
    d->m_GreenSlider = new ColorSliderEdit(green_gradient);
    d->m_GreenSlider->setToolTip(tr("Green"));
    d->m_BlueSlider = new ColorSliderEdit(blue_gradient);
    d->m_BlueSlider->setToolTip(tr("Blue"));
    d->m_RgbAlphaSlider = new ColorSliderEdit(alpha_gradient);
    d->m_RgbAlphaSlider->setToolTip(tr("Alpha"));

    d->m_HslHueSlider = new ColorSliderEdit(hue_gradient);
    d->m_HslHueSlider->setToolTip(tr("Hue"));
    d->m_HslSaturationSlider = new ColorSliderEdit(bw_gradient);
    d->m_HslSaturationSlider->setToolTip(tr("Saturation"));
    d->m_LightnessSlider = new ColorSliderEdit(bw_gradient);
    d->m_LightnessSlider->setToolTip(tr("Lightness"));
    d->m_HslAlphaSlider = new ColorSliderEdit(alpha_gradient);
    d->m_HslAlphaSlider->setToolTip(tr("Alpha"));

    d->m_HsvHueSlider = new ColorSliderEdit(hue_gradient);
    d->m_HsvHueSlider->setToolTip(tr("Hue"));
    d->m_HsvSaturationSlider = new ColorSliderEdit(bw_gradient);
    d->m_HsvSaturationSlider->setToolTip(tr("Saturation"));
    d->m_HsvValueSlider = new ColorSliderEdit(bw_gradient);
    d->m_HsvValueSlider->setToolTip(tr("Value"));
    d->m_HsvAlphaSlider = new ColorSliderEdit(alpha_gradient);
    d->m_HsvAlphaSlider->setToolTip(tr("Alpha"));

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(d->m_Wheel);
    mid_layout->addWidget(d->m_ValueSlider);
    mid_layout->setContentsMargins(5, 0, 5, 0);

    QPushButton* rgb_button = new QPushButton("&RGB");
    rgb_button->setMaximumHeight(20);
    rgb_button->setCheckable(true);
    rgb_button->setFlat(true);
    rgb_button->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_1));

    QPushButton* hsl_button = new QPushButton("HS&L");
    hsl_button->setMaximumHeight(20);
    hsl_button->setCheckable(true);
    hsl_button->setFlat(true);
    hsl_button->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_2));

    QPushButton* hsv_button = new QPushButton("HS&V");
    hsv_button->setMaximumHeight(20);
    hsv_button->setCheckable(true);
    hsv_button->setFlat(true);
    hsv_button->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_3));

    d->m_ButtonGroup = new QButtonGroup;
    d->m_ButtonGroup->addButton(rgb_button, 0);
    d->m_ButtonGroup->addButton(hsl_button, 1);
    d->m_ButtonGroup->addButton(hsv_button, 2);

    QHBoxLayout* stack_button_layout = new QHBoxLayout;
    stack_button_layout->setSpacing(0);
    stack_button_layout->addWidget(rgb_button);
    stack_button_layout->addWidget(hsl_button);
    stack_button_layout->addWidget(hsv_button);

    d->m_SliderStack = new QStackedWidget;
    d->m_SliderStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QWidget* rgb_widget     = new QWidget;
    QVBoxLayout* rgb_layout = new QVBoxLayout;
    rgb_widget->setLayout(rgb_layout);
    rgb_layout->setSpacing(0);
    rgb_layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("R"));
    slayout->addWidget(d->m_RedSlider);
    rgb_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("G"));
    slayout->addWidget(d->m_GreenSlider);
    rgb_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("B"));
    slayout->addWidget(d->m_BlueSlider);
    rgb_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    d->m_RgbAlphaLabel = new QLabel("A");
    slayout->addWidget(d->m_RgbAlphaLabel);
    slayout->addWidget(d->m_RgbAlphaSlider);
    rgb_layout->addLayout(slayout);

    QWidget* hsl_widget     = new QWidget;
    QVBoxLayout* hsl_layout = new QVBoxLayout;
    hsl_widget->setLayout(hsl_layout);
    hsl_layout->setSpacing(0);
    hsl_layout->setContentsMargins(0, 0, 0, 0);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("H"));
    slayout->addWidget(d->m_HslHueSlider);
    hsl_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("S"));
    slayout->addWidget(d->m_HslSaturationSlider);
    hsl_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("L"));
    slayout->addWidget(d->m_LightnessSlider);
    hsl_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    d->m_HslAlphaLabel = new QLabel("A");
    slayout->addWidget(d->m_HslAlphaLabel);
    slayout->addWidget(d->m_HslAlphaSlider);
    hsl_layout->addLayout(slayout);

    QWidget* hsv_widget     = new QWidget;
    QVBoxLayout* hsv_layout = new QVBoxLayout;
    hsv_widget->setLayout(hsv_layout);
    hsv_layout->setSpacing(0);
    hsv_layout->setContentsMargins(0, 0, 0, 0);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("H"));
    slayout->addWidget(d->m_HsvHueSlider);
    hsv_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("S"));
    slayout->addWidget(d->m_HsvSaturationSlider);
    hsv_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("V"));
    slayout->addWidget(d->m_HsvValueSlider);
    hsv_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    d->m_HsvAlphaLabel = new QLabel("A");
    slayout->addWidget(d->m_HsvAlphaLabel);
    slayout->addWidget(d->m_HsvAlphaSlider);
    hsv_layout->addLayout(slayout);

    d->m_SliderStack->addWidget(rgb_widget);
    d->m_SliderStack->addWidget(hsl_widget);
    d->m_SliderStack->addWidget(hsv_widget);
    d->m_SliderStack->setCurrentIndex(0);
    rgb_button->setChecked(true);

    layout->addLayout(top_layout);
    layout->addLayout(mid_layout);
    layout->addLayout(stack_button_layout);
    layout->addWidget(d->m_SliderStack);

    layout->setContentsMargins(2, 2, 2, 2);
    d->m_Frame->setLayout(layout);

    connect(d->m_ButtonGroup, &QButtonGroup::idClicked, d->m_SliderStack, &QStackedWidget::setCurrentIndex);

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

    auto svchanging = [this, d](qreal val, ColorChannel channel)
    {
        valueToColor(d->m_Color, d->m_EditType, channel, val);

        updateColor(d->m_Color);
        Q_EMIT colorChanging(d->m_Color);
    };

    auto svchanged = [this, d](qreal val, ColorChannel channel)
    {
        valueToColor(d->m_Color, d->m_EditType, channel, val);

        updateColor(d->m_Color);
        Q_EMIT colorChanged(d->m_Color);
    };

    connect(d->m_ValueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Value); });
    connect(
        d->m_ValueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Value); });

    connect(
        d->m_RedSlider, &SliderEdit::valueChanging, [svchanging](qreal val) { svchanging(val, ColorChannel::Red); });
    connect(d->m_RedSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Red); });

    connect(d->m_GreenSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Green); });
    connect(
        d->m_GreenSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Green); });

    connect(
        d->m_BlueSlider, &SliderEdit::valueChanging, [svchanging](qreal val) { svchanging(val, ColorChannel::Blue); });
    connect(d->m_BlueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Blue); });

    connect(d->m_RgbAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(d->m_RgbAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    connect(d->m_HslHueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HslHue); });
    connect(
        d->m_HslHueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::HslHue); });

    connect(d->m_HslSaturationSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HslSaturation); });
    connect(d->m_HslSaturationSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::HslSaturation); });

    connect(d->m_LightnessSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Lightness); });
    connect(d->m_LightnessSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Lightness); });

    connect(d->m_HslAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(d->m_HslAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    connect(d->m_HsvHueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HsvHue); });
    connect(
        d->m_HsvHueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::HsvHue); });

    connect(d->m_HsvSaturationSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HsvSaturation); });
    connect(d->m_HsvSaturationSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::HsvSaturation); });

    connect(d->m_HsvValueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Value); });
    connect(d->m_HsvValueSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Value); });

    connect(d->m_HsvAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(d->m_HsvAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    // sync color of all child widgets
    d->m_Wheel->setColor(d->m_Color);

    QWidget::setTabOrder(d->m_Hex, d->m_RedSlider);
    QWidget::setTabOrder(d->m_RedSlider, d->m_GreenSlider);
    QWidget::setTabOrder(d->m_GreenSlider, d->m_BlueSlider);
    QWidget::setTabOrder(d->m_BlueSlider, d->m_RgbAlphaSlider);
    QWidget::setTabOrder(d->m_RgbAlphaSlider, d->m_HslHueSlider);
    QWidget::setTabOrder(d->m_HslHueSlider, d->m_HslSaturationSlider);
    QWidget::setTabOrder(d->m_HslSaturationSlider, d->m_LightnessSlider);
    QWidget::setTabOrder(d->m_LightnessSlider, d->m_HslAlphaSlider);
    QWidget::setTabOrder(d->m_HslAlphaSlider, d->m_HsvHueSlider);
    QWidget::setTabOrder(d->m_HsvHueSlider, d->m_HsvSaturationSlider);
    QWidget::setTabOrder(d->m_HsvSaturationSlider, d->m_HsvValueSlider);
    QWidget::setTabOrder(d->m_HsvValueSlider, d->m_HsvAlphaSlider);
    QWidget::setTabOrder(d->m_HsvAlphaSlider, d->m_Hex);
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

    if (d->m_EditType == ColorPicker::Float)
    {
        d->m_ValueSlider->updateValue(color.valueF());

        d->m_RedSlider->updateValue(color.redF());
        d->m_GreenSlider->updateValue(color.greenF());
        d->m_BlueSlider->updateValue(color.blueF());
        d->m_RgbAlphaSlider->updateValue(color.alphaF());

        d->m_HslHueSlider->updateValue(color.hslHueF());
        d->m_HslSaturationSlider->updateValue(color.hslSaturationF());
        d->m_LightnessSlider->updateValue(color.lightnessF());
        d->m_HslAlphaSlider->updateValue(color.alphaF());

        d->m_HsvHueSlider->updateValue(color.hsvHueF());
        d->m_HsvSaturationSlider->updateValue(color.hsvSaturationF());
        d->m_HsvValueSlider->updateValue(color.valueF());
        d->m_HsvAlphaSlider->updateValue(color.alphaF());
    }
    else
    {
        d->m_ValueSlider->updateValue(color.value());

        d->m_RedSlider->updateValue(color.red());
        d->m_GreenSlider->updateValue(color.green());
        d->m_BlueSlider->updateValue(color.blue());
        d->m_RgbAlphaSlider->updateValue(color.alpha());

        d->m_HslHueSlider->updateValue(color.hslHue());
        d->m_HslSaturationSlider->updateValue(color.hslSaturation());
        d->m_LightnessSlider->updateValue(color.lightness());
        d->m_HslAlphaSlider->updateValue(color.alpha());

        d->m_HsvHueSlider->updateValue(color.hsvHue());
        d->m_HsvSaturationSlider->updateValue(color.hsvSaturation());
        d->m_HsvValueSlider->updateValue(color.value());
        d->m_HsvAlphaSlider->updateValue(color.alpha());
    }

    if (d->m_Color != color)
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
        Q_EMIT colorChanged(d->m_Color);
    }
}

void ColorPickerPopup::showEvent(QShowEvent* event)
{
    Q_D(ColorPickerPopup);
    QPoint p(pos());
    int fw           = d->m_Frame->lineWidth();
    QMargins margins = d->m_Frame->layout()->contentsMargins();
    p.setX(p.x() - margins.left() - fw);
    p.setY(p.y() - margins.top() - fw);
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
    if (event->type() == QEvent::ActivationChange && !isActiveWindow())
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
    d->m_RgbAlphaLabel->setVisible(visible);
    d->m_HslAlphaLabel->setVisible(visible);
    d->m_HsvAlphaLabel->setVisible(visible);
    d->m_RgbAlphaSlider->setVisible(visible);
    d->m_HslAlphaSlider->setVisible(visible);
    d->m_HsvAlphaSlider->setVisible(visible);
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

    auto update_slider = [&](SliderEdit* w, ColorChannel c)
    {
        switch (type)
        {
            case ColorPicker::EditType::Int:
                w->setRange(0, c == ColorChannel::HslHue || c == ColorChannel::HsvHue ? 359 : 255);
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

    update_slider(d->m_ValueSlider, ColorChannel::Value);

    update_slider(d->m_RedSlider, ColorChannel::Red);
    update_slider(d->m_GreenSlider, ColorChannel::Green);
    update_slider(d->m_BlueSlider, ColorChannel::Blue);
    update_slider(d->m_RgbAlphaSlider, ColorChannel::Alpha);

    update_slider(d->m_HslHueSlider, ColorChannel::HslHue);
    update_slider(d->m_HslSaturationSlider, ColorChannel::HslSaturation);
    update_slider(d->m_LightnessSlider, ColorChannel::Lightness);
    update_slider(d->m_HslAlphaSlider, ColorChannel::Alpha);

    update_slider(d->m_HsvHueSlider, ColorChannel::HsvHue);
    update_slider(d->m_HsvSaturationSlider, ColorChannel::HsvSaturation);
    update_slider(d->m_HsvValueSlider, ColorChannel::Value);
    update_slider(d->m_HsvAlphaSlider, ColorChannel::Alpha);
}
