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

#include "../src/VulkanBench.h"

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "minimal");
    QGuiApplication app(argc, argv);
    QTextStream aus(stdout);

    qmlRegisterType<VulkanEngine>("Benchymark", 1, 0, "VulkanEngine");
    qmlRegisterType<VulkanView>("Benchymark", 1, 0, "VulkanView");

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
    stufe->setProperty("width", 540);
    stufe->setProperty("height", 960);
    stufe->setProperty("areaTop", 60);
    stufe->setProperty("areaBottom", 960);
    bauteil.completeCreate();
    if (bauteil.isError()) {
        for (const QQmlError &e : bauteil.errors())
            aus << "QML-Fehler: " << e.toString() << endl;
        return 1;
    }
    aus << "Stufe geladen: " << pfad << endl;

    QObject::connect(stufe, SIGNAL(finished()), &app, SLOT(quit()));
    QTimer::singleShot(30000, &app, SLOT(quit()));   // Notbremse
    app.exec();

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
