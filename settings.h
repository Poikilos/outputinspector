#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>
#include <map>

// region CC BY-SA 4.0 https://stackoverflow.com/a/217605/4541104

#include <algorithm>
#include <cctype>
#include <locale>

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

static inline std::string strip(std::string s) {
    trim(s);
    return s;
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

// endregion CC BY-SA 4.0 https://stackoverflow.com/a/217605/4541104


class Settings
{
private:
    static std::vector<std::string> trues;
    // const static std::vector<std::string> trues {"true", "1", "on", "yes"};
    // QSettings* qs = nullptr;
    /**< QSettings doesn't work well as a superclass,
                                  so contain it (See <https://www.qtcentre.org/
                                  threads/2786-Qsettings> and
                                  <https://forum.qt.io/topic/43870/subclassing-
                                  qsettings-give-error-on-the-position-of-ini-
                                  file-in-linux-kubuntu/3>). */
public:
    std::map<std::string, std::string> dat;
    std::string filePath;
    std::vector<std::string> checkedKeys;

    static bool is_truthy(std::string value);

    Settings();
    Settings(std::string filePath);
    ~Settings();
    bool readIni(std::string filePath);

    bool getBool(std::string key);
    int getInt(std::string key);
    std::string getString(std::string key);
    void setValue(std::string key, std::string value);
    bool setIfMissing(std::string key, std::string value);
    void remove(std::string key);

    bool contains(std::string key);
    std::string _fileName;
    std::string fileName();
    void sync();
    std::string sDebug;
};


#endif // SETTINGS_H
