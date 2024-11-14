
#include "mainwindow.h"

#include <QApplication>

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "OpenHXServer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    QCoreApplication::setOrganizationName("");
    QCoreApplication::setApplicationName("OpenHXServer");
    QCoreApplication::setApplicationVersion("0.1");

    MainWindow *w = MainWindow::instance();
    w->show();
    int result = a.exec();
    delete w;
    return result;
}
