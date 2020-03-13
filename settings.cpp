#include <QTextStream>

#include "settings.h"

QStringList Settings::trues = QStringList() << "true" << "1" << "on" << "yes";

Settings::Settings()
{
    this->qs = new QSettings("poikilos.org");
}

Settings::Settings(QString filePath)
{
    readIni(filePath);
}

Settings::~Settings()
{
    if (this->qs != nullptr) {
        this->qs->sync();
        delete this->qs;
    }
}

bool Settings::readIni(QString filePath)
{
    this->filePath = filePath;
    if (this->qs != nullptr) {
        delete this->qs;
    } else { //if (this->qs == nullptr) {
    }
    this->qs = new QSettings(filePath, QSettings::IniFormat);
    return true;
}

/**
 * @brief Instead of using qvariant.toBool, use toString and custom trues list.
 * @param name
 * @return
 */
bool Settings::getBool(QString key)
{
    static const QString bad="<%outputinspector(missing)%>";
    if (this->qs != nullptr) {
        if (this->qs->contains(key)) {
            if (!checkedKeys.contains(key)) {
                this->sDebug += key + ":" + this->qs->value(key).toString() + ".  ";
                checkedKeys.append(key);
            }
            return truthy(this->qs->value(key).toString());
        } else if (!checkedKeys.contains(key)) {
            this->sDebug += "No " + key + " line is in " + this->qs->fileName() + ".  ";
            checkedKeys.append(key);
        }
    }
    return false;
}

int Settings::getInt(QString key)
{
    int value = 0;
    if (this->qs != nullptr) {
        if (this->qs->contains(key)) {
            if (!checkedKeys.contains(key)) {
                this->sDebug += key + ":" + this->qs->value(key).toString() + ".  ";
                checkedKeys.append(key);
            }
            bool ok;
            value = this->qs->value(key).toInt(&ok);
            if (!ok)
                this->sDebug += "Converting " + key + "(value '"
                                + this->qs->value(key).toString() + "') failed.";
        } else if (!checkedKeys.contains(key)) {
            this->sDebug += "No " + key + " line is in " + this->qs->fileName() + ".  ";
            checkedKeys.append(key);
        }
    }
    return value;
}

QString Settings::getString(QString key)
{
    if (this->qs != nullptr) {
        if (this->qs->contains(key)) {
            if (!checkedKeys.contains(key)) {
                this->sDebug += key + ":" + this->qs->value(key).toString() + ".  ";
                checkedKeys.append(key);
            }
            return this->qs->value(key).toString();
        } else if (!checkedKeys.contains(key)) {
            this->sDebug += "No " + key + " line is in " + this->qs->fileName() + ".  ";
            checkedKeys.append(key);
        }
    }
    return "";
}

void Settings::setValue(QString key, QVariant value)
{
    if (this->qs != nullptr) {
        this->qs->setValue(key, value);
    }
}

void Settings::setIfMissing(QString key, QVariant value)
{
    if (this->qs != nullptr) {
        if (!this->qs->contains(key)) {
            this->qs->setValue(key, value);
        }
    }
    else this->sDebug += "setIfMissing tried to set " + key
            + "before qs was ready.";
}

void Settings::remove(QString key)
{
    if (this->qs != nullptr) {
        if (!this->qs->contains(key)) {
            this->qs->remove(key);
        }
    }
    else this->sDebug += "remove tried to remove " + key
            + "before qs was ready.";
}

/*
QVariant Settings::value(QString key)
{
    return this->qs->value(key);
}
*/

bool Settings::contains(QString key)
{
    if (this->qs != nullptr) {
        return this->qs->contains(key);
    }
    return false;
}

/**
 * @brief Check if the value is in a list of "trues" (true yes, on, 1, ...).
 * @param value
 * @return
 */
bool Settings::truthy(QString value)
{
    for (auto s : Settings::trues) {
        if (value.toLower()==s)
            return true;
    }
    return false;
}
