/*
 * Prueft die Stufe VULKAN ohne App und ohne Fenster: baut die Maschine auf,
 * misst zwei Sekunden und schreibt, was herauskommt. Damit laesst sich die
 * Vulkan-Seite pruefen, ohne auf dem Geraet ein Fenster zu oeffnen.
 *
 * Bauen und laufen lassen: tools/vulkan-selbsttest.sh
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QCoreApplication>
#include <QTextStream>
#include <cstdlib>

#include "../src/VulkanBench.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream aus(stdout);
    const int w = argc > 1 ? atoi(argv[1]) : 1080;
    const int h = argc > 2 ? atoi(argv[2]) : 1800;
    const int ms = argc > 3 ? atoi(argv[3]) : 2000;

    VulkanEngine e;
    QObject::connect(&e, &VulkanEngine::changed, &app, [&] {
        if (e.phase() == -1) {
            aus << "kein Vulkan: " << e.reason() << endl;
            app.quit();
        } else if (e.phase() == 3) {
            aus << "Geraet:  " << e.device() << ", API " << e.api()
                << ", Treiber " << e.driver() << endl;
            aus << "Flaeche: " << w << "x" << h << endl;
            aus << "Bilder:  " << e.frames() << endl;
            aus << "GPU:     " << QString::number(e.gpuMs(), 'f', 3) << " ms (Median), "
                << QString::number(e.bestMs(), 'f', 3) << " ms (bestes), "
                << QString::number(e.fps(), 'f', 1) << " Bilder/s" << endl;
            app.quit();
        }
    });
    e.start(w, h, 500, ms);
    return app.exec();
}
