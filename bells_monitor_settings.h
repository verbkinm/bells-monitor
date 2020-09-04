#ifndef BELLS_MONITOR_SETTINGS_H
#define BELLS_MONITOR_SETTINGS_H

#include <QSettings>

class Bells_monitor_settings
{
public:
    Bells_monitor_settings();


    int server_port() const;
    QString server_ip_str() const;
    bool fullScreen() const;
    QString textSize() const;
    bool showTime() const;
    int tableChangeTimer() const;

    QString getLogo_path() const;

private:
    void readSettings();

    QSettings _settings;
    bool _showTime;
    QString _server_ip_str, _textSize;
    int _server_port;
    bool _fullScreen;
    int _tableChangeTimer;
    QString logo_path;
};

#endif // BELLS_MONITOR_SETTINGS_H
