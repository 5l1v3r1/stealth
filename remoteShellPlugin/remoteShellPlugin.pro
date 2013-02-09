TARGET=remoteshellplugin
TEMPLATE = lib
#CONFIG += console
CONFIG -= qt

QMAKE_CXXFLAGS_RTTI_ON -= -frtti
QMAKE_CXXFLAGS_RTTI_OFF = -fno-rtti
QMAKE_CXXFLAGS += -nostdlib -fno-exceptions -fno-rtti -fno-omit-frame-pointer -std=gnu++0x


DEFINES +=BUILD_DLL _WIN32
DEFINES -= UNICODE QT_LARGEFILE_SUPPORT

QMAKE_LFLAGS +=  -nostartfiles -fno-exceptions -nostdlib

#LIBS += -L$$quote(../ArkLib) -lArkLib -lntdll -lWs2_32 --static
LIBS += -lntdll -lkernel32 -lWs2_32 -lmsvcrt -luser32 -lAdvapi32

SOURCES +=\
	../PluginManager/rtl.cpp \
	main.cpp \
	remoteshell.cpp

HEADERS += \
	../PluginManager/plugininterface.h\
	main.h \
	remoteshell.h

