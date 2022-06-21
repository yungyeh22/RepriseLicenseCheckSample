TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

# Use #pragma comment(lib, "<lib file name>") because we might use this module in other compiler
# LIBS += shell32.lib ole32.lib crypt32.lib


SOURCES += \
        LicenseCheck.cpp \
        Util/LibraryLoader/DynamicLoadLibrary.cpp \
        Util/LibraryLoader/ILibrary.cpp \
        Util/Reprise/RepriseDynamicLoadLibrary.cpp \
        Util/Reprise/RepriseOperationManager.cpp \
        main.cpp

HEADERS += \
    LicenseCheck.h \
    Util/LibraryLoader/DynamicLoadLibrary.h \
    Util/LibraryLoader/ILibrary.h \
    Util/Reprise/RepriseDynamicLoadLibrary.h \
    Util/Reprise/RepriseOperationManager.h
