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
// Ohne Bildsynchronisierung (es haengt kein Schirm an dem Bild) sind die
// Bilder/s nicht gedeckelt; sie sagen, was die GPU an diesem Bild schaffen
// wuerde, nicht was die Anzeige zeigt.
//
// Auf Geraeten ohne libvulkan sagt die Stufe das und ist fertig.
import QtQuick 2.6
import Benchymark 1.0

Item {
    id: stage
    anchors.fill: parent

    property real areaTop: 0
    property real areaBottom: height
    readonly property bool system: true
    readonly property bool selfTimed: true
    property string live: ""
    signal finished()

    // eine Sekunde vorheizen, zehn messen
    readonly property int preheatMs: 1000
    readonly property int measureMs: 10000
    property bool ended: false

    VulkanView {
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
            if (phase === -1)
                stage.live = qsTrId("id-vulkan-missing") + " · " + reason
            else if (phase === 1)
                //% "warming up"
                stage.live = qsTrId("id-vulkan-warming")
            else if (phase >= 2)
                stage.live = Math.round(engine.fps) + " fps · " + engine.gpuMs.toFixed(1) + " ms"
            if ((phase === 3 || phase === -1) && !stage.ended) {
                stage.ended = true
                stage.finished()
            }
        }
    }

    Component.onCompleted: engine.start(Math.round(width),
                                        Math.round(areaBottom - areaTop),
                                        preheatMs, measureMs)
    Component.onDestruction: engine.stop()

    // eine Zeile: Bilder je Sekunde fuer dasselbe Bild wie SHADER HEAVY
    function results() {
        if (engine.phase === -1)
            return [{ key: "VULKAN", name: qsTrId("id-stage-vulkan"),
                      value: "–", unit: engine.reason }]
        return [{ key: "VULKAN", name: qsTrId("id-stage-vulkan"),
                  value: Math.round(engine.fps), unit: "fps" }]
    }

    //% "Vulkan shader"
    readonly property string _n: qsTrId("id-stage-vulkan")
    //% "no Vulkan here"
    readonly property string _m: qsTrId("id-vulkan-missing")
    //% "warming up"
    readonly property string _w: qsTrId("id-vulkan-warming")
}
