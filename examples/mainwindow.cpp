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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QStyle>

#include <ZtWidgets/colorpicker.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    auto set_label_color = [this](const QColor& color)
    {
        QPalette palette = ui->label->palette();
        palette.setColor(ui->label->foregroundRole(), color);
        ui->label->setPalette(palette);
    };

    connect(ui->colorPickerArgb, &ColorPicker::colorChanging, this, set_label_color);
    connect(ui->colorPickerArgb, &ColorPicker::colorChanged, this, set_label_color);

    connect(ui->colorPickerRgb, &ColorPicker::colorChanging, this, set_label_color);
    connect(ui->colorPickerRgb, &ColorPicker::colorChanged, this, set_label_color);

    connect(ui->colorPickerArgb, &ColorPicker::colorChanged, ui->colorPickerRgb, &ColorPicker::updateColor);
    connect(ui->colorPickerRgb, &ColorPicker::colorChanged, ui->colorPickerArgb, &ColorPicker::updateColor);

    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}
