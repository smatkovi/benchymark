// SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
// SPDX-License-Identifier: GPL-3.0-or-later
//
// GLES 3: derselbe Shader wie SHADER HEAVY, aber im eigenen Kontext und ohne
// Fenster in einen Rahmenpuffer im Speicher gezeichnet. Zusammen mit der
// Stufe davor (SHADER HEAVY auf dem Schirm durch QtQuick) und der Stufe
// VULKAN ergibt das drei Zahlen fuer dieselbe Rechnung: was QtQuick daraus
// macht, was blankes GLES schafft, und was Vulkan schafft.
//
// Gemessen wird mit GL_EXT_disjoint_timer_query, sonst mit der Wanduhr um
// glFinish(). Gezaehlt wird wie bei den gezeichneten Stufen, einmal in der
// Sekunde von der Seite abgeholt -- daher Mittel, Minimum und laengstes Bild
// wie ueberall sonst.
import QtQuick 2.6
import Benchymark 1.0

Item {
    id: stage
    anchors.fill: parent

    property real areaTop: 0
    property real areaBottom: height
    property string live: ""
    property int wantedVersion: 3
    property string resultKey: "GLES-3"
    property string resultName: qsTrId("id-stage-gles3")
    signal finished()

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
        text: engine.phase === -1 ? engine.reason : engine.glVersion
        color: "white"
        opacity: 0.85
        font.pixelSize: stage.width * 0.04
        width: stage.width * 0.9
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }

    GlesEngine {
        id: engine
        view: bild
        wanted: stage.wantedVersion
        onChanged: {
            if (phase === -1) {
                stage.live = engine.reason
                stage.finished()
            }
        }
    }

    Component.onCompleted: engine.start(Math.round(width),
                                        Math.round(areaBottom - areaTop))
    Component.onDestruction: engine.stop()

    // nur fuer den Fall, dass dieser Kontext nicht zu haben ist
    function results() {
        return [{ key: stage.resultKey, name: stage.resultName,
                  value: "–", unit: engine.reason }]
    }

    //% "GLES 3 shader"
    readonly property string _n3: qsTrId("id-stage-gles3")
}
