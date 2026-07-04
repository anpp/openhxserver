import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtQuick.Controls.Material 2.0

ApplicationWindow {
    id: window
    visible: true
    width: 800; height: 600
    title: qsTr("OpenHX")

    Material.accent: Material.Blue
    background: Rectangle { color: window.palette.window }


    StackView {
        id: rootStack
        anchors.fill: parent

        initialItem: mainScreenComponent
    }

    Component {
        id: mainScreenComponent
        MainScreen {}
    }

    Component {
        id: rectSettings
        PageSettings {}
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
