import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

ApplicationWindow {
    id: window
    visible: true
    width: 800; height: 600
    title: qsTr("OpenHX Server")

    background: Rectangle { color: window.palette.window }

    property int currentRowIndex: -1

    header: ToolBar {
        id: topToolBar

        background: Rectangle {
            color: window.palette.window
            border.color: window.palette.mid
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: 5
                spacing: 10

                ToolButton {
                    id: startButton
                    //text: qsTr("Start")
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Start")

                    icon.source: "qrc:/images/icons/play-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: startButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                }

                ToolButton {
                    id: stopButton
                    //text: qsTr("Stop")
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Stop")

                    icon.source: "qrc:/images/icons/stop-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: stopButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                }
                ToolButton {
                    id: pauseButton
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Pause")

                    icon.source: "qrc:/images/icons/pause-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: pauseButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                }

                ToolSeparator { Layout.fillHeight: true }

                ToolButton {
                    id: packedButton
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Packed data")

                    icon.source: "qrc:/images/icons/package-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: packedButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                }

                ToolSeparator { Layout.fillHeight: true }

                ToolButton {
                    id: settingsButton
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Settings")

                    icon.source: "qrc:/images/icons/settings-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: settingsButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                }

                Item { Layout.fillWidth: true }
            }

            // Горизонтальный разделитель между тулбаром и главным окном
            Rectangle {
                Layout.fillWidth: true
                height: 1
                //color: "#bcbcbc"
            }

            TabBar {
                id: tabBar
                Layout.fillWidth: true
                background: Rectangle { color: window.palette.window }

                Component.onCompleted: {
                        tabBar.palette.windowText = window.palette.windowText
                        tabBar.palette.text = window.palette.text
                    }

                TabButton {
                        id: tabImages
                        text: qsTr("Images")
                        icon.source: "qrc:/images/icons/disk-light.png"

                        //icon.color: tabImages.checked ? tabBar.palette.highlight : tabBar.palette.windowText

                        palette.text: tabImages.checked ? window.palette.highlight : window.palette.windowText
                        palette.windowText: tabImages.checked ? window.palette.highlight : window.palette.windowText
                }

                    TabButton {
                        id: tabLog
                        text: qsTr("Log")
                        icon.source: "qrc:/images/icons/log-light.png"
                        icon.color: tabLog.checked ? tabBar.palette.highlight : tabBar.palette.windowText

                        contentItem: Text {
                            text: tabLog.text
                            font: tabLog.font
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabLog.checked ? tabBar.palette.highlight : tabBar.palette.windowText
                        }
                    }

                    TabButton {
                        id: tabSettings
                        text: qsTr("Settings")
                        icon.source: "qrc:/images/icons/settings-light.png"
                        icon.color: tabSettings.checked ? tabBar.palette.highlight : tabBar.palette.windowText

                        contentItem: Text {
                            text: tabSettings.text
                            font: tabSettings.font
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: tabSettings.checked ? tabBar.palette.highlight : tabBar.palette.windowText
                        }
                    }
                }
        }
    }

    ButtonGroup {
        id: interfaceGroup

        onCheckedButtonChanged: {
            if (checkedButton) {
                console.log("Radio:", checkedButton.text)
                console.log("ID:", checkedButton.id)
            }
        }
    }
    StackLayout {
        anchors.fill: parent
        currentIndex: tabBar.currentIndex        

        Item {
            id: imagesPage

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                //блок выбора лоадера и .SAV файла
                ColumnLayout {
                    Layout.fillWidth: true

                    // Loader
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RadioButton {
                            id: rbLoader
                            ButtonGroup.group: interfaceGroup
                            checked: true
                            text: qsTr("Loader")
                            Layout.preferredWidth: 90
                        }

                        RowLayout {
                                    Layout.fillWidth: true
                                    enabled: rbLoader.checked

                                    TextField {
                                        id: pathField1
                                        Layout.fillWidth: true
                                        readOnly: true
                                        placeholderText: qsTr("Select loader file...")
                                    }
                                    Button {
                                        icon.source: "qrc:/images/icons/folder-open-light.png"
                                        icon.width: 24
                                        icon.height: 24
                                        onClicked: fileDialogLoader.open()
                                    }
                                }
                    }

                    // .SAV
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RadioButton {
                            id: rbSav
                            ButtonGroup.group: interfaceGroup
                            text: qsTr(".SAV")
                            Layout.preferredWidth: 90
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            enabled: rbSav.checked

                            TextField {
                            id: pathField2
                            Layout.fillWidth: true
                            readOnly: true
                            placeholderText: qsTr("Select .SAV file...")
                            }
                            Button {
                                icon.source: "qrc:/images/icons/folder-open-light.png"
                                icon.width: 24
                                icon.height: 24
                                onClicked: fileDialogSAV.open()
                            }
                        }
                    }
                }
                // ДЕКОРАТИВНЫЙ РАЗДЕЛИТЕЛЬ (Линия)
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    //color: "#dbdbdb"
                }
                Item {
                         Layout.preferredHeight: 1
                }

                // Список .dsk образов
                ListView {
                    id: fileList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: DiskImagesModel
                    clip: true
                    focus: true
                    currentIndex: -1

                    header: Rectangle {
                        width: fileList.width
                        height: 30
                        //color: "#e0e0e0"
                        color: window.palette.window
                        border.color: window.palette.mid

                        z: 2

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 10

                            Text { text: qsTr("Device"); Layout.preferredWidth: 40; font.bold: true }
                            ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }
                            Text { text: qsTr("Image"); Layout.preferredWidth: 90; font.bold: true }
                            ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }
                            Text { text: qsTr("File"); Layout.fillWidth: true; font.bold: true }
                        }
                    }

                    delegate: ItemDelegate {
                        id: control
                        width: fileList.width
                        height: 40
                        property bool isSelected: index === fileList.currentIndex

                        onClicked: {
                            fileList.currentIndex = index
                            fileList.forceActiveFocus()
                        }

                        background: Rectangle {
                            implicitWidth: control.width
                            implicitHeight: control.height

                            color: control.isSelected
                                               ? control.palette.highlight
                                               : (index % 2 === 0 ? control.palette.window : Qt.darker(control.palette.window, 1.05))

                            // Легкая линия-разделитель снизу
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 1
                                color: control.palette.midlight
                            }
                        }

                        contentItem: RowLayout {
                            spacing: 10
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text {
                                text: model.hxIndex
                                Layout.preferredWidth: 40
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected ? control.palette.highlightedText : control.palette.text
                            }
                            ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }
                            Text {
                                text: model.imageName
                                Layout.preferredWidth: 90
                                font.bold: true
                                elide: Text.ElideMiddle
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected ? control.palette.highlightedText : control.palette.text
                            }
                            ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }
                            Text {
                                text: model.fileName ? decodeURIComponent(model.fileName) : qsTr("<Empty>")
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected
                                    ? control.palette.highlightedText
                                    : (model.fileName ? control.palette.text
                                                      : (control.palette.placeholderText !== undefined ? control.palette.placeholderText : "#888888"))
                            }

                            ToolButton {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32

                                visible: control.isSelected && model.fileName !== ""
                                enabled: control.isSelected && model.fileName !== ""

                                icon.source: "qrc:/images/icons/clear-light.png"
                                icon.width: 20
                                icon.height: 20
                                icon.color: control.isSelected ? control.palette.highlightedText : (hovered ? control.palette.highlight : control.palette.windowText)

                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Clear")

                                onClicked: {
                                    model.fileName = "" // то же самое что DiskImagesModel.setFileNameAt(index, "")
                                    DiskImagesModel.save();
                                }
                            }
                            ToolButton {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32

                                visible: control.isSelected
                                enabled: control.isSelected

                                icon.source: "qrc:/images/icons/folder-open-light.png"
                                icon.width: 20
                                icon.height: 20
                                //icon.color: hovered ? control.palette.highlight : control.palette.windowText
                                icon.color: control.isSelected ? control.palette.highlightedText : (hovered ? control.palette.highlight : control.palette.windowText)

                                ToolTip.visible: hovered
                                ToolTip.text: qsTr("Select disk image")

                                onClicked: {
                                    currentRowIndex = index
                                    fileDialogDSK.open()
                                }
                            }
                        }
                    }
                }
            }

            FileDialog {
                id: fileDialogLoader;
                nameFilters: ["Binary files (*.bin)"];
                onAccepted: {
                    let pathStr = decodeURIComponent(fileDialogLoader.file.toString())
                    pathField1.text = pathStr.split('/').pop()
                    }
            }
            FileDialog {
                id: fileDialogSAV;
                //nameFilters: ["SAV files (*.sav)"];
                onAccepted: {
                    let pathStr = decodeURIComponent(fileDialogSAV.file.toString())
                    pathField2.text = pathStr.split('/').pop()
                }
            }
            FileDialog {
                id: fileDialogDSK;
                nameFilters: ["Dsk files (*.dsk)"];
                onAccepted: {
                    if (currentRowIndex >= 0) {
                        var localPath = fileDialogDSK.file.toLocaleString()
                        var cleanPath = localPath.replace("file:///", "")
                        DiskImagesModel.setFileNameAt(currentRowIndex, cleanPath)
                        DiskImagesModel.save();
                    }
                }
            }


        }
        // Экран 2: Лог
        TextArea {
            readOnly: true
            text: "Лог будет здесь..."
        }

        // Экран 3: Настройки
        ColumnLayout {
            TextField { placeholderText: "Test"; text: Settings.getSetting("savfile") }
            Button { text: qsTr("Save"); onClicked: Settings.save() }
        }
    }
    footer: ToolBar {
            id: bottomStatusBar
            height: 35

            background: Rectangle {
                color: window.palette.window
                border.color: window.palette.mid
                border.width: 1
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 15

                RowLayout {
                    spacing: 6
                    Rectangle {
                        width: 10; height: 10; radius: 5
                        color: "red"
                    }
                    Text {
                        text: qsTr("Port: ")
                        font.pointSize: 10
                    }
                }

                ToolSeparator { Layout.fillHeight: true; topPadding: 4; bottomPadding: 4 }

                Text {
                    text: qsTr("State: Ready")
                    font.pointSize: 10
                    Layout.fillWidth: true
                }

                Text {
                    text: "v1.0.0"
                    font.pointSize: 9
                    color: "gray"
                }
            }
    }
}
