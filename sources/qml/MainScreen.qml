import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import QtQuick.Controls.Material 2.0
import OpenHX.ServerTypes 1.0
import OpenHX.SettingsTypes 1.0

Page {
    id: mainScreen

    Timer {
        id: initTimer
        interval: 50
        running: true
        repeat: false

        onTriggered: {
            mainScreen.updateHXServer()
        }
    }

    function updateHXServer() {
        //console.log("updateHX")

        HXServer.setPortName(Settings.getSetting("name", SettingsTypes.KindSet.ComPort))
        HXServer.setPortSettings()
        HXServer.setLoader(Settings.getSetting("loader", SettingsTypes.KindSet.Misc))
        HXServer.setSAVFile(Settings.getSetting("savfile", SettingsTypes.KindSet.Misc))

        if (Settings.getBoolSetting("HXMode", SettingsTypes.KindSet.Misc))
            HXServer.setServerMode(ServerTypes.ServerMode.HXMode)
        else
            HXServer.setServerMode(ServerTypes.ServerMode.SAVMode)
    }

    //Component.onCompleted: {
    //    mainScreen.updateHXServer()
    //}

    background: Rectangle { color: mainWindow.palette.window }

    property int currentRowIndex: -1

    //Обработка сигнала из settingsPage
    Connections {
        id: settingsConnection
        target: null

        function onSettingsChanged() {
            mainScreen.updateHXServer();
        }
    }

    //Обработка сигналов от HXServer
    Connections {
        target: HXServer

        function onLog(value, color, b_state, b_clear_last) {
            let message = "";
            let date_color = "#0000ff";

            if (!b_state) {
                message = value;
            } else {
                message = HXServer.nameState + " " + value;
            }

            message = "<font color=\"" + color + "\">" + message + "</font>";

            if (b_clear_last)
                teLog.undo()

            let currentDateTime = Qt.formatDateTime(new Date(), "[dd.MM.yyyy hh:mm:ss]");
            let datePart = "<font color=\"" + date_color + "\">" + currentDateTime + "</font> ";

            teLog.append(datePart + message)
        }

        function onDump(byteArray, input) {
            let view = new DataView(byteArray)
            let hexParts = []

            for (let i = 0; i < view.byteLength; i++) {
                let hex = view.getUint8(i).toString(16).toUpperCase()
                hexParts.push(hex.length < 2 ? "0" + hex : hex)
            }

            let hexString = hexParts.join(" ")
            if (hexString.length === 0) return;
            let color = input ? "#006400" : "#00008B"
            let formattedLine = "<font color='" + color + "'>" + hexString + "</font><br>"

            teDump.append(formattedLine)
        }

        function onStateChanged(ServerState) {
            switch(ServerState)
            {
            case ServerTypes.ServerStates.Closed:
                startButton.enabled = false;
                stopButton.enabled = false;
                pauseButton.enabled = false;
                settingsButton.enabled = true;
                break;
            case ServerTypes.ServerStates.Opened:
                startButton.enabled = false;
                stopButton.enabled = false;
                pauseButton.enabled = false;
                settingsButton.enabled = true;
                break;
            case ServerTypes.ServerStates.Ready:
                startButton.enabled = true;
                stopButton.enabled = false;
                pauseButton.enabled = false;
                settingsButton.enabled = true;
                break;
            case ServerTypes.ServerStates.Waiting:
                startButton.enabled = false;
                stopButton.enabled = true;
                pauseButton.enabled = false;
                settingsButton.enabled = false;
                break;
            case ServerTypes.ServerStates.Processing:
                startButton.enabled = false;
                stopButton.enabled = true
                pauseButton.enabled = true;
                settingsButton.enabled = false;
                break;
            case ServerTypes.ServerStates.Paused:
                startButton.enabled = true;
                stopButton.enabled = true;
                pauseButton.enabled = false;
                settingsButton.enabled = false;
                break;
            case ServerTypes.ServerStates.Error:
                startButton.enabled = false;
                stopButton.enabled = false;
                pauseButton.enabled = false;
                settingsButton.enabled = true;

                break;
            default:
                break;
            }
            fileList.enabled = settingsButton.enabled || (ServerState === ServerTypes.ServerStates.Paused)
        }

    }

    header: ColumnLayout {
        spacing: 0

        ToolBar {
            id: topToolBar
            Layout.fillWidth: true
            spacing: 0

            background: Rectangle {
                color: mainWindow.palette.window
                border.color: mainWindow.palette.mid
                border.width: 1
            }

            RowLayout {
                anchors.fill: parent
                Layout.fillWidth: true
                Layout.margins: 5
                spacing: 5

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
                    opacity: enabled ? 1.0 : 0.3

                    onClicked: { HXServer.start() }
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
                    opacity: enabled ? 1.0 : 0.3

                    onClicked: { HXServer.stop() }
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
                    opacity: enabled ? 1.0 : 0.3

                    onClicked: { HXServer.pause() }
                }

                ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }

                ToolButton {
                    id: packedButton
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Packed data")

                    icon.source: "qrc:/images/icons/package-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: (packedButton.hovered || packedButton.checked) ? topToolBar.palette.highlight : topToolBar.palette.windowText
                    opacity: enabled ? 1.0 : 0.3

                    checkable: true

                    onClicked: { HXServer.setPackedData(checked) }
                }

                ToolSeparator { Layout.fillHeight: true; leftPadding: 0; rightPadding: 0 }

                ToolButton {
                    id: settingsButton
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Settings")

                    icon.source: "qrc:/images/icons/settings-light.png"
                    icon.width: 32
                    icon.height: 32
                    display: AbstractButton.TextBesideIcon
                    icon.color: settingsButton.hovered ? topToolBar.palette.highlight : topToolBar.palette.windowText
                    opacity: enabled ? 1.0 : 0.3

                    onClicked: {
                        let settingsPage = rootStack.push(rectSettings);

                        if (settingsPage)
                            settingsConnection.target = settingsPage;
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }
        // Горизонтальный разделитель между тулбаром и главным окном
        Rectangle {
            Layout.fillWidth: true
            height: 1
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            background: Rectangle { color: mainWindow.palette.window }
            Layout.margins: 5

            TabButton {
                text: qsTr("Images"); icon.source: "qrc:/images/icons/disk-light.png"
                Material.foreground: checked ? tabBar.palette.highlight : mainWindow.palette.text
            }

            TabButton {
                text: qsTr("Log"); icon.source: "qrc:/images/icons/log-light.png"
                Material.foreground: checked ? tabBar.palette.highlight : mainWindow.palette.text
            }

            TabButton {
                text: qsTr("Port dump"); icon.source: "qrc:/images/icons/dump-light.png"
                Material.foreground: checked ? tabBar.palette.highlight : mainWindow.palette.text
            }
        }
    }


    ButtonGroup {
        id: interfaceGroup

        onClicked: (button) => {

                       var isHX = (button === rbLoader)
                       Settings.setSetting("HXMode", isHX, SettingsTypes.KindSet.Misc)
                       Settings.saveSettingsByKind(SettingsTypes.KindSet.Misc)
                       mainScreen.updateHXServer()
                   }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        Binding {
            target: tabBar
            property: "currentIndex"
            value: swipeView.currentIndex
        }

        Item {
            id: imagesPage

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
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
                            text: qsTr("Loader")
                            Layout.preferredWidth: 90
                            checked: Settings.getBoolSetting("HXMode", SettingsTypes.KindSet.Misc)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            enabled: rbLoader.checked

                            TextField {
                                id: pathLoader
                                Layout.fillWidth: true
                                readOnly: true
                                placeholderText: qsTr("Select loader file...")

                                Component.onCompleted: {
                                    var loader = Settings.getSetting("loader", SettingsTypes.KindSet.Misc);
                                    var loader_view = decodeURIComponent(loader)
                                    loader_view = loader_view.split('/').pop()
                                    text = loader_view
                                }

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
                            checked: !Settings.getBoolSetting("HXMode", SettingsTypes.KindSet.Misc)
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            enabled: rbSav.checked

                            TextField {
                                id: pathSAV
                                Layout.fillWidth: true
                                readOnly: true
                                placeholderText: qsTr("Select .SAV file...")

                                Component.onCompleted: {
                                    var SAV = Settings.getSetting("savfile", SettingsTypes.KindSet.Misc);
                                    var SAV_view = decodeURIComponent(SAV)
                                    SAV_view = SAV_view.split('/').pop()
                                    text = SAV_view
                                }

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
                //Rectangle {
                //    Layout.fillWidth: true
                //    anchors.margins: 0
                //    height: 1
                //}
                //Item {
                //         Layout.preferredHeight: 1
                //}

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
                        color: mainWindow.palette.window
                        border.color: mainWindow.palette.mid

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
                    let localPath = file.toLocaleString()
                    let cleanPath = localPath.replace("file:///", "")
                    Settings.setSetting("loader", cleanPath, SettingsTypes.KindSet.Misc)
                    Settings.saveSettingsByKind(SettingsTypes.KindSet.Misc)
                    pathLoader.text = decodeURIComponent(cleanPath).split('/').pop()
                    mainScreen.updateHXServer()
                }
            }
            FileDialog {
                id: fileDialogSAV;
                //nameFilters: ["SAV files (*.sav)"];
                onAccepted: {
                    let localPath = file.toLocaleString()
                    let cleanPath = localPath.replace("file:///", "")
                    Settings.setSetting("savfile", cleanPath, SettingsTypes.KindSet.Misc)
                    Settings.saveSettingsByKind(SettingsTypes.KindSet.Misc)
                    pathSAV.text = decodeURIComponent(cleanPath).split('/').pop()
                    mainScreen.updateHXServer()
                }
            }
            FileDialog {
                id: fileDialogDSK;
                nameFilters: ["Dsk files (*.dsk)"];
                onAccepted: {
                    if (currentRowIndex >= 0) {
                        let localPath = fileDialogDSK.file.toLocaleString()
                        let cleanPath = localPath.replace("file:///", "")
                        DiskImagesModel.setFileNameAt(currentRowIndex, cleanPath)
                        DiskImagesModel.save();
                    }
                }
            }


        }
        // Экран 2: Лог
        Item {
            id: logPageWrapper

            ScrollView {
                id: logScrollView
                anchors.fill: parent
                contentWidth: width
                contentHeight: teLog.implicitHeight
                clip: true

                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextEdit {
                    id: teLog
                    width: logScrollView.width

                    leftPadding: 5
                    rightPadding: 5
                    topPadding: 5
                    bottomPadding: 5

                    readOnly: true;
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    text: ""

                    onTextChanged: { logScrollView.ScrollBar.vertical.position = 1.0 - logScrollView.ScrollBar.vertical.size }
                }
            }
        }
        // Экран 3: Дамп
        Item {
            id: dumpPageWrapper

            ScrollView {
                id: dumpScrollView
                anchors.fill: parent
                contentWidth: width
                contentHeight: teDump.implicitHeight
                clip: true

                ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TextEdit {
                    id: teDump
                    width: dumpScrollView.width

                    leftPadding: 5
                    rightPadding: 5
                    topPadding: 5
                    bottomPadding: 5

                    readOnly: true;
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    text: ""

                    onTextChanged: { dumpScrollView.ScrollBar.vertical.position = 1.0 - dumpScrollView.ScrollBar.vertical.size }
                }
            }
        }
    }
}
