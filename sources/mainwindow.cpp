
#include "mainwindow.h"
#include "settings_dialog/settings_editor.h"
#include "settings_dialog/settings_images.h"
#include "settings.h"
#include "server/images.h"
#include "port_dump.h"
#include "port_state.h"

#include <QDockWidget>
#include <QTimer>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QDateTime>


class IconDockStyle: public QProxyStyle{

    QIcon m_icon;
public:
    IconDockStyle(const QIcon& icon,  QStyle* style = 0)
        : QProxyStyle(style)
        , m_icon(icon)
        {};

    virtual ~IconDockStyle() {};

    virtual void drawControl(ControlElement element, const QStyleOption* option,
                             QPainter* painter, const QWidget* widget = 0) const
    {
        if(element == QStyle::CE_DockWidgetTitle)
        {
            int width = pixelMetric(QStyle::PM_ToolBarIconSize);

            int margin = 1; //baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);

            QPoint icon_point(margin + option->rect.left(), margin + option->rect.center().y() - width /2 );

            painter->drawPixmap(icon_point, m_icon.pixmap(width, width));

            const_cast<QStyleOption*>(option)->rect = option->rect.adjusted(width, 0, 0, 0);
        }
        baseStyle()->drawControl(element, option, painter, widget);
    }
};


MainWindow* MainWindow::m_self = nullptr;

//----------------------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    setupUi(this);
    setWindowIcon(QIcon(":/images/icons/cpu-light.png"));
    //QApplication::setStyle("fusion");
    settings = Settings::instance(this, "OpenHXServer", "OpenHXServer");
    settings->load();

    hxserver = std::make_unique<HXServer>(this);
    connect(hxserver.get(), &HXServer::port_opened, this, &MainWindow::port_opened);
    connect(hxserver.get(), &HXServer::log, this, &MainWindow::log);    
    connect(hxserver.get(), &HXServer::dump, this, &MainWindow::dump);
    connect(hxserver.get(), &HXServer::baudRateChanged, this, &MainWindow::baudRateChanged);
    connect(hxserver.get(), &HXServer::flowControlChanged, this, &MainWindow::flowControlChanged);
    connect(hxserver.get(), &HXServer::stateChanged, this, &MainWindow::stateHXChanged);

    m_PortLabel.setText(m_port_label);
    m_baudRateLabel.setText(m_baudRate_label);
    m_flowControlLabel.setText(m_flowControl_label);
    m_StateLabel.setText(m_state_label);

    statusbar->addWidget(&m_PortLabel);
    statusbar->addWidget(&m_baudRateLabel);
    statusbar->addWidget(&m_flowControlLabel);
    statusbar->addWidget(&m_StateLabel, 1);

    initWidgets();
    initActions();

    QCoreApplication::processEvents();
    updateHXServer();
}


