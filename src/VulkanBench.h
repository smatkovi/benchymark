/*
 * SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * SailfishOS: die Stufe VULKAN. Sie rechnet denselben Shader wie SHADER HEAVY
 * (qml/stages/ShaderHigh.qml), aber ueber Vulkan und ohne Fenster: in ein Bild
 * im Speicher, gemessen mit Vulkans eigenen Zeitstempeln. Damit steht neben
 * der Zahl, die QtQuick ueber GLES schafft, die Zahl desselben Bildes auf dem
 * anderen Weg.
 *
 * libvulkan wird zur Laufzeit geladen (dlopen), nicht gebunden: auf Geraeten
 * ohne libhybris-libvulkan soll die App laufen und die Stufe schlicht sagen,
 * dass es hier kein Vulkan gibt.
 */
#ifndef VULKANBENCH_H
#define VULKANBENCH_H

#include <QAtomicInt>
#include <QElapsedTimer>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QQuickPaintedItem>
#include <QString>
#include <QVector>

#include "SystemBench.h"   // FunctionThread

// Zeigt, was Vulkan gezeichnet hat. Das Bild kommt vom Arbeitsfaden, einmal
// in der Sekunde; gemalt wird es auf dem Oberflaechenfaden.
class OffscreenView : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit OffscreenView(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;
    // aus jedem Faden aufrufbar
    void setPicture(const QImage &bild);
    // Jeden n-ten Punkt nehmen statt zu glaetten: QImage::scaled mit
    // SmoothTransformation kostet auf 1,8 Mpix ueber hundert Millisekunden,
    // und die landen als laengstes Bild in der Messung. Die Vorschau muss
    // nur zeigen, dass gezeichnet wird.
    static QImage verkleinern(const uchar *punkte, int w, int h, int teiler, bool spiegeln);
private:
    mutable QMutex m_lock;
    QImage m_bild;
};

class VulkanEngine : public QObject
{
    Q_OBJECT
    // -1 kein Vulkan, 0 ruht, 1 heizt vor, 2 misst, 3 fertig
    Q_PROPERTY(int phase READ phase NOTIFY changed)
    Q_PROPERTY(QString device READ device NOTIFY changed)
    Q_PROPERTY(QString driver READ driver NOTIFY changed)
    Q_PROPERTY(QString api READ api NOTIFY changed)
    // warum es nicht geht, in einem Satz
    Q_PROPERTY(QString reason READ reason NOTIFY changed)
    // Median der gemessenen Bilder in ms, und was das an Bildern je Sekunde
    // waere (ohne Bildsynchronisierung -- es haengt kein Schirm daran)
    Q_PROPERTY(qreal gpuMs READ gpuMs NOTIFY changed)
    Q_PROPERTY(qreal bestMs READ bestMs NOTIFY changed)
    Q_PROPERTY(qreal fps READ fps NOTIFY changed)
    Q_PROPERTY(int frames READ frames NOTIFY changed)
    Q_PROPERTY(OffscreenView *view READ view WRITE setView)
public:
    explicit VulkanEngine(QObject *parent = nullptr);
    ~VulkanEngine() override;

    int phase() const { QMutexLocker l(&m_lock); return m_phase; }
    QString device() const { QMutexLocker l(&m_lock); return m_device; }
    QString driver() const { QMutexLocker l(&m_lock); return m_driver; }
    QString api() const { QMutexLocker l(&m_lock); return m_api; }
    QString reason() const { QMutexLocker l(&m_lock); return m_reason; }
    qreal gpuMs() const { QMutexLocker l(&m_lock); return m_gpuMs; }
    qreal bestMs() const { QMutexLocker l(&m_lock); return m_bestMs; }
    qreal fps() const { QMutexLocker l(&m_lock); return m_gpuMs > 0 ? 1000.0 / m_gpuMs : 0.0; }
    int frames() const { QMutexLocker l(&m_lock); return m_frames; }
    OffscreenView *view() const { return m_view; }
    void setView(OffscreenView *v) { m_view = v; }

    // Breite und Hoehe in Pixeln, Vorheizen und Messen in Millisekunden.
    // measureMs <= 0 heisst: laufen, bis stop() kommt -- so macht es die
    // Stufe, die wie jede andere von der Seite nach ihrer Zeit beendet wird.
    Q_INVOKABLE void start(int w, int h, int preheatMs = 0, int measureMs = 0);
    Q_INVOKABLE void stop();

    // Dieselben zwei Fragen wie beim FrameCounter, damit die Seite eine
    // Vulkan-Stufe genauso abfragen kann wie eine gezeichnete: Bilder seit
    // dem letzten Mal, und das laengste Bild darin.
    Q_INVOKABLE int take() { return m_seit.fetchAndStoreRelaxed(0); }
    Q_INVOKABLE int takeWorstMs()
    {
        QMutexLocker lock(&m_lock);
        const int w = int(m_worstNs / 1000000);
        m_worstNs = 0;
        return w;
    }

signals:
    void changed();

private:
    void work(int w, int h, int preheatMs, int measureMs);
    void melde(int phase, const QString &grund = QString());

    mutable QMutex m_lock;
    FunctionThread *m_thread = nullptr;
    QAtomicInt m_stop;
    QPointer<OffscreenView> m_view;
    QAtomicInt m_seit;
    qint64 m_worstNs = 0;
    int m_phase = 0;
    int m_frames = 0;
    qreal m_gpuMs = 0;
    qreal m_bestMs = 0;
    QString m_device, m_driver, m_api, m_reason;
};

#endif // VULKANBENCH_H
