/*
 * Copyright (c) 2016 Victor Wåhlström
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

#include "slidereditplugin.h"
#include <ZtWidgets/slideredit.h>

SliderEditPlugin::SliderEditPlugin(QObject* parent)
    : QObject(parent)
    , m_Initialized(false)
{}

void SliderEditPlugin::initialize(QDesignerFormEditorInterface*)
{
    if (m_Initialized)
        return;

    m_Initialized = true;
}

bool SliderEditPlugin::isInitialized() const
{
    return m_Initialized;
}

QWidget* SliderEditPlugin::createWidget(QWidget* parent)
{
    return new SliderEdit(parent);
}

QString SliderEditPlugin::name() const
{
    return "SliderEdit";
}

QString SliderEditPlugin::group() const
{
    return "Input Widgets [ZtWidgets]";
}

QIcon SliderEditPlugin::icon() const
{
    return QIcon();
}

QString SliderEditPlugin::toolTip() const
{
    return tr("An editable, horizontal slider widget.");
}

QString SliderEditPlugin::whatsThis() const
{
    return tr("A horizontal slider widget with editing capabilities.");
}

bool SliderEditPlugin::isContainer() const
{
    return false;
}

QString SliderEditPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"SliderEdit\" name=\"sliderEdit\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>25</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string></string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string></string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString SliderEditPlugin::includeFile() const
{
    return "<ZtWidgets/slideredit.h>";
}
