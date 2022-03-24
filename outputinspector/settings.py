#!/usr/bin/env python
import sys

from outputinspector.reporting import (
    warn,
    error,
    debug,
    critical,
    fatal,
)

_TRUTHIES = ["True", "1", "on", "yes"]


def is_truthy(self, value):
    '''
    @brief Check if the value is in a list of "Settings._TRUTHIES" (True yes, on, 1, ...).
    @param value
    @return
    '''
    for s in _TRUTHIES:
        if value.lower() == s:
            return True

    return False


class Settings:
    #@staticmethod
    #def is_truthy(QString value)

    #Settings()
    #Settings(QString filePath)
    #~Settings()
    #bool readIni(QString filePath)

    #bool getBool(QString key)
    #int getInt(QString key)
    #QString getString(QString key)
    #void setValue(QString key, value)
    #bool setIfMissing(QString key, value)
    #void remove(QString key)
    # QVariant value(QString key)

    #bool contains(QString key)
    #QString fileName()
    #void sync()
    #QString sDebug

    #endif # SETTINGS_H

    def __init__(self, path):
        # path="outputinspector.conf"
        self.path = path
        self.data = None  # or {}
        # public:
        self.filePath = ""
        self.checkedKeys = []
        if path is not None:
            if os.path.isfile(path):
                self.readIni(path)
        else:
            error("The Settings object has no path.")

    def __del__(self):
        if self.data is not None:
            self.sync()
        else:
            warn("The settings data was not initialized before `del`.")

    def readIni(self, path):
        lineN = 0
        self.path = path
        objName = None
        with open(path, 'r') as ins:
            for rawL in ins:
                if self.data is None:
                    self.data = {}
                lineN += 1
                line = rawL.strip()
                if line.startswith("#"):
                    continue
                if len(line) < 1:
                    continue
                if line.startswith("[") and line.endswith("]"):
                    objName = line[1:-1].strip()
                    continue

                signI = line.find("=")
                if signI < 1:
                    error("{}:{}: [outputinspector settings]"
                          " ERROR: Missing '='"
                          "".format(path, lineN))
                    continue
                name = line[:signI].strip()
                value = line[signI+1:].strip()
                self.data[name] = value


    '''*
     * @brief Instead of using qvariant.toBool, toString and custom Settings._TRUTHIES list.
     * @param name
     * @return
     '''
    def getBool(self, key):
        # self.getBool_bad="<%outputinspector(missing)%>"
        # ^ formerly static bad (was unused)
        if self.data is not None:
            if key in self.data.keys():
                if not self.checkedKeys.contains(key):
                    self.sDebug += key + ":" + self.data[key] + ".  "
                    self.checkedKeys.append(key)

                return is_truthy(self.data[key])

            elif not self.checkedKeys.contains(key):
                self.sDebug += "No " + key + " line is in " + self.path + ".  "
                self.checkedKeys.append(key)


        return False


    def getInt(self, key):
        value = 0
        if self.data is None:
            return 0

        if key in self.data.keys():
            if not self.checkedKeys.contains(key):
                self.sDebug += key + ":{}.  ".format(self.data[key])
                self.checkedKeys.append(key)
            value = int(self.data[key])

        elif not self.checkedKeys.contains(key):
            self.sDebug += "No " + key + " line is in " + self.path + ".  "
            self.checkedKeys.append(key)
        return value

    def getString(self, key):
        if self.data is None:
            return ""
        if key in self.data.keys():
            if not self.checkedKeys.contains(key):
                self.sDebug += key + ":{}.  ".format(self.data[key])
                self.checkedKeys.append(key)

            return self.data[key]

        elif not self.checkedKeys.contains(key):
            self.sDebug += "No " + key + " line is in " + self.path + ".  "
            self.checkedKeys.append(key)
        return ""


    def setValue(self, key, value):
        if self.data is None:
            error("self.data is None in setValue")
            return False
        self.data[key] = value
        return True



    def setIfMissing(self, key, value):
        '''
        @brief Set the variable if it is missing (such as for defaults).
        @param key the name of the settings variable
        @param value the value
        @return whether the value was actually changed (check sDebug for issues)
        '''
        changed = False
        if self.data is not None:
            if key not in self.data.keys():
                self.data[key] = value
                changed = True

        else:
            self.sDebug += ("setIfMissing tried to set " + key
                            + "before Settings.data was ready.")
        return changed


    def remove(self, key):
        if self.data is not None:
            if key in self.data.keys():
                del self.data[key]
        else:
            self.sDebug += ("remove tried to remove " + key
                            + "before Settings.data was ready.")


    '''
    def value(self, key):
        return self.data[key]

    '''

    def contains(self, key):
        if self.data is not None:
            return key in self.data.keys()
        return False

    def fileName(self):
        return self.path

    def sync(self):
        if self.data is None:
            warn("The settings data was not initialized before `sync`.")
            return False
        with open(self.path, 'w') as outs:
            for k, v in self.data.items():
                outs.write("{}={}\n".format(k, v))
        return True




