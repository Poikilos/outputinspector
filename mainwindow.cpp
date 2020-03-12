#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QTextStream>
#include <QThread>

int iErrors = 0;
int iWarnings = 0;
int iTODOs = 0;
int iKate2TabWidth = 8;
// int iKate3TabWidth = 8;
int iCompilerTabWidth = 6;
QStringList sFiles;
//this is a very long comment (for clang-format test) Lorem ipsum dolor sit amet, consectetur adipiscing elit. Pellentesque maximus risus lectus, nec egestas tellus aliquam id. Praesent suscipit, dui in tincidunt venenatis, mi purus convallis tellus, vel pellentesque magna nulla sit amet mauris.
//this is a comment with no space before or after slashes (for clang-format test)
bool bDebug = false; //this is a trailing comment with no space before or after slashes (for clang-format test)
bool bDebugTabs = false;
#ifdef QT_DEBUG
bool bDebugParser = true; // enables line-by-line parser output
#else
bool bDebugParser = false; // enables line-by-line parser output
#endif
QString sDebug = "";
QStatusBar* statusbarNow;
// region scripting
bool bFormatErr = false;
QString sFormatErrs;
bool bShowWarningsLast = false; //TODO: implement this
int iKateRevisionMajor = 0; // i.e. 2.5.9 is 2, kde3 version; and 3.0.3 is 3, the kde4 version
bool bForceOffset = false;
bool bCompensateForKateTabDifferences = true;

class OutputInspectorSleepThread : public QThread {
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};
std::map<QString, QString> config;
bool configHas(QString key)
{
    return (config.find(key) != config.end());
}
QString configString(QString key)
{
    QString sReturn;
    std::map<QString, QString>::iterator it = config.find(key);
    if (it != config.end())
        sReturn = it->second; // first is key second is value
    return sReturn;
}
bool convertToBool(QString s)
{
    QString sLower = s.toLower();
    return (sLower == "true") || (sLower == "yes") || (sLower == "on") || (sLower == "1");
}
bool configBool(QString key)
{
    return convertToBool(configString(key));
}
int configInt(QString key)
{
    QString sReturn;
    std::map<QString, QString>::iterator it = config.find(key);
    int iReturn = 0;
    if (it != config.end()) {
        sReturn = it->second;
        bool bTest;
        iReturn = sReturn.toInt(&bTest, 10);
        if (!bTest) {
            if (sFormatErrs == "")
                sFormatErrs = sReturn;
            else
                sFormatErrs = ", " + sReturn;
            bFormatErr = true;
        }
    }
    return iReturn;
}
// endregion scripting
// region TODO: remove script vars and call configInt() directly
int xEditorOffset = 0;
int yEditorOffset = 0;
// endregion TODO: remove script vars and call configInt() directly

// Functor for Contains such as for multi-needle searches
// (see [single variable] initializer list in constructor for how haystack is
// obtained)
template <class T>
class ContainsF {
    T haystack;

public:
    ContainsF(T val)
        : haystack(val)
    {
    }
    bool operator()(T needle)
    {
        return haystack.contains(needle);
    }
};

// version which uses functor
template <class T>
bool containsAnyF(T haystack, std::list<T>& needles)
{
    return count_if(needles.begin(), needles.end(), ContainsF<T>(haystack)) > 0; // FIXME: not tested
}

template <class T>
bool contains(T haystack, T needle)
{
    return haystack.contains(needle);
}

template <class T>
bool contains_any(T haystack, std::list<T>& needles, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    return count_if(needles.begin(), needles.end(), bind(contains, haystack, std::placeholders::_1)) > 0;
}

template <class T>
bool qcontains(T haystack, T needle, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    return haystack.contains(needle, cs);
}

