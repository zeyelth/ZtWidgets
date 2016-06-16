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

#include <ZtWidgets/colorhexedit.h>

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

ColorHexEdit::ColorHexEdit(QWidget *parent)
    : ColorWidgetBase(parent),
      m_Modified(false)
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setSpacing(0);

    QLabel* hash_label = new QLabel;
    hash_label->setText("#");
    layout->addWidget(hash_label);

    m_LineEdit = new QLineEdit;
    setDisplayAlpha(true);

    auto on_editing_finished = [this]()
    {
        if (m_Modified)
        {
            m_Modified = false;
            emit colorChanged(m_Color);
        }
    };

    connect(m_LineEdit, &QLineEdit::textEdited, this, &ColorHexEdit::onTextEdited);
    connect(m_LineEdit, &QLineEdit::editingFinished, this, on_editing_finished);

    layout->addWidget(m_LineEdit);
    setLayout(layout);
}

int ColorHexEdit::editWidth() const
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

void ColorHexEdit::updateColor(const QColor& color)
{
    m_LineEdit->setText(colorToString(color, displayAlpha()));
    ColorWidgetBase::updateColor(color);
}

void ColorHexEdit::onTextEdited(const QString& text)
{
    if (text.size() < m_LineEdit->maxLength())
    {
        int pos = m_LineEdit->cursorPosition();
        m_LineEdit->insert("0");
        m_LineEdit->setCursorPosition(pos);
    }

    if (m_LineEdit->text() == colorToString(m_Color, displayAlpha()))
    {
        return;
    }

    m_Modified = true;
    m_Color.setNamedColor("#"+text);
    emit colorChanging(m_Color);
}

void ColorHexEdit::setDisplayAlpha(bool visible)
{
    if (visible)
    {
        m_LineEdit->setMaxLength(8);
        m_LineEdit->setInputMask(">HHHHHHHH");
        m_LineEdit->setPlaceholderText("AARRGGBB");
        m_LineEdit->setToolTip(tr("A hexadecimal value on the form AARRGGBB:\n\nAA = alpha\nRR = red\nGG = green\nBB = blue"));
    }
    else
    {
        m_LineEdit->setMaxLength(6);
        m_LineEdit->setInputMask(">HHHHHH");
        m_LineEdit->setPlaceholderText("RRGGBB");
        m_LineEdit->setToolTip(tr("A hexadecimal value on the form RRGGBB:\n\nRR = red\nGG = green\nBB = blue"));
    }

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    m_LineEdit->setFont(font);
    m_LineEdit->setAlignment(Qt::AlignCenter);

    m_LineEdit->setFixedWidth(editWidth());
    m_LineEdit->setText(colorToString(m_Color, visible));
}

bool ColorHexEdit::displayAlpha()
{
   return m_LineEdit->maxLength() == 8;
}
