
TARGET = openhx

QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 5): QT += statemachine


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

CONFIG += c++17
#QMAKE_CXXFLAGS += -pie
#QMAKE_LFLAGS += -pie

# На Qt6 пока только под Андроид с дин. сборкой
greaterThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -stdlib=libc++
greaterThan(QT_MAJOR_VERSION, 5): LIBS += -lc++_shared


# Если сборка НЕ под андроид — добавляем виджеты
!android {
    QT += widgets

    QT += qml quick quickcontrols2 #для отладки на десктопе

    SOURCES += mainwindow.cpp
    HEADERS += mainwindow.h
    FORMS   += mainwindow.ui
}

# Если сборка ПОД андроид — добавляем QML
android {
    QT += qml quick quickcontrols2
    QT += widgets

    SOURCES += mainwindow.cpp
    HEADERS += mainwindow.h
    FORMS   += mainwindow.ui

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml \
    android/src/hx/openhx/helper/SerialHelper.java

}


SOURCES += \
    main.cpp \
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
    settings_dialog/settings_images.cpp \
    settings_dialog/settings_misc.cpp

HEADERS += \
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
    settings_dialog/settings_images.h \
    settings_dialog/settings_misc.h \
    settingswrapper.h

FORMS += \
    port_dump.ui \
    settings_dialog/settings_com_port.ui \
    settings_dialog/settings_images.ui \
    settings_dialog/settings_misc.ui


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
    images.qrc \
    qml.qrc

DISTFILES += \
    qml/main.qml




