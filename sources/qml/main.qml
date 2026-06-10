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
        TabButton { text: qsTr("Images") }
        TabButton { text: qsTr("Log") }
        TabButton { text: qsTr("Settings") }
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

                    // Поле 1
                    RowLayout {
                        Layout.fillWidth: true
                        TextField { id: pathField1; Layout.fillWidth: true; readOnly: true }
                        Button {
                            //text: "";
                            icon.source: "qrc:/images/icons/folder-open-light.png"
                            icon.width: 24
                            icon.height: 24
                            onClicked: fileDialogLoader.open()
                        }
                    }

                    // Поле 2
                    RowLayout {
                        Layout.fillWidth: true
                        TextField { id: pathField2; Layout.fillWidth: true; readOnly: true }
                        Button {
                            //text: "";
                            icon.source: "qrc:/images/icons/folder-open-light.png"
                            icon.width: 24
                            icon.height: 24

                            onClicked: fileDialogSav.open()
                        }
                    }
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
            FileDialog { id: fileDialogLoader; nameFilters: ["Binary files (*.bin)"]; onAccepted: pathField1.text = file }
            FileDialog { id: fileDialogSav; onAccepted: pathField2.text = file }

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
