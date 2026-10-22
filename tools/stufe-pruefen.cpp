/*
 * Laedt qml/stages/Vulkan.qml so, wie die App es tut -- aber ohne Fenster:
 * Typen anmelden, Bauteil erzeugen, Groesse vor dem Fertigstellen setzen
 * (sonst startet die Stufe mit Breite 0), laufen lassen, results() lesen.
 *
 * So faellt ein Tippfehler im QML auf, ohne dass auf dem Arbeitsgeraet etwas
 * auf den Schirm kommt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QTextStream>
#include <QTimer>
#include <QVariant>
#include <QtQml>

#include "../src/GlesBench.h"
#include "../src/VulkanBench.h"

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "minimal");
    QGuiApplication app(argc, argv);
    QTextStream aus(stdout);

    qmlRegisterType<VulkanEngine>("Benchymark", 1, 0, "VulkanEngine");
    qmlRegisterType<OffscreenView>("Benchymark", 1, 0, "OffscreenView");
    qmlRegisterType<GlesEngine>("Benchymark", 1, 0, "GlesEngine");

    const QString pfad = argc > 1 ? QString::fromLocal8Bit(argv[1])
                                  : QStringLiteral("qml/stages/Vulkan.qml");
    QQmlEngine engine;
    QQmlComponent bauteil(&engine, QUrl::fromLocalFile(pfad));
    if (bauteil.isError()) {
        for (const QQmlError &e : bauteil.errors())
            aus << "QML-Fehler: " << e.toString() << endl;
        return 1;
    }

    QObject *stufe = bauteil.beginCreate(engine.rootContext());
    if (!stufe) {
        aus << "Bauteil liess sich nicht erzeugen" << endl;
        return 1;
    }
    // Groesse vor dem Fertigstellen: Component.onCompleted startet den Lauf
    const int breite = argc > 3 ? atoi(argv[3]) : 540;
    const int hoehe = argc > 4 ? atoi(argv[4]) : 960;
    stufe->setProperty("width", breite);
    stufe->setProperty("height", hoehe);
    stufe->setProperty("areaTop", 0);
    stufe->setProperty("areaBottom", hoehe);
    bauteil.completeCreate();
    if (bauteil.isError()) {
        for (const QQmlError &e : bauteil.errors())
            aus << "QML-Fehler: " << e.toString() << endl;
        return 1;
    }
    aus << "Stufe geladen: " << pfad << endl;

    QObject::connect(stufe, SIGNAL(finished()), &app, SLOT(quit()));
    const int sekunden = argc > 2 ? atoi(argv[2]) : 4;
    QTimer::singleShot(sekunden * 1000, &app, SLOT(quit()));
    app.exec();

    // wie die Seite: Bilder und laengstes Bild abholen
    QVariant quelle = stufe->property("frameSource");
    if (QObject *q = quelle.value<QObject *>()) {
        int bilder = 0, schlimmstes = 0;
        QMetaObject::invokeMethod(q, "take", Q_RETURN_ARG(int, bilder));
        QMetaObject::invokeMethod(q, "takeWorstMs", Q_RETURN_ARG(int, schlimmstes));
        aus << "Bilder in " << sekunden << " s: " << bilder
            << " (" << double(bilder) / sekunden << " fps), laengstes Bild "
            << schlimmstes << " ms" << endl;
    }

    QVariant zeilen;
    QMetaObject::invokeMethod(stufe, "results", Q_RETURN_ARG(QVariant, zeilen));
    aus << "live:    " << stufe->property("live").toString() << endl;
    for (const QVariant &z : zeilen.toList()) {
        const QVariantMap m = z.toMap();
        aus << "Ergebnis: " << m.value("key").toString() << " = "
            << m.value("value").toString() << " " << m.value("unit").toString() << endl;
    }
    delete stufe;
    return 0;
}
