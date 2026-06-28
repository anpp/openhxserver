import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.0

Page {
    id: settingsPage
    background: Rectangle { color: window.palette.window }

    header: ToolBar {
        id: settingsBar
        Layout.fillWidth: true
        spacing: 0

        background: Rectangle {
            color: window.palette.window
            border.color: window.palette.mid
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
                Material.foreground: window.palette.text
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width

        ColumnLayout {
            width: parent.width
            spacing: 20
        }
    }
}
