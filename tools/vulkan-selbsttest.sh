#!/bin/sh
# Baut und laeuft den Selbsttest der Stufe VULKAN auf dem Geraet (ohne App,
# ohne Fenster). Braucht qt5-qtcore-devel, qt5-qtdeclarative-devel,
# qt5-qtgui-devel und vulkan-headers -- alles aus dem Jolla-Repo.
set -e
cd "$(dirname "$0")/.."
MOC=$(ls /usr/lib64/qt5/bin/moc /usr/bin/moc-qt5 /usr/bin/moc 2>/dev/null | head -1)
mkdir -p bau
"$MOC" src/VulkanBench.h -o bau/moc_VulkanBench.cpp
Q=/usr/include/qt5
g++ -std=c++11 -fPIC -O2 -Wall \
    -I"$Q" -I"$Q/QtCore" -I"$Q/QtGui" -I"$Q/QtQml" -I"$Q/QtQuick" -Isrc \
    tools/vulkan-selbsttest.cpp src/VulkanBench.cpp bau/moc_VulkanBench.cpp \
    -o bau/vulkan-selbsttest \
    -lQt5Core -lQt5Gui -lQt5Qml -lQt5Quick -ldl
echo "gebaut: bau/vulkan-selbsttest"
exec bau/vulkan-selbsttest "$@"
