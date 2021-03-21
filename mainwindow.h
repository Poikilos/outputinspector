#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>
#include "settings.h"
// #include <map>



namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static QString unmangledPath(QString path);

    bool m_DebugBadHints = true;
    const QString COLLECT_REUSE = "REUSE"; /**< The target in the analyzed
                                                output should be also used as
                                                the jump location for the
                                                following lines. */
    const QString STACK_LOWER = "LOWER"; /**< The code reference is further
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
    const int ROLE_COLLECTED_FILE = Qt::UserRole;
    const int ROLE_ROW = Qt::UserRole + 1;
    const int ROLE_COL = Qt::UserRole + 2;
    const int ROLE_LOWER = Qt::UserRole + 3;
    const int ROLE_COLLECTED_LINE = Qt::UserRole + 4;
    const int ROLE_DETAILS = Qt::UserRole + 5;
    std::list<QString> sInternalFlags;
    std::list<QString> sSectionBreakFlags;
    std::list<QStringList> enclosures;
    std::map<QString, QBrush> brushes;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void addLine(QString, bool);
    void init(QString);
    bool isFatalSourceError(QString);
    std::map<QString, QString>* lineInfo(const QString line, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    void lineInfo(std::map<QString, QString>* info, const QString sLineOriginal, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    QString absPathOrSame(QString filePath);
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
private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);
    void readInput();

private:
    Ui::MainWindow *ui;
    void CompensateForEditorVersion();

    QStringList m_ToDoFlags = {"TODO","FIXME"};
    QString m_Error = "Error";
    QString m_Warning = "Warning";
    QString m_CommentToken = "//";

    QList<QListWidgetItem*> lwiWarnings;
    QList<QListWidgetItem*> lwiToDos;
    int m_LineCount = 0;
    int m_NonBlankLineCount = 0;
    QString m_ActualJump; /**< Store the jump in case the file & line# are on
                               a different line than the error, such as with
                               nosetests. */
    QString m_ActualJumpLine;
    bool m_IsJumpLower = true;


    QString m_MasterLine;
    QString m_ActualJumpRow;
    QString m_ActualJumpColumn;

    void pushWarnings(); /**< Push warnings to the GUI. */

    QTimer* inTimer; /**< Read standard input lines regularly **/

    int iErrors = 0;
    int iWarnings = 0;
    int iTODOs = 0;
    QStringList m_Files;
};

#endif // MAINWINDOW_H
