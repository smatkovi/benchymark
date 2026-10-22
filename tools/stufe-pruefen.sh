#!/bin/sh
# Prueft qml/stages/Vulkan.qml auf dem Geraet, ohne Fenster.
set -e
cd "$(dirname "$0")/.."
MOC=$(ls /usr/lib64/qt5/bin/moc /usr/bin/moc-qt5 /usr/bin/moc 2>/dev/null | head -1)
mkdir -p bau bau/lib
# libGLESv2.so fehlt auf dem Geraet, nur libGLESv2.so.2 liegt da
[ -e bau/lib/libGLESv2.so ] || ln -s /usr/lib64/libGLESv2.so.2 bau/lib/libGLESv2.so
"$MOC" src/VulkanBench.h -o bau/moc_VulkanBench.cpp
"$MOC" src/GlesBench.h -o bau/moc_GlesBench.cpp
Q=/usr/include/qt5
g++ -std=c++11 -fPIC -O2 -Wall \
    -I"$Q" -I"$Q/QtCore" -I"$Q/QtGui" -I"$Q/QtQml" -I"$Q/QtQuick" -Isrc -Iextern \
    tools/stufe-pruefen.cpp src/VulkanBench.cpp src/GlesBench.cpp \
    bau/moc_VulkanBench.cpp bau/moc_GlesBench.cpp \
    -o bau/stufe-pruefen -lQt5Core -lQt5Gui -lQt5Qml -lQt5Quick -lEGL -lGLESv2 -ldl \
    -L bau/lib
exec bau/stufe-pruefen "$@"
