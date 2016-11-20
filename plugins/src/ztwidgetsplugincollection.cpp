#include "ztwidgetsplugincollection.h"

ZtWidgetsPluginCollection::ZtWidgetsPluginCollection(QObject *parent)
    : QObject(parent)
{
    m_Widgets.append(new ColorPickerPlugin(this));
    m_Widgets.append(new SliderEditPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> ZtWidgetsPluginCollection::customWidgets() const
{
    return m_Widgets;
}
