#
#  Fast plotter adapted from QCustomPlot Plot Examples
#

# This doesn't work. Both the generated Makefiles don't build.
# CONFIG = debug_and_release

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = fastPlot
TEMPLATE = app

QMAKE_CXXFLAGS = -std=c++11

SOURCES += main.cpp\
           mainwindow.cpp \
           reader.cpp \
           ../../Qt/qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.h \
         ../../Qt/qcustomplot/qcustomplot.h

FORMS    += mainwindow.ui

