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
    enum class KindSet {
        All = 0,
        Misc,
        ComPort,
        Environment
    };
    Q_ENUM(KindSet)

    explicit SettingsWrapper(QObject *parent = nullptr) : QObject(parent) {}

    //----------------------------------------------------------------------------------------------------------------------
    QStringList getPortsList() const
    {
        return Settings::instance().getPortsList();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void updatePorts()
    {
        emit portsChanged();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE QVariant getSetting(const QString& title, KindSet ks = KindSet::All) const
    {
        return Settings::instance().getSetting(title, castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE bool getBoolSetting(const QString& title, KindSet ks = KindSet::All) const
    {
        return Settings::instance().getSetting(title, castKindSet(ks)).toBool();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void setSetting(const QString& title, const QVariant& value, KindSet ks = KindSet::All)
    {
        Settings::instance().setSetting(title, value, castKindSet(ks));
        emit settingChanged(title);
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE bool isChanged(const QString& title, KindSet ks = KindSet::All) const
    {
        return Settings::instance().isChanged(title, castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void clear(KindSet ks)
    {
        Settings::instance().clear(castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void load()
    {
        Settings::instance().load();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void saveSettingsByKind(KindSet ks)
    {
        Settings::instance().saveSettingsByKind(castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void save()
    {
        Settings::instance().save();
    }

signals:
    void settingChanged(const QString& title);
    void portsChanged();


private:
    //----------------------------------------------------------------------------------------------------------------------
    static kindset castKindSet(KindSet ks) {
        switch (ks) {
        case KindSet::Misc:        return kindset::misc;
        case KindSet::ComPort:     return kindset::com_port;
        case KindSet::Environment: return kindset::environment;
        default:                   return kindset::all;
        }
    }

};

#endif // SETTINGSWRAPPER_H
