/*
 * SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * SailfishOS: die Stufen GLES 2 und GLES 3 -- derselbe Shader wie SHADER
 * HEAVY, aber in einem eigenen Kontext und ohne Fenster in einen Rahmenpuffer
 * im Speicher gezeichnet. Damit steht neben der Zahl, die QtQuick auf dem
 * Schirm schafft, und neben der Vulkan-Zahl auch die des blanken GLES-Weges,
 * einmal mit ES 2 und einmal mit ES 3.
 *
 * Der Kontext ist ein eigener (kein Teilen mit Qts Zeichenfaden), die
 * EGL-Anzeige aber dieselbe wie die der App -- **nie eglTerminate() rufen**,
 * das wuerde Qt die Anzeige unter den Fuessen wegziehen.
 */
#ifndef GLESBENCH_H
#define GLESBENCH_H

#include <QAtomicInt>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QString>

#include "SystemBench.h"   // FunctionThread
#include "VulkanBench.h"   // OffscreenView

class GlesEngine : public QObject
{
    Q_OBJECT
    // -1 geht nicht, 0 ruht, 2 misst, 3 fertig
    Q_PROPERTY(int phase READ phase NOTIFY changed)
    // 2 oder 3; was wirklich herauskam, steht in glVersion
    Q_PROPERTY(int wanted READ wanted WRITE setWanted NOTIFY changed)
    Q_PROPERTY(QString renderer READ renderer NOTIFY changed)
    Q_PROPERTY(QString glVersion READ glVersion NOTIFY changed)
    Q_PROPERTY(QString reason READ reason NOTIFY changed)
    Q_PROPERTY(bool gpuTimed READ gpuTimed NOTIFY changed)
    Q_PROPERTY(qreal gpuMs READ gpuMs NOTIFY changed)
    Q_PROPERTY(OffscreenView *view READ view WRITE setView)
public:
    explicit GlesEngine(QObject *parent = nullptr);
    ~GlesEngine() override;

    int phase() const { QMutexLocker l(&m_lock); return m_phase; }
    int wanted() const { return m_wanted; }
    void setWanted(int v) { m_wanted = v; }
    QString renderer() const { QMutexLocker l(&m_lock); return m_renderer; }
    QString glVersion() const { QMutexLocker l(&m_lock); return m_glVersion; }
    QString reason() const { QMutexLocker l(&m_lock); return m_reason; }
    bool gpuTimed() const { QMutexLocker l(&m_lock); return m_gpuTimed; }
    qreal gpuMs() const { QMutexLocker l(&m_lock); return m_gpuMs; }
    OffscreenView *view() const { return m_view; }
    void setView(OffscreenView *v) { m_view = v; }

    Q_INVOKABLE void start(int w, int h);
    Q_INVOKABLE void stop();

    // wie beim FrameCounter: Bilder seit dem letzten Mal, laengstes darin
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
    void work(int w, int h);
    void melde(int phase, const QString &grund = QString());

    mutable QMutex m_lock;
    FunctionThread *m_thread = nullptr;
    QAtomicInt m_stop;
    QAtomicInt m_seit;
    QPointer<OffscreenView> m_view;
    qint64 m_worstNs = 0;
    int m_phase = 0;
    int m_wanted = 3;
    bool m_gpuTimed = false;
    qreal m_gpuMs = 0;
    QString m_renderer, m_glVersion, m_reason;
};

#endif // GLESBENCH_H
