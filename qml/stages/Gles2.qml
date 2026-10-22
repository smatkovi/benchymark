// SPDX-FileCopyrightText: 2026 Sebastian Matkovich <github.com/smatkovi>
// SPDX-License-Identifier: GPL-3.0-or-later
//
// GLES 2: dieselbe Stufe, nur mit einem ES-2-Kontext und GLSL ES 1.00 --
// genau der Weg, den QtQuick auf Qt 5.6 nimmt, aber ohne Fenster und ohne
// Szenengraph dazwischen.
import QtQuick 2.6

Gles3 {
    wantedVersion: 2
    resultKey: "GLES-2"
    resultName: qsTrId("id-stage-gles2")

    //% "GLES 2 shader"
    readonly property string _n2: qsTrId("id-stage-gles2")
}
