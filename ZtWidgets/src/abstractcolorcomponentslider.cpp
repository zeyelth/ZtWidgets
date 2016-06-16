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

#include <ZtWidgets/abstractcolorcomponentslider.h>


AbstractColorComponentSlider::AbstractColorComponentSlider(Components components, quint32 width, const QColor& color0, const QColor& color1, QWidget* parent)
    : ColorWidgetBase(parent),
      m_GradientColor0(color0),
      m_GradientColor1(color1),
      m_Components(components),
      m_Width(width)
{
    setFocusPolicy(Qt::StrongFocus);
}

void AbstractColorComponentSlider::setActiveComponents(Components components)
{
    m_Components = components;
}

void AbstractColorComponentSlider::setGradient(const QColor& color0, const QColor& color1)
{
    m_GradientColor0 = color0;
    m_GradientColor1 = color1;
}

QString AbstractColorComponentSlider::componentName(Component component, bool abbreviate)
{
    switch(component)
    {
        case Component::Red:
            return abbreviate ? tr("R") : tr("Red");
        case Component::Green:
            return abbreviate ? tr("G") : tr("Green");
        case Component::Blue:
            return abbreviate ? tr("B") : tr("Blue");
        case Component::Alpha:
            return abbreviate ? tr("A") : tr("Alpha");
        case Component::Hue:
            return abbreviate ? tr("H") : tr("Hue");
        case Component::Saturation:
            return abbreviate ? tr("S") : tr("Saturation");
        case Component::Value:
            return abbreviate ? tr("V") : tr("Value");
        default:
            return QString();
    }
}

QString AbstractColorComponentSlider::activeComponentsName() const
{
    QList<Component> components;
    QString name;
    components << Component::Red << Component::Green << Component::Blue << Component::Alpha << Component::Hue << Component::Saturation << Component::Value;

    bool abbreviation = componentCount() > 1;
    for(auto it = components.constBegin(); it != components.constEnd(); ++it)
    {
        if (m_Components & *it)
        {
            name += componentName(*it, abbreviation);
        }
    }

    return name.isEmpty() ? "Unknown" : name;
}

quint32 AbstractColorComponentSlider::componentCount() const
{
    quint32 count = 0;
    int val = m_Components;
    while (val > 0)
    {
        if ((val & 1) == 1)
        {
            ++count;
        }

        val >>= 1;
    }
    return count;
}

qreal AbstractColorComponentSlider::componentsValueF() const
{
    qreal val = 0;
    if (m_Components & Component::Red)
        val += m_Color.redF();
    if (m_Components & Component::Green)
        val += m_Color.greenF();
    if (m_Components & Component::Blue)
        val += m_Color.blueF();
    if (m_Components & Component::Alpha)
        val += m_Color.alphaF();
    if (m_Components & Component::Hue)
        val += m_Color.hueF();
    if (m_Components & Component::Saturation)
        val += m_Color.saturationF();
    if (m_Components & Component::Value)
        val += m_Color.valueF();

    return val / qreal(componentCount());
}

int AbstractColorComponentSlider::componentsValue() const
{
    int val = 0;
    if (m_Components & Component::Red)
        val += m_Color.red();
    if (m_Components & Component::Green)
        val += m_Color.green();
    if (m_Components & Component::Blue)
        val += m_Color.blue();
    if (m_Components & Component::Alpha)
        val += m_Color.alpha();
    if (m_Components & Component::Hue)
        val += m_Color.hue();
    if (m_Components & Component::Saturation)
        val += m_Color.saturation();
    if (m_Components & Component::Value)
        val += m_Color.value();

    return val / componentCount();
}

void AbstractColorComponentSlider::updateActiveComponents(qreal value)
{
    if ((m_Components & Component::RGBA) && (m_Components & Component::HSV))
    {
        qWarning("Can't update RGBA and HSV components at the same time.");
        return;
    }

    QColor color;
    if (m_Components & Component::RGBA)
    {
        float r = m_Components & Component::Red ? value : m_Color.redF();
        float g = m_Components & Component::Green ? value : m_Color.greenF();
        float b = m_Components & Component::Blue ? value : m_Color.blueF();
        float a = m_Components & Component::Alpha ? value : m_Color.alphaF();
        color.setRgbF(r, g, b, a);
    }
    else if (m_Components & Component::HSV)
    {
        float a = m_Color.alphaF();
        float h = m_Components & Component::Hue ? value : m_Color.hueF();
        float s = m_Components & Component::Saturation ? value : m_Color.saturationF();
        float v = m_Components & Component::Value ? value : m_Color.valueF();
        color.setHsvF(h, s, v, a);
    }
    else
    {
        qWarning("Active components set to neither RGBA nor HSV.");
        return;
    }

    ColorWidgetBase::updateColor(color);
}

void AbstractColorComponentSlider::updateActiveComponents(int value)
{
    if ((m_Components & Component::RGBA) && (m_Components & Component::HSV))
    {
        qWarning("Can't update RGBA and HSV components at the same time.");
        return;
    }

    QColor color;
    if (m_Components & Component::RGBA)
    {
        int r = m_Components & Component::Red ? value : m_Color.red();
        int g = m_Components & Component::Green ? value : m_Color.green();
        int b = m_Components & Component::Blue ? value : m_Color.blue();
        int a = m_Components & Component::Alpha ? value : m_Color.alpha();
        color.setRgb(r, g, b, a);
    }
    else if (m_Components & Component::HSV)
    {
        int a = m_Color.alpha();
        int h = m_Components & Component::Hue ? value : m_Color.hue();
        int s = m_Components & Component::Saturation ? value : m_Color.saturation();
        int v = m_Components & Component::Value ? value : m_Color.value();
        color.setHsv(h, s, v, a);
    }
    else
    {
        qWarning("Active components set to neither RGBA nor HSV.");
        return;
    }

    ColorWidgetBase::updateColor(color);
}
