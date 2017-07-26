INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dfileiconprovider.h \
    $$PWD/dthumbnailprovider.h \
    $$PWD/dwindowmanagerhelper.h \
    $$PWD/dwidgetutil.h

SOURCES += \
    $$PWD/dfileiconprovider.cpp \
    $$PWD/dthumbnailprovider.cpp \
    $$PWD/dwindowmanagerhelper.cpp \
    $$PWD/dwidgetutil.cpp

packagesExist(gtk+-2.0) {
    DEFINES += USE_GTK_PLUS_2_0
    INCLUDE_PATH = $$system(pkg-config --cflags-only-I gtk+-2.0)
    INCLUDEPATH += $$replace(INCLUDE_PATH, -I, )
}

includes.files += $$PWD/*.h \
            $$PWD/DWidgetUtil \
            $$PWD/DThumbnailProvider \
            $$PWD/DFileIconProvider \
            $$PWD/DWindowManagerHelper
