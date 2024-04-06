QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    yolov5.cpp

HEADERS += \
    mainwindow.h \
    yolov5.h

FORMS += \
    mainwindow.ui

win32
{
    # INCLUDEPATH += E:\Environment\opencv452\include\
    #                E:\Environment\opencv452\include\opencv2
    # LIBS += E:\Environment\opencv452\x64\mingw\lib\libopencv*

    INCLUDEPATH += E:\Environment\opencv460-cuda\include\
                   E:\Environment\opencv460-cuda\include\opencv2

}
msvc {
QMAKE_CFLAGS += /utf-8
QMAKE_CXXFLAGS += /utf-8
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
