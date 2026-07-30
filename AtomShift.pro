#-------------------------------------------------
#
# Project created by QtCreator 2026-06-19T20:48:02
#
#-------------------------------------------------

QT       += core gui phonon

TARGET = AtomShift
TEMPLATE = app

# Define Version
VERSION = 1.0.0

INCLUDEPATH += src/core src/symbian

SOURCES += \
    src/core/board.cpp \
    src/core/levelloader.cpp \
    src/core/movehistory.cpp \
    src/symbian/gameboardwidget.cpp \
    src/symbian/main.cpp \
    src/symbian/mainwindow.cpp \
    src/core/gametimer.cpp \
    src/symbian/mainmenuwidget.cpp \
    src/symbian/levelpackselectwidget.cpp \
    src/core/savemanager.cpp \
    src/symbian/levelselectwidget.cpp \
    src/symbian/bondimagecache.cpp \
    src/symbian/atomimagecache.cpp \
    src/symbian/audiomanager.cpp

HEADERS += \
    src/core/atom.h \
    src/core/bond.h \
    src/core/board.h \
    src/core/leveldata.h \
    src/core/levelloader.h \
    src/core/movehistory.h \
    src/symbian/gameboardwidget.h \
    src/symbian/mainwindow.h \
    src/core/gametimer.h \
    src/symbian/mainmenuwidget.h \
    src/symbian/levelpackselectwidget.h \
    src/core/savedata.h \
    src/core/savemanager.h \
    src/symbian/levelselectwidget.h \
    src/symbian/bondimagecache.h \
    src/symbian/atomimagecache.h \
    src/symbian/audiomanager.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY =

symbian {
    TARGET.UID3 = 0xe4ec3860
    DEPLOYMENT += gameimages gamelevels gamesounds
    gameimages.sources = res/images
    gamelevels.sources = res/levels
    gamesounds.sources = res/sounds
    ICON += AS_icon.svg
    QMAKE_TARGET_COMPANY = Alireza Elahi
    vendor = \
        "%{\"$$QMAKE_TARGET_COMPANY\"}" \
        ":\"$$QMAKE_TARGET_COMPANY\""
    default_deployment.pkg_prerules += vendor
    # TARGET.CAPABILITY +=
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}
