#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QString>
#include <QStringList>

class Settings
{
private:
    static QStringList trues;
    // const static QStringList trues {"true", "1", "on", "yes"};
    QSettings* qs = nullptr; /**< QSettings doesn't work well as a superclass,
                                  so contain it (See <https://www.qtcentre.org/
                                  threads/2786-Qsettings> and
                                  <https://forum.qt.io/topic/43870/subclassing-
                                  qsettings-give-error-on-the-position-of-ini-
                                  file-in-linux-kubuntu/3>). */
public:
    QString filePath;
    QStringList checkedKeys;

    static bool is_truthy(QString value);

    Settings();
    Settings(QString filePath);
    ~Settings();
    bool readIni(QString filePath);

    bool getBool(QString key);
    int getInt(QString key);
    QString getString(QString key);
    void setValue(QString key, QVariant value);
    bool setIfMissing(QString key, QVariant value);
    void remove(QString key);
    // QVariant value(QString key);

    bool contains(QString key);
    QString fileName();
    void sync();
    QString sDebug;
};


#endif // SETTINGS_H
