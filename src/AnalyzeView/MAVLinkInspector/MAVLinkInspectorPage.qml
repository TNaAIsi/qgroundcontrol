import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Window
import QtGraphs

import QGroundControl
import QGroundControl.Controls

AnalyzePage {
    id: root
    headerComponent:    headerComponent
    pageComponent:      pageComponent
    allowPopout:        true

    property var    curSystem:          controller ? controller.activeSystem : null
    property var    curMessage:         curSystem && curSystem.messages.count ? curSystem.messages.get(curSystem.selected) : null
    property int    curCompID:          0
    property real   maxButtonWidth:     0
    property bool   transwingOnly:      false

    function selectFirstTranswingMessage() {
        if (!curSystem || !curSystem.messages || curSystem.messages.count === 0) {
            return
        }
        if (curMessage && curMessage.isTranswing && (curCompID === 0 || curCompID === curMessage.compId)) {
            return
        }
        let fallbackIdx = -1
        for (let i = 0; i < curSystem.messages.count; i++) {
            const msg = curSystem.messages.get(i)
            if (!msg || !msg.isTranswing) {
                continue
            }
            if (curCompID !== 0 && curCompID !== msg.compId) {
                continue
            }
            if (msg.isTranswingStatus) {
                curSystem.selected = i
                return
            }
            if (fallbackIdx < 0) {
                fallbackIdx = i
            }
        }
        if (fallbackIdx >= 0) {
            curSystem.selected = fallbackIdx
        }
    }

    MAVLinkInspectorController {
        id: controller
    }

    function updateEnabledStatus(repeater, message, chart) {
        if(!message) {
            return;
        }

        for (let i = 0; i < repeater.count; i++) {
            let checkBox = repeater.itemAt(i)
            if(!checkBox) {
                continue
            }
            const messageField = message.fields.get(i)
            checkBox.enabled = isCheckboxEnabled(checkBox, messageField, chart)
        }
    }

    function isCheckboxEnabled(checkBox, messageField, chart) {
        if(checkBox.checkState === Qt.Checked) {
            return true
        }
        if(!messageField.selectable) {
            return false
        }
        if(messageField.series !== null) {
            return false
        }
        return chart.roomForNewDimension()
    }

    Component {
        id:  headerComponent
        //-- Header
        RowLayout {
            id:                 header
            anchors.left:       parent.left
            anchors.right:      parent.right
            QGCLabel {
                text:           qsTr("Inspect real time MAVLink messages.")
            }
            RowLayout {
                Layout.alignment:   Qt.AlignRight
                visible:            curSystem ? controller.systemNames.length > 1 || curSystem.compIDsStr.length > 2 : false
                QGCComboBox {
                    id:             systemCombo
                    model:          controller.systemNames
                    sizeToContents: true
                    visible:        controller.systemNames.length > 1
                    onActivated: (index) =>  { controller.setActiveSystem(controller.systems.get(index).id) }

                    Connections {
                        target: controller
                        function onActiveSystemChanged() {
                            for (var systemIndex=0; systemIndex<controller.systems.count; systemIndex++) {
                                if (controller.systems.get(systemIndex) == curSystem) {
                                    systemCombo.currentIndex = systemIndex
                                    curCompID = 0
                                    cidCombo.currentIndex = 0
                                    break
                                }
                            }
                        }
                    }
                }
                QGCComboBox {
                    id:             cidCombo
                    model:          curSystem ? curSystem.compIDsStr : []
                    sizeToContents: true
                    visible:        curSystem ? curSystem.compIDsStr.length > 2 : false
                    onActivated: (index) => {
                        if(curSystem && curSystem.compIDsStr.length > 1) {
                            if(index < 1)
                                curCompID = 0
                            else
                                curCompID = curSystem.compIDs[index - 1]
                        }
                    }
                }
            }
        }
    }

    Component {
        id:                         pageComponent
        ColumnLayout {
            width:                  availableWidth
            height:                 availableHeight
            spacing:                ScreenTools.defaultFontPixelHeight * 0.5

            // Filter bar lives inside pageComponent so it remains visible on short Android screens
            RowLayout {
                id:                 filterBar
                Layout.fillWidth:   true
                spacing:            ScreenTools.defaultFontPixelWidth * 0.5

                QGCButton {
                    text:           qsTr("All")
                    checkable:      true
                    checked:        !root.transwingOnly
                    onClicked:      root.transwingOnly = false
                }
                QGCButton {
                    text:           qsTr("Transwing")
                    checkable:      true
                    checked:        root.transwingOnly
                    onClicked: {
                        root.transwingOnly = true
                        root.selectFirstTranswingMessage()
                    }
                }
                Item { Layout.fillWidth: true }
            }

            Row {
                Layout.fillWidth:   true
                Layout.fillHeight:  true
                spacing:            ScreenTools.defaultFontPixelWidth
                //-- Messages (Buttons)
                QGCFlickable {
                    id:                 buttonGrid
                    flickableDirection: Flickable.VerticalFlick
                    width:              maxButtonWidth
                    height:             parent.height
                    contentWidth:       width
                    contentHeight:      buttonCol.height
                    ColumnLayout {
                        id:             buttonCol
                        anchors.left:   parent.left
                        anchors.right:  parent.right
                        spacing:        ScreenTools.defaultFontPixelHeight * 0.25
                        Repeater {
                            model:      curSystem ? curSystem.messages : []
                            delegate:   MAVLinkMessageButton {
                                text:       object.name + (object.fieldSelected ?  " *" : "")
                                compID:     object.compId
                                checked:    curSystem ? (curSystem.selected === index) : false
                                messageHz:  object.actualRateHz
                                visible:    (curCompID === 0 || curCompID === compID) && (!root.transwingOnly || object.isTranswing)
                                onClicked: {
                                    curSystem.selected = index
                                }
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
                //-- Empty state for Transwing filter
                QGCLabel {
                    visible:            root.transwingOnly && (!curMessage || !curMessage.isTranswing || (curCompID !== 0 && curCompID !== curMessage.compId))
                    width:              parent.width - buttonGrid.width - ScreenTools.defaultFontPixelWidth
                    height:             parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment:  Text.AlignVCenter
                    wrapMode:           Text.WordWrap
                    text:               qsTr("Waiting for Transwing telemetry (DEBUG_FLOAT_ARRAY #350 TRANSWING v4 or VTOL messages)...")
                }
                //-- Message Data
                QGCFlickable {
                    id:                 messageGrid
                    visible:            curMessage !== null && (curCompID === 0 || curCompID === curMessage.compId) && (!root.transwingOnly || curMessage.isTranswing)
                    flickableDirection: Flickable.VerticalFlick
                    width:              parent.width - buttonGrid.width - ScreenTools.defaultFontPixelWidth
                    height:             parent.height
                    contentWidth:       width
                    contentHeight:      messageCol.height
                Column {
                    id:                 messageCol
                    width:              parent.width
                    spacing:            ScreenTools.defaultFontPixelHeight * 0.25
                    GridLayout {
                        columns:        2
                        columnSpacing:  ScreenTools.defaultFontPixelWidth
                        rowSpacing:     ScreenTools.defaultFontPixelHeight * 0.25
                        QGCLabel {
                            text: qsTr("Message:")
                            Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 20
                        }
                        QGCLabel {
                            color: qgcPal.accent
                            text: curMessage ? curMessage.name + ' (' + curMessage.id + ')' : ""
                        }

                        QGCLabel { text: qsTr("Component:") }
                        QGCLabel { text: curMessage ? curMessage.compId : "" }

                        QGCLabel { text: qsTr("Count:") }
                        QGCLabel { text: curMessage ? curMessage.count : "" }

                        QGCLabel { text: qsTr("Actual Rate:") }
                        QGCLabel { text: curMessage ? curMessage.actualRateHz.toFixed(1) + qsTr("Hz") : "" }

                        QGCLabel { text: qsTr("Set Rate:") }
                        QGCComboBox {
                            id: msgRateCombo
                            textRole: "text"
                            valueRole: "value"
                            model: [
                                { value: -1, text: qsTr("Disabled") },
                                { value: 0, text: qsTr("Default") },
                                { value: 1, text: qsTr("1Hz") },
                                { value: 2, text: qsTr("2Hz") },
                                { value: 3, text: qsTr("3Hz") },
                                { value: 4, text: qsTr("4Hz") },
                                { value: 5, text: qsTr("5Hz") },
                                { value: 6, text: qsTr("6Hz") },
                                { value: 7, text: qsTr("7Hz") },
                                { value: 8, text: qsTr("8Hz") },
                                { value: 9, text: qsTr("9Hz") },
                                { value: 10, text: qsTr("10Hz") },
                                { value: 25, text: qsTr("25Hz") },
                                { value: 50, text: qsTr("50Hz") },
                                { value: 100, text: qsTr("100Hz") }
                            ]
                            Layout.alignment: Qt.AlignLeft
                            sizeToContents: true
                            Component.onCompleted: reset()
                            onActivated: (index) => controller.setMessageInterval(currentValue)
                            function reset() { currentIndex = indexOfValue(0) }
                            Connections {
                                target: root
                                function onCurMessageChanged() { msgRateCombo.reset() }
                            }
                            Connections {
                                target: curMessage
                                function onTargetRateHzChanged() {
                                    const target_index = indexOfValue(curMessage.targetRateHz)
                                    if(target_index != -1) {
                                        currentIndex = target_index
                                    }
                                }
                            }
                        }
                    }
                    Item { height: ScreenTools.defaultFontPixelHeight * 0.5; width: 1 }
                    //-- Transwing v4 Structured Summary
                    ColumnLayout {
                        id:         transwingSummarySection
                        visible:    curMessage !== null && curMessage.isTranswingStatus
                        width:      parent.width
                        spacing:    ScreenTools.defaultFontPixelHeight * 0.5

                        // Group 1: Mechanism
                        Rectangle {
                            Layout.fillWidth:   true
                            implicitHeight:     mechCol.implicitHeight + ScreenTools.defaultFontPixelHeight
                            color:              qgcPal.windowShade
                            radius:             ScreenTools.defaultFontPixelHeight * 0.25
                            ColumnLayout {
                                id:             mechCol
                                anchors.left:   parent.left
                                anchors.right:  parent.right
                                anchors.top:    parent.top
                                anchors.margins: ScreenTools.defaultFontPixelHeight * 0.5
                                spacing:        ScreenTools.defaultFontPixelHeight * 0.25
                                QGCLabel {
                                    text:       qsTr("Mechanism / 机构")
                                    font.bold:  true
                                    color:      qgcPal.accent
                                }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns:        ScreenTools.isShortScreen ? 2 : 4
                                    columnSpacing:  ScreenTools.defaultFontPixelWidth
                                    rowSpacing:     ScreenTools.defaultFontPixelHeight * 0.25
                                    Repeater {
                                        model: [
                                            { label: qsTr("Wing Angle"),     fieldIdx: 2,  unit: "°" },
                                            { label: qsTr("Wing Setpoint"),  fieldIdx: 3,  unit: "°" },
                                            { label: qsTr("Rate SP"),        fieldIdx: 4,  unit: "°/s" },
                                            { label: qsTr("Accel SP"),       fieldIdx: 5,  unit: "°/s²" },
                                            { label: qsTr("Jerk SP"),        fieldIdx: 6,  unit: "°/s³" },
                                            { label: qsTr("Collective Tilt"),fieldIdx: 7,  unit: "°" },
                                            { label: qsTr("Tracking Error"), fieldIdx: 8,  unit: "°" },
                                            { label: qsTr("Sample Age"),     fieldIdx: 12, unit: "ms" }
                                        ]
                                        delegate: ColumnLayout {
                                            readonly property var fieldObj: (curMessage && curMessage.isTranswingStatus && modelData.fieldIdx < curMessage.fields.count) ? curMessage.fields.get(modelData.fieldIdx) : null
                                            Layout.fillWidth: true
                                            spacing: 0
                                            QGCLabel {
                                                text:       modelData.label
                                                font.pointSize: ScreenTools.smallFontPointSize
                                                color:      qgcPal.textSecondary
                                            }
                                            QGCLabel {
                                                text:       (fieldObj ? fieldObj.value : "-") + (modelData.unit ? " " + modelData.unit : "")
                                                font.bold:  true
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Group 2: Transition
                        Rectangle {
                            Layout.fillWidth:   true
                            implicitHeight:     transCol.implicitHeight + ScreenTools.defaultFontPixelHeight
                            color:              qgcPal.windowShade
                            radius:             ScreenTools.defaultFontPixelHeight * 0.25
                            ColumnLayout {
                                id:             transCol
                                anchors.left:   parent.left
                                anchors.right:  parent.right
                                anchors.top:    parent.top
                                anchors.margins: ScreenTools.defaultFontPixelHeight * 0.5
                                spacing:        ScreenTools.defaultFontPixelHeight * 0.25
                                QGCLabel {
                                    text:       qsTr("Transition / 转换")
                                    font.bold:  true
                                    color:      qgcPal.accent
                                }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns:        ScreenTools.isShortScreen ? 2 : 4
                                    columnSpacing:  ScreenTools.defaultFontPixelWidth
                                    rowSpacing:     ScreenTools.defaultFontPixelHeight * 0.25
                                    Repeater {
                                        model: [
                                            { label: qsTr("Phase"),          fieldIdx: 9,  unit: "" },
                                            { label: qsTr("FW Weight"),      fieldIdx: 10, unit: "" },
                                            { label: qsTr("Complete"),       fieldIdx: 19, unit: "" },
                                            { label: qsTr("Abort Reason"),   fieldIdx: 16, unit: "" },
                                            { label: qsTr("Attitude Error"), fieldIdx: 13, unit: "°" },
                                            { label: qsTr("Tilt Error"),     fieldIdx: 21, unit: "°" },
                                            { label: qsTr("Heading Error"),  fieldIdx: 22, unit: "°" },
                                            { label: qsTr("Altitude Loss"),  fieldIdx: 14, unit: "m" },
                                            { label: qsTr("Collective Cmd"), fieldIdx: 15, unit: "" }
                                        ]
                                        delegate: ColumnLayout {
                                            readonly property var fieldObj: (curMessage && curMessage.isTranswingStatus && modelData.fieldIdx < curMessage.fields.count) ? curMessage.fields.get(modelData.fieldIdx) : null
                                            Layout.fillWidth: true
                                            spacing: 0
                                            QGCLabel {
                                                text:       modelData.label
                                                font.pointSize: ScreenTools.smallFontPointSize
                                                color:      qgcPal.textSecondary
                                            }
                                            QGCLabel {
                                                text:       (fieldObj ? fieldObj.value : "-") + (modelData.unit ? " " + modelData.unit : "")
                                                font.bold:  true
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // Group 3: Handover & Protection
                        Rectangle {
                            Layout.fillWidth:   true
                            implicitHeight:     handoverCol.implicitHeight + ScreenTools.defaultFontPixelHeight
                            color:              qgcPal.windowShade
                            radius:             ScreenTools.defaultFontPixelHeight * 0.25
                            ColumnLayout {
                                id:             handoverCol
                                anchors.left:   parent.left
                                anchors.right:  parent.right
                                anchors.top:    parent.top
                                anchors.margins: ScreenTools.defaultFontPixelHeight * 0.5
                                spacing:        ScreenTools.defaultFontPixelHeight * 0.25
                                QGCLabel {
                                    text:       qsTr("Handover & Protection / 接管与保护")
                                    font.bold:  true
                                    color:      qgcPal.accent
                                }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns:        ScreenTools.isShortScreen ? 2 : 4
                                    columnSpacing:  ScreenTools.defaultFontPixelWidth
                                    rowSpacing:     ScreenTools.defaultFontPixelHeight * 0.25
                                    Repeater {
                                        model: [
                                            { label: qsTr("Takeover Avail"), fieldIdx: 17, unit: "" },
                                            { label: qsTr("Control Owner"),  fieldIdx: 18, unit: "" },
                                            { label: qsTr("Handover Age"),   fieldIdx: 20, unit: "ms" },
                                            { label: qsTr("Status Mask"),    fieldIdx: 11, unit: "" }
                                        ]
                                        delegate: ColumnLayout {
                                            readonly property var fieldObj: (curMessage && curMessage.isTranswingStatus && modelData.fieldIdx < curMessage.fields.count) ? curMessage.fields.get(modelData.fieldIdx) : null
                                            Layout.fillWidth: true
                                            spacing: 0
                                            QGCLabel {
                                                text:       modelData.label
                                                font.pointSize: ScreenTools.smallFontPointSize
                                                color:      qgcPal.textSecondary
                                            }
                                            QGCLabel {
                                                text:       (fieldObj ? fieldObj.value : "-") + (modelData.unit ? " " + modelData.unit : "")
                                                font.bold:  true
                                            }
                                        }
                                    }
                                }
                                Flow {
                                    Layout.fillWidth: true
                                    spacing:        ScreenTools.defaultFontPixelWidth * 0.5
                                    Repeater {
                                        model: [
                                            { label: qsTr("Wing Valid"),    fieldIdx: 23 },
                                            { label: qsTr("FB Healthy"),    fieldIdx: 24 },
                                            { label: qsTr("Trk OK"),        fieldIdx: 25 },
                                            { label: qsTr("Flight Ctrl"),   fieldIdx: 26 },
                                            { label: qsTr("Ground Test"),   fieldIdx: 27 },
                                            { label: qsTr("FB Hold"),       fieldIdx: 28 },
                                            { label: qsTr("Pilot Locked"),  fieldIdx: 29 },
                                            { label: qsTr("Fresh"),         fieldIdx: 30 }
                                        ]
                                        delegate: Rectangle {
                                            readonly property var fieldObj: (curMessage && curMessage.isTranswingStatus && modelData.fieldIdx < curMessage.fields.count) ? curMessage.fields.get(modelData.fieldIdx) : null
                                            readonly property bool flagActive: fieldObj !== null && fieldObj.value.indexOf("1") === 0
                                            implicitWidth:  flagLabel.implicitWidth + ScreenTools.defaultFontPixelWidth
                                            implicitHeight: flagLabel.implicitHeight + ScreenTools.defaultFontPixelHeight * 0.3
                                            radius:         ScreenTools.defaultFontPixelHeight * 0.2
                                            color:          flagActive ? qgcPal.colorGreen : qgcPal.windowShadeDark
                                            QGCLabel {
                                                id:                 flagLabel
                                                anchors.centerIn:   parent
                                                text:               modelData.label
                                                font.pointSize:     ScreenTools.smallFontPointSize
                                                color:              flagActive ? qgcPal.accentText : qgcPal.textSecondary
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Item { height: ScreenTools.defaultFontPixelHeight * 0.5; width: 1 }
                    //---------------------------------------------------------
                    GridLayout {
                        id:                 msgInfoGrid
                        columns:            5
                        columnSpacing:      ScreenTools.defaultFontPixelWidth  * 0.25
                        rowSpacing:         0
                        width:              parent.width
                        QGCLabel {
                            text:       qsTr("Name")
                        }
                        QGCLabel {
                            text:       qsTr("Value")
                        }
                        QGCLabel {
                            text:       qsTr("Type")
                        }
                        QGCLabel {
                            text:       qsTr("Plot 1")
                        }
                        QGCLabel {
                            text:       qsTr("Plot 2")
                        }

                        //---------------------------------------------------------
                        Rectangle {
                            Layout.columnSpan:  5
                            Layout.fillWidth:   true
                            height:             1
                            color:              qgcPal.textPrimary
                        }
                        //---------------------------------------------------------

                        Repeater {
                            model:      curMessage ? curMessage.fields : []
                            delegate:   QGCLabel {
                                Layout.row:         index + 2
                                Layout.column:      0
                                Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 20
                                text:               object.name
                            }
                        }
                        Repeater {
                            model:      curMessage ? curMessage.fields : []
                            delegate:   QGCLabel {
                                Layout.row:         index + 2
                                Layout.column:      1
                                Layout.minimumWidth: msgInfoGrid.width * 0.25
                                Layout.maximumWidth: msgInfoGrid.width * 0.25
                                text:               object.value
                                elide:              Text.ElideRight
                            }
                        }
                        Repeater {
                            model:      curMessage ? curMessage.fields : []
                            delegate:   QGCLabel {
                                Layout.row:         index + 2
                                Layout.column:      2
                                Layout.minimumWidth: ScreenTools.defaultFontPixelWidth * 10
                                text:               object.type
                                elide:              Text.ElideRight
                            }
                        }
                        Repeater {
                            id: chart1Repeater
                            model:      curMessage ? curMessage.fields : []
                            delegate:   QGCCheckBox {
                                Layout.row:         index + 2
                                Layout.column:      3
                                Layout.alignment:   Qt.AlignHCenter
                                checked:            object.series !== null && object.chartIndex === 0
                                onClicked: {
                                    if(checked) {
                                        chart1.addDimension(object)
                                    } else {
                                        chart1.delDimension(object)
                                    }
                                    updateEnabledStatus(chart1Repeater, curMessage, chart1)
                                    updateEnabledStatus(chart2Repeater, curMessage, chart2)
                                }
                                Component.onCompleted: updateEnabledStatus(chart1Repeater, curMessage, chart1)
                            }
                        }
                        Repeater {
                            id: chart2Repeater
                            model:      curMessage ? curMessage.fields : []
                            delegate:   QGCCheckBox {
                                Layout.row:         index + 2
                                Layout.column:      4
                                Layout.alignment:   Qt.AlignHCenter
                                checked:            object.series !== null && object.chartIndex === 1
                                onClicked: {
                                    if(checked) {
                                        chart2.addDimension(object)
                                    } else {
                                        chart2.delDimension(object)
                                    }
                                    updateEnabledStatus(chart2Repeater, curMessage, chart2)
                                    updateEnabledStatus(chart1Repeater, curMessage, chart1)
                                }
                                Component.onCompleted: updateEnabledStatus(chart2Repeater, curMessage, chart2)
                            }
                        }
                    }
                    Item { height: ScreenTools.defaultFontPixelHeight * 0.25; width: 1 }
                    MAVLinkChart {
                        id:                     chart1
                        height:                 visible ? ScreenTools.defaultFontPixelHeight * 20 : 0
                        width:                  parent.width
                        inspectorController:    controller
                        chartIndex:             0
                    }
                    MAVLinkChart {
                        id:                     chart2
                        height:                 visible ? ScreenTools.defaultFontPixelHeight * 20 : 0
                        width:                  parent.width
                        inspectorController:    controller
                        chartIndex:             1
                    }
                }
            }
            }
        }
    }
}
