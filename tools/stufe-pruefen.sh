#!/bin/sh
# Prueft qml/stages/Vulkan.qml auf dem Geraet, ohne Fenster.
set -e
cd "$(dirname "$0")/.."
MOC=$(ls /usr/lib64/qt5/bin/moc /usr/bin/moc-qt5 /usr/bin/moc 2>/dev/null | head -1)
mkdir -p bau
"$MOC" src/VulkanBench.h -o bau/moc_VulkanBench.cpp
Q=/usr/include/qt5
g++ -std=c++11 -fPIC -O2 -Wall \
    -I"$Q" -I"$Q/QtCore" -I"$Q/QtGui" -I"$Q/QtQml" -I"$Q/QtQuick" -Isrc \
    tools/stufe-pruefen.cpp src/VulkanBench.cpp bau/moc_VulkanBench.cpp \
    -o bau/stufe-pruefen -lQt5Core -lQt5Gui -lQt5Qml -lQt5Quick -ldl
exec bau/stufe-pruefen "$@"
