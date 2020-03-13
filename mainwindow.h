#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
// #include <map>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool bDebugBadHint=true;
    const QString COLLECT_REUSE = "REUSE"; /**< The target in the analyzed
                                                output should be also used as
                                                the jump location for the
                                                following lines. */
    const QString STACK_LOWER = "LOWER"; /**< The code reference is further
                                              down in the call stack, so it is
                                              probably not pointing to the
                                              relevant code. */
    const int PARSE_MARKER_FILE = 0; /**< Markers[PARSE_MARKER_FILE]
                                          is the opener for the file path
                                          (blank if the file path starts
                                          at the begginning of the line). */
    const int PARSE_MARKER_PARAM_A = 1; /**< Markers[PARSE_MARKER_PARAM_A]
                                             is the first coordinate marker
                                             (blank if none, such as grep--
                                             -n is automatically added if you
                                             use the included ogrep script). */
    const int PARSE_MARKER_PARAM_B = 2; /**< Markers[PARSE_MARKER_PARAM_B]
                                             is the second coordinate
                                             delimiter (blank if no column). */
    const int PARSE_MARKER_END_PARAMS = 3; /**< Markers[PARSE_MARKER_END_PARAMS]
                                                ParamsEnder (what is after last
                                                coord). */
    const int PARSE_COLLECT = 4; /**< Markers[PARSE_COLLECT]
                                      determines the mode
                                      for connecting lines. For possible values
                                      and their behaviors, see the documentation
                                      for COLLECT_REUSE (or future COLLECT_*
                                      constants). */
    const int PARSE_STACK = 5; /**< Markers[PARSE_STACK]
                                    flags a pattern as being for a callstack,
                                    such as to connect it to a previous error
                                    (see documentation for STACK_LOWER or for
                                    any later-added STACK_* constants). */
    const int PARSE_PARTS_COUNT = 6;
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
    // QString sErrorsListFileName="err.txt";
    ~MainWindow();
    void readConfig();
    void setConfigValue(QString k, QString v);
    void init(QString);
    bool isFatalSourceError(QString);
    std::map<QString, QString>* lineInfo(const QString sLine, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    void lineInfo(std::map<QString, QString>* info, const QString sLineOriginal, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    QString absPathOrSame(QString sFile);

private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void CompensateForEditorVersion();
    QString getConvertedSourceErrorAndWarnElseGetUnmodified(QString);
    void cacheConfig();
};

#endif // MAINWINDOW_H
