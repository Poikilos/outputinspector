#include "settings.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

std::vector<std::string> Settings::trues = { "true", "1", "on", "yes"};

Settings::Settings()
{
    // this->qs = new QSettings("outputinspector.conf", QSettings::IniFormat);
}

Settings::Settings(string filePath)
{
    readIni(filePath);
}

Settings::~Settings()
{
    /*
    if (this->qs != nullptr) {
        this->qs->sync();
        delete this->qs;
    }
    else {
        cerr << "WARNING: The QSettings object was not initialized before the Settings object was deleted.";
    }
    */
}

bool Settings::readIni(string filePath)
{
    this->filePath = filePath;
    /*
    if (this->qs != nullptr) {
        delete this->qs;
    } else { //if (this->qs == nullptr) {
    }
    */
    // this->qs = new QSettings(filePath, QSettings::IniFormat);
    return true;
}

/**
 * @brief Instead of using qvariant.toBool, use toString and custom trues list.
 * @param name
 * @return
 */
bool Settings::getBool(string key)
{
    static const string bad="<%outputinspector(missing)%>";
    /*
    if (this->qs != nullptr) {
        if (this->qs->contains(key)) {
            if (!checkedKeys.contains(key)) {
                this->sDebug += key + ":" + this->qs->value(key).toString() + ".  ";
                checkedKeys.push_back(key);
            }
            return is_truthy(this->qs->value(key).toString());
        } else if (!checkedKeys.contains(key)) {
            this->sDebug += "No " + key + " line is in " + this->qs->fileName() + ".  ";
            checkedKeys.push_back(key);
        }
    }
    */
    if (this->contains(key)) {
        return Settings::is_truthy(this->dat[key]);
    }
    else {
        this->sDebug += "No " + key + " line is in " + this->fileName() + ".  ";
    }
    return false;
}

int Settings::getInt(string key)
{
    int value = 0;
    /*
    if (this->qs != nullptr) {
        if (this->qs->contains(key)) {
            if (!checkedKeys.contains(key)) {
                this->sDebug += key + ":" + this->qs->value(key).toString() + ".  ";
                checkedKeys.push_back(key);
            }
            bool ok;
            value = this->qs->value(key).toInt(&ok);
            if (!ok)
                this->sDebug += "Converting " + key + "(value '"
                                + this->qs->value(key).toString() + "') failed.";
        } else if (!inList.contains(key)) {
            this->sDebug += "No " + key + " line is in " + this->qs->fileName() + ".  ";
            checkedKeys.push_back(key);
        }
    }
    */
    if (this->contains(key)) {
        std::string::size_type sz;   // alias of size_t
        value = std::stoi(this->dat[key], &sz);
    }
    else {
        this->sDebug += "No " + key + " line is in " + this->fileName() + ".  ";
    }
    return value;
}

string Settings::getString(string key)
{
    if (this->contains(key)) {
        return this->dat[key];
    }
    else {
        this->sDebug += "No " + key + " line is in " + this->fileName() + ".  ";
    }
    return "";
}

void Settings::setValue(string key, string value)
{
    // std::cerr << "setValue(" << key << ", " << value << ")" << std::endl;
    this->dat[key] = value;
    /*
    if (this->qs != nullptr) {
        this->qs->setValue(key, value);
    }
    */
}

/**
 * @brief Set the variable if it is missing (such as for defaults).
 * @param key the name of the settings variable
 * @param value the new value
 * @return whether the value was actually changed (check sDebug for issues)
 */
bool Settings::setIfMissing(string key, string value)
{
    bool changed = false;
    if (!this->contains(key)) {
        this->dat[key] = value;
        changed = true;
    }
    return changed;
}

void Settings::remove(string key)
{
    std::map<string, string>::iterator it;
    it = this->dat.find(key);
    if (it != this->dat.end()) {
        this->dat.erase(it);
    }

    /*
    if (this->qs != nullptr) {
        if (!this->qs->contains(key)) {
            this->qs->remove(key);
        }
    }
    else this->sDebug += "remove tried to remove " + key
            + "before qs was ready.";
    */
}

bool Settings::contains(string key)
{
    /*
    if (this->qs != nullptr) {
        return this->qs->contains(key);
    }
    */
    return dat.find(key) != dat.end();
}

string Settings::fileName()
{
    return this->_fileName;
    return "";
}

void Settings::sync()
{
    /*
    if (this->qs != nullptr) {
        this->qs->sync();
    }
    else {
        cerr << "WARNING: [outputinspector] Saving settings with sync() failed because the QSettings object was not initialized.";
    }
    */
}

/**
 * @brief Check if the value is in a list of "trues" (true yes, on, 1, ...).
 * @param value
 * @return
 */
bool Settings::is_truthy(string value)
{
    return std::find(Settings::trues.begin(), Settings::trues.end(), value) != Settings::trues.end();
}
