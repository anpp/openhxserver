import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 400
    height: 300
    title: "QML Пример"

    Column {
        anchors.centerIn: parent
        spacing: 10

        Button {
            id: buttonChangeText
            text: "Изменить текст"
            onClicked: label.text = Settings.getSetting("savfile")
        }

        Button {
            id: buttonShowMessage
            text: "Показать сообщение"
            onClicked: Settings.load()
        }

        Label {
            id: label
            text: "Привет, мир!"
        }
    }
}
