#include <QFile>
#include "bells_monitor_settings.h"

Bells_monitor_settings::Bells_monitor_settings() :
    _settings(QSettings::IniFormat, QSettings::UserScope, "LYCEUM", "Bells-monitor")
{
    this->readSettings();
}

void Bells_monitor_settings::readSettings()
{
    QFile file_settings(_settings.fileName());

    if( !(file_settings.exists()) )
    {
        _settings.setValue("_settings/server_ip", "localhost");
        _settings.setValue("_settings/server_port", 80);
        _settings.setValue("_settings/textSize", 48);
        _settings.setValue("_settings/fullScreen", false);
        _settings.setValue("_settings/tableChangeTimer", 15);
        _settings.setValue("_settings/showTime", true);
        _settings.setValue("_settings/logo", "logo.png");
    }
    _server_ip_str = _settings.value("_settings/server_ip",  "localhost").toString();
    _server_port = _settings.value("_settings/server_port", 8083).toInt();
    _textSize = _settings.value("_settings/textSize", 48).toString();
    _fullScreen = _settings.value("_settings/fullScreen", false).toBool();
    _tableChangeTimer = _settings.value("_settings/tableChangeTimer", 15).toInt();
    _tableChangeTimer *=1000;
    _showTime = _settings.value("_settings/showTime", true).toBool();
    logo_path = _settings.value("_settings/logo", "logo.png").toString();

    file_settings.close();
}

QString Bells_monitor_settings::getLogo_path() const
{
    return logo_path;
}

int Bells_monitor_settings::tableChangeTimer() const
{
    return _tableChangeTimer;
}

bool Bells_monitor_settings::showTime() const
{
    return _showTime;
}

QString Bells_monitor_settings::textSize() const
{
    return _textSize;
}

bool Bells_monitor_settings::fullScreen() const
{
    return _fullScreen;
}

QString Bells_monitor_settings::server_ip_str() const
{
    return _server_ip_str;
}

int Bells_monitor_settings::server_port() const
{
    return _server_port;
}
