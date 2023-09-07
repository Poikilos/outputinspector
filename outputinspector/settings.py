#!/usr/bin/env python
from __future__ import print_function
import sys
import os
import inspect

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(MODULE_DIR)
# print("[settings] loading", file=sys.stderr)
"""
DO NOT import! Avoid circular import: outputinspector/__init__.py imports this
try:
    import outputinspector
except ImportError as ex:
    if (("No module named 'outputinspector'" in str(ex))  # Python 3
            or ("No module named outputinspector" in str(ex))):  # Python 2
        sys.path.insert(0, REPO_DIR)
        import outputinspector
    else:
        raise ex
"""
'''
from outputinspector import (
    warn,
    echo0,
    # echo1,
    # critical,
    # fatal,
)
'''
# ^ Nothing can be imported, since __init__ imports settings (otherwise
#   an incomplete module initialization error occurs).

verbosity = 0

def warn(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def echo0(*args, **kwargs):  # formerly error
    print(*args, file=sys.stderr, **kwargs)
    return True

def echo1(*args, **kwargs):  # formerly error
    if verbosity < 1:
        return False
    print(*args, file=sys.stderr, **kwargs)
    return True

_TRUTHIES = ["True", "1", "on", "yes"]


def is_truthy(value):
    '''
    @brief Check if the value is in a list of "Settings.
    _TRUTHIES" (True yes, on, 1, ...).
    @param value
    @return
    '''
    for true_str in _TRUTHIES:
        if value.lower() == true_str.lower():
            return True

    return False


class Settings:
    # @staticmethod
    # def is_truthy(QString value)

    # Settings()
    # Settings(QString filePath)
    # ~Settings()
    # bool readIni(QString filePath)

    # bool getBool(QString key)
    # int getInt(QString key)
    # QString getString(QString key)
    # void setValue(QString key, value)
    # bool setIfMissing(QString key, value)
    # void remove(QString key)
    # QVariant value(QString key)

    # bool contains(QString key)
    # QString fileName()
    # void sync()

    # endif # SETTINGS_H

    def __init__(self, path):
        self.sDebug = ""
        # path="outputinspector.conf"
        self.path = path
        self.data = None  # or {}
        self.autosave = True
        # public:
        self.filePath = ""
        self.checkedKeys = []
        if path is not None:
            if os.path.isfile(path):
                self.readIni(path)
            else:
                self.data = {}
        else:
            self.data = {}
            echo0("The Settings object has no path.")

    def __del__(self):
        try:
            callerName = inspect.stack()[1][3]
        except KeyError:
            # FIXME: Why does this happen?
            # KeyError: '__main__'
            # in inspect.py
            callerName = None
        # Python 3.5: inspect.stack()[1].function (namedtuple, so [3] still ok)
        prefix = "[Settings __del__ via %s] " % callerName
        if self.data is not None:
            self.sync()
        else:
            echo1(prefix+"The settings data was not initialized before `del`.")

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
                    echo0("{}:{}: [outputinspector settings]"
                          " ERROR: Missing '='"
                          "".format(path, lineN))
                    continue
                name = line[:signI].strip()
                value = line[signI+1:].strip()
                self.data[name] = value

    '''*
     * @brief Instead of using qvariant.toBool, toString and custom
     Settings._TRUTHIES list.
     * @param name
     * @return
     '''
    def getBool(self, key):
        # self.getBool_bad="<%outputinspector(missing)%>"
        # ^ formerly static bad (was unused)
        if self.data is not None:
            if key in self.data.keys():
                if key not in self.checkedKeys:
                    self.sDebug += key + ":" + self.data[key] + ".  "
                    self.checkedKeys.append(key)

                return is_truthy(self.data[key])

            elif key not in self.checkedKeys:
                self.sDebug += "No " + key + " line is in " + self.path + ".  "
                self.checkedKeys.append(key)

        return False

    def getInt(self, key):
        value = 0
        if self.data is None:
            return 0

        if key in self.data.keys():
            if key not in self.checkedKeys:
                self.sDebug += key + ":{}.  ".format(self.data[key])
                self.checkedKeys.append(key)
            value = int(self.data[key])

        elif key not in self.checkedKeys:
            self.sDebug += "No " + key + " line is in " + self.path + ".  "
            self.checkedKeys.append(key)
        return value

    def getString(self, key):
        if self.data is None:
            return ""
        if key in self.data.keys():
            if key not in self.checkedKeys:
                self.sDebug += key + ":{}.  ".format(self.data[key])
                self.checkedKeys.append(key)

            return self.data[key]

        elif key not in self.checkedKeys:
            self.sDebug += "No " + key + " line is in " + self.path + ".  "
            self.checkedKeys.append(key)
        return ""

    def setValue(self, key, value):
        if self.data is None:
            echo0("self.data is None in setValue")
            return False
        changed = self.data.get('key') != value
        self.data[key] = value
        if changed and self.autosave:
            self.sync()
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
                if self.autosave():
                    self.sync()
        else:
            self.sDebug += ("setIfMissing tried to set " + key
                            + "before Settings.data was ready.")
            raise RuntimeError(self.sDebug)
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
        try:
            with open(self.path, 'w') as outs:
                for k, v in self.data.items():
                    outs.write("{}={}\n".format(k, v))
        except NameError as ex:
            if "'open' is not defined" in str(ex):
                echo0("Warning: tried to save during garbage collection"
                      " but Python's builtin 'open' was already disposed.")
            else:
                raise
        return True
