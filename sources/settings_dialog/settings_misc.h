#ifndef SETTINGS_MISC_H
#define SETTINGS_MISC_H

#include <QWidget>

namespace Ui {
class SettingsMisc;
}

class SettingsMisc : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsMisc(QWidget *parent = nullptr);
    ~SettingsMisc();

    void setPathToEmulator(const QString &value);
    const QString& pathToEmulator() const { return m_PathToEmulator; };

    void load();
    void save();

private:
    Ui::SettingsMisc *ui;

    QAction* m_openExeAction = nullptr;

    QString m_PathToEmulator;

public slots:
    void openPathToEm();

};

#endif // SETTINGS_MISC_H
