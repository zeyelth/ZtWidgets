# -*- coding: utf-8 -*-
'''
Copyright (c) 2017 Victor Wåhlström

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
'''

from PyQt5.QtWidgets import QMainWindow
from PyQt5 import uic
import os


class MainWindow(QMainWindow):
    '''
    Main Window setup example with a .ui file and custom widgets
    '''
    def __init__(self, parent=None):
        '''
        constructor
        '''
        super().__init__(parent)

        ui_path = os.path.join(os.path.dirname(__file__), '..', 'forms', 'mainwindow.ui')
        uic.loadUi(ui_path, self)

        def set_label_color(color):
            palette = self.label.palette()
            palette.setColor(self.label.foregroundRole(), color)
            self.label.setPalette(palette)

        self.colorPickerArgb.colorChanging.connect(set_label_color)
        self.colorPickerArgb.colorChanged.connect(set_label_color)

        self.colorPickerRgb.colorChanging.connect(set_label_color)
        self.colorPickerRgb.colorChanged.connect(set_label_color)

        self.colorPickerArgb.colorChanged.connect(self.colorPickerRgb.updateColor)
        self.colorPickerRgb.colorChanged.connect(self.colorPickerArgb.updateColor)

        self.show()
