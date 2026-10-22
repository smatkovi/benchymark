/*
 * SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "GlesBench.h"

#include <QElapsedTimer>
#include <QImage>
#include <QMetaObject>
#include <QVector>
#include <algorithm>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace {

// Der Fragment-Shader von SHADER HEAVY (qml/stages/ShaderHigh.qml), Zeichen
// fuer Zeichen -- bis auf qt_Opacity, das es hier nicht gibt. Die Koepfe
// unterscheiden ES 2 und ES 3; die Genauigkeitsangabe ist dieselbe wie in der
// QML-Stufe, sonst vergliche man fp16 mit fp32 statt die Wege.
const char *kKopfEckpunkt(bool es3)
{
    return es3 ? "#version 300 es\n#define EIN in\n#define RAUS out\n"
               : "#define EIN attribute\n#define RAUS varying\n";
}

const char *kKopfFragment(bool es3)
{
    return es3 ? "#version 300 es\n"
                 "precision highp float;\n"
                 "#define EIN in\n"
                 "out vec4 ausfarbe;\n#define FARBE ausfarbe\n"
               : "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
                 "precision highp float;\n"
                 "#else\n"
                 "precision mediump float;\n"
                 "#endif\n"
                 "#define EIN varying\n"
                 "#define FARBE gl_FragColor\n";
}

const char *kEckpunkt =
    "EIN vec2 ort;\n"
    "RAUS vec2 uv;\n"
    "void main() { uv = ort; gl_Position = vec4(ort * 2.0 - 1.0, 0.0, 1.0); }\n";

const char *kFragment =
    "EIN vec2 uv;\n"
    "uniform float t;\n"
    "float hash(vec2 p)\n"
    "{\n"
    "    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);\n"
    "}\n"
    "float noise(vec2 p)\n"
    "{\n"
    "    vec2 i = floor(p);\n"
    "    vec2 f = fract(p);\n"
    "    vec2 u = f * f * (3.0 - 2.0 * f);\n"
    "    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),\n"
    "               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);\n"
    "}\n"
    "float fbm(vec2 p)\n"
    "{\n"
    "    float v = 0.0;\n"
    "    float a = 0.5;\n"
    "    for (int i = 0; i < 7; ++i) {\n"
    "        v += a * noise(p);\n"
    "        p *= 2.03;\n"
    "        a *= 0.5;\n"
    "    }\n"
    "    return v;\n"
    "}\n"
    "void main()\n"
    "{\n"
    "    vec2 p = uv * 2.0 - 1.0;\n"
    "    float r = length(p);\n"
    "    vec2 q = vec2(fbm(p * 3.0 + t * 0.090), fbm(p * 3.0 - t * 0.066));\n"
    "    vec2 s = vec2(fbm(p * 3.0 + q * 2.5 + t * 0.042),\n"
    "                  fbm(p * 3.0 + q * 2.5 - t * 0.054));\n"
    "    float v = fbm(p * 4.0 + s * 3.0);\n"
    "    vec3 col = mix(vec3(0.05, 0.10, 0.25), vec3(0.35, 0.85, 0.95), v);\n"
    "    col = mix(col, vec3(0.95, 0.55, 0.25), clamp(s.x * 0.6, 0.0, 1.0));\n"
    "    col *= mix(0.70, 1.0, smoothstep(1.04, 0.66, r));\n"
    "    FARBE = vec4(col, 1.0);\n"
    "}\n";

GLuint stufeBauen(GLenum art, const char *kopf, const char *rumpf, QString *fehler)
{
    const char *teile[2] = { kopf, rumpf };
    GLuint s = glCreateShader(art);
    glShaderSource(s, 2, teile, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048] = { 0 };
        glGetShaderInfoLog(s, sizeof log - 1, nullptr, log);
        *fehler = QStringLiteral("Shader: %1").arg(QString::fromLatin1(log).simplified().left(120));
        glDeleteShader(s);
        return 0;
    }
    return s;
}

} // namespace

GlesEngine::GlesEngine(QObject *parent) : QObject(parent) {}

GlesEngine::~GlesEngine() { stop(); }

void GlesEngine::melde(int phase, const QString &grund)
{
    {
        QMutexLocker lock(&m_lock);
        m_phase = phase;
        if (!grund.isEmpty())
            m_reason = grund;
    }
    QMetaObject::invokeMethod(this, "changed", Qt::QueuedConnection);
}

void GlesEngine::start(int w, int h)
{
    stop();
    if (w < 16 || h < 16) {
        melde(-1, QStringLiteral("Flaeche zu klein"));
        return;
    }
    m_stop.store(0);
    m_thread = new FunctionThread([this, w, h] { work(w, h); });
    m_thread->start();
}

void GlesEngine::stop()
{
    m_stop.store(1);
    if (m_thread) {
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
}

void GlesEngine::work(int w, int h)
{
    const bool es3 = m_wanted >= 3;

    // Dieselbe Anzeige wie die App (Wayland). eglInitialize zaehlt mit, ein
    // zweiter Aufruf stoert nicht -- eglTerminate steht hier bewusst
    // nirgends, es wuerde Qt die Anzeige schliessen.
    EGLDisplay anzeige = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (anzeige == EGL_NO_DISPLAY) {
        melde(-1, QStringLiteral("keine EGL-Anzeige"));
        return;
    }
    EGLint ma = 0, mi = 0;
    if (!eglInitialize(anzeige, &ma, &mi)) {
        melde(-1, QStringLiteral("eglInitialize 0x%1").arg(eglGetError(), 0, 16));
        return;
    }

    EGLint cattr[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                       EGL_RENDERABLE_TYPE, es3 ? 0x00000040 /* ES3 */ : EGL_OPENGL_ES2_BIT,
                       EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
                       EGL_ALPHA_SIZE, 8, EGL_NONE };
    EGLConfig cfg;
    EGLint anz = 0;
    if (!eglChooseConfig(anzeige, cattr, &cfg, 1, &anz) || !anz) {
        melde(-1, QStringLiteral("keine Pufferflaechen-Belegung fuer ES %1").arg(es3 ? 3 : 2));
        return;
    }
    EGLint pattr[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
    EGLSurface flaeche = eglCreatePbufferSurface(anzeige, cfg, pattr);
    EGLint kattr[] = { EGL_CONTEXT_CLIENT_VERSION, es3 ? 3 : 2, EGL_NONE };
    EGLContext kontext = eglCreateContext(anzeige, cfg, EGL_NO_CONTEXT, kattr);
    if (flaeche == EGL_NO_SURFACE || kontext == EGL_NO_CONTEXT ||
        !eglMakeCurrent(anzeige, flaeche, flaeche, kontext)) {
        melde(-1, QStringLiteral("kein ES-%1-Kontext").arg(es3 ? 3 : 2));
        if (kontext != EGL_NO_CONTEXT) eglDestroyContext(anzeige, kontext);
        if (flaeche != EGL_NO_SURFACE) eglDestroySurface(anzeige, flaeche);
        return;
    }

    const auto text = [](GLenum name) {
        const GLubyte *v = glGetString(name);
        return v ? QString::fromLatin1(reinterpret_cast<const char *>(v)) : QString();
    };
    const QString erweiterungen = text(GL_EXTENSIONS);
    void (*glGenQueriesEXT)(GLsizei, GLuint *) = nullptr;
    void (*glBeginQueryEXT)(GLenum, GLuint) = nullptr;
    void (*glEndQueryEXT)(GLenum) = nullptr;
    void (*glGetQueryObjectui64vEXT)(GLuint, GLenum, quint64 *) = nullptr;
    GLuint zeitfrage = 0;
    if (erweiterungen.contains(QLatin1String("GL_EXT_disjoint_timer_query"))) {
        glGenQueriesEXT = reinterpret_cast<void (*)(GLsizei, GLuint *)>(eglGetProcAddress("glGenQueriesEXT"));
        glBeginQueryEXT = reinterpret_cast<void (*)(GLenum, GLuint)>(eglGetProcAddress("glBeginQueryEXT"));
        glEndQueryEXT = reinterpret_cast<void (*)(GLenum)>(eglGetProcAddress("glEndQueryEXT"));
        glGetQueryObjectui64vEXT = reinterpret_cast<void (*)(GLuint, GLenum, quint64 *)>(
                                       eglGetProcAddress("glGetQueryObjectui64vEXT"));
        if (glGenQueriesEXT && glBeginQueryEXT && glEndQueryEXT && glGetQueryObjectui64vEXT)
            glGenQueriesEXT(1, &zeitfrage);
    }
    {
        QMutexLocker lock(&m_lock);
        m_renderer = text(GL_RENDERER);
        m_glVersion = text(GL_VERSION);
        m_gpuTimed = zeitfrage != 0;
    }

    // Ziel: ein Rahmenpuffer im Speicher, so gross wie die Stufenflaeche
    GLuint bildtextur = 0, rahmen = 0;
    glGenTextures(1, &bildtextur);
    glBindTexture(GL_TEXTURE_2D, bildtextur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenFramebuffers(1, &rahmen);
    glBindFramebuffer(GL_FRAMEBUFFER, rahmen);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bildtextur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        melde(-1, QStringLiteral("Rahmenpuffer unvollstaendig"));
        eglMakeCurrent(anzeige, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(anzeige, kontext);
        eglDestroySurface(anzeige, flaeche);
        return;
    }
    glViewport(0, 0, w, h);

    QString fehler;
    GLuint ve = stufeBauen(GL_VERTEX_SHADER, kKopfEckpunkt(es3), kEckpunkt, &fehler);
    GLuint fr = ve ? stufeBauen(GL_FRAGMENT_SHADER, kKopfFragment(es3), kFragment, &fehler) : 0;
    GLuint programm = 0;
    if (ve && fr) {
        programm = glCreateProgram();
        glAttachShader(programm, ve);
        glAttachShader(programm, fr);
        glBindAttribLocation(programm, 0, "ort");
        glLinkProgram(programm);
        GLint ok = 0;
        glGetProgramiv(programm, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[2048] = { 0 };
            glGetProgramInfoLog(programm, sizeof log - 1, nullptr, log);
            fehler = QStringLiteral("Programm: %1").arg(QString::fromLatin1(log).simplified().left(120));
            programm = 0;
        }
    }
    if (!programm) {
        melde(-1, fehler.isEmpty() ? QStringLiteral("Shader faellt durch") : fehler);
        eglMakeCurrent(anzeige, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(anzeige, kontext);
        eglDestroySurface(anzeige, flaeche);
        return;
    }
    glUseProgram(programm);
    const GLint ortZeit = glGetUniformLocation(programm, "t");

    static const GLfloat dreieck[] = { 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 2.0f };
    GLuint eckpuffer = 0;
    glGenBuffers(1, &eckpuffer);
    glBindBuffer(GL_ARRAY_BUFFER, eckpuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof dreieck, dreieck, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    QVector<uchar> lese(size_t(w) * h * 4);
    QElapsedTimer uhr;
    uhr.start();
    QElapsedTimer vorschau;
    vorschau.start();
    melde(2);

    while (!m_stop.load()) {
        // t wie in SHADER HEAVY: eine Einheit je Sekunde
        const float t = float(uhr.elapsed()) / 1000.0f;
        QElapsedTimer wand;
        wand.start();
        if (zeitfrage)
            glBeginQueryEXT(0x88BF /* GL_TIME_ELAPSED_EXT */, zeitfrage);
        glClearColor(0.02f, 0.04f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1f(ortZeit, t);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        if (zeitfrage)
            glEndQueryEXT(0x88BF);
        glFinish();
        double ms = double(wand.nsecsElapsed()) / 1.0e6;
        if (zeitfrage) {
            quint64 ns = 0;
            glGetQueryObjectui64vEXT(zeitfrage, 0x8866 /* GL_QUERY_RESULT */, &ns);
            if (ns)
                ms = double(ns) / 1.0e6;
        }

        m_seit.ref();
        {
            QMutexLocker lock(&m_lock);
            if (qint64(ms * 1.0e6) > m_worstNs)
                m_worstNs = qint64(ms * 1.0e6);
            m_gpuMs = ms;
        }

        if (m_view && vorschau.elapsed() >= 1000) {
            vorschau.restart();
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, lese.data());
            // GL faengt unten an: beim Verkleinern gleich spiegeln
            m_view->setPicture(OffscreenView::verkleinern(lese.constData(), w, h, 4, true));
        }
    }

    glDeleteBuffers(1, &eckpuffer);
    glDeleteProgram(programm);
    glDeleteShader(ve);
    glDeleteShader(fr);
    glDeleteFramebuffers(1, &rahmen);
    glDeleteTextures(1, &bildtextur);
    eglMakeCurrent(anzeige, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(anzeige, kontext);
    eglDestroySurface(anzeige, flaeche);
    melde(3);
}
