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

  public:
    explicit ColorPickerPopupPrivate();

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

ColorPickerPopupPrivate::ColorPickerPopupPrivate()
    : m_Frame(nullptr)
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
    , m_Impl(new ColorPickerPopupPrivate())
{
    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_QuitOnClose, false);

    QVBoxLayout* main_layout = new QVBoxLayout;
    m_Impl->m_Frame               = new QFrame;
    m_Impl->m_Frame->setFrameStyle(QFrame::Panel);
    m_Impl->m_Frame->setFrameShadow(QFrame::Raised);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_Impl->m_Frame);
    setLayout(main_layout);

    QVBoxLayout* layout = new QVBoxLayout;
    m_Impl->m_Hex            = new ColorHexEdit;

    m_Impl->m_Display = new ColorDisplay;

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->addWidget(m_Impl->m_Display);
    top_layout->addWidget(m_Impl->m_Hex);
    top_layout->addStretch(100);

    m_Impl->m_Wheel = new HueSaturationWheel;
    m_Impl->m_Wheel->setToolTip(tr("Hue/Saturation"));
    QSizePolicy size_policy;
    size_policy.setVerticalPolicy(QSizePolicy::Expanding);
    size_policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_Impl->m_Wheel->setSizePolicy(size_policy);

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

    m_Impl->m_ValueSlider = new ColorSliderEdit(bw_gradient);
    m_Impl->m_ValueSlider->setToolTip(tr("Value"));
    m_Impl->m_ValueSlider->setOrientation(Qt::Vertical);

    QPalette p = palette();
    p.setBrush(QPalette::Text, Qt::white);

    m_Impl->m_RedSlider = new ColorSliderEdit(red_gradient);
    m_Impl->m_RedSlider->setToolTip(tr("Red"));
    m_Impl->m_GreenSlider = new ColorSliderEdit(green_gradient);
    m_Impl->m_GreenSlider->setToolTip(tr("Green"));
    m_Impl->m_BlueSlider = new ColorSliderEdit(blue_gradient);
    m_Impl->m_BlueSlider->setToolTip(tr("Blue"));
    m_Impl->m_RgbAlphaSlider = new ColorSliderEdit(alpha_gradient);
    m_Impl->m_RgbAlphaSlider->setToolTip(tr("Alpha"));

    m_Impl->m_HslHueSlider = new ColorSliderEdit(hue_gradient);
    m_Impl->m_HslHueSlider->setToolTip(tr("Hue"));
    m_Impl->m_HslSaturationSlider = new ColorSliderEdit(bw_gradient);
    m_Impl->m_HslSaturationSlider->setToolTip(tr("Saturation"));
    m_Impl->m_LightnessSlider = new ColorSliderEdit(bw_gradient);
    m_Impl->m_LightnessSlider->setToolTip(tr("Lightness"));
    m_Impl->m_HslAlphaSlider = new ColorSliderEdit(alpha_gradient);
    m_Impl->m_HslAlphaSlider->setToolTip(tr("Alpha"));

    m_Impl->m_HsvHueSlider = new ColorSliderEdit(hue_gradient);
    m_Impl->m_HsvHueSlider->setToolTip(tr("Hue"));
    m_Impl->m_HsvSaturationSlider = new ColorSliderEdit(bw_gradient);
    m_Impl->m_HsvSaturationSlider->setToolTip(tr("Saturation"));
    m_Impl->m_HsvValueSlider = new ColorSliderEdit(bw_gradient);
    m_Impl->m_HsvValueSlider->setToolTip(tr("Value"));
    m_Impl->m_HsvAlphaSlider = new ColorSliderEdit(alpha_gradient);
    m_Impl->m_HsvAlphaSlider->setToolTip(tr("Alpha"));

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(m_Impl->m_Wheel);
    mid_layout->addWidget(m_Impl->m_ValueSlider);
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

    m_Impl->m_ButtonGroup = new QButtonGroup;
    m_Impl->m_ButtonGroup->addButton(rgb_button, 0);
    m_Impl->m_ButtonGroup->addButton(hsl_button, 1);
    m_Impl->m_ButtonGroup->addButton(hsv_button, 2);

    QHBoxLayout* stack_button_layout = new QHBoxLayout;
    stack_button_layout->setSpacing(0);
    stack_button_layout->addWidget(rgb_button);
    stack_button_layout->addWidget(hsl_button);
    stack_button_layout->addWidget(hsv_button);

    m_Impl->m_SliderStack = new QStackedWidget;
    m_Impl->m_SliderStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QWidget* rgb_widget     = new QWidget;
    QVBoxLayout* rgb_layout = new QVBoxLayout;
    rgb_widget->setLayout(rgb_layout);
    rgb_layout->setSpacing(0);
    rgb_layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("R"));
    slayout->addWidget(m_Impl->m_RedSlider);
    rgb_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("G"));
    slayout->addWidget(m_Impl->m_GreenSlider);
    rgb_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("B"));
    slayout->addWidget(m_Impl->m_BlueSlider);
    rgb_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    m_Impl->m_RgbAlphaLabel = new QLabel("A");
    slayout->addWidget(m_Impl->m_RgbAlphaLabel);
    slayout->addWidget(m_Impl->m_RgbAlphaSlider);
    rgb_layout->addLayout(slayout);

    QWidget* hsl_widget     = new QWidget;
    QVBoxLayout* hsl_layout = new QVBoxLayout;
    hsl_widget->setLayout(hsl_layout);
    hsl_layout->setSpacing(0);
    hsl_layout->setContentsMargins(0, 0, 0, 0);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("H"));
    slayout->addWidget(m_Impl->m_HslHueSlider);
    hsl_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("S"));
    slayout->addWidget(m_Impl->m_HslSaturationSlider);
    hsl_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("L"));
    slayout->addWidget(m_Impl->m_LightnessSlider);
    hsl_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    m_Impl->m_HslAlphaLabel = new QLabel("A");
    slayout->addWidget(m_Impl->m_HslAlphaLabel);
    slayout->addWidget(m_Impl->m_HslAlphaSlider);
    hsl_layout->addLayout(slayout);

    QWidget* hsv_widget     = new QWidget;
    QVBoxLayout* hsv_layout = new QVBoxLayout;
    hsv_widget->setLayout(hsv_layout);
    hsv_layout->setSpacing(0);
    hsv_layout->setContentsMargins(0, 0, 0, 0);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("H"));
    slayout->addWidget(m_Impl->m_HsvHueSlider);
    hsv_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("S"));
    slayout->addWidget(m_Impl->m_HsvSaturationSlider);
    hsv_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("V"));
    slayout->addWidget(m_Impl->m_HsvValueSlider);
    hsv_layout->addLayout(slayout);

    slayout            = new QHBoxLayout;
    m_Impl->m_HsvAlphaLabel = new QLabel("A");
    slayout->addWidget(m_Impl->m_HsvAlphaLabel);
    slayout->addWidget(m_Impl->m_HsvAlphaSlider);
    hsv_layout->addLayout(slayout);

    m_Impl->m_SliderStack->addWidget(rgb_widget);
    m_Impl->m_SliderStack->addWidget(hsl_widget);
    m_Impl->m_SliderStack->addWidget(hsv_widget);
    m_Impl->m_SliderStack->setCurrentIndex(0);
    rgb_button->setChecked(true);

    layout->addLayout(top_layout);
    layout->addLayout(mid_layout);
    layout->addLayout(stack_button_layout);
    layout->addWidget(m_Impl->m_SliderStack);

    layout->setContentsMargins(2, 2, 2, 2);
    m_Impl->m_Frame->setLayout(layout);

    connect(m_Impl->m_ButtonGroup, &QButtonGroup::idClicked, m_Impl->m_SliderStack, &QStackedWidget::setCurrentIndex);

    connect(m_Impl->m_Wheel, &HueSaturationWheel::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Wheel, &HueSaturationWheel::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(m_Impl->m_Wheel, &HueSaturationWheel::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Wheel, &HueSaturationWheel::colorChanging, this, &ColorPickerPopup::colorChanging);

    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Hex, &ColorHexEdit::colorChanging, this, &ColorPickerPopup::colorChanging);

    connect(m_Impl->m_Display, &ColorDisplay::clicked, this, &ColorPickerPopup::hide);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanged, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanged, this, &ColorPickerPopup::colorChanged);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanging, this, &ColorPickerPopup::updateColor);
    connect(m_Impl->m_Display, &ColorDisplay::colorChanging, this, &ColorPickerPopup::colorChanging);

    auto svchanging = [this](qreal val, ColorChannel channel)
    {
        valueToColor(m_Impl->m_Color, m_Impl->m_EditType, channel, val);

        updateColor(m_Impl->m_Color);
        Q_EMIT colorChanging(m_Impl->m_Color);
    };

    auto svchanged = [this](qreal val, ColorChannel channel)
    {
        valueToColor(m_Impl->m_Color, m_Impl->m_EditType, channel, val);

        updateColor(m_Impl->m_Color);
        Q_EMIT colorChanged(m_Impl->m_Color);
    };

    connect(m_Impl->m_ValueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Value); });
    connect(
        m_Impl->m_ValueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Value); });

    connect(
        m_Impl->m_RedSlider, &SliderEdit::valueChanging, [svchanging](qreal val) { svchanging(val, ColorChannel::Red); });
    connect(m_Impl->m_RedSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Red); });

    connect(m_Impl->m_GreenSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Green); });
    connect(
        m_Impl->m_GreenSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Green); });

    connect(
        m_Impl->m_BlueSlider, &SliderEdit::valueChanging, [svchanging](qreal val) { svchanging(val, ColorChannel::Blue); });
    connect(m_Impl->m_BlueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::Blue); });

    connect(m_Impl->m_RgbAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(m_Impl->m_RgbAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    connect(m_Impl->m_HslHueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HslHue); });
    connect(
        m_Impl->m_HslHueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::HslHue); });

    connect(m_Impl->m_HslSaturationSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HslSaturation); });
    connect(m_Impl->m_HslSaturationSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::HslSaturation); });

    connect(m_Impl->m_LightnessSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Lightness); });
    connect(m_Impl->m_LightnessSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Lightness); });

    connect(m_Impl->m_HslAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(m_Impl->m_HslAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    connect(m_Impl->m_HsvHueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HsvHue); });
    connect(
        m_Impl->m_HsvHueSlider, &SliderEdit::valueChanged, [svchanged](qreal val) { svchanged(val, ColorChannel::HsvHue); });

    connect(m_Impl->m_HsvSaturationSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::HsvSaturation); });
    connect(m_Impl->m_HsvSaturationSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::HsvSaturation); });

    connect(m_Impl->m_HsvValueSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Value); });
    connect(m_Impl->m_HsvValueSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Value); });

    connect(m_Impl->m_HsvAlphaSlider,
            &SliderEdit::valueChanging,
            [svchanging](qreal val) { svchanging(val, ColorChannel::Alpha); });
    connect(m_Impl->m_HsvAlphaSlider,
            &SliderEdit::valueChanged,
            [svchanged](qreal val) { svchanged(val, ColorChannel::Alpha); });

    // sync color of all child widgets
    m_Impl->m_Wheel->setColor(m_Impl->m_Color);

    QWidget::setTabOrder(m_Impl->m_Hex, m_Impl->m_RedSlider);
    QWidget::setTabOrder(m_Impl->m_RedSlider, m_Impl->m_GreenSlider);
    QWidget::setTabOrder(m_Impl->m_GreenSlider, m_Impl->m_BlueSlider);
    QWidget::setTabOrder(m_Impl->m_BlueSlider, m_Impl->m_RgbAlphaSlider);
    QWidget::setTabOrder(m_Impl->m_RgbAlphaSlider, m_Impl->m_HslHueSlider);
    QWidget::setTabOrder(m_Impl->m_HslHueSlider, m_Impl->m_HslSaturationSlider);
    QWidget::setTabOrder(m_Impl->m_HslSaturationSlider, m_Impl->m_LightnessSlider);
    QWidget::setTabOrder(m_Impl->m_LightnessSlider, m_Impl->m_HslAlphaSlider);
    QWidget::setTabOrder(m_Impl->m_HslAlphaSlider, m_Impl->m_HsvHueSlider);
    QWidget::setTabOrder(m_Impl->m_HsvHueSlider, m_Impl->m_HsvSaturationSlider);
    QWidget::setTabOrder(m_Impl->m_HsvSaturationSlider, m_Impl->m_HsvValueSlider);
    QWidget::setTabOrder(m_Impl->m_HsvValueSlider, m_Impl->m_HsvAlphaSlider);
    QWidget::setTabOrder(m_Impl->m_HsvAlphaSlider, m_Impl->m_Hex);
}

