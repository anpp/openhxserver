
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QPointer>
#include <QLabel>
#include <memory>
#include "ui_mainwindow.h"

#include "server/hxserver.h"


class SettingsImages;
class Settings;
class PortDump;
class PortState;

class MainWindow : public QMainWindow, protected Ui::MainWindow

{
    Q_OBJECT

    static MainWindow* m_self;

public:
    static MainWindow* instance(QWidget *parent = nullptr) {
        if(nullptr == m_self)
            m_self = new MainWindow(parent);
        return m_self;
    }
    ~MainWindow();

protected:
    MainWindow(QWidget *parent = nullptr);

private:
    std::unique_ptr<HXServer> hxserver;

    QLabel m_PortLabel;
    QLabel m_baudRateLabel;
    QLabel m_flowControlLabel;
    QLabel m_StateLabel;

    QString m_port_label = tr("Port name: ");
    QString m_baudRate_label = tr("Baud rate: ");
    QString m_flowControl_label = tr("Flow control: ");
    QString m_state_label = tr("State: ");

    std::shared_ptr<Settings> settings;
    std::unique_ptr<SettingsImages> images_widget;
    std::unique_ptr<PortDump> dump_widget;
    std::unique_ptr<PortState> portState_widget;

    QVector<QPointer<QAction>> m_actions;
    QPointer<QAction> m_toggleImages;
    QPointer<QAction> m_togglePortDump;

    QPointer<QAction> m_startAction;
    QPointer<QAction> m_stopAction;
    QPointer<QAction> m_pauseAction;
    QPointer<QAction> m_packedDataAction;

    void initActions();    
    void initWidgets();
    void updateImagesWidget();

public slots:
    void settings_edit();
    void apply_settings();
    void port_opened(const QString& portName);
    void log(const QString& value, const QColor& color, bool b_state = false, bool b_clear_last = false);
    void updateHXServer();
    void stateHXChanged(HXServer::ServerStates state);
    void dump(const QByteArray& value, bool in = true);
    void baudRateChanged(quint32 baudRate);
    void flowControlChanged(const QString& flowControl);
};

#endif // MAINWINDOW_H
