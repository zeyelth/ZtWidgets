/*
 * Copyright (c) 2016-2017 Victor Wåhlström
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

%Import QtCore/QtCoremod.sip
%Import QtWidgets/QtWidgetsmod.sip

%Module(name=ZtWidgets.slideredit)

class SliderEdit : public QWidget
{
%TypeHeaderCode
#include <ZtWidgets/slideredit.h>
%End
public:

    enum SliderComponent
    {
        Text,
        Marker,
        Gauge,
    };

    typedef QFlags<SliderEdit::SliderComponent> SliderComponents;

    enum SliderBehaviorFlag
    {
        SnapToPrecision,
        AllowValueUnderflow,
        AllowValueOverflow,
    };

    typedef QFlags<SliderEdit::SliderBehaviorFlag> SliderBehavior;

    enum ValueMapping
    {
        LinearScale,
        LogarithmicScale,
    };

    explicit SliderEdit(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~SliderEdit();

    QSize sizeHint() const;

    qreal minimum() const;
    void setMinimum(qreal minimum);

    qreal maximum() const;
    void setMaximum(qreal maximum);

    void setRange(qreal minimum, qreal maximum);

    bool editable() const;
    void setEditable(bool editable);

    qreal singleStep() const;
    void setSingleStep(qreal step);

    qreal pageStep() const;
    void setPageStep(qreal step);

    qreal value() const;
    void setValue(qreal value);

    void updateValue(qreal value);
    void setUnit(const QString& unit);

    const QString& unit() const;

    void setLabel(const QString& label);
    const QString& label() const;

    void setPrecision(quint32 precision);
    quint32 precision() const;

    void setSliderComponents(SliderComponents components);
    SliderComponents sliderComponents() const;

    void setSliderBehavior(SliderBehavior behavior);
    SliderBehavior sliderBehavior() const;

    void setValueMapping(ValueMapping mapping);
    ValueMapping valueMapping() const;

    void setAlignment(Qt::Alignment alignment);
    Qt::Alignment alignment() const;

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

Q_SIGNALS:
    void valueChanged(const qreal& value);
    void valueChanging(const qreal& value);
};


QFlags<SliderEdit::SliderComponent> operator|(SliderEdit::SliderComponent f1, QFlags<SliderEdit::SliderComponent> f2);
QFlags<SliderEdit::SliderBehaviorFlag> operator|(SliderEdit::SliderBehaviorFlag f1, QFlags<SliderEdit::SliderBehaviorFlag> f2);
