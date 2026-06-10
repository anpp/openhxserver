
#define HX_QML_INTERFACE

#ifdef HX_QML_INTERFACE
    #include <QGuiApplication>
    #include <QQmlApplicationEngine>
    #include <QQmlContext>
    #include "server/hxserver.h"
    #include "settingswrapper.h"
#else
    #include <QApplication>
    #include <QLocale>
    #include <QTranslator>
    #include "mainwindow.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("");
    QCoreApplication::setApplicationName("OpenHXServer");
    QCoreApplication::setApplicationVersion("0.1");

#ifdef HX_QML_INTERFACE
    QGuiApplication app(argc, argv);

    Settings::instance(nullptr, "OpenHXServer", "OpenHXServer"); //для создания синглтона Settings
    Settings::instance()->load();

    HXServer hxserver;
    SettingsWrapper Settings;

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("Settings", &Settings);
    engine.rootContext()->setContextProperty("HXServer", &hxserver);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                         if (!obj && url == objUrl)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);

    QObject::connect(&engine, &QQmlApplicationEngine::warnings, [](const QList<QQmlError> &errors) {
        for (const QQmlError &error : errors) {
            qDebug() << "QML Error:" << error.toString();
        }
    });

    engine.load(url);
    return app.exec();
#else
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "OpenHXServer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow *w = MainWindow::instance();
    w->show();

    int result = app.exec();
    delete w;
    return result;
#endif
}
