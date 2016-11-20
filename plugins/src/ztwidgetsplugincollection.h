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

#ifndef ZTWIDGETSPLUGINCOLLECTION_H
#define ZTWIDGETSPLUGINCOLLECTION_H

#include "colorpickerplugin.h"
#include "slidereditplugin.h"

#include <QtDesigner/QtDesigner>
#include <QtCore/qplugin.h>


class ZtWidgetsPluginCollection : public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    ZtWidgetsPluginCollection(QObject* parent = Q_NULLPTR);

    QList<QDesignerCustomWidgetInterface*> customWidgets() const Q_DECL_OVERRIDE;

private:
    QList<QDesignerCustomWidgetInterface*> m_Widgets;
};

#endif // ZTWIDGETSPLUGINCOLLECTION_H
