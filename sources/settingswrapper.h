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
        auto s = Settings::instance();
        return s ? s->getPortsList() : QStringList();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void updatePorts()
    {
        emit portsChanged();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE QVariant getSetting(const QString& title, KindSet ks = KindSet::All) const
    {
        auto s = Settings::instance();
        if (!s) return QVariant();
        return s->getSetting(title, castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE bool getBoolSetting(const QString& title, KindSet ks = KindSet::All) const
    {
        auto s = Settings::instance();
        if (!s) return false;
        return s->getSetting(title, castKindSet(ks)).toBool();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void setSetting(const QString& title, const QVariant& value, KindSet ks = KindSet::All)
    {
        auto s = Settings::instance();
        if (s) {
            s->setSetting(title, value, castKindSet(ks));
            emit settingChanged(title);
        }
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE bool isChanged(const QString& title, KindSet ks = KindSet::All) const
    {
        auto s = Settings::instance();
        return s ? s->isChanged(title, castKindSet(ks)) : false;
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void clear(KindSet ks)
    {
        auto s = Settings::instance();
        if (s) s->clear(castKindSet(ks));
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void load()
    {
        auto s = Settings::instance();
        if (s) s->load();
    }

    //----------------------------------------------------------------------------------------------------------------------
    Q_INVOKABLE void saveSettingsByKind(KindSet ks)
    {
        auto s = Settings::instance();
        if (s) s->saveSettingsByKind(castKindSet(ks));
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
