#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings.h"
// #include <map>
#include <vector>
#include <list>
#include <cstddef>

bool startswith(std::string haystack, std::string needle);
std::string os_path_join(std::string p1, std::string p2);

class OIColor
{
public:
    int r;
    int g;
    int b;
    static OIColor fromRgb(int r, int g,int b);
};

class OIBrush
{
public:
    OIBrush(OIColor color);
    OIColor _color;
};


class OIWidget
{
    std::string text();
    std::vector<std::string> data;
};

class OIListWidgetItem
{

};

const int OI_USER_ROLES_START = 1000;

namespace Ui {
class MainWindow;
}

class MainWindow
{
public:
    static std::string unmangledPath(std::string path);
    bool verbose = false; // This is manually set to true for debug only.
    bool m_DebugBadHints = true;
    const std::string COLLECT_REUSE = "REUSE"; /**< The target in the analyzed
                                                output should be also used as
                                                the jump location for the
                                                following lines. */
    const std::string STACK_LOWER = "LOWER"; /**< The code reference is further
                                              down in the call stack, so it is
                                              probably not pointing to the
                                              relevant code. */
    const int TOKEN_FILE = 0; /**< def[TOKEN_FILE]
                                   is the opener for the file path
                                   (blank if the file path starts
                                   at the begginning of the line). */
    const int TOKEN_PARAM_A = 1; /**< def[TOKEN_PARAM_A]
                                      is the first coordinate token
                                      (blank if none, such as grep--
                                      -n is automatically added if you
                                      use the included ogrep script). */
    const int TOKEN_PARAM_B = 2; /**< def[TOKEN_PARAM_B]
                                      is the second coordinate
                                      delimiter (blank if no column). */
    const int TOKEN_END_PARAMS = 3; /**< def[TOKEN_END_PARAMS]
                                         ParamsEnder (what is after last
                                         coord). */
    const int PARSE_COLLECT = 4; /**< def[PARSE_COLLECT]
                                      determines the mode
                                      for connecting lines. For possible values
                                      and their behaviors, see the documentation
                                      for COLLECT_REUSE (or future COLLECT_*
                                      constants). */
    const int PARSE_STACK = 5; /**< def[PARSE_STACK]
                                    flags a pattern as being for a callstack,
                                    such as to connect it to a previous error
                                    (see documentation for STACK_LOWER or for
                                    any later-added STACK_* constants). */
    const int PARSE_DESCRIPTION = 5; /**< def[PARSE_STACK]
                                          describes the parser mode (def)
                                          in a human-readable way.*/
    const int PARSE_PARTS_COUNT = 7;
    const int ROLE_COLLECTED_FILE = OI_USER_ROLES_START;
    const int ROLE_ROW = OI_USER_ROLES_START + 1;
    const int ROLE_COL = OI_USER_ROLES_START + 2;
    const int ROLE_LOWER = OI_USER_ROLES_START + 3;
    const int ROLE_COLLECTED_LINE = OI_USER_ROLES_START + 4;
    const int ROLE_DETAILS = OI_USER_ROLES_START + 5;
    std::list<std::string> sInternalFlags;
    std::list<std::string> sSectionBreakFlags;
    std::list<std::vector<std::string>> enclosures;
    std::map<std::string, OIBrush> brushes;

    MainWindow();
    ~MainWindow();
    void addLine(std::string, bool);
    void init(std::string);
    bool isFatalSourceError(std::string);
    std::map<std::string, std::string>* lineInfo(const std::string line, const std::string actualJump, const std::string actualJumpLine, bool isPrevCallPrevLine);
    void lineInfo(std::map<std::string, std::string>* info, const std::string sLineOriginal, const std::string actualJump, const std::string actualJumpLine, bool isPrevCallPrevLine);
    void debug(std::string msg);
    void warn(std::string msg);
    void info(std::string msg);
    std::string absPathOrSame(std::string filePath);
    Settings* settings = nullptr;
    bool m_EnableTabDebugMsg = false;
    bool m_CompensateForKateTabDifferences = true;
    int m_KateMajorVer = 0; /**< Use 2 to represent the 2.5.9 (kde3
                                     version); and 3 for 3.0.3 (kde4 version),
                                     etc. */
    #ifdef QT_DEBUG
    bool m_Verbose = true;
    bool m_VerboseParsing = true; /**< Enable line-by-line parser output */
    #else
    bool m_Verbose = false;
    bool m_VerboseParsing = false;
    #endif
private:
    void on_mainListWidget_itemDoubleClicked(OIWidget* item);
    void readInput();

private:
    Ui::MainWindow *ui;
    void CompensateForEditorVersion();

    std::vector<std::string> m_ToDoFlags = {"TODO","FIXME"};
    std::string m_Error = "Error";
    std::string m_Warning = "Warning";
    std::string m_CommentToken = "//";

    std::list<OIListWidgetItem*> lwiWarnings;
    std::list<OIListWidgetItem*> lwiToDos;
    int m_LineCount = 0;
    int m_NonBlankLineCount = 0;
    std::string m_ActualJump; /**< Store the jump in case the file & line# are on
                               a different line than the error, such as with
                               nosetests. */
    std::string m_ActualJumpLine;
    bool m_IsJumpLower = true;


    std::string m_MasterLine;
    std::string m_ActualJumpRow;
    std::string m_ActualJumpColumn;

    void pushWarnings(); /**< Push warnings to the GUI. */

    // QTimer* inTimer; /**< Read standard input lines regularly **/

    int iErrors = 0;
    int iWarnings = 0;
    int iTODOs = 0;
    std::vector<std::string> m_Files;
};

#endif // MAINWINDOW_H
