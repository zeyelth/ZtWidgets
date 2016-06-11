TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = ZtWidgets \
          plugins \
          examples

plugins.depends = ZtWidgets
examples.depends = ZtWidgets
