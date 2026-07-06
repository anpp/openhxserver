import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.0
import OpenHX.SettingsTypes 1.0

Page {
    id: settingsPage
    signal settingsChanged()

    focus: true

    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Back) {
                            event.accepted = true

                            rootStack.pop()
                        }
                    }

    background: Rectangle { color: mainWindow.palette.window }

    header: ToolBar {
        id: settingsBar
        Layout.fillWidth: true
        spacing: 0

        background: Rectangle {
            color: mainWindow.palette.window
            border.color: mainWindow.palette.mid
            border.width: 1
        }

        RowLayout {
            anchors.fill: parent

            ToolButton {
                id: backButton
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Back")

                icon.source: "qrc:/images/icons/arrow_left.png"

                icon.width: 32
                icon.height: 32
                display: AbstractButton.TextBesideIcon
                icon.color: backButton.hovered ? settingsBar.palette.highlight : settingsBar.palette.windowText

                onClicked: rootStack.pop()
            }

            Label {
                text: qsTr("Settings")
                font.bold: true
                Layout.leftMargin: 10
                Material.foreground: mainWindow.palette.text
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width
        clip: true

        ColumnLayout {
            width: parent.width
            anchors.margins: 16
            spacing: 20

            Label {
                text: qsTr("Serial port")
                font.bold: true
            }

            ComboBox {
                id: portComboBox
                Layout.fillWidth: true

                model: Settings.portsList

                Component.onCompleted: {
                    var savedPort = Settings.getSetting("name", SettingsTypes.KindSet.ComPort);
                    let idx = find(savedPort);
                    if (idx !== -1) {
                        currentIndex = idx;
                    }
                }

                Item { Layout.preferredHeight: 20 }
            }

            Label {
                text: qsTr("Baud rate")
                font.bold: true
            }

            ComboBox {
                id: baudRateComboBox
                Layout.fillWidth: true

                model: [9600, 19200, 38400, 57600, 115200]

                Component.onCompleted: {
                    var savedBaudRate = Settings.getSetting("baudRate", SettingsTypes.KindSet.ComPort);
                    let idx = find(savedBaudRate);
                    if (idx !== -1) {
                        currentIndex = idx;
                    }
                }

                Item { Layout.preferredHeight: 20 }
            }

            Label {
                text: qsTr("Flow control")
                font.bold: true
            }

            ComboBox {
                id: flowControlComboBox
                Layout.fillWidth: true

                textRole: "text"
                valueRole: "value"

                model: ListModel {
                    ListElement { text: "None";       value: 0 }
                    ListElement { text: "RTS/CTS";    value: 1 }
                    ListElement { text: "XON/XOFF";   value: 2 }
                }

                Component.onCompleted: {
                    var savedFlowControl = Settings.getSetting("flowControl", SettingsTypes.KindSet.ComPort);
                    let idx = indexOfValue(savedFlowControl);
                    if (idx !== -1) {
                        currentIndex = idx;
                    }
                }

                Item { Layout.preferredHeight: 20 }
            }

        }
    }

    footer: Button {
        text: qsTr("Save")
        onClicked: {
            Settings.setSetting("name", portComboBox.currentText, SettingsTypes.KindSet.ComPort);
            Settings.setSetting("baudRate", Number(baudRateComboBox.currentText), SettingsTypes.KindSet.ComPort);
            Settings.setSetting("flowControl", flowControlComboBox.model.get(flowControlComboBox.currentIndex).value, SettingsTypes.KindSet.ComPort);

            Settings.saveSettingsByKind(SettingsTypes.KindSet.ComPort)
            settingsPage.settingsChanged()

            Qt.callLater(() => {
                             rootStack.pop()
                         })
        }
    }
}