ColorPickerPopup::~ColorPickerPopup()
{
    delete m_Impl;
}

void ColorPickerPopup::updateColor(const QColor& color)
{
    m_Impl->m_Wheel->updateColor(color);
    m_Impl->m_Hex->updateColor(color);
    m_Impl->m_Display->updateColor(color);

    if (m_Impl->m_EditType == ColorPicker::Float)
    {
        m_Impl->m_ValueSlider->updateValue(color.valueF());

        m_Impl->m_RedSlider->updateValue(color.redF());
        m_Impl->m_GreenSlider->updateValue(color.greenF());
        m_Impl->m_BlueSlider->updateValue(color.blueF());
        m_Impl->m_RgbAlphaSlider->updateValue(color.alphaF());

        m_Impl->m_HslHueSlider->updateValue(color.hslHueF());
        m_Impl->m_HslSaturationSlider->updateValue(color.hslSaturationF());
        m_Impl->m_LightnessSlider->updateValue(color.lightnessF());
        m_Impl->m_HslAlphaSlider->updateValue(color.alphaF());

        m_Impl->m_HsvHueSlider->updateValue(color.hsvHueF());
        m_Impl->m_HsvSaturationSlider->updateValue(color.hsvSaturationF());
        m_Impl->m_HsvValueSlider->updateValue(color.valueF());
        m_Impl->m_HsvAlphaSlider->updateValue(color.alphaF());
    }
    else
    {
        m_Impl->m_ValueSlider->updateValue(color.value());

        m_Impl->m_RedSlider->updateValue(color.red());
        m_Impl->m_GreenSlider->updateValue(color.green());
        m_Impl->m_BlueSlider->updateValue(color.blue());
        m_Impl->m_RgbAlphaSlider->updateValue(color.alpha());

        m_Impl->m_HslHueSlider->updateValue(color.hslHue());
        m_Impl->m_HslSaturationSlider->updateValue(color.hslSaturation());
        m_Impl->m_LightnessSlider->updateValue(color.lightness());
        m_Impl->m_HslAlphaSlider->updateValue(color.alpha());

        m_Impl->m_HsvHueSlider->updateValue(color.hsvHue());
        m_Impl->m_HsvSaturationSlider->updateValue(color.hsvSaturation());
        m_Impl->m_HsvValueSlider->updateValue(color.value());
        m_Impl->m_HsvAlphaSlider->updateValue(color.alpha());
    }

    if (m_Impl->m_Color != color)
    {
        m_Impl->m_Color = color;
        update();
    }
}