template <class T>
bool qcontains_any(T haystack, std::list<T>& needles, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    return count_if(needles.begin(), needles.end(), bind(qcontains<T>, haystack, std::placeholders::_1, cs)) > 0;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // init(sErrorsListFileName);
    // formats with "\n" at end must be AFTER other single-param formats that have
    // same PARSE_MARKER_FILE and PARSE_PARAM_A, because "\n" is forced
    // (which would leave extra stuff at the end if there are more markings)
    statusbarNow = ui->statusBar;
    {
        QStringList sNoseErrorMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sNoseErrorMarkers.append("");
        sNoseErrorMarkers[PARSE_MARKER_FILE] = "  File ";
        sNoseErrorMarkers[PARSE_MARKER_PARAM_A] = ", line ";
        sNoseErrorMarkers[PARSE_MARKER_PARAM_B] = ")";
        sNoseErrorMarkers[PARSE_MARKER_END_PARAMS] = "";
        sNoseErrorMarkers[PARSE_COLLECT] = "";
        sNoseErrorMarkers[PARSE_STACK] = "";
        enclosures.push_back(sNoseErrorMarkers);
    }
    {
        QStringList sNoseLowerTracebackMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sNoseLowerTracebackMarkers.append("");
        sNoseLowerTracebackMarkers[PARSE_MARKER_FILE] = "  File ";
        sNoseLowerTracebackMarkers[PARSE_MARKER_PARAM_A] = ", line ";
        sNoseLowerTracebackMarkers[PARSE_MARKER_PARAM_B] = "";
        sNoseLowerTracebackMarkers[PARSE_MARKER_END_PARAMS] = ",";
        sNoseLowerTracebackMarkers[PARSE_COLLECT] = COLLECT_REUSE;
        sNoseLowerTracebackMarkers[PARSE_STACK] = STACK_LOWER;
        enclosures.push_back(sNoseLowerTracebackMarkers);
    }
    {
        QStringList sNoseSyntaxErrorMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sNoseSyntaxErrorMarkers.append("");
        sNoseSyntaxErrorMarkers[PARSE_MARKER_FILE] = "ERROR: Failure: SyntaxError (invalid syntax ("; // 0: file marker
        sNoseSyntaxErrorMarkers[PARSE_MARKER_PARAM_A] = ", line "; // 1: ParamA marker
        sNoseSyntaxErrorMarkers[PARSE_MARKER_PARAM_B] = ""; // 2: ParamB marker (coordinate delimiter; blank if no column)
        sNoseSyntaxErrorMarkers[PARSE_MARKER_END_PARAMS] = ")"; // 3: ParamsEnder (what is after last coord)
        sNoseSyntaxErrorMarkers[PARSE_COLLECT] = ""; // 4: COLLECT (mode for when line in analyzed output should be also used as jump for following lines)
        sNoseSyntaxErrorMarkers[PARSE_STACK] = ""; // 5: set as LOWER or not (lower in stack, so probably not the error you're looking for)
        enclosures.push_back(sNoseSyntaxErrorMarkers);
    }
    {
        QStringList sNoseUpperTracebackMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sNoseUpperTracebackMarkers.append("");
        sNoseUpperTracebackMarkers[PARSE_MARKER_FILE] = "  File ";
        sNoseUpperTracebackMarkers[PARSE_MARKER_PARAM_A] = ", line ";
        sNoseUpperTracebackMarkers[PARSE_MARKER_PARAM_B] = "";
        sNoseUpperTracebackMarkers[PARSE_MARKER_END_PARAMS] = "\n";
        sNoseUpperTracebackMarkers[PARSE_COLLECT] = COLLECT_REUSE;
        sNoseUpperTracebackMarkers[PARSE_STACK] = "";
        enclosures.push_back(sNoseUpperTracebackMarkers);
    }
    {
        QStringList sMinetestLuaTracebackMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sMinetestLuaTracebackMarkers.append("");
        sMinetestLuaTracebackMarkers[PARSE_MARKER_FILE] = "ERROR[Main]:";
        sMinetestLuaTracebackMarkers[PARSE_MARKER_PARAM_A] = ":";
        sMinetestLuaTracebackMarkers[PARSE_MARKER_PARAM_B] = "";
        sMinetestLuaTracebackMarkers[PARSE_MARKER_END_PARAMS] = ":";
        sMinetestLuaTracebackMarkers[PARSE_COLLECT] = COLLECT_REUSE;
        sMinetestLuaTracebackMarkers[PARSE_STACK] = "";
        enclosures.push_back(sMinetestLuaTracebackMarkers);
    }
    {
        // TODO: (?) This comment said, "default must iterate LAST (in back)"
        //   BUT Grep syntax is simpler, so must come after it.
        QStringList sDefaultMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sDefaultMarkers.append("");
        sDefaultMarkers[PARSE_MARKER_FILE] = "";
        sDefaultMarkers[PARSE_MARKER_PARAM_A] = "(";
        sDefaultMarkers[PARSE_MARKER_PARAM_B] = ",";
        sDefaultMarkers[PARSE_MARKER_END_PARAMS] = ")";
        sDefaultMarkers[PARSE_COLLECT] = "";
        sDefaultMarkers[PARSE_STACK] = "";
        enclosures.push_back(sDefaultMarkers);
    }
    {
        QStringList sPyCodeStyleMarkers;
        // -n option for grep shows line # like:
        // <filename>:<number>:
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sPyCodeStyleMarkers.append("");
        sPyCodeStyleMarkers[PARSE_MARKER_FILE] = "";
        sPyCodeStyleMarkers[PARSE_MARKER_PARAM_A] = ":";
        sPyCodeStyleMarkers[PARSE_MARKER_PARAM_B] = ":";
        sPyCodeStyleMarkers[PARSE_MARKER_END_PARAMS] = ":";
        sPyCodeStyleMarkers[PARSE_COLLECT] = "";
        sPyCodeStyleMarkers[PARSE_STACK] = "";
        enclosures.push_back(sPyCodeStyleMarkers);
    }
    {
        QStringList sGrepWithLineMarkers;
        // -n option for grep shows line # like:
        // <filename>:<number>:
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sGrepWithLineMarkers.append("");
        sGrepWithLineMarkers[PARSE_MARKER_FILE] = "";
        sGrepWithLineMarkers[PARSE_MARKER_PARAM_A] = ":";
        sGrepWithLineMarkers[PARSE_MARKER_PARAM_B] = "";
        sGrepWithLineMarkers[PARSE_MARKER_END_PARAMS] = ":";
        sGrepWithLineMarkers[PARSE_COLLECT] = "";
        sGrepWithLineMarkers[PARSE_STACK] = "";
        enclosures.push_back(sGrepWithLineMarkers);
    }
    {
        QStringList sGrepPlainMarkers;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            sGrepPlainMarkers.append("");
        sGrepPlainMarkers[PARSE_MARKER_FILE] = "";
        sGrepPlainMarkers[PARSE_MARKER_PARAM_A] = "";
        sGrepPlainMarkers[PARSE_MARKER_PARAM_B] = "";
        sGrepPlainMarkers[PARSE_MARKER_END_PARAMS] = ":";
        sGrepPlainMarkers[PARSE_COLLECT] = "";
        sGrepPlainMarkers[PARSE_STACK] = "";
        enclosures.push_back(sGrepPlainMarkers);
    }
    brushes["TracebackNotTop"] = QBrush(QColor::fromRgb(128, 60, 0));
    brushes["Unusable"] = QBrush(Qt::lightGray);
    brushes["Internal"] = QBrush(QColor::fromRgb(192, 192, 100));
    brushes["Warning"] = QBrush(QColor::fromRgb(192, 120, 80));
    brushes["WarningDetails"] = QBrush(QColor::fromRgb(255, 180, 120));
    brushes["Error"] = QBrush(QColor::fromRgb(80, 0, 0));
    brushes["ErrorDetails"] = QBrush(QColor::fromRgb(160, 80, 80));
    brushes["ToDo"] = QBrush(Qt::darkGreen);
    brushes["Default"] = QBrush(Qt::black);

    sInternalFlags.push_back("/site-packages/");
    assert(qcontains_any<QString>("/usr/lib/python2.7/site-packages/nose/importer.py", sInternalFlags));

    sSectionBreakFlags.push_back("--------");
    assert(qcontains_any<QString>("---------------------", sSectionBreakFlags));

    qDebug().noquote() << "qDebug() stream is active.";
    // qInfo().noquote() << "qInfo() stream is active.";
    // qWarning().noquote() << "qWarning() stream is active.";
    // qCritical().noquote() << "qCritical() stream is active.";
    // qFatal().noquote() << "qFatal() stream is active.";
    if (bDebug)
        qInfo().noquote() << "lists:";
    // this for loop has the brace on the next line (for clang-format test):
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++) {
        // qInfo().noquote() << "  -";
        // for (int i=0; i<(*itList).length(); i++) {
        //     qInfo().noquote() << "    - " << (*itList)[i];
        // }
        assert(itList->size() >= PARSE_PARTS_COUNT); // qInfo().noquote() << "  items->size(): " << itList->size();
        if (bDebug)
            qInfo().noquote() << "  items: ['" + (*itList).join("', '") + "']";
    }
}

