// SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
// SPDX-License-Identifier: GPL-3.0-or-later
//
// VULKAN: dasselbe Bild wie SHADER HEAVY, nur auf dem anderen Weg. Der
// Fragment-Shader der Stufe ist Zeichen fuer Zeichen nach Vulkan-GLSL
// uebertragen (src/vulkan-cloud.frag), gezeichnet wird ohne Fenster in ein
// Bild im Speicher, gemessen mit Vulkans eigenen Zeitstempeln. Darum steht
// die Stufe gleich hinter SHADER HEAVY: dieselbe Arbeit, dieselben Pixel,
// anderer Weg -- und die GPU ist von der Stufe davor schon warm.
//
// Die Stufe zaehlt ihre Bilder selbst (`frameSource`), weil das Fenster von
// ihrer Arbeit nichts sieht. Die Seite holt sich die Zahl einmal in der
// Sekunde ab wie bei jeder gezeichneten Stufe, und es stehen am Ende
// Mittel, Minimum und laengstes Bild da -- wie bei den anderen auch.
//
// Ohne Bildsynchronisierung (es haengt kein Schirm an dem Bild) sind die
// Bilder/s nicht gedeckelt; sie sagen, was die GPU an diesem Bild schaffen
// wuerde, nicht was die Anzeige zeigt.
//
// Auf einem Geraet ohne libvulkan sagt die Stufe das und ist sofort fertig:
// dann ist sie eine Systemstufe mit einer Zeile statt einer mit fps.
import QtQuick 2.6
import Benchymark 1.0

Item {
    id: stage
    anchors.fill: parent

    property real areaTop: 0
    property real areaBottom: height
    property string live: ""
    signal finished()

    // Solange Vulkan da ist, ist das hier eine ganz gewoehnliche fps-Stufe.
    // Fehlt es, wird sie zur Systemstufe mit einer erklaerenden Zeile und
    // beendet sich selbst, statt eine Minute lang Nullen zu sammeln.
    readonly property bool system: engine.phase === -1
    readonly property bool selfTimed: engine.phase === -1
    readonly property var frameSource: engine

    OffscreenView {
        id: bild
        x: 0
        y: stage.areaTop
        width: stage.width
        height: stage.areaBottom - stage.areaTop
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: stage.areaTop + stage.width * 0.06
        text: engine.phase === -1 ? qsTrId("id-vulkan-missing")
                                  : engine.device + " · Vulkan " + engine.api
        color: "white"
        opacity: 0.85
        font.pixelSize: stage.width * 0.045
    }

    VulkanEngine {
        id: engine
        view: bild
        onChanged: {
            if (phase === -1) {
                stage.live = qsTrId("id-vulkan-missing") + " · " + reason
                stage.finished()
            }
        }
    }

    // Kein Vorheizen: die Seite wirft die erste Sekunde ohnehin weg, und
    // SHADER HEAVY lief gerade davor. Laeuft, bis die Seite die Stufe
    // abraeumt.
    Component.onCompleted: engine.start(Math.round(width),
                                        Math.round(areaBottom - areaTop), 0, 0)
    Component.onDestruction: engine.stop()

    // nur fuer den Fall ohne Vulkan: eine Zeile statt fps
    function results() {
        return [{ key: "VULKAN", name: qsTrId("id-stage-vulkan"),
                  value: "–", unit: engine.reason }]
    }

    //% "Vulkan shader"
    readonly property string _n: qsTrId("id-stage-vulkan")
    //% "no Vulkan here"
    readonly property string _m: qsTrId("id-vulkan-missing")
}