//----------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    settings->save(); 
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::initActions()
{
    m_actions.push_back(actionSettings);

    if(m_toggleImages)
    {
        menuWindows->addAction(m_toggleImages);
        m_actions.push_back(m_toggleImages);
    }
    if(m_togglePortDump)
    {
        menuWindows->addAction(m_togglePortDump);
        m_actions.push_back(m_togglePortDump);
    }


    m_startAction = new QAction(QIcon(":/images/icons/play-light.png"), tr("&Start"), this);
    m_actions.push_back(m_startAction);
    //m_startAction->setShortcuts(QKeySequence::);
    m_startAction->setToolTip(tr("Start"));
    connect(m_startAction, &QAction::triggered, hxserver.get(), &HXServer::start);

    m_stopAction = new QAction(QIcon(":/images/icons/stop-light.png"), tr("&Stop"), this);
    m_actions.push_back(m_stopAction);
    //m_stopAction->setShortcuts(QKeySequence::);
    m_stopAction->setToolTip(tr("Stop"));
    connect(m_stopAction, &QAction::triggered, hxserver.get(), &HXServer::stop);

    m_pauseAction = new QAction(QIcon(":/images/icons/pause-light.png"), tr("&Pause"), this);
    m_actions.push_back(m_pauseAction);
    m_pauseAction->setToolTip(tr("Pause"));
    connect(m_pauseAction, &QAction::triggered, hxserver.get(), &HXServer::pause);

    m_packedDataAction = new QAction(QIcon(":/images/icons/package-light.png"), tr("Packed data"), this);
    m_actions.push_back(m_packedDataAction);
    m_packedDataAction->setToolTip(tr("Packed data (RTS/CTS flow control only)"));
    m_packedDataAction->setCheckable(true);
    connect(m_packedDataAction, &QAction::triggered, hxserver.get(), &HXServer::setPackedData);

    toolBar->addAction(m_startAction);
    toolBar->addAction(m_stopAction);
    toolBar->addAction(m_pauseAction);
    toolBar->addSeparator();
    toolBar->addAction(m_packedDataAction);

    menuMain->addAction(m_startAction);
    menuMain->addAction(m_stopAction);
    menuMain->addAction(m_pauseAction);

    connect(actionSettings, &QAction::triggered, this, &MainWindow::settings_edit);
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::updateHXServer()
{
    hxserver->setPortName(settings->COMSettings().name);
    hxserver->setPortSettings();
    hxserver->setLoader(settings->getSetting("loader", kindset::misc).toString());
    hxserver->setSAVFile(settings->getSetting("savfile", kindset::misc).toString());

    bool HXMode = settings->getSetting("HXMode", kindset::misc).toBool();
    if(HXMode)
        hxserver->setServerMode(HXServer::ServerMode::HXMode);
    else
        hxserver->setServerMode(HXServer::ServerMode::SAVMode);
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::stateHXChanged(HXServer::ServerStates state)
{
    m_StateLabel.setText(m_state_label + hxserver->nameState() + "  ");
    switch(state)
    {
    case HXServer::ServerStates::Closed:
        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(true);
        break;
    case HXServer::ServerStates::Opened:
        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(true);
        break;
    case HXServer::ServerStates::Ready:
        m_startAction->setEnabled(true);
        m_stopAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(true);
        break;
        case HXServer::ServerStates::Waiting:
        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(true);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(false);
        break;
    case HXServer::ServerStates::Processing:
        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(true);
        m_pauseAction->setEnabled(true);
        actionSettings->setEnabled(false);
        break;
    case HXServer::ServerStates::Paused:
        m_startAction->setEnabled(true);
        m_stopAction->setEnabled(true);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(false);
        break;
    case HXServer::ServerStates::Error:
        m_startAction->setEnabled(false);
        m_stopAction->setEnabled(false);
        m_pauseAction->setEnabled(false);
        actionSettings->setEnabled(true);
        break;

    default:
        break;
    }
    images_widget->setEnabled(actionSettings->isEnabled() || state == HXServer::ServerStates::Paused);
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::dump(const QByteArray &value, bool in)
{
    if(dump_widget)
    {
        if(in)
            return dump_widget->add(value, Qt::darkGreen);
        dump_widget->append(value, Qt::darkBlue);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::baudRateChanged(quint32 baudRate)
{
    m_baudRateLabel.setText(m_baudRate_label + (baudRate ? QString::number(baudRate) : "") + "  ");
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::flowControlChanged(const QString &flowControl)
{
    m_flowControlLabel.setText(m_flowControl_label + flowControl + "  ");
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::initWidgets()
{
    images_widget = std::make_unique<SettingsImages>(hxserver->images());
    images_widget->setSaveImmediate(true);
    images_widget->update();
    connect(images_widget.get(), &SettingsImages::updateHX, this, &MainWindow::updateHXServer);

#ifndef Q_OS_MACOS
    dockImages->setStyle(new IconDockStyle(QIcon(":/images/icons/disk-light.png"), dockImages->style()));
#endif
    dockImages->widget()->layout()->addWidget(images_widget.get());
    m_toggleImages = dockImages->toggleViewAction();
    m_toggleImages->setShortcut(QKeySequence("F11"));

#ifndef Q_OS_MACOS
    dockDump->setStyle(new IconDockStyle(QIcon(":/images/icons/dump-light.png"), dockDump->style()));
#endif
    dump_widget = std::make_unique<PortDump>();
    dockDump->widget()->layout()->addWidget(dump_widget.get());
    m_togglePortDump = dockDump->toggleViewAction();
    m_togglePortDump->setShortcut(QKeySequence("F12"));

    portState_widget = std::make_unique<PortState>();
    statusbar->addPermanentWidget(portState_widget.get());
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::updateImagesWidget()
{
    hxserver->images().load();
    if(images_widget)
        images_widget->update();
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::settings_edit()
{    
    Images images_copy;

    images_copy = hxserver->images();

    std::unique_ptr<SettingsEditor> dialog_settings = std::make_unique<SettingsEditor>(images_copy, this);
    connect(dialog_settings.get(), &SettingsEditor::apply, this, &MainWindow::apply_settings);

    if(dialog_settings->exec() == QDialog::Accepted)
        apply_settings();
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::apply_settings()
{
    updateImagesWidget();
    updateHXServer();
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::port_opened(const QString &portName)
{
    m_PortLabel.setText(m_port_label + portName + "  ");
}

//----------------------------------------------------------------------------------------------------------------------
void MainWindow::log(const QString& value, const QColor& color, bool b_state)
{
    QString message;
    QString date_color = "#0000ff";

    if(!b_state)
        message = value;
    else
        message = hxserver->nameState() + " " + value;

    message = "<FONT color=" + color.name(QColor::HexRgb) + ">" + message + "</FONT>";

    message = "<FONT color=" + date_color + ">" + "[" + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "]</FONT> " + message;
    teLog->append(message);
}


