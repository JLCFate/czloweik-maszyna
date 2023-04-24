#-------------------------------------------------
#
# Project created by QtCreator 2016-10-06T21:05:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HCI_gesture1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videothread.cpp

HEADERS  += mainwindow.h \
    videothread.h

FORMS    += mainwindow.ui

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4
}
Win32 {
    INCLUDEPATH += C:\OpenCV2.2\include\
    LIBS += -LC:\OpenCV2.2\lib \
    -lopencv_core220 \
    -lopencv_highgui220 \
    -lopencv_imgproc220 \
    -lopencv_features2d220 \
    -lopencv_calib3d220
}
