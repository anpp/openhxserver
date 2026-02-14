
TARGET = openhx

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += serialport

CONFIG += c++14
QMAKE_CXXFLAGS += -pie
#QMAKE_LFLAGS += -pie

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    port_dump.cpp \
    port_state.cpp \
    server/hxserver.cpp \
    server/imagedsk.cpp \
    server/images.cpp \
    server/serialport.cpp \
    settings.cpp \
    settings_dialog/images_delegate.cpp \
    settings_dialog/images_model.cpp \
    settings_dialog/settings_com_port.cpp \
    settings_dialog/settings_editor.cpp \
    settings_dialog/settings_images.cpp

HEADERS += \
    mainwindow.h \
    port_dump.h \
    port_state.h \
    server/hxserver.h \
    server/imagedsk.h \
    server/images.h \
    server/serialport.h \
    settings.h \
    settings_dialog/images_delegate.h \
    settings_dialog/images_model.h \
    settings_dialog/settings_com_port.h \
    settings_dialog/settings_editor.h \
    settings_dialog/settings_images.h

FORMS += \
    mainwindow.ui \
    port_dump.ui \
    settings_dialog/settings_com_port.ui \
    settings_dialog/settings_images.ui

TRANSLATIONS += \
    OpenHXServer_ru_RU.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    bin.qrc \
    images.qrc

