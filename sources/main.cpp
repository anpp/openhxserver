
#ifdef HX_QML_INTERFACE
    #include <QGuiApplication>
    //#include <QQuickStyle>
    #include <QQmlApplicationEngine>
    #include <QQmlContext>
    #include "settings_dialog/images_model.h"
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef HX_QML_INTERFACE
    //QQuickStyle::setStyle("Material");
    QGuiApplication app(argc, argv);

    Settings::instance(nullptr, "OpenHXServer", "OpenHXServer"); //для создания синглтона Settings
    Settings::instance().load();

    HXServer hxserver;
    SettingsWrapper Settings;
    std::unique_ptr<ImagesModel> DiskImagesModel = std::make_unique<ImagesModel>(hxserver.images());

    QQmlApplicationEngine engine;

    qmlRegisterUncreatableType<HXServer>("OpenHX.ServerTypes", 1, 0, "ServerTypes", "Cannot create ServerTypes in QML");
    qmlRegisterUncreatableType<SettingsWrapper>("OpenHX.SettingsTypes", 1, 0, "SettingsTypes", "Cannot create SettingsTypes in QML");

    engine.rootContext()->setContextProperty("Settings", &Settings);
    engine.rootContext()->setContextProperty("HXServer", &hxserver);
    engine.rootContext()->setContextProperty("DiskImagesModel", DiskImagesModel.get());

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
