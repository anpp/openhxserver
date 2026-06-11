import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

ApplicationWindow {
    visible: true
    width: 800; height: 600
    title: "OpenHX Server"

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
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RadioButton {
                            id: rbLoader
                            ButtonGroup.group: interfaceGroup
                            checked: true
                            text: qsTr("Loader")
                            //Layout.preferredWidth: 90
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
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        RadioButton {
                            id: rbSav
                            ButtonGroup.group: interfaceGroup
                            text: qsTr(".SAV:")
                            //Layout.preferredWidth: 90
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

                // Список .dsk обазов
                ListView {
                    id: fileList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: Model // модель, надо взять из HXServer

                    header: Rectangle {
                        width: fileList.width; height: 30; color: "lightgrey"
                        RowLayout {
                            anchors.fill: parent
                            Text { text: qsTr("Device"); Layout.preferredWidth: 100 }
                            Text { text: qsTr("Image"); Layout.preferredWidth: 100 }
                            Text { text: qsTr("File"); Layout.fillWidth: true }
                        }
                    }

                    delegate: RowLayout {
                        width: fileList.width
                        Text { text: model.fileName; Layout.preferredWidth: 100 }
                        Text { text: model.fileSize; Layout.preferredWidth: 100 }
                        Button {
                            text: qsTr("Open")
                            Layout.fillWidth: true
                            onClicked: console.log("", model.filePath)
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
