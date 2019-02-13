#-------------------------------------------------
#
# Project created by QtCreator 2019-02-11T11:18:10
#
#-------------------------------------------------

QT       += core gui multimedia network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_ICONS = logo.ico
VERSION = 1.0.0.0

QMAKE_TARGET_COMPANY = modern technical solutions
QMAKE_TARGET_PRODUCT = Phone 1.1
QMAKE_TARGET_DESCRIPTION = VOIP Application
QMAKE_TARGET_COPYRIGHT = modern technical solutions

TARGET = GATE
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

DEFINES += HAVE_CONFIG_H

INCLUDEPATH += ../GATE/speex-1.2.0/include/
INCLUDEPATH += ../GATE/speex-1.2.0/libspeex/
INCLUDEPATH += ../GATE/speex-1.2.0/src/
INCLUDEPATH += ../GATE/speex-1.2.0/win32/

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        speex-1.2.0/libspeex/bits.c \
        speex-1.2.0/libspeex/cb_search.c \
        speex-1.2.0/libspeex/exc_5_64_table.c \
        speex-1.2.0/libspeex/exc_5_256_table.c \
        speex-1.2.0/libspeex/exc_8_128_table.c \
        speex-1.2.0/libspeex/exc_10_16_table.c \
        speex-1.2.0/libspeex/exc_10_32_table.c \
        speex-1.2.0/libspeex/exc_20_32_table.c \
        speex-1.2.0/libspeex/filters.c \
        speex-1.2.0/libspeex/gain_table.c \
        speex-1.2.0/libspeex/gain_table_lbr.c \
        speex-1.2.0/libspeex/hexc_10_32_table.c \
        speex-1.2.0/libspeex/hexc_table.c \
        speex-1.2.0/libspeex/high_lsp_tables.c \
        speex-1.2.0/libspeex/kiss_fft.c \
        speex-1.2.0/libspeex/kiss_fftr.c \
        speex-1.2.0/libspeex/lpc.c \
        speex-1.2.0/libspeex/lsp.c \
        speex-1.2.0/libspeex/lsp_tables_nb.c \
        speex-1.2.0/libspeex/ltp.c \
        speex-1.2.0/libspeex/modes.c \
        speex-1.2.0/libspeex/modes_wb.c \
        speex-1.2.0/libspeex/nb_celp.c \
        speex-1.2.0/libspeex/quant_lsp.c \
        speex-1.2.0/libspeex/sb_celp.c \
        speex-1.2.0/libspeex/smallft.c \
        speex-1.2.0/libspeex/speex.c \
        speex-1.2.0/libspeex/speex_callbacks.c \
        speex-1.2.0/libspeex/speex_header.c \
        speex-1.2.0/libspeex/stereo.c \
        speex-1.2.0/libspeex/vbr.c \
        speex-1.2.0/libspeex/vq.c \
        speex-1.2.0/libspeex/window.c \
    audioinputdevice.cpp \
    checksum.cpp \
    udpworker.cpp \
    udpcontroller.cpp \
    qcustomplot.cpp

HEADERS += \
        mainwindow.h \
    audioinputdevice.h \
    checksum.h \
    udpworker.h \
    udpcontroller.h \
    qcustomplot.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