void ColorPickerPopup::setColor(const QColor& color)
{
    if (m_Impl->m_Color != color)
    {
        updateColor(color);
        Q_EMIT colorChanged(m_Impl->m_Color);
    }
}

void ColorPickerPopup::showEvent(QShowEvent* event)
{
    QPoint p(pos());
    int fw           = m_Impl->m_Frame->lineWidth();
    QMargins margins = m_Impl->m_Frame->layout()->contentsMargins();
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
    return m_Impl->m_Hex->displayAlpha();
}

void ColorPickerPopup::setDisplayAlpha(bool visible)
{
    m_Impl->m_RgbAlphaLabel->setVisible(visible);
    m_Impl->m_HslAlphaLabel->setVisible(visible);
    m_Impl->m_HsvAlphaLabel->setVisible(visible);
    m_Impl->m_RgbAlphaSlider->setVisible(visible);
    m_Impl->m_HslAlphaSlider->setVisible(visible);
    m_Impl->m_HsvAlphaSlider->setVisible(visible);
    m_Impl->m_Hex->setDisplayAlpha(visible);
}

ColorPicker::EditType ColorPickerPopup::editType() const
{
    return m_Impl->m_EditType;
}

void ColorPickerPopup::setEditType(ColorPicker::EditType type)
{
    m_Impl->m_EditType = type;

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

    update_slider(m_Impl->m_ValueSlider, ColorChannel::Value);

    update_slider(m_Impl->m_RedSlider, ColorChannel::Red);
    update_slider(m_Impl->m_GreenSlider, ColorChannel::Green);
    update_slider(m_Impl->m_BlueSlider, ColorChannel::Blue);
    update_slider(m_Impl->m_RgbAlphaSlider, ColorChannel::Alpha);

    update_slider(m_Impl->m_HslHueSlider, ColorChannel::HslHue);
    update_slider(m_Impl->m_HslSaturationSlider, ColorChannel::HslSaturation);
    update_slider(m_Impl->m_LightnessSlider, ColorChannel::Lightness);
    update_slider(m_Impl->m_HslAlphaSlider, ColorChannel::Alpha);

    update_slider(m_Impl->m_HsvHueSlider, ColorChannel::HsvHue);
    update_slider(m_Impl->m_HsvSaturationSlider, ColorChannel::HsvSaturation);
    update_slider(m_Impl->m_HsvValueSlider, ColorChannel::Value);
    update_slider(m_Impl->m_HsvAlphaSlider, ColorChannel::Alpha);
}
