QT       += core gui
QT       += serialport
QT       += network


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

RC_ICONS= favicon.ico

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += DEBUG_MODE
DEFINES += MES_TEST

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MesManager/MesManager.cpp \
    common/mainhandler.cpp \
    gasTightManager/SerialHelper.cpp \
    gasTightManager/gastight.c \
    gasTightManager/gastightmanager.cpp \
    gasTightManager/serial.c \
    main.cpp \
    mainwin.cpp \
    para/Constant.cpp \
    subwinsetting.cpp

HEADERS += \
    MesManager/MesManager.h \
    common/SimpleThreadQueue.h \
    common/ThreadMsgQueuePtr.h \
    common/log.h \
    common/mainhandler.h \
    gasTightManager/GasTightcallback.h \
    gasTightManager/SerialHelper.h \
    gasTightManager/gastight.h \
    gasTightManager/gastightmanager.h \
    gasTightManager/serial.h \
    mainwin.h \
    para/Constant.h \
    subwinsetting.h

FORMS += \
    mainwin.ui \
    subwinsetting.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
