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
#include <ZtWidgets/colorhexedit.h>
#include "colordisplay_p.h"
#include <ZtWidgets/huesaturationwheel.h>

#include <QtGui/QResizeEvent>
#include <QtGui/QPainter>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QFontDatabase>

#include <QtCore/QtMath>

//! @cond Doxygen_Suppress
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

static void drawCheckerboard(QPainter& painter, const QRect& rect, quint32 size)
{
    QColor color1(153, 153, 152);
    QColor color2(102, 102, 102);

    painter.save();
    painter.fillRect(rect, color1);
    QRect square(0, 0, size, size);
    quint32 step_x(size * 2);
    quint32 step_y(size);
    bool odd = true;
    while(square.top() < rect.bottom())
    {
        while(square.left() < rect.right())
        {
            painter.fillRect(square, color2);
            square.moveLeft(square.left() + step_x);
        }

        square.moveLeft(0);
        if(odd)
        {
            square.moveLeft(square.left() + step_x * 0.5);
        }

        square.moveTop(square.top() + step_y);
        odd = !odd;
    }

    painter.restore();
}

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

class Popup : public ColorWidgetBase
{

public:
    explicit Popup(QWidget* parent = Q_NULLPTR);

    void updateColor(const QColor& color) override;
    bool displayAlpha();
    void setDisplayAlpha(bool visible);

    ColorPicker::EditType editType();
    void setEditType(ColorPicker::EditType edit_type);

protected:
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onSliderValueChanging(qreal val, ColorChannel channel);
    void onSliderValueChanged(qreal val, ColorChannel channel);

private:
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
    ColorPicker::EditType m_EditType;
};

Popup::Popup(QWidget* parent)
    : ColorWidgetBase(parent)
{
    setWindowFlags(Qt::Window | Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_QuitOnClose, false);
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

    m_ValueSlider = new ColorSliderEdit(Qt::white, Qt::black);
    m_ValueSlider->setOrientation(Qt::Vertical);

    QPalette p = palette();
    p.setBrush(QPalette::Text, Qt::white);
    m_RedSlider = new ColorSliderEdit(Qt::black, Qt::red);
    m_GreenSlider = new ColorSliderEdit(Qt::black, Qt::green);
    m_BlueSlider = new ColorSliderEdit(Qt::black, Qt::blue);
    m_AlphaSlider = new ColorSliderEdit(QColor(255, 255, 255, 0), Qt::white);

    QHBoxLayout* mid_layout = new QHBoxLayout;
    mid_layout->addWidget(m_Wheel);
    mid_layout->addWidget(m_ValueSlider);
    mid_layout->setContentsMargins(5, 0, 5, 0);

    QVBoxLayout* bottom_layout = new QVBoxLayout;
    bottom_layout->setSpacing(0);

    QHBoxLayout* slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("R"));
    slayout->addWidget(m_RedSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("G"));
    slayout->addWidget(m_GreenSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    slayout->addWidget(new QLabel("B"));
    slayout->addWidget(m_BlueSlider);
    bottom_layout->addLayout(slayout);

    slayout = new QHBoxLayout;
    m_AlphaLabel = new QLabel("A");
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

    connect(m_Display, &ColorDisplay::colorChanged, this, &Popup::updateColor);
    connect(m_Display, &ColorDisplay::colorChanged, this, &Popup::colorChanged);
    connect(m_Display, &ColorDisplay::colorChanging, this, &Popup::updateColor);
    connect(m_Display, &ColorDisplay::colorChanging, this, &Popup::colorChanging);

    connect(m_ValueSlider, &SliderEdit::valueChanging, [this](qreal val) { onSliderValueChanging(val, ColorChannel::HsvValue); });
    connect(m_ValueSlider, &SliderEdit::valueChanged, [this](qreal val) { onSliderValueChanged(val, ColorChannel::HsvValue); });

    connect(m_RedSlider, &SliderEdit::valueChanging, [this](qreal val) { onSliderValueChanging(val, ColorChannel::Red); });
    connect(m_RedSlider, &SliderEdit::valueChanged, [this](qreal val) { onSliderValueChanged(val, ColorChannel::Red); });

    connect(m_GreenSlider, &SliderEdit::valueChanging, [this](qreal val) { onSliderValueChanging(val, ColorChannel::Green); });
    connect(m_GreenSlider, &SliderEdit::valueChanged, [this](qreal val) { onSliderValueChanged(val, ColorChannel::Green); });

    connect(m_BlueSlider, &SliderEdit::valueChanging, [this](qreal val) { onSliderValueChanging(val, ColorChannel::Blue); });
    connect(m_BlueSlider, &SliderEdit::valueChanged, [this](qreal val) { onSliderValueChanged(val, ColorChannel::Blue); });

    connect(m_AlphaSlider, &SliderEdit::valueChanging, [this](qreal val) { onSliderValueChanging(val, ColorChannel::Alpha); });
    connect(m_AlphaSlider, &SliderEdit::valueChanged, [this](qreal val) { onSliderValueChanged(val, ColorChannel::Alpha); });

    // sync color of all child widgets
    m_Wheel->setColor(m_Color);

    QWidget::setTabOrder(m_Hex, m_RedSlider);
    QWidget::setTabOrder(m_RedSlider, m_GreenSlider);
    QWidget::setTabOrder(m_GreenSlider, m_BlueSlider);
    QWidget::setTabOrder(m_BlueSlider, m_AlphaSlider);
    QWidget::setTabOrder(m_AlphaSlider, m_Hex);
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

void Popup::onSliderValueChanging(qreal val, ColorChannel channel)
{
    valueToColor(m_Color, m_EditType, channel, val);

    updateColor(m_Color);
    emit colorChanging(m_Color);
}

void Popup::onSliderValueChanged(qreal val, ColorChannel channel)
{
    valueToColor(m_Color, m_EditType, channel, val);

    updateColor(m_Color);
    emit colorChanged(m_Color);
}

void Popup::updateColor(const QColor& color)
{
    m_Wheel->updateColor(color);
    m_Hex->updateColor(color);
    m_Display->updateColor(color);

    if(m_EditType == ColorPicker::Float)
    {
        m_RedSlider->updateValue(color.redF());
        m_GreenSlider->updateValue(color.greenF());
        m_BlueSlider->updateValue(color.blueF());
        m_AlphaSlider->updateValue(color.alphaF());
        m_ValueSlider->updateValue(color.valueF());
    }
    else
    {
        m_RedSlider->updateValue(color.red());
        m_GreenSlider->updateValue(color.green());
        m_BlueSlider->updateValue(color.blue());
        m_AlphaSlider->updateValue(color.alpha());
        m_ValueSlider->updateValue(color.value());
    }

    ColorWidgetBase::updateColor(color);
}

void Popup::showEvent(QShowEvent* event)
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
    activateWindow();
}