bool MainWindow::isFatalSourceError(QString sErrStreamLine)
{
    return (
        sErrStreamLine.indexOf("Can't open", 0, Qt::CaseInsensitive) > -1 //jshint could not find a source file
        || sErrStreamLine.indexOf("Too many errors", 0, Qt::CaseInsensitive) > -1 //jshint already showed the error for this line, but can't display more errors
        || sErrStreamLine.indexOf("could not be found", 0, Qt::CaseInsensitive) > -1 //mcs could not find a source file
        || sErrStreamLine.indexOf("compilation failed", 0, Qt::CaseInsensitive) > -1 //mcs could not compile the sources
    );
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::init(QString sErrorsListFileName)
{
    if (!bForceOffset)
        CompensateForEditorVersion();
    if (sErrorsListFileName.length() == 0) {
        sErrorsListFileName = "err.txt";
    }
    // QTextStream err(stderr);  // avoid quotes and escapes caused by qWarning().noquote() being derived from qDebug()--alternative to qWarning().noquote().noquote()<<
    QList<QListWidgetItem*> lwiWarnings;
    QList<QListWidgetItem*> lwiToDos;
    QFile qfileTest(sErrorsListFileName);
    QString sLine;
    QString sError = "Error";
    QString sWarning = "Warning";
    QString sCommentMark = "//";
    QStringList sToDoFlags;
    sToDoFlags.append("TODO");
    sToDoFlags.append("FIXME");
    // int cutToDoCount = 2;
    // ui->mainListWidget is a QListWidget
    // setCentralWidget(ui->mainListWidget);
    // ui->mainListWidget->setSizePolicy(QSizePolicy::)
    int lineCount = 0;
    int nonBlankLineCount = 0;
    QString actualJump = ""; // in case file & line# on different line than error, such as with nosetests
    QString actualJumpLine = "";
    bool isJumpLower = true;
    std::map<QString, QString>* info = new std::map<QString, QString>;
    if (qfileTest.exists()) {
        if (qfileTest.open(QFile::ReadOnly)) { //| QFile::Translate
            QTextStream qtextNow(&qfileTest);
            QString sLineMaster;
            QString actualJumpRow;
            QString actualJumpColumn;
            while (!qtextNow.atEnd()) {
                sLine = qtextNow.readLine(0); //does trim off newline characters
                lineCount++;
                QString sLinePrev = sLine;
                sLineMaster = sLine;

                if (sLine.length() > 0) {

                    if (sLine.trimmed().length() > 0)
                        nonBlankLineCount++;
                    if (isFatalSourceError(sLine)) {
                        ui->mainListWidget->addItem(new QListWidgetItem(sLine + " <your compiler (or other tool) recorded this fatal or summary error before outputinspector ran>", ui->mainListWidget));
                    } else if (qcontains_any<QString>(sLine, sSectionBreakFlags)) {
                        actualJump = "";
                        actualJumpLine = "";
                        actualJumpRow = "";
                        actualJumpColumn = "";
                        QListWidgetItem* lwi = new QListWidgetItem(sLine);
                        lwi->setForeground(brushes["Regular"]);
                        ui->mainListWidget->addItem(lwi);
                    } else {
                        sLine = getConvertedSourceErrorAndWarnElseGetUnmodified(sLine);
                        bool jshint_enable = false;
                        if (sLine != sLinePrev)
                            jshint_enable = true;

                        lineInfo(info, sLine, actualJump, actualJumpLine, true);
                        if (info->at("master") == "true") {
                            actualJump = info->at("file");
                            actualJumpLine = sLine;
                            actualJumpRow = info->at("row");
                            actualJumpColumn = info->at("column");
                            isJumpLower = (info->at("lower") == "true");
                            qDebug().noquote() << "(master) set actualJump to '" + actualJump + "'";
                        } else {
                            qDebug().noquote() << "(not master) line: '" + sLine + "'";
                        }
                        bool isWarning = false;
                        QString sColorPrefix = "Error";
                        if (actualJump.length() > 0) {
                            sLineMaster = actualJumpLine;
                        }
                        if (sLineMaster.indexOf(sWarning, 0, Qt::CaseInsensitive) > -1) {
                            isWarning = true;
                            sColorPrefix = "Warning";
                        }
                        // do not specify ui->mainListWidget on new, otherwise will be added automatically
                        QListWidgetItem* lwi = new QListWidgetItem(sLine);
                        if (actualJumpRow.length() > 0) {
                            lwi->setData(ROLE_ROW, QVariant(actualJumpRow));
                            lwi->setData(ROLE_COL, QVariant(actualJumpColumn));
                        } else {
                            lwi->setData(ROLE_ROW, QVariant(info->at("row")));
                            lwi->setData(ROLE_COL, QVariant(info->at("column")));
                        }
                        if (actualJump.length() > 0) {
                            lwi->setData(ROLE_COLLECTED_FILE, QVariant(actualJump));
                            if (info->at("lower") == "true")
                                lwi->setForeground(brushes["TracebackNotTop"]);
                            else if (info->at("good") == "true")
                                lwi->setForeground(brushes[sColorPrefix]);
                            else
                                lwi->setForeground(brushes[sColorPrefix + "Details"]);
                        } else {
                            lwi->setData(ROLE_COLLECTED_FILE, QVariant(info->at("file")));
                            if (info->at("good") == "true")
                                lwi->setForeground(brushes[sColorPrefix]);
                            else
                                lwi->setForeground(brushes["Unusable"]);
                        }
                        if (qcontains_any<QString>(sLineMaster, this->sInternalFlags)) {
                            lwi->setForeground(brushes["Internal"]);
                        }
                        lwi->setData(ROLE_COLLECTED_LINE, QVariant(sLineMaster));
                        lwi->setData(ROLE_DETAILS, QVariant(sLine != sLineMaster));
                        lwi->setData(ROLE_LOWER, QVariant(info->at("lower")));
                        if (info->at("good") == "true") {
                            if (isWarning)
                                iWarnings++;
                            else
                                iErrors++;
                        }
                        if (bShowWarningsLast && isWarning)
                            lwiWarnings.append(lwi);
                        else
                            ui->mainListWidget->addItem(lwi);

                        QString sTargetLanguage = (*info)["language"];

                        if (sTargetLanguage.length() > 0) {
                            if (sTargetLanguage == "python" || sTargetLanguage == "sh") {
                                sCommentMark = "#";
                            } else if (sTargetLanguage == "c++" || sTargetLanguage == "c" || sTargetLanguage == "php"
                                || sTargetLanguage == "js" || sTargetLanguage == "java") {
                                sCommentMark = "//";
                            } else if (sTargetLanguage == "bat") {
                                sCommentMark = "rem ";
                            }
                        }

                        if ((jshint_enable && (*info)["file"].endsWith(".js")) || sLine.indexOf(sError, 0, Qt::CaseInsensitive) > -1) {
                            // TODO?: if (jshint_enable || sLine.indexOf("previous error",0,Qt::CaseInsensitive)<0) iErrors++;
                            // if (bShowWarningsLast) sErrors.append(sLine);
                        }

                        if (configBool("FindTODOs")) {
                            if (info->at("good") == "true") {
                                QString sFileX = absPathOrSame(info->at("file")); // =sLine.mid(0,sLine.indexOf("("));
                                if (!sFiles.contains(sFileX, Qt::CaseSensitive)) {
                                    sFiles.append(sFileX);
                                    QFile qfileSource(sFileX);
                                    if (bDebug)
                                        qDebug() << "outputinspector trying to open '" + sFileX + "'...";
                                    // if (!qfileSource.open(QFile::ReadOnly)) {
                                    // }
                                    if (qfileSource.open(QFile::ReadOnly)) {
                                        QTextStream qtextSource(&qfileSource);
                                        int iSourceLineFindToDo = 0;
                                        while (!qtextSource.atEnd()) {
                                            QString sSourceLine = qtextSource.readLine(0);
                                            iSourceLineFindToDo++; // add first since compiler line numbering starts at 1
                                            int iToDoFound = -1;
                                            int iCommentFound = sSourceLine.indexOf(sCommentMark, 0, Qt::CaseInsensitive);
                                            if (iCommentFound > -1) {
                                                for (int i=0; i<sToDoFlags.length(); i++) {
                                                    iToDoFound = sSourceLine.indexOf(sToDoFlags[i], iCommentFound + 1, Qt::CaseInsensitive);
                                                    if (iToDoFound > -1)
                                                        break;
                                                }
                                            }
                                            if (iToDoFound > -1) {
                                                QString sNumLine;
                                                sNumLine.setNum(iSourceLineFindToDo, 10);
                                                QString sNumPos;
                                                int iCookedStart = iToDoFound;
                                                for (int iNow = 0; iNow < sSourceLine.length(); iNow++) {
                                                    if (sSourceLine.mid(iNow, 1) == "\t")
                                                        iCookedStart += (iCompilerTabWidth - 1);
                                                    else
                                                        break;
                                                }
                                                iCookedStart += 1; // start numbering at 1 to mimic compiler
                                                iCookedStart += 2; // +2 to start after slashes
                                                sNumPos.setNum(iCookedStart, 10);
                                                QString sLineToDo = sFileX + "(" + sNumLine + "," + sNumPos + ") " + sSourceLine.mid(iToDoFound);
                                                QListWidgetItem* lwi = new QListWidgetItem(sLineToDo);
                                                lwi->setData(ROLE_ROW, QVariant(sNumLine));
                                                lwi->setData(ROLE_COL, QVariant(sNumPos));
                                                lwi->setData(ROLE_COLLECTED_FILE, QVariant(sFileX));
                                                lwi->setData(ROLE_LOWER, QVariant("false"));
                                                lwi->setData(ROLE_COLLECTED_LINE, QVariant(sLineToDo));
                                                lwi->setData(ROLE_DETAILS, QVariant("false"));
                                                if (qcontains_any<QString>(sLineMaster, this->sInternalFlags)) {
                                                    lwi->setForeground(brushes["Internal"]);
                                                } else {
                                                    lwi->setForeground(brushes["ToDo"]);
                                                }
                                                lwiToDos.append(lwi);
                                                iTODOs++;
                                            }
                                        } // end while not at end of source file
                                        if (bDebug)
                                            qDebug() << "outputinspector finished reading sourcecode";
                                        if (bDebug)
                                            qDebug() << "(processed " << iSourceLineFindToDo << " line(s))";
                                        qfileSource.close();
                                    } // end if could open sourcecode
                                    else {
                                        qWarning().noquote() << "[outputinspector] could not open source file '" + sFileX + "'";
                                    }
                                } // end if list does not already contain this file
                            } // end if found filename ender
                            else if (bDebug)
                                qDebug() << "[outputinspector] WARNING: filename ender in " + sLine;
                        } // end if getIniBool("FindTODOs")
                        else
                            qDebug() << "[outputinspector] WARNING: getIniBool(\"FindTODOs\") off so skipped parsing " + sLine;
                    } // end if a regular line (not fatal, not formatting)
                } // end if length>0 (after trim using 0 option for readLine)
            } // end while not at end of file named sErrorsListFileName
            qfileTest.close();
            QString sNumErrors;
            sNumErrors.setNum(iErrors, 10);
            QString sNumWarnings;
            sNumWarnings.setNum(iWarnings, 10);
            QString sNumTODOs;
            sNumTODOs.setNum(iTODOs, 10);
            if (lwiWarnings.length() > 0) {
                for (auto it = lwiWarnings.begin(); it != lwiWarnings.end(); ++it) {
                    ui->mainListWidget->addItem(*it);
                }
            }
            if (configBool("FindTODOs")) {
                for (auto it = lwiToDos.begin(); it != lwiToDos.end(); ++it) {
                    ui->mainListWidget->addItem(*it);
                }
            }
            if (!configBool("ExitOnNoErrors")) {
                if (lineCount == 0) {
                    QListWidgetItem* lwiNew = new QListWidgetItem("#" + sErrorsListFileName + ": INFO (generated by outputinspector) 0 lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                } else if (nonBlankLineCount == 0) {
                    QListWidgetItem* lwiNew = new QListWidgetItem("#" + sErrorsListFileName + ": INFO (generated by outputinspector) 0 non-blank lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                }
            }
            // else hide errors since will exit anyway if no errors
            QString sMsg = "Errors: " + sNumErrors + "; Warnings:" + sNumWarnings;
            if (configBool("FindTODOs"))
                sMsg += "; TODOs:" + sNumTODOs;
            ui->statusBar->showMessage(sMsg, 0);
            if (configBool("ExitOnNoErrors")) {
                if (iErrors<1) {
                    qInfo() << "Closing since no errors...";
                    // QCoreApplication::quit(); // doesn't work
                    // aptr->exit(); // doesn't work (QApplication*)
                    // aptr->quit(); // doesn't work
                    // aptr->closeAllWindows(); // doesn't work
                    // if the event loop is not running, this function (QCoreApplication::exit()) does nothing
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            QString my_path = QCoreApplication::applicationFilePath();
            QMessageBox::information(this, "Output Inspector - Help", my_path + ": Output Inspector cannot read the output file due to permissions or other read error (tried \"./" + sErrorsListFileName + "\").");
        }
    } // end if could open file named sErrorsListFileName
    else {
        QString my_path = QCoreApplication::applicationFilePath();
        QMessageBox::information(this, "Output Inspector - Help", my_path + ": Output Inspector cannot find the output file to process (tried \"./" + sErrorsListFileName + "\").");
    }
    delete info;
} // end init

void MainWindow::readConfig()
{
    QFile qfileTest("/etc/outputinspector.conf");
    QString sLine;
    if (qfileTest.open(QFile::ReadOnly)) { //| QFile::Translate
        QTextStream qtextNow(&qfileTest);
        while (!qtextNow.atEnd()) {
            sLine = qtextNow.readLine(0); //does trim off newline characters
            int iSign = sLine.indexOf("=");
            if ((sLine.length() > 2) && (iSign > 0) && (iSign < (sLine.length() - 1))) {
                config[sLine.mid(0, iSign).trimmed()] = sLine.mid(iSign + 1, sLine.length() - (iSign + 1)).trimmed();
            }
        }
        cacheConfig();
        if (bDebug)
            QMessageBox::information(this, "Output Inspector - Debug", sDebug);
        if (bFormatErr) {
            QMessageBox::information(this, "Output Inspector - Configuration", "There were errors in the following: " + sFormatErrs + ".  The configuration file \"/etc/outputinspector.conf\" must not contain spaces or special characters.");
        }
        qfileTest.close();
    } else {
        if (bDebug)
            QMessageBox::information(this, "Output Inspector - Configuration", "The configuration file \"/etc/outputinspector.conf\" could not be read.");
        setConfigValue("kate", "/usr/lib/kde4/bin/kate");
    }
    if (!config.count("kate") && !bDebug) {
        QMessageBox::information(this, "Output Inspector - Configuration", "/etc/outputinspector.conf has no line reading \"kate=/usr/bin/kate\" so reverting to /usr/lib/kde4/bin/kate (in order to try to detect path and prevent this error, try running the following terminal command from inside the outputinspector directory: sudo ./install)");
    }
}

void MainWindow::setConfigValue(QString k, QString v)
{
    config[k] = v;
} // end readini
void MainWindow::CompensateForEditorVersion()
{
    bool bFound = false;
    QStringList sVersionArgs;
    QString sFileTemp = "/tmp/outputinspector.using.kate.version.tmp";
    sVersionArgs.append("--version");
    sVersionArgs.append(" > " + sFileTemp);
    // QProcess::execute(IniString("kate"), sVersionArgs);
    system((char*)QString(configString("kate") + " --version > " + sFileTemp).toLocal8Bit().data());
    OutputInspectorSleepThread::msleep(125);

    QFile qfileTmp(sFileTemp);
    QString sLine;
    if (qfileTmp.open(QFile::ReadOnly)) { // | QFile::Translate
        // detect Kate version using output of Kate command above
        QTextStream qtextNow(&qfileTmp);
        QString sKateOpener = "Kate: ";
        while (!qtextNow.atEnd()) {
            sLine = qtextNow.readLine(0); // does trim off newline characters
            if (bDebug)
                QMessageBox::information(this, "Output Inspector - Finding Kate version...", sLine);
            if (sLine.startsWith(sKateOpener, Qt::CaseInsensitive)) {
                int iDot = sLine.indexOf(".", 0);
                if (iDot > -1) {
                    bool bTest;
                    bFound = true;
                    iKateRevisionMajor = QString(sLine.mid(6, iDot - 6)).toInt(&bTest, 10);
                }
            }
        }
        qfileTmp.close();
    } // end if could open temp file
    QString sRevisionMajor;
    sRevisionMajor.setNum(iKateRevisionMajor, 10);
    if (bDebug)
        QMessageBox::information(this, "Output Inspector - Kate Version", bFound ? ("Detected Kate " + sRevisionMajor) : "Could not detect Kate version.");
    if (iKateRevisionMajor > 2) {
        xEditorOffset = 0;
        yEditorOffset = 0;
    } else {
        xEditorOffset = 0;
        yEditorOffset = 0;
        // NOTE: no longer needed
        // xEditorOffset=-1;
        // yEditorOffset=-1;
    }
} // end CompensateForEditorVersion

// converts jshint output such as:
// functions.js: line 32, col 26, Use '!==' to compare with 'null'.
// to mcs format which is a result of:
// etc/foo.cs(10,24): error CS0103: The name `Path' does not exist in the current context
QString MainWindow::getConvertedSourceErrorAndWarnElseGetUnmodified(QString sLine)
{
    QString jshint_filename_ender = ": line ";
    int src_jshint_filename_ender_i = sLine.indexOf(jshint_filename_ender);
    // on purpose for readability:
    // * string operations are done separately and as soon as required info becomes available
    // * offset is used even on the second indexOf (even thougth first search term theoretically does not ever contain the second one)
    if (src_jshint_filename_ender_i > -1) {
        QString src_filename_s = sLine.mid(0, src_jshint_filename_ender_i);
        int src_row_i = src_jshint_filename_ender_i + jshint_filename_ender.length();
        QString src_line_ender = ", col ";
        int src_line_ender_i = sLine.indexOf(src_line_ender, src_row_i + 1);
        if (src_line_ender_i > -1) {
            int src_row_len = src_line_ender_i - src_row_i;
            QString src_line_s = sLine.mid(src_row_i, src_row_len);
            int src_col_i = src_line_ender_i + src_line_ender.length();
            QString col_closer = ", ";
            int src_col_ender_i = sLine.indexOf(col_closer, src_col_i + 1);
            if (src_col_ender_i > -1) {
                int src_col_len = src_col_ender_i - src_col_i;
                QString src_col_s = sLine.mid(src_col_i, src_col_len);
                int src_comment_i = src_col_ender_i + col_closer.length();
                QString src_comment_s = sLine.mid(src_comment_i);
                sLine = src_filename_s + "(" + src_line_s + "," + src_col_s + "): " + src_comment_s;
                // if (bDebugBadHint) {
                //     QMessageBox::information(this,"Output Inspector - Parsing Notice","error format was converted to "+sLine);
                //     bDebugBadHint=false;
                // }
            } else if (bDebugBadHint) {
                QMessageBox::information(this, "Output Inspector - Parsing Error", "jshint parsing error: missing '" + col_closer + "' after column number after '" + src_line_ender + "' after '" + jshint_filename_ender + "'");
                bDebugBadHint = false;
            }
        } else if (bDebugBadHint) {
            QMessageBox::information(this, "Output Inspector - Parsing Error", "jshint parsing error: missing '" + src_line_ender + "' after '" + jshint_filename_ender + "'");
            bDebugBadHint = false;
        }
    } else if (bDebugBadHint) {
        if (bDebug) {
            QMessageBox::information(this, "Output Inspector - Parsing Notice", "Detected mcs error format"); //debug only
            bDebugBadHint = false;
        }
    }
    return sLine;
}

void MainWindow::cacheConfig()
{
    QString sTemp;
    if (configHas("kate")) {
        sDebug += "kate:" + configString("kate") + ".  ";
    } else
        sDebug += "No kate line found in /etc/outputinspector.conf.  ";
    if (configHas("ExitOnNoErrors")) {
        sTemp = configBool("ExitOnNoErrors") ? "yes" : "no";
        sDebug += "ExitOnNoErrors:" + sTemp + ".  ";
    } else
        sDebug += "No ExitOnNoErrors line found in /etc/outputinspector.conf.  ";
    if (configHas("FindTODOs")) {
        sTemp = configBool("FindTODOs") ? "yes" : "no";
        sDebug += "FindTODOs:" + sTemp + ".  ";
    } else
        sDebug += "No FindTODOs line found in /etc/outputinspector.conf.  ";
    if (configHas("xEditorOffset")) {
        xEditorOffset = configInt("xEditorOffset");
        sTemp.setNum(xEditorOffset, 10);
        sDebug += "xEditorOffset:" + sTemp + ".  ";
        bForceOffset = true;
    } else
        sDebug += "No xEditorOffset line found in /etc/outputinspector.conf.  ";
    if (configHas("yEditorOffset")) {
        yEditorOffset = configInt("yEditorOffset");
        sTemp.setNum(yEditorOffset, 10);
        sDebug += "yEditorOffset:" + sTemp + ".  ";
        bForceOffset = true;
    } else
        sDebug += "No yEditorOffset line found in /etc/outputinspector.conf.  ";
    if (configHas("Kate2TabWidth")) {
        iKate2TabWidth = configInt("Kate2TabWidth");
        sTemp.setNum(iKate2TabWidth, 10);
        sDebug += "Kate2TabWidth:" + sTemp + ".  ";
    } else
        sDebug += "No Kate2TabWidth line found in /etc/outputinspector.conf.  ";
    // if (configHas("Kate3TabWidth")) {
    //     iKate3TabWidth=configInt("Kate3TabWidth");
    //     sTemp.setNum(iKate3TabWidth,10);
    //     sDebug+="Kate3TabWidth:"+sTemp+".  ";
    // }
    // else sDebug+="No Kate3TabWidth line found in /etc/outputinspector.conf.  ";
    if (configHas("CompilerTabWidth")) {
        iCompilerTabWidth = configInt("CompilerTabWidth");
        sTemp.setNum(iCompilerTabWidth, 10);
        sDebug += "CompilerTabWidth:" + sTemp + ".  ";
    } else
        sDebug += "No CompilerTabWidth line found in /etc/outputinspector.conf.  ";
    if (configHas("ShowWarningsLast")) {
        bShowWarningsLast = configBool("ShowWarningsLast");
        sTemp = bShowWarningsLast ? "yes" : "no";
        sDebug += "ShowWarningsLast:" + sTemp + ".  ";
    } else
        sDebug += "No ShowWarningsLast line found in /etc/outputinspector.conf.  ";

} // end getConvertedSourceErrorAndWarnElseGetUnmodified

std::map<QString, QString>* MainWindow::lineInfo(const QString sLine, QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine)
{
    std::map<QString, QString>* info = new std::map<QString, QString>();
    lineInfo(info, sLine, actualJump, actualJumpLine, isPrevCallPrevLine);
    return info;
}

void MainWindow::lineInfo(std::map<QString, QString>* info, const QString sLineOriginal, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine)
{
    (*info)["file"] = ""; // same as info->at("file")
    (*info)["row"] = "";
    (*info)["sLine"] = sLineOriginal;
    (*info)["column"] = "";
    (*info)["language"] = ""; // only if language can be detected from this line
    (*info)["good"] = "false";
    (*info)["lower"] = "false";
    (*info)["master"] = "false";
    (*info)["color"] = "Default";
    QString sLine = sLineOriginal;
    sLine = getConvertedSourceErrorAndWarnElseGetUnmodified(sLine);

    QString sFileMarker;
    QString sParamAMarker;
    QString sParamBMarker;
    QString sEndParamsMarker;
    int iFileMarker = -1;
    int iFile = -1;
    int iParamAMarker = -1;
    int iParamA = -1;
    int iParamBMarker = -1;
    int iParamB = -1;
    int iEndParamsMarker = -1;
    QRegExp nonDigitRE("\\D");
    QRegExp nOrZRE("\\d*"); // a digit (\d), zero or more times (*)
    QRegExp numOrMoreRE("\\d+"); // a digit (\d), 1 or more times (+)
    if (bDebugParser) {
        qInfo().noquote() << "`" + sLineOriginal + "`:";
    }
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++) {
        if ((((*itList)[PARSE_MARKER_FILE]).length() == 0) || sLine.contains((*itList)[PARSE_MARKER_FILE])) {
            sFileMarker = (*itList)[PARSE_MARKER_FILE];
            if (bDebugParser) {
                if (sFileMarker.length() > 0)
                    qInfo().noquote() << "  looking for sFileMarker '" + sFileMarker + "'";
                }
            sParamAMarker = (*itList)[PARSE_MARKER_PARAM_A];
            sParamBMarker = (*itList)[PARSE_MARKER_PARAM_B]; // coordinate delimiter (blank if no column)
            sEndParamsMarker = (*itList)[PARSE_MARKER_END_PARAMS]; // what is after last coord ("\n" if line ends)
            if (sFileMarker.length() != 0)
                iFileMarker = sLine.indexOf(sFileMarker);
            else
                iFileMarker = 0; // if file path at begining of line
            if (iFileMarker > -1) {
                if (bDebugParser) {
                    qInfo().noquote() << "  has '" + sFileMarker + "' @ " << iFileMarker
                                      << ">= START";
                }

                if (sParamAMarker.length() > 0) {
                    iParamAMarker = sLine.indexOf(
                        sParamAMarker, iFileMarker + sFileMarker.length()
                    );
                    if (iParamAMarker >=0) {
                        if (!sLine.mid(iParamAMarker+sParamAMarker.length(), 1).contains(numOrMoreRE)) {
                            // Don't allow the opener if the next character is
                            // not a digit.
                            iParamAMarker = -1;
                        }
                    }
                } else if (sEndParamsMarker.length() > 0) {
                    iParamAMarker = sLine.indexOf(sEndParamsMarker);
                    if (iParamAMarker < 0) {
                        iParamAMarker = sLine.length();
                    }
                } else {
                    iParamAMarker = sLine.length();
                    // sParamAMarker = "<forced marker=\"" + sParamAMarker.replace("\"", "\\\"") + "\">";
                }
                if (iParamAMarker > -1) {
                    if (bDebugParser) {
                        qInfo().noquote() << "    has pre-ParamA '" + sParamAMarker + "' @"
                                          << iParamAMarker << " (after " << sFileMarker.length() << "-long file marker at "
                                          << iFileMarker
                                          << " ending at " << (iFileMarker + sFileMarker.length())
                                          << ")"; // such as ', line '
                    }
                    iParamA = iParamAMarker + sParamAMarker.length();
                    if (sParamBMarker.length() > 0) {
                        // There should be no B if there is no A, so failing
                        // in that case is OK.
                        iParamBMarker = sLine.indexOf(sParamBMarker, iParamA);
                    }
                    else {
                        iParamBMarker = iParamA;
                        // sParamBMarker = "<forced marker=\"" + sParamBMarker.replace("\"", "\\\"") + "\">";
                    }
                    if (iParamBMarker > -1) {
                        if (bDebugParser) {
                            qInfo().noquote() << "      has pre-ParamB marker '" + sParamBMarker + "' @"
                                              << iParamBMarker << " at or after ParamA marker ending at"
                                              << (iParamAMarker + sParamAMarker.length());
                        }
                        // if (sParamBMarker != (*itList)[PARSE_MARKER_PARAM_B])
                        //    sParamBMarker = ""; // since may be used to locate next value
                        if (sParamBMarker.length() > 0)
                            iParamB = iParamBMarker + sParamBMarker.length();
                        else
                            iParamB = iParamBMarker;
                        if (sEndParamsMarker.length() == 0) {
                            iEndParamsMarker = iParamB;
                            if (bDebugParser) {
                                qInfo().noquote() << "  using iParamB for iEndParamsMarker: " << iParamB;
                            }
                        } else if (sEndParamsMarker != "\n") {
                            iEndParamsMarker = sLine.indexOf(sEndParamsMarker, iParamB);
                        } else {
                            iEndParamsMarker = sLine.length();
                            // sEndParamsMarker = "<forced marker=\"" + sEndParamsMarker.replace("\"", "\\\"").replace("\n", "\\n") + "\">";
                        }
                        if (iEndParamsMarker > -1) {
                            if (sParamBMarker.length() == 0) {
                                iParamBMarker = iEndParamsMarker; // so iParamA can be calculated correctly if ends at iEndParamsMarker
                                iParamB = iEndParamsMarker;
                            }
                            if ((*itList)[PARSE_COLLECT] == COLLECT_REUSE)
                                (*info)["master"] = "true";
                            if ((*itList)[PARSE_STACK] == STACK_LOWER)
                                (*info)["lower"] = "true";
                            if (bDebugParser) {
                                qInfo().noquote() << "        has post-params '" + sEndParamsMarker.replace('\n', '\\n') + "' ending@"
                                                  << iEndParamsMarker << ">=" << (iParamBMarker + sParamBMarker.length()) << "="
                                                  << iParamBMarker << "+" << sParamBMarker.length() << "in '" + sLine + "'";
                            }
                            if (sEndParamsMarker != (*itList)[PARSE_MARKER_END_PARAMS]) {
                                // since could be used for more stuff after 2 params in future versions,
                                // length should be 0 if not found but forced:
                                sEndParamsMarker = "";
                            }
                            iFile = iFileMarker + sFileMarker.length();                            
                            break;
                        } else if (bDebugParser) {
                            qInfo().noquote() << "        no post-params '" + sEndParamsMarker + "' >="
                                              << (iParamBMarker + sParamBMarker.length()) << "in '" + sLine + "'";
                        }
                    } else if (bDebugParser) {
                        qInfo().noquote() << "      no pre-ParamB '" + sParamBMarker + "' >="
                                          << (iParamAMarker + sParamAMarker.length()) << "in '" + sLine + "'";
                    }
                } else if (bDebugParser) {
                    qInfo().noquote() << "    no pre-paramA '" + sParamAMarker + "' >="
                                      << (iFileMarker + sFileMarker.length());
                }
            } else if (bDebugParser) {
                qInfo().noquote() << "  no pre-File '" + sFileMarker + "' >= START";
            }
        }
    }

    // qInfo().noquote() << "iFileMarker: " << iFileMarker;
    // qInfo().noquote() << "iParamAMarker: " << iParamAMarker;
    // qInfo().noquote() << "iParamBMarker: " << iParamBMarker;
    // qInfo().noquote() << "iEndParamsMarker: " << iEndParamsMarker;
    // qInfo().noquote() << "sFileMarker: " << sFileMarker;
    // qInfo().noquote() << "sParamAMarker: " << sParamAMarker;
    // qInfo().noquote() << "sParamBMarker: " << sParamBMarker;
    // qInfo().noquote() << "sEndParamsMarker: " << sEndParamsMarker;
    if (iEndParamsMarker >= 0) {
        // Even if closer is not present,
        // iEndParamsMarker is set to length() IF applicable to this enclosure

        QString sFile = sLine.mid(iFile, iParamAMarker - iFile);
        sFile = sFile.trimmed();
        if (sFile.length() >= 2) {
            if ((sFile.startsWith('"') && sFile.endsWith('"')) || (sFile.startsWith('\'') && sFile.endsWith('\''))) {
                sFile = sFile.mid(1, sFile.length() - 2);
            }
        }
        (*info)["file"] = sFile;
        (*info)["row"] = sLine.mid(iParamA, iParamBMarker - iParamA);
        if (sParamBMarker.length() > 0)
            (*info)["column"] = sLine.mid(iParamB, iEndParamsMarker - iParamB);
        else
            (*info)["column"] = "";
        if (bDebugParser) qInfo().noquote() << "        file '" + sLine.mid(iFile, iParamAMarker - iFile) + "'";
        // if (bDebugParser) qInfo().noquote() << "        row '" + sLine.mid(iParamA, iParamBMarker - iParamA) + "'";
        if (bDebugParser) qInfo().noquote() << "        row '" + (*info)["row"] + "'";
        if (bDebugParser) qInfo().noquote() << "          length " << iParamBMarker << "-" << iParamA;
        //if (bDebugParser) qInfo().noquote() << "        col '" + sLine.mid(iParamB, iEndParamsMarker - iParamB) + "'";
        if (bDebugParser) qInfo().noquote() << "        col '" + (*info)["column"] + "'";
        if (bDebugParser) qInfo().noquote() << "          length " << iEndParamsMarker << "-" << iParamB;

        if (sFile.endsWith(".py", Qt::CaseSensitive))
            (*info)["language"] = "python";
        else if (sFile.endsWith(".pyw", Qt::CaseSensitive))
            (*info)["language"] = "python";
        else if (sFile.endsWith(".cpp", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (sFile.endsWith(".h", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (sFile.endsWith(".hpp", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (sFile.endsWith(".c", Qt::CaseSensitive))
            (*info)["language"] = "c";
        else if (sFile.endsWith(".js", Qt::CaseSensitive))
            (*info)["language"] = "js";
        else if (sFile.endsWith(".java", Qt::CaseSensitive))
            (*info)["language"] = "java";
        else if (sFile.endsWith(".bat", Qt::CaseSensitive))
            (*info)["language"] = "bat";
        else if (sFile.endsWith(".sh", Qt::CaseSensitive))
            (*info)["language"] = "sh";
        else if (sFile.endsWith(".command", Qt::CaseSensitive))
            (*info)["language"] = "sh";
        else if (sFile.endsWith(".php", Qt::CaseSensitive))
            (*info)["language"] = "php";
        qDebug().noquote() << "  detected file: '" + sFile + "'";
    }
    (*info)["good"] = (iEndParamsMarker > -1) ? "true" : "false";
    if ((*info)["good"] == "true") {
        if (actualJump.length() > 0 && (*info)["master"] == "false") {
            qDebug() << "INFO: nosetests output was detected, but the line is not first"
                     << "line of a known multi-line error format, so flagging as"
                     << "details (must be a sample line of code or something).";
            (*info)["good"] = "false"; // TODO: possibly eliminate this for fault tolerance
                                       // (different styles in same output)
            (*info)["details"] = "true";
            (*info)["file"] = "";
        }
    }
}

QString MainWindow::absPathOrSame(QString sFile)
{
    QString sFileAbs;
    QString sCwd = QDir::currentPath(); // current() returns a QDir object
    // Don't use QDir::separator(), since this only is detectable on *nix & bsd-like OS:
    QDir cwDir = QDir(sCwd);
    QString setuptoolsTryPkgPath = QDir::cleanPath(sCwd + QDir::separator() + cwDir.dirName());
    if (bDebug)
        qInfo().noquote() << "setuptoolsTryPkgPath:" << setuptoolsTryPkgPath;
    sFileAbs = sFile.startsWith("/", Qt::CaseInsensitive) ? sFile : (sCwd + "/" + sFile);
    if (!QFile(sFileAbs).exists() && QDir(setuptoolsTryPkgPath).exists())
        sFileAbs = QDir::cleanPath(setuptoolsTryPkgPath + QDir::separator() + sFile);
    return sFileAbs;
}

void MainWindow::on_mainListWidget_itemDoubleClicked(QListWidgetItem* item)
{
    QString sLine = item->text();
    QString actualJump = item->data(ROLE_COLLECTED_FILE).toString(); // item->toolTip();
    QString actualJumpLine = item->data(ROLE_COLLECTED_LINE).toString(); // item->toolTip();
    if (actualJumpLine.length() > 0)
        sLine = actualJumpLine;
    bool bTest = false;
    QString sFile = (item->data(ROLE_COLLECTED_FILE)).toString();
    QString sFileAbs = sFile;
    QString sErr;
    if (sFile.length() > 0) {
        if (bDebug) {
            qInfo().noquote() << "clicked_file: '" + sFile + "'";
            qInfo().noquote() << "tooltip: '" + item->toolTip() + "'";
        }
        sFileAbs = absPathOrSame(sFile);
        QString sRowInTarget = (item->data(ROLE_ROW)).toString();
        QString sColInTarget = (item->data(ROLE_COL)).toString();
        if (bDebug) {
            qInfo().noquote() << "sRowInTarget: '" + sRowInTarget + "'";
            qInfo().noquote() << "sColInTarget: '" + sColInTarget + "'";
        }
        int iRowTarget = sRowInTarget.toInt(&bTest, 10);
        int iColInTarget = sColInTarget.toInt(&bTest, 10);

        cacheConfig(); // TODO: deprecate this (mostly used to set xEditorOffset & yEditorOffset--make them local)
        // region only for Kate <= 2
        iRowTarget += yEditorOffset;
        sRowInTarget.setNum(iRowTarget, 10);
        iColInTarget += xEditorOffset;
        sColInTarget.setNum(iColInTarget, 10);
        // endregion only for Kate <= 2
        if (bCompensateForKateTabDifferences) {
            QFile qfileSource(sFileAbs);
            QString sLine;
            // bool bFoundKateCmd=false;
            int iParseSourceLine = 0;
            if (qfileSource.open(QFile::ReadOnly)) { //| QFile::Translate
                QTextStream qtextNow(&qfileSource);
                while (!qtextNow.atEnd()) {
                    sLine = qtextNow.readLine(0); //does trim off newline characters
                    if (iParseSourceLine == ((iRowTarget - yEditorOffset) - 1)) {
                        int iCountTabs = 0;
                        for (int iNow = 0; iNow < sLine.length(); iNow++) {
                            if (sLine.mid(iNow, 1) == "\t")
                                iCountTabs++;
                            else
                                break;
                        }
                        QString sDebug;
                        if (iCountTabs > 0) {
                            sDebug.setNum(iCountTabs, 10);
                            sDebug = "tabs:" + sDebug;
                            // if subtracted 1 for kate 2, the 1st character after a line with 1 tab is currently iColInTarget==6,  otherwise it is 7
                            // if subtracted 1 for kate 2, the 2nd character after a line with 1 tab is currently iColInTarget==7,  otherwise it is 8
                            // if subtracted 1 for kate 2, the 1st character after a line with 2tabs is currently iColInTarget==12, otherwise it is 13
                            // if subtracted 1 for kate 2, the 2nd character after a line with 2tabs is currently iColInTarget==13, otherwise it is 14
                            if (iKateRevisionMajor < 3)
                                iColInTarget -= xEditorOffset;
                            sDebug += "; sColInTarget-old:" + sColInTarget;
                            iColInTarget -= iCountTabs * (iCompilerTabWidth - 1);
                            //iColInTarget+=xEditorOffset;
                            sColInTarget.setNum(iColInTarget, 10);
                            sDebug += "; iColInTarget-abs:" + sColInTarget;
                            // if above worked, then iColInTarget is now an absolute character (counting tabs as 1 character)
                            // if subtracted 1 for kate 2, the 1st character after a line with 1 tab has now become iColInTarget==1,  otherwise it is 2 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 2nd character after a line with 1 tab has now become iColInTarget==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 1st character after a line with 2tabs has now become iColInTarget==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 2nd character after a line with 2tabs has now become iColInTarget==3,  otherwise it is 4 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            if (iKateRevisionMajor < 3) {
                                // Kate 2.5.9 reads a 'c' argument value of 0 as the beginning of the line and 1 as the first character after the leading tabs
                                if (iColInTarget < iCountTabs)
                                    iColInTarget = 0;
                                else {
                                    // iColInTarget currently starts at 1 at the beginning of the line
                                    iColInTarget -= (iCountTabs);
                                    sColInTarget.setNum(iColInTarget, 10);
                                    sDebug += "; iColInTarget-StartAt1-rel-to-nontab:" + sColInTarget;
                                    // iColInTarget now starts at 1 starting from the first text after tabs
                                    int iRegen = 1;
                                    sDebug += "; skips:";
                                    // int iTotalSkip=0;
                                    // for (int iNow=1; iNow<(iKate2TabWidth*2+1)&&iNow<iColInTarget; iNow+=iKate2TabWidth) {

                                    // }

                                    // This approximates how Kate 2 traverses tabs (the 'c' argument actually can't reach certain positions directly after the tabs):
                                    if (iCountTabs > 2)
                                        iColInTarget += iCountTabs - 2;
                                    for (int iNow = 1; iNow < iColInTarget; iNow++) {
                                        if (iNow <= (iCountTabs - 1) * iKate2TabWidth + 1) {
                                            if (iNow != 1 && (iNow - 1) % iKate2TabWidth == 0) {
                                                iRegen++; // only add if it is 4,7,10,etc where adder is iKate2TabWidth (1+iKate2TabWidth*position)
                                                sDebug += "-";
                                            }
                                        } else {
                                            iRegen++;
                                        }
                                    }
                                    iColInTarget = iRegen; // +( (iCountTabs>3&&iCountTabs<6) ? iCountTabs : 0 );
                                    // end accounting for kate gibberish column translation
                                }
                            }
                            // else kate 3+, which handles tabs as absolute positions
                            sColInTarget.setNum(iColInTarget, 10);
                            sDebug += "; sColInTarget-new:" + sColInTarget;
                            if (bDebugTabs)
                                QMessageBox::information(this, "Output Inspector - Debug tab compensation", sDebug);
                        } // end if iCountTabs>0
                        break;
                    } // if correct line found
                    iParseSourceLine++;
                } // while not at end of source file
                qfileSource.close();
            } // end if can open source file
            else {
                sErr = "Specified file '" + sFile + "' does not exist or is not accessible (if path looks right, try running from the location where it exists instead of '" + QDir::currentPath() + "')";
            }
        } // end if bCompensateForKateTabDifferences
        // QString sArgs="-u "+sFileAbs+" -l "+sRowInTarget+" -c "+sColInTarget;
        // QProcess qprocNow(configString("kate")+sArgs);
        // qprocNow
        if (QFile(sFileAbs).exists()) {
            sDebug = configString("kate");
            QStringList qslistArgs;
            // NOTE: -u is not needed at least as of kate 16.12.3 which does not create additional
            // instances of kate
            // qslistArgs.append("-u");
            // sDebug+=" -u";
            // qslistArgs.append("\""+sFileAbs+"\"");
            qslistArgs.append(sFileAbs);
            sDebug += " " + sFileAbs;
            qslistArgs.append("--line"); // split into separate arg, otherwise geany complains that
                                         // it doesn't understand the arg "--line 1"
            qslistArgs.append(sRowInTarget);
            sDebug += " --line " + sRowInTarget;
            // qslistArgs.append(sRowInTarget);
            qslistArgs.append("--column"); // NOTE: -c is column in kate, but alternate config dir
                                           // in geany, so use --column
            qslistArgs.append(sColInTarget); // NOTE: -c is column in kate, but alternate config dir
                                             // in geany, so use --column
            sDebug += " --column " + sColInTarget;
            // qslistArgs.append(sColInTarget);
            // qWarning().noquote() << "qslistArgs: " << qslistArgs;
            QProcess::startDetached(configString("kate"), qslistArgs);
            if (!QFile::exists(configString("kate"))) {
                // ok to run anyway for fault tolerance, since may be in system path
                QMessageBox::information(this, "Output Inspector - Configuration", configString("kate") + " cannot be accessed.  Try setting the value of kate in /etc/outputinspector.conf");
            }
            // if (bDebug)
            statusbarNow->showMessage(sDebug, 0);
            // system(sCmd);//stdlib
            // QMessageBox::information(this,"test",sCmd);
        } else
            sErr = "Specified file '" + sFileAbs + "' does not exist (try a different line, or try running from the location where it exists instead of '" + QDir::currentPath() + "')";
    } // end if line is in proper format
    else
        sErr = "Could not detect error format";
    if (sErr.length() > 0) {
        if (sFile.length() > 0) {
            qWarning().noquote() << sErr << " in '" + sLine + "':";
            qWarning().noquote() << "  actualJump: " + item->data(this->ROLE_COLLECTED_FILE).toString();
            qWarning().noquote() << "  actualJumpLine: " + item->data(this->ROLE_COLLECTED_LINE).toString();
            qWarning().noquote() << "  info:";
            std::map<QString, QString>* info = lineInfo(sLine, actualJump, actualJumpLine, false);
            // for (auto const& it : (*info)) {  // >=C++11 only (use dot notation not -> below if using this)
            for (auto it = info->begin(); it != info->end(); it++) {
                qWarning().noquote() << "    " << it->first // key
                        + ": '" + it->second + "'"; //value
            }
            QMessageBox::information(this, "Output Inspector", sErr);
            // QMessageBox::information(this,"Output Inspector","'"+sFileAbs+"' cannot be accessed (try a different line, or if this line's path looks right, try running from the location where it exists instead of '"+QDir::currentPath()+"')");
            // or pasting the entire line to 'Issues' link on web-based git repository
        } else {
            qInfo().noquote() << "No file was detected in line: '" + sLine + "'";
            qInfo().noquote() << "ERROR: '" + sErr + "'";
        }
    }
} // end MainWindow::on_mainListWidget_itemDoubleClicked
