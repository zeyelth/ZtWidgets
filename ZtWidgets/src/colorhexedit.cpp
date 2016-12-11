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

#include "colorhexedit_p.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionFrame>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtCore/Qt>
#include <QtCore/QSize>
#include <QtCore/QString>


static QString colorToString(const QColor& color, bool include_alpha)
{
    QLatin1Char fill('0');
    if (include_alpha)
    {
        return QString("%1%2%3%4")
                .arg(color.alpha(), 2, 16, fill)
                .arg(color.red(), 2, 16, fill)
                .arg(color.green(), 2, 16, fill)
                .arg(color.blue(), 2, 16, fill).toUpper();
    }

    return QString("%1%2%3")
            .arg(color.red(), 2, 16, fill)
            .arg(color.green(), 2, 16, fill)
            .arg(color.blue(), 2, 16, fill).toUpper();
}

//! @cond Doxygen_Suppress
class ColorHexEditPrivate
{
    Q_DISABLE_COPY(ColorHexEditPrivate)
    Q_DECLARE_PUBLIC(ColorHexEdit)

private:
    explicit ColorHexEditPrivate(ColorHexEdit* hex_edit);

    int editWidth() const;
    void refresh();

    ColorHexEdit* const q_ptr;

    QColor m_Color;
    QLineEdit* m_LineEdit;
    bool m_DisplayAlpha : 1;
    bool m_Modified : 1;
};

ColorHexEditPrivate::ColorHexEditPrivate(ColorHexEdit* hex_edit)
    : q_ptr(hex_edit)
    , m_Color(Qt::white)
    , m_LineEdit(nullptr)
    , m_DisplayAlpha(true)
    , m_Modified(false)
{}

int ColorHexEditPrivate::editWidth() const
{
    QFontMetrics fm(m_LineEdit->fontMetrics());
    const QStyle* style = m_LineEdit->style();
    QStyleOptionFrame option;
    option.initFrom(m_LineEdit);
    int width = fm.averageCharWidth() * m_LineEdit->maxLength();
    width += style->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, m_LineEdit) * 2;
    // input mask makes the cursor about 5 pixels wide when no character is selected
    // this is not reflected by QStyle::PM_TextCursorWidth. Qt Bug?
    //width += style->pixelMetric(QStyle::PM_TextCursorWidth, &option, m_LineEdit) * 2;
    // increase width by one character to compensate for now
    width += fm.averageCharWidth();
    int height = fm.height();

    QSize size(width, height);
    size = style->sizeFromContents(QStyle::CT_LineEdit, &option, size, m_LineEdit);

    return size.width();
}

void ColorHexEditPrivate::refresh()
{
    Q_Q(ColorHexEdit);

    if (m_DisplayAlpha)
    {
        m_LineEdit->setMaxLength(8);
        m_LineEdit->setInputMask(">HHHHHHHH");
        m_LineEdit->setPlaceholderText("AARRGGBB");
        m_LineEdit->setToolTip(q->tr("A hexadecimal value on the form AARRGGBB:\n\nAA = alpha\nRR = red\nGG = green\nBB = blue"));
    }
    else
    {
        m_LineEdit->setMaxLength(6);
        m_LineEdit->setInputMask(">HHHHHH");
        m_LineEdit->setPlaceholderText("RRGGBB");
        m_LineEdit->setToolTip(q->tr("A hexadecimal value on the form RRGGBB:\n\nRR = red\nGG = green\nBB = blue"));
    }

    m_LineEdit->setAlignment(Qt::AlignCenter);

    m_LineEdit->setFixedWidth(editWidth());
    m_LineEdit->setText(colorToString(m_Color, q->displayAlpha()));
}

//! @endcond


ColorHexEdit::ColorHexEdit(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ColorHexEditPrivate(this))
{
    Q_D(ColorHexEdit);
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* hash_label = new QLabel;
    hash_label->setText("#");
    layout->addWidget(hash_label);

    d->m_LineEdit = new QLineEdit;

    auto on_editing_finished = [this]()
    {
        Q_D(ColorHexEdit);
        if (d->m_Modified)
        {
            d->m_Modified = false;
            emit colorChanged(d->m_Color);
        }
    };

    auto on_text_edited = [this](const QString& text)
    {
        Q_D(ColorHexEdit);
        if (text.size() < d->m_LineEdit->maxLength())
        {
            int pos = d->m_LineEdit->cursorPosition();
            d->m_LineEdit->insert("0");
            d->m_LineEdit->setCursorPosition(pos);
        }

        if (d->m_LineEdit->text() == colorToString(d->m_Color, displayAlpha()))
        {
            return;
        }

        d->m_Modified = true;
        d->m_Color.setNamedColor("#"+text);
        emit colorChanging(d->m_Color);
    };


    connect(d->m_LineEdit, &QLineEdit::textEdited, this, on_text_edited);
    connect(d->m_LineEdit, &QLineEdit::editingFinished, this, on_editing_finished);

    layout->addWidget(d->m_LineEdit);
    setLayout(layout);
}

ColorHexEdit::~ColorHexEdit()
{
    delete d_ptr;
}

void ColorHexEdit::showEvent(QShowEvent* event)
{
    Q_D(ColorHexEdit);
    // refresh the UI here as fonts, styles, and other things affecting this widget may have changed
    d->refresh();
    QWidget::showEvent(event);
}

void ColorHexEdit::updateColor(const QColor& color)
{
    Q_D(ColorHexEdit);

    if(d->m_Color != color)
    {
        d->m_LineEdit->setText(colorToString(color, displayAlpha()));
        d->m_Color = color;
        update();
    }
}

void ColorHexEdit::setDisplayAlpha(bool visible)
{
    Q_D(ColorHexEdit);
    d->m_DisplayAlpha = visible;
    d->refresh();
}

bool ColorHexEdit::displayAlpha()
{
   Q_D(ColorHexEdit);
   return d->m_DisplayAlpha;
}
