import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

ApplicationWindow {
    visible: true
    width: 800; height: 600
    title: "OpenHX Server"

    property int currentRowIndex: -1

    header: TabBar {
        id: tabBar
        TabButton { text: qsTr("Images") ;icon.source: "qrc:/images/icons/disk-light.png"}
        TabButton { text: qsTr("Log")  ;icon.source: "qrc:/images/icons/log-light.png"}
        TabButton { text: qsTr("Settings")  ;icon.source: "qrc:/images/icons/settings-light.png"}
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
                            text: qsTr(".SAV:")
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
                    color: "#dbdbdb"
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
                        color: "#e0e0e0"
                        z: 2

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            spacing: 10

                            Text { text: qsTr("Device"); Layout.preferredWidth: 60; font.bold: true }
                            Text { text: qsTr("Image"); Layout.preferredWidth: 100; font.bold: true }
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

                            //color: (index % 2 === 0 ? "#ffffff" : "#f5f5f5")
                            color: control.isSelected
                                               ? control.palette.highlight
                                               : (index % 2 === 0 ? "#ffffff" : "#f5f5f5")

                            // Легкая линия-разделитель снизу
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 1
                                color: "#e8e8e8"
                            }
                        }

                        contentItem: RowLayout {
                            spacing: 10
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10

                            Text {
                                text: model.hxIndex
                                Layout.preferredWidth: 60
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected ? control.palette.highlightedText : control.palette.text
                            }
                            Text {
                                text: model.imageName
                                Layout.preferredWidth: 100
                                font.bold: true
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected ? control.palette.highlightedText : control.palette.text
                            }
                            Text {
                                text: model.fileName ? decodeURIComponent(model.fileName) : qsTr("<Empty>")
                                Layout.fillWidth: true
                                elide: Text.ElideMiddle
                                verticalAlignment: Text.AlignVCenter
                                color: control.isSelected
                                    ? control.palette.highlightedText
                                    : (model.fileName ? control.palette.text : "gray")
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
                    let pathStr = decodeURIComponent(file.toString())
                    pathField1.text = pathStr.split('/').pop()
                    }
            }
            FileDialog {
                id: fileDialogSAV;
                //nameFilters: ["SAV files (*.sav)"];
                onAccepted: {
                    let pathStr = decodeURIComponent(file.toString())
                    pathField2.text = pathStr.split('/').pop()
                }
            }
            FileDialog {
                id: fileDialogDSK;
                nameFilters: ["Dsk files (*.dsk)"];
                onAccepted: {
                    if (currentRowIndex >= 0) {
                        var localPath = fileDialogDSK.file.toLocaleString()
                        var cleanPath = fileDialogDSK.file.toString().replace("file:///", "")
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
}