void Popup::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::ActivationChange && !isActiveWindow())
    {
        hide();
    }
    ColorWidgetBase::changeEvent(event);
}

bool Popup::displayAlpha()
{
    return m_Hex->displayAlpha();
}

void Popup::setDisplayAlpha(bool visible)
{
    m_AlphaLabel->setVisible(visible);
    m_AlphaSlider->setVisible(visible);
    m_Hex->setDisplayAlpha(visible);
}

ColorPicker::EditType Popup::editType()
{
    return m_EditType;
}

void Popup::setEditType(ColorPicker::EditType edit_type)
{
    m_EditType = edit_type;

    auto update_slider = [&](SliderEdit* w)
    {
        switch(edit_type)
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

    update_slider(m_RedSlider);
    update_slider(m_GreenSlider);
    update_slider(m_BlueSlider);
    update_slider(m_AlphaSlider);
    update_slider(m_ValueSlider);
}

class ColorPickerPrivate
{
    Q_DISABLE_COPY(ColorPickerPrivate)
    Q_DECLARE_PUBLIC(ColorPicker)

private:
    explicit ColorPickerPrivate(ColorPicker*);

    ColorPicker* const q_ptr;

    ColorHexEdit* m_Hex;
    ColorDisplay* m_Display;
    Popup* m_Popup;
    ColorPicker::EditType m_EditType;
    bool m_DisplayAlpha : 1;
};

ColorPickerPrivate::ColorPickerPrivate(ColorPicker* colorpicker)
    : q_ptr(colorpicker)
    , m_Hex(nullptr)
    , m_Display(nullptr)
    , m_Popup(nullptr)
    , m_EditType(ColorPicker::Float)
    , m_DisplayAlpha(true)
{}

//! @endcond

ColorPicker::ColorPicker(QWidget *parent)
    : ColorWidgetBase(parent)
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

    auto connectfunc = [this](ColorWidgetBase* w)
    {
        connect(w, &ColorWidgetBase::colorChanged, this, &ColorPicker::updateColor);
        connect(w, &ColorWidgetBase::colorChanged, this, &ColorPicker::colorChanged);
        connect(w, &ColorWidgetBase::colorChanging, this, &ColorPicker::updateColor);
        connect(w, &ColorWidgetBase::colorChanging, this, &ColorPicker::colorChanging);
    };

    auto on_display_clicked = [this, connectfunc]()
    {
        Q_D(ColorPicker);
        if(!d->m_Popup)
        {
            d->m_Popup = new Popup;
            d->m_Popup->setMinimumSize(185, 290);
            d->m_Popup->setMaximumSize(185, 290);
            d->m_Popup->setDisplayAlpha(d->m_DisplayAlpha);
            d->m_Popup->setEditType(d->m_EditType);
            d->m_Popup->setFont(this->font());
            d->m_Popup->setColor(m_Color);
            connectfunc(d->m_Popup);
        }

        d->m_Popup->move(mapToGlobal(rect().topLeft()));
        d->m_Popup->show();
    };

    connect(d->m_Display, &ColorDisplay::clicked, this, on_display_clicked);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanged, this, &ColorPicker::colorChanged);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::updateColor);
    connect(d->m_Display, &ColorDisplay::colorChanging, this, &ColorPicker::colorChanging);

    connectfunc(d->m_Hex);

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

    ColorWidgetBase::updateColor(color);
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
