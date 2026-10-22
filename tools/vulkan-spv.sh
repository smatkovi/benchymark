#!/bin/sh
# Uebersetzt die beiden Vulkan-Shader zu SPIR-V und legt sie als Kopfdatei
# in den Baum (src/vulkan-spv.h). Auf dem Telefon gibt es keinen glslc, auf
# dem Baurechner schon -- darum laeuft das hier ueber ssh, so wie die .qsb
# der Uhrenfassung auch vorgebaut mitgeliefert werden.
#
#   tools/vulkan-spv.sh [baurechner]
set -e
WIRT=${1:-arch}
HIER=$(cd "$(dirname "$0")/.." && pwd)
FERN=\$HOME/.cache/benchymark-spv

scp -q "$HIER/src/vulkan-full.vert" "$HIER/src/vulkan-cloud.frag" "$WIRT:/tmp/" 2>/dev/null || {
    ssh "$WIRT" "mkdir -p ~/.cache/benchymark-spv"
    scp -q "$HIER/src/vulkan-full.vert" "$HIER/src/vulkan-cloud.frag" "$WIRT:~/.cache/benchymark-spv/"
}
ssh "$WIRT" 'cd ~/.cache/benchymark-spv 2>/dev/null || cd /tmp; \
    glslc -O --target-env=vulkan1.0 vulkan-full.vert  -mfmt=c -o voll.spv.txt; \
    glslc -O --target-env=vulkan1.0 vulkan-cloud.frag -mfmt=c -o wolke.spv.txt; \
    echo "// Erzeugt von tools/vulkan-spv.sh -- nicht von Hand aendern."; \
    echo "#ifndef VULKAN_SPV_H"; echo "#define VULKAN_SPV_H"; echo; \
    echo "#include <stdint.h>"; echo; \
    echo "static const uint32_t kSpvVoll[] ="; cat voll.spv.txt; echo ";"; echo; \
    echo "static const uint32_t kSpvWolke[] ="; cat wolke.spv.txt; echo ";"; echo; \
    echo "#endif"' > "$HIER/src/vulkan-spv.h"
wc -c "$HIER/src/vulkan-spv.h"
