#ifndef SETTINGSWRAPPER_H
#define SETTINGSWRAPPER_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QStringList>
#include <memory>
#include "settings.h"

class SettingsWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList portsList READ getPortsList NOTIFY portsChanged)

public:
    explicit SettingsWrapper(QObject *parent = nullptr) : QObject(parent) {}

//----------------------------------------------------------------------------------------------------------------------
    QStringList getPortsList() const
    {
        auto s = Settings::instance();
        return s ? s->getPortsList() : QStringList();
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void updatePorts()
    {
        emit portsChanged();
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE QVariant getSetting(const QString& title, kindset ks = kindset::all) const
    {
        auto s = Settings::instance();
        if (!s) return QVariant();
        return s->getSetting(title, ks);
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void setSetting(const QString& title, const QVariant& value, kindset ks = kindset::all)
    {
        auto s = Settings::instance();
        if (s) {
            s->setSetting(title, value, ks);
            emit settingChanged(title);
        }
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE bool isChanged(const QString& title, kindset ks = kindset::all) const
    {
        auto s = Settings::instance();
        return s ? s->isChanged(title, ks) : false;
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void clear(kindset ks)
    {
        auto s = Settings::instance();
        if (s) s->clear(ks);
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void load()
    {
        auto s = Settings::instance();
        if (s) s->load();
    }

//----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void save()
    {
        auto s = Settings::instance();
        if (s) s->save();
    }

signals:
    void settingChanged(const QString& title);
    void portsChanged();

private:
};

#endif // SETTINGSWRAPPER_H
