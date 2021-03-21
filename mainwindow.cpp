#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug> // Provide standard qt output streams.
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QTextStream>
#include <QThread>
#include <QStandardPaths>
#include <iostream>//this is a trailing comment with no space before or after slashes (for clang-format test)

#include "settings.h"

class OutputInspectorSleepThread : public QThread
{
public:
    static void msleep(unsigned long msecs)
    {
        QThread::msleep(msecs);
    }
};

// endregion scripting

/**
 * This is a Functor for "Contains," such as for multi-needle searches
 * (see [single variable] initializer list in constructor for how haystack is
 * obtained)
 */
template <class T>
class ContainsF
{
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

/**
 * This version uses the functor to allow using count_if with a
 * param that is determined later (by the functor using the param).
 */
template <class T>
bool containsAnyF(T haystack, std::list<T>& needles)
{
    return count_if(needles.begin(), needles.end(), ContainsF<T>(haystack)) > 0; // FIXME: Test containsAnyF
}

template <class T>
bool contains(T haystack, T needle)
{
    return haystack.contains(needle);
}

/*
template <class T>
bool contains_any(T haystack, std::list<T>& needles, Qt::CaseSensitivity cs = Qt::CaseSensitive)
{
    // TODO: use unused parameter cs.
    return count_if(needles.begin(), needles.end(), bind(contains, haystack, std::placeholders::_1)) > 0;
}
*/

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

QString MainWindow::unmangledPath(QString path)
{
    QRegularExpression literalDotsRE("\\.\\.\\.+"); /**< Match 2 dots followed by more. */
    QRegularExpressionMatch match = literalDotsRE.match(path);
    bool verbose = false; // This is manually set to true for debug only.
    if (match.hasMatch())
    {
        // int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        QList<int> tryOffsets = QList<int>();
        tryOffsets.append(-2);
        tryOffsets.append(1);
        tryOffsets.append(0);
        for (auto tryOffset : tryOffsets)
        {
            QString tryPath = path.mid(end+tryOffset);
            if (QFile(tryPath).exists())
            {
                if (verbose)
                {
                    qInfo().noquote().nospace()
                            << "[outputinspector]"
                            << " transformed *.../dir into ../dir: \""
                            << tryPath << "\"";
                }
                return tryPath;
            }
            else
            {
                if (verbose)
                {
                    qInfo().noquote().nospace()
                            << QString::fromStdString("[outputinspector] There is no \"")
                            << QString::fromStdString(tryPath.toStdString())
                            << "\" in the current directory (\""
                            << QString::fromStdString(QDir::currentPath().toStdString())
                            << "\")";
                }
            }
        }
    }
    else
    {
        if (verbose) qInfo() << "[outputinspector] There is no \"...\" in the cited path: \"" << path << "\"";
    }
    return path;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // QDir configDir = QStandardPaths::StandardLocation(QStandardPaths::ConfigLocation());
    // QStandardPaths::StandardLocation(QStandardPaths::HomeLocation)
    // ^ same as QDir::homePath()
    // See <https://stackoverflow.com/questions/32525196/
    //   how-to-get-a-settings-storage-path-in-a-cross-platform-way-in-qt>
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) qFatal("Cannot determine settings storage location");
    QDir d{path};
    if (!d.exists())
    {
        d.mkpath(d.absolutePath());
    }
    QString filePath = QDir::cleanPath(d.absolutePath() + QDir::separator() + "settings.txt");
    QFile f{filePath};
    /// TODO: fill this in or remove it (and the comments).
    if (d.exists())
    {
        if (!f.exists())
        {
            // if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            //     f.write("Hello, World");
        }
        else
        {

        }
    }
    // qInfo().noquote().nospace() << "Creating settings: " << filePath;
    this->settings = new Settings(filePath);
    qInfo().noquote().nospace()
            << "[outputinspector]"
            << " used the settings file \"" << this->settings->fileName() << "\"";
    // if (QFile("/etc/outputinspector.conf").exists()) {
    //     this->config = new Settings("/etc/outputinspector.conf");
    // }
    // else if (QFile("/etc/outputinspector.conf").exists()) {
    //     this->config = new Settings("/etc/outputinspector.conf");
    // }
    this->settings->setIfMissing("Kate2TabWidth", 8);
    this->settings->setIfMissing("CompilerTabWidth", 6);
    this->settings->setIfMissing("ShowWarningsLast", false);
    /// TODO: Implement ShowWarningsLast (but ignore it and behave as if it were
    /// false if there is anything in stdin).
    this->settings->setIfMissing("FindTODOs", true);
    if (this->settings->contains("kate"))
    {
        bool changed = this->settings->setIfMissing("editor", this->settings->getString("kate"));
        this->settings->remove("kate");
        this->settings->sync();
        if (changed)
            qInfo().noquote().nospace()
                    << "[outputinspector]" << " transferred the old setting"
                    << " 'kate=" << this->settings->getString("kate")
                    << "' to 'editor=" << this->settings->getString("editor")
                    << "'.";
        else
            qInfo().noquote().nospace()
                    << "[outputinspector]" << " ignored the deprecated setting"
                    << " 'kate=" << this->settings->getString("kate")
                    << "' in favor of 'editor="
                    << this->settings->getString("editor")
                    << "'.";
    }
    this->settings->setIfMissing("editor", "/usr/bin/geany");


    // init(errorsListFileName);
    // formats with "\n" at end must be AFTER other single-param formats that have
    // same TOKEN_FILE and PARSE_PARAM_A, because "\n" is forced
    // (which would leave extra stuff at the end if there are more tokenings)
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "  File ";
        def[TOKEN_PARAM_A] = ", line ";
        def[TOKEN_PARAM_B] = ")";
        def[TOKEN_END_PARAMS] = "";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Nose error";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "  File ";
        def[TOKEN_PARAM_A] = ", line ";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ",";
        def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = STACK_LOWER;
        def[PARSE_DESCRIPTION] = "Nose lower traceback";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "ERROR: Failure: SyntaxError (invalid syntax (";
        def[TOKEN_PARAM_A] = ", line ";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ")";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Nose syntax error";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "  File ";
        def[TOKEN_PARAM_A] = ", line ";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = "\n";
        def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Nose upper traceback";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "ERROR[Main]:";
        def[TOKEN_PARAM_A] = ":";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ":";
        // def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Minetest Lua traceback";
        enclosures.push_back(def);
    }
    {
        // An example of jshint output is the entire next comment:
        // functions.js: line 32, col 26, Use '!==' to compare with 'null'.
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = ": line ";
        def[TOKEN_PARAM_B] = ", col ";
        def[TOKEN_END_PARAMS] = ", ";
        // def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "hint from jshint";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        // TODO: change to "WARNING\[Server\].* accessed at " (requires:
        // implementing regex)
        def[TOKEN_FILE] = " accessed at ";
        def[TOKEN_PARAM_A] = ":";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = "\n";
        // def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Minetest access warning";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        /// TODO: change to "WARNING\[Server\].* accessed at " (requires:
        // implementing regex)
        def[TOKEN_FILE] = " inside a function at ";
        def[TOKEN_PARAM_A] = ":";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ".";
        // def[PARSE_COLLECT] = COLLECT_REUSE;
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "Minetest warning 'inside a function'";
        enclosures.push_back(def);
    }
    {
        QStringList def; /**< This is a fallback definition that applies
                              to various parsers.
                              Simpler definitions must be attempted in
                              order from most to least complex to avoid
                              false positives (Now there are even
                              simpler ones after this one). */

        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = "(";
        def[TOKEN_PARAM_B] = ",";
        def[TOKEN_END_PARAMS] = ")";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "generic ('path(row,*col)')";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = ":";
        def[TOKEN_PARAM_B] = ":";
        def[TOKEN_END_PARAMS] = ":";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "pycodestyle-like";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        // -n option for grep shows line # like:
        // <filename>:<number>:
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = ":";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ":";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "grep -n result";
        enclosures.push_back(def);
    }
    {
        QStringList def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.append("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = "";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ":";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "grep-like result (path then colon)";
        enclosures.push_back(def);
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
    if (this->m_Verbose)
        qInfo().noquote() << "lists:";
    // this for loop has the brace on the next line (for clang-format test):
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++)
    {
        // qInfo().noquote() << "  -";
        // for (int i=0; i<(*itList).length(); i++) {
        //     qInfo().noquote() << "    - " << (*itList)[i];
        // }
        assert(itList->size() >= PARSE_PARTS_COUNT); // qInfo().noquote() << "  items->size(): " << itList->size();
        if (this->m_Verbose)
            qInfo().noquote() << "  items: ['" + (*itList).join("', '") + "']";
    }
}

/**
 * @brief Check whether your parser reports that your code has a fatal error.
 * @param line stderr output from your parser
 * @return
 */
bool MainWindow::isFatalSourceError(QString line)
{
    return (
               line.indexOf("Can't open", 0, Qt::CaseInsensitive) > -1 //jshint could not find a source file
               || line.indexOf("Too many errors", 0, Qt::CaseInsensitive) > -1 //jshint already showed the error for this line, but can't display more errors
               || line.indexOf("could not be found", 0, Qt::CaseInsensitive) > -1 //mcs could not find a source file
               || line.indexOf("compilation failed", 0, Qt::CaseInsensitive) > -1 //mcs could not compile the sources
           );
}

MainWindow::~MainWindow()
{
    delete this->inTimer;
    delete this->settings;
    delete ui;
}
void MainWindow::init(QString errorsListFileName)
{
    if (!(this->settings->contains("xEditorOffset") || this->settings->contains("yEditorOffset")))
        CompensateForEditorVersion();
    if (errorsListFileName.length() == 0)
    {
        QString tryPath = "debug.txt";
        if (QFile(tryPath).exists())
        {
            errorsListFileName = tryPath;
            qInfo().noquote().nospace() << "[outputinspector]"
                                        << " detected \"" << tryPath << "\"...examining...";
        }
        else
            errorsListFileName = "err.txt";
    }
    // QTextStream err(stderr);  // avoid quotes and escapes caused by qWarning().noquote() being derived from qDebug()--alternative to qWarning().noquote().noquote()<<

    QFile qfileTest(errorsListFileName);
    // this->m_ToDoFlags.append("TODO");
    // this->m_ToDoFlags.append("FIXME");
    // int cutToDoCount = 2;
    // ui->mainListWidget is a QListWidget
    // setCentralWidget(ui->mainListWidget);
    // ui->mainListWidget->setSizePolicy(QSizePolicy::)
    // OutputInspectorSleepThread::msleep(150); // wait for stdin (doesn't work)
    if (std::cin.rdbuf()->in_avail() > 1)
    {
        // TODO: fix this--this never happens (Issue #16)
        qInfo().noquote().nospace() << "[outputinspector]"
                                    << " detected standard input (such as from a console pipe)"
                                    << "...skipping \"" << errorsListFileName << "\"...";
    }
    else if (qfileTest.exists())
    {
        if (qfileTest.open(QFile::ReadOnly))   //| QFile::Translate
        {
            QTextStream qtextNow(&qfileTest);
            while (!qtextNow.atEnd())
            {
                // NOTE: Readline trims off newline characters.
                this->addLine(qtextNow.readLine(0), false);
            } // end while not at end of file named errorsListFileName
            qfileTest.close();
            QString sNumErrors;
            sNumErrors.setNum(iErrors, 10);
            QString sNumWarnings;
            sNumWarnings.setNum(iWarnings, 10);
            QString sNumTODOs;
            sNumTODOs.setNum(iTODOs, 10);
            pushWarnings();
            if (this->settings->getBool("FindTODOs"))
            {
                for (auto it = this->lwiToDos.begin(); it != this->lwiToDos.end(); ++it)
                {
                    ui->mainListWidget->addItem(*it);
                }
            }
            if (!this->settings->getBool("ExitOnNoErrors"))
            {
                if (this->m_LineCount == 0)
                {
                    QListWidgetItem* lwiNew = new QListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                }
                else if (this->m_NonBlankLineCount == 0)
                {
                    QListWidgetItem* lwiNew = new QListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 non-blank lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                }
            }
            // else hide errors since will exit anyway if no errors
            QString sMsg = "Errors: " + sNumErrors + "; Warnings:" + sNumWarnings;
            if (this->settings->getBool("FindTODOs"))
                sMsg += "; TODOs:" + sNumTODOs;
            ui->statusBar->showMessage(sMsg, 0);
            if (this->settings->getBool("ExitOnNoErrors"))
            {
                if (iErrors<1)
                {
                    qInfo() << "Closing since no errors...";
                    // QCoreApplication::quit(); // doesn't work
                    // aptr->exit(); // doesn't work (QApplication*)
                    // aptr->quit(); // doesn't work
                    // aptr->closeAllWindows(); // doesn't work
                    // if the event loop is not running, this function (QCoreApplication::exit()) does nothing
                    exit(EXIT_FAILURE);
                }
            }
        }
        else
        {
            if (std::cin.rdbuf()->in_avail() < 1)
            {
                QString my_path = QCoreApplication::applicationFilePath();
                QString title = "Output Inspector - Help";
                QString msg = my_path + ": Output Inspector cannot read the output file due to permissions or other read error (tried \"./" + errorsListFileName + "\").";
                QMessageBox::information(this, title, msg);
                // this->addLine(title + ":" + msg, true);
            }
        }
    } // end if could open file named errorsListFileName
    else
    {
        if (std::cin.rdbuf()->in_avail() < 1)
        {
            QString my_path = QCoreApplication::applicationFilePath();
            QString title = "Output Inspector - Help";
            QString msg = my_path + ": Output Inspector cannot find the output file to process (tried \"./" + errorsListFileName + "\").";
            QMessageBox::information(this, title, msg);
            // this->addLine(title + ":" + msg, true);
        }
    }
    this->inTimer = new QTimer(this);
    this->inTimer->setInterval(500);  // milliseconds
    connect(this->inTimer, SIGNAL(timeout()), this, SLOT(readInput()));
    this->inTimer->start();
} // end init

/**
 * This method will add or collect a line. This method sets some related private
 * variables for the purpose of connecting a line (such as a callstack line)
 * to a previous line.
 *
 * @brief Add a line.
 * @param line a line from standard output or error from a program
 * @param enablePush Push the line to the GUI right away (This is best for
 *        when reading information from standard input).
 */
void MainWindow::addLine(QString line, bool enablePush)
{
    this->m_LineCount++;
    QString originalLine = line;
    this->m_MasterLine = line;
    // TODO: debug performance of new and delete
    std::map<QString, QString>* info = new std::map<QString, QString>;
    if (line.length() > 0)
    {
        if (line.trimmed().length() > 0)
            this->m_NonBlankLineCount++;
        if (isFatalSourceError(line))
        {
            ui->mainListWidget->addItem(new QListWidgetItem(line + " <your compiler (or other tool) recorded this fatal or summary error before outputinspector ran>", ui->mainListWidget));
        }
        else if (qcontains_any<QString>(line, sSectionBreakFlags))
        {
            this->m_ActualJump = "";
            this->m_ActualJumpLine = "";
            this->m_ActualJumpRow = "";
            this->m_ActualJumpColumn = "";
            QListWidgetItem* lwi = new QListWidgetItem(line);
            lwi->setForeground(brushes["Regular"]);
            ui->mainListWidget->addItem(lwi);
        }
        else
        {
            // lineInfo does the actual parsing:
            lineInfo(info, line, this->m_ActualJump, this->m_ActualJumpLine, true);

            if (info->at("master") == "true")
            {
                this->m_ActualJump = info->at("file");
                this->m_ActualJumpLine = line;
                this->m_ActualJumpRow = info->at("row");
                this->m_ActualJumpColumn = info->at("column");
                this->m_IsJumpLower = (info->at("lower") == "true");
                qDebug().noquote() << "(master) set actualJump to '" + this->m_ActualJump + "'";
            }
            else
            {
                qDebug().noquote() << "(not master) line: '" + line + "'";
            }
            bool isWarning = false;
            QString sColorPrefix = "Error";
            if (this->m_ActualJump.length() > 0)
            {
                this->m_MasterLine = this->m_ActualJumpLine;
            }
            if (this->m_MasterLine.indexOf(this->m_Warning, 0, Qt::CaseInsensitive) > -1)
            {
                isWarning = true;
                sColorPrefix = "Warning";
            }
            // do not specify ui->mainListWidget on new, otherwise will be added automatically
            QListWidgetItem* lwi = new QListWidgetItem(line);
            if (this->m_ActualJumpRow.length() > 0)
            {
                lwi->setData(ROLE_ROW, QVariant(this->m_ActualJumpRow));
                lwi->setData(ROLE_COL, QVariant(this->m_ActualJumpColumn));
            }
            else
            {
                lwi->setData(ROLE_ROW, QVariant(info->at("row")));
                lwi->setData(ROLE_COL, QVariant(info->at("column")));
            }
            if (this->m_ActualJump.length() > 0)
            {
                lwi->setData(ROLE_COLLECTED_FILE, QVariant(this->m_ActualJump));
                if (info->at("lower") == "true")
                    lwi->setForeground(brushes["TracebackNotTop"]);
                else if (info->at("good") == "true")
                    lwi->setForeground(brushes[sColorPrefix]);
                else
                    lwi->setForeground(brushes[sColorPrefix + "Details"]);
            }
            else
            {
                lwi->setData(ROLE_COLLECTED_FILE, QVariant(info->at("file")));
                if (info->at("good") == "true")
                    lwi->setForeground(brushes[sColorPrefix]);
                else
                    lwi->setForeground(brushes["Unusable"]);
            }
            if (qcontains_any<QString>(this->m_MasterLine, this->sInternalFlags))
            {
                lwi->setForeground(brushes["Internal"]);
            }
            lwi->setData(ROLE_COLLECTED_LINE, QVariant(this->m_MasterLine));
            lwi->setData(ROLE_DETAILS, QVariant(line != this->m_MasterLine));
            lwi->setData(ROLE_LOWER, QVariant(info->at("lower")));
            if (info->at("good") == "true")
            {
                if (isWarning)
                    iWarnings++;
                else
                    iErrors++;
            }
            if (this->settings->getBool("ShowWarningsLast") && isWarning)
                this->lwiWarnings.append(lwi);
            else
                ui->mainListWidget->addItem(lwi);

            QString sTargetLanguage = (*info)["language"];

            if (sTargetLanguage.length() > 0)
            {
                if (sTargetLanguage == "python" || sTargetLanguage == "sh")
                {
                    this->m_CommentToken = "#";
                }
                else if (sTargetLanguage == "c++" || sTargetLanguage == "c" || sTargetLanguage == "php"
                         || sTargetLanguage == "js" || sTargetLanguage == "java")
                {
                    this->m_CommentToken = "//";
                }
                else if (sTargetLanguage == "bat")
                {
                    this->m_CommentToken = "rem ";
                }
            }

            //if ((is_jshint && (*info)["file"].endsWith(".js")) || line.indexOf(this->m_Error, 0, Qt::CaseInsensitive) > -1) {
            //  // TODO?: if (is_jshint|| line.indexOf("previous error",0,Qt::CaseInsensitive)<0) iErrors++;
            //  // if (this->config->getBool("ShowWarningsLast")) this->m_Errors.append(line);
            //}

            if (this->settings->getBool("FindTODOs"))
            {
                if (info->at("good") == "true")
                {
                    QString sFileX;// = unmangledPath(info->at("file"));
                    sFileX = absPathOrSame(sFileX); // =line.mid(0,line.indexOf("("));
                    if (!this->m_Files.contains(sFileX, Qt::CaseSensitive))
                    {
                        this->m_Files.append(sFileX);
                        QFile qfileSource(sFileX);
                        if (this->m_Verbose)
                            qDebug() << "outputinspector trying to open '" + sFileX + "'...";
                        // if (!qfileSource.open(QFile::ReadOnly)) {
                        // }
                        if (qfileSource.open(QFile::ReadOnly))
                        {
                            QTextStream qtextSource(&qfileSource);
                            int iSourceLineFindToDo = 0;
                            while (!qtextSource.atEnd())
                            {
                                QString sSourceLine = qtextSource.readLine(0);
                                iSourceLineFindToDo++; // Increment this now since the compiler's line numbering starts with 1.
                                int iToDoFound = -1;
                                int iCommentFound = sSourceLine.indexOf(this->m_CommentToken, 0, Qt::CaseInsensitive);
                                if (iCommentFound > -1)
                                {
                                    for (int i=0; i<this->m_ToDoFlags.length(); i++)
                                    {
                                        iToDoFound = sSourceLine.indexOf(this->m_ToDoFlags[i], iCommentFound + 1, Qt::CaseInsensitive);
                                        if (iToDoFound > -1)
                                            break;
                                    }
                                }
                                if (iToDoFound > -1)
                                {
                                    QString sNumLine;
                                    sNumLine.setNum(iSourceLineFindToDo, 10);
                                    QString sNumPos;
                                    int processedCol = iToDoFound;
                                    for (int citedI = 0; citedI < sSourceLine.length(); citedI++)
                                    {
                                        if (sSourceLine.mid(citedI, 1) == "\t")
                                            processedCol += (this->settings->getInt("CompilerTabWidth") - 1);
                                        else
                                            break;
                                    }
                                    processedCol += 1; // start numbering at 1 to mimic compiler
                                    processedCol += 2; // +2 to start after slashes
                                    sNumPos.setNum(processedCol, 10);
                                    QString sLineToDo = sFileX + "(" + sNumLine + "," + sNumPos + ") " + sSourceLine.mid(iToDoFound);
                                    QListWidgetItem* lwi = new QListWidgetItem(sLineToDo);
                                    lwi->setData(ROLE_ROW, QVariant(sNumLine));
                                    lwi->setData(ROLE_COL, QVariant(sNumPos));
                                    lwi->setData(ROLE_COLLECTED_FILE, QVariant(sFileX));
                                    lwi->setData(ROLE_LOWER, QVariant("false"));
                                    lwi->setData(ROLE_COLLECTED_LINE, QVariant(sLineToDo));
                                    lwi->setData(ROLE_DETAILS, QVariant("false"));
                                    if (qcontains_any<QString>(this->m_MasterLine, this->sInternalFlags))
                                    {
                                        lwi->setForeground(brushes["Internal"]);
                                    }
                                    else
                                    {
                                        lwi->setForeground(brushes["ToDo"]);
                                    }
                                    this->lwiToDos.append(lwi);
                                    iTODOs++;
                                }
                            } // end while not at end of source file
                            if (this->m_Verbose)
                                qDebug() << "outputinspector finished reading sourcecode";
                            if (this->m_Verbose)
                                qDebug() << "(processed " << iSourceLineFindToDo << " line(s))";
                            qfileSource.close();
                        } // end if could open sourcecode
                        else
                        {
                            qWarning().noquote() << "[outputinspector] did not scan a file that is cited by the log but that is not present: '" + sFileX + "'";
                        }
                    } // end if list does not already contain this file
                } // end if found filename ender
                else if (this->m_Verbose)
                    qDebug() << "[outputinspector] WARNING: filename ender in " + line;
            } // end if getIniBool("FindTODOs")
            else
                qDebug() << "[outputinspector] WARNING: getIniBool(\"FindTODOs\") off so skipped parsing " + line;
        } // end if a regular line (not fatal, not formatting)
    } // end if length>0 (after trim using 0 option for readLine)
    if (enablePush)
    {
        this->pushWarnings();
    }
    delete info;
}

void MainWindow::CompensateForEditorVersion()
{
    bool isFound = false;
    QStringList sVersionArgs;
    QString sFileTemp = "/tmp/outputinspector.using.kate.version.tmp";
    sVersionArgs.append("--version");
    sVersionArgs.append(" > " + sFileTemp);
    // QProcess::execute(IniString("editor"), sVersionArgs);
    system((char*)QString(this->settings->getString("editor") + " --version > " + sFileTemp).toLocal8Bit().data());
    OutputInspectorSleepThread::msleep(125);

    QFile qfileTmp(sFileTemp);
    QString line;
    if (qfileTmp.open(QFile::ReadOnly))   // | QFile::Translate
    {
        // detect Kate version using output of Kate command above
        QTextStream qtextNow(&qfileTmp);
        QString sKateOpener = "Kate: ";
        while (!qtextNow.atEnd())
        {
            line = qtextNow.readLine(0); // does trim off newline characters
            if (this->m_Verbose)
                QMessageBox::information(this, "Output Inspector - Finding Kate version...", line);
            if (line.startsWith(sKateOpener, Qt::CaseInsensitive))
            {
                int iDot = line.indexOf(".", 0);
                if (iDot > -1)
                {
                    bool ok;
                    isFound = true;
                    this->m_KateMajorVer = QString(line.mid(6, iDot - 6)).toInt(&ok, 10);
                }
            }
        }
        qfileTmp.close();
    } // end if could open temp file
    QString sRevisionMajor;
    sRevisionMajor.setNum(this->m_KateMajorVer, 10);
    if (this->m_Verbose)
        QMessageBox::information(this, "Output Inspector - Kate Version", isFound ? ("Detected Kate " + sRevisionMajor) : "Could not detect Kate version.");
    if (this->m_KateMajorVer > 2)
    {
        this->settings->setValue("xEditorOffset", 0);
        this->settings->setValue("yEditorOffset", 0);
    }
    else
    {
        this->settings->setValue("xEditorOffset", 0);
        this->settings->setValue("yEditorOffset", 0);
        // NOTE: The values are no longer necessary.
        // this->config->setValue("xEditorOffset", -1);
        // this->config->setValue("yEditorOffset", -1);
    }
}

void MainWindow::pushWarnings()
{
    if (this->lwiWarnings.length() > 0)
    {
        for (auto it = this->lwiWarnings.begin(); it != this->lwiWarnings.end(); ++it)
        {
            ui->mainListWidget->addItem(*it);
        }
        this->lwiWarnings.clear();
    }
}

std::map<QString, QString>* MainWindow::lineInfo(const QString line, QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine)
{
    std::map<QString, QString>* info = new std::map<QString, QString>();
    lineInfo(info, line, actualJump, actualJumpLine, isPrevCallPrevLine);
    return info;
}

void MainWindow::lineInfo(std::map<QString, QString>* info, const QString originalLine, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine)
{
    (*info)["file"] = ""; // same as info->at("file")
    (*info)["row"] = "";
    (*info)["line"] = originalLine;
    (*info)["column"] = "";
    (*info)["language"] = ""; // only if language can be detected from this line
    (*info)["good"] = "false";
    (*info)["lower"] = "false";
    (*info)["master"] = "false";
    (*info)["color"] = "Default";
    QString line = originalLine;

    QString fileToken;
    QString paramAToken;
    QString paramBToken;
    QString endParamsToken;
    int fileTokenI = -1;
    int fileI = -1;
    int paramATokenI = -1;
    int paramAI = -1;
    int paramBTokenI = -1;
    int paramBI = -1;
    int endParamsTokenI = -1;
    QRegExp nonDigitRE("\\D");
    QRegExp nOrZRE("\\d*"); // a digit (\d), zero or more times (*)
    QRegExp numOrMoreRE("\\d+"); // a digit (\d), 1 or more times (+)
    if (this->m_VerboseParsing)
    {
        qInfo().noquote() << "`" + originalLine + "`:";
    }
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++)
    {
        if ((((*itList)[TOKEN_FILE]).length() == 0) || line.contains((*itList)[TOKEN_FILE]))
        {
            fileToken = (*itList)[TOKEN_FILE];
            if (this->m_VerboseParsing)
            {
                if (fileToken.length() > 0)
                    qInfo().noquote() << "  looking for fileToken '" + fileToken + "'";
            }
            paramAToken = (*itList)[TOKEN_PARAM_A];
            paramBToken = (*itList)[TOKEN_PARAM_B]; // coordinate delimiter (blank if no column)
            endParamsToken = (*itList)[TOKEN_END_PARAMS]; // what is after last coord ("\n" if line ends)
            if (fileToken.length() != 0)
                fileTokenI = line.indexOf(fileToken);
            else
                fileTokenI = 0; // if file path at begining of line
            if (fileTokenI > -1)
            {
                if (this->m_VerboseParsing)
                {
                    qInfo().noquote() << "  has '" + fileToken + "' @ " << fileTokenI
                                      << ">= START";
                }

                if (paramAToken.length() > 0)
                {
                    paramATokenI = line.indexOf(
                                       paramAToken, fileTokenI + fileToken.length()
                                   );
                    if (paramATokenI >=0)
                    {
                        if (!line.mid(paramATokenI+paramAToken.length(), 1).contains(numOrMoreRE))
                        {
                            // Don't allow the opener if the next character is
                            // not a digit.
                            paramATokenI = -1;
                        }
                    }
                }
                else if (endParamsToken.length() > 0)
                {
                    paramATokenI = line.indexOf(endParamsToken);
                    if (paramATokenI < 0)
                    {
                        paramATokenI = line.length();
                    }
                }
                else
                {
                    paramATokenI = line.length();
                    // paramAToken = "<forced token=\"" + paramAToken.replace("\"", "\\\"") + "\">";
                }
                if (paramATokenI > -1)
                {
                    if (this->m_VerboseParsing)
                    {
                        qInfo().noquote() << "    has pre-ParamA '" + paramAToken + "' @"
                                          << paramATokenI << " (after " << fileToken.length() << "-long file token at "
                                          << fileTokenI
                                          << " ending at " << (fileTokenI + fileToken.length())
                                          << ")"; // such as ', line '
                    }
                    paramAI = paramATokenI + paramAToken.length();
                    if (paramBToken.length() > 0)
                    {
                        // There should be no B if there is no A, so failing
                        // in that case is OK.
                        paramBTokenI = line.indexOf(paramBToken, paramAI);
                    }
                    else
                    {
                        paramBTokenI = paramAI;
                        // paramBToken = "<forced token=\"" + paramBToken.replace("\"", "\\\"") + "\">";
                    }
                    if (paramBTokenI > -1)
                    {
                        if (this->m_VerboseParsing)
                        {
                            qInfo().noquote() << "      has pre-ParamB token '" + paramBToken + "' @"
                                              << paramBTokenI << " at or after ParamA token ending at"
                                              << (paramATokenI + paramAToken.length());
                        }
                        // if (paramBToken != (*itList)[TOKEN_PARAM_B])
                        //    paramBToken = ""; // since may be used to locate next value
                        if (paramBToken.length() > 0)
                            paramBI = paramBTokenI + paramBToken.length();
                        else
                            paramBI = paramBTokenI;
                        if (endParamsToken.length() == 0)
                        {
                            endParamsTokenI = paramBI;
                            if (this->m_VerboseParsing)
                            {
                                qInfo().noquote() << "  using paramBI for endParamsTokenI: " << paramBI;
                            }
                        }
                        else if (endParamsToken != "\n")
                        {
                            endParamsTokenI = line.indexOf(endParamsToken, paramBI);
                        }
                        else
                        {
                            endParamsTokenI = line.length();
                            // endParamsToken = "<forced token=\"" + endParamsToken.replace("\"", "\\\"").replace("\n", "\\n") + "\">";
                        }
                        if (endParamsTokenI > -1)
                        {
                            if (paramBToken.length() == 0)
                            {
                                paramBTokenI = endParamsTokenI; // so paramAI can be calculated correctly if ends at endParamsTokenI
                                paramBI = endParamsTokenI;
                            }
                            if ((*itList)[PARSE_COLLECT] == COLLECT_REUSE)
                                (*info)["master"] = "true";
                            if ((*itList)[PARSE_STACK] == STACK_LOWER)
                                (*info)["lower"] = "true";
                            if (this->m_VerboseParsing)
                            {
                                qInfo().noquote() << "        has post-params '" + endParamsToken.replace("\n", "\\n") + "' ending@"
                                                  << endParamsTokenI << ">=" << (paramBTokenI + paramBToken.length()) << "="
                                                  << paramBTokenI << "+" << paramBToken.length() << "in '" + line + "'";
                            }
                            if (endParamsToken != (*itList)[TOKEN_END_PARAMS])
                            {
                                // since could be used for more stuff after 2 params in future versions,
                                // length should be 0 if not found but forced:
                                endParamsToken = "";
                            }
                            fileI = fileTokenI + fileToken.length();
                            break;
                        }
                        else if (this->m_VerboseParsing)
                        {
                            qInfo().noquote() << "        no post-params '" + endParamsToken + "' >="
                                              << (paramBTokenI + paramBToken.length()) << "in '" + line + "'";
                        }
                    }
                    else if (this->m_VerboseParsing)
                    {
                        qInfo().noquote() << "      no pre-ParamB '" + paramBToken + "' >="
                                          << (paramATokenI + paramAToken.length()) << "in '" + line + "'";
                    }
                }
                else if (this->m_VerboseParsing)
                {
                    qInfo().noquote() << "    no pre-paramA '" + paramAToken + "' >="
                                      << (fileTokenI + fileToken.length());
                }
            }
            else if (this->m_VerboseParsing)
            {
                qInfo().noquote() << "  no pre-File '" + fileToken + "' >= START";
            }
        }
    }

    // qInfo().noquote() << "fileTokenI: " << fileTokenI;
    // qInfo().noquote() << "paramATokenI: " << paramATokenI;
    // qInfo().noquote() << "paramBTokenI: " << paramBTokenI;
    // qInfo().noquote() << "endParamsTokenI: " << endParamsTokenI;
    // qInfo().noquote() << "fileToken: " << fileToken;
    // qInfo().noquote() << "paramAToken: " << paramAToken;
    // qInfo().noquote() << "paramBToken: " << paramBToken;
    // qInfo().noquote() << "endParamsToken: " << endParamsToken;
    if (fileI >= 0 && (paramATokenI > fileI || endParamsToken > fileI))
    {
        // Even if closer is not present,
        // endParamsTokenI is set to length() IF applicable to this enclosure

        QString filePath;
        if (paramATokenI > fileI)
            filePath = line.mid(fileI, paramATokenI - fileI);
        else
            filePath = line.mid(fileI, endParamsTokenI - fileI);

        filePath = filePath.trimmed();
        if (filePath.length() >= 2)
        {
            if ((filePath.startsWith('"') && filePath.endsWith('"')) || (filePath.startsWith('\'') && filePath.endsWith('\'')))
            {
                filePath = filePath.mid(1, filePath.length() - 2);
            }
        }
        qDebug() << "[outputinspector][debug] file path before unmangling: \"" << filePath << "\"";
        filePath = unmangledPath(filePath);
        (*info)["file"] = filePath;
        (*info)["row"] = line.mid(paramAI, paramBTokenI - paramAI);
        if (paramBToken.length() > 0)
            (*info)["column"] = line.mid(paramBI, endParamsTokenI - paramBI);
        else
            (*info)["column"] = "";
        if (this->m_VerboseParsing) qInfo().noquote() << "        file '" + line.mid(fileI, paramATokenI - fileI) + "'";
        // if (this->m_VerboseParsing) qInfo().noquote() << "        row '" + line.mid(paramAI, paramBTokenI - paramAI) + "'";
        if (this->m_VerboseParsing) qInfo().noquote() << "        row '" + (*info)["row"] + "'";
        if (this->m_VerboseParsing) qInfo().noquote() << "          length " << paramBTokenI << "-" << paramAI;
        //if (this->m_VerboseParsing) qInfo().noquote() << "        col '" + line.mid(paramBI, endParamsTokenI - paramBI) + "'";
        if (this->m_VerboseParsing) qInfo().noquote() << "        col '" + (*info)["column"] + "'";
        if (this->m_VerboseParsing) qInfo().noquote() << "          length " << endParamsTokenI << "-" << paramBI;

        if (filePath.endsWith(".py", Qt::CaseSensitive))
            (*info)["language"] = "python";
        else if (filePath.endsWith(".pyw", Qt::CaseSensitive))
            (*info)["language"] = "python";
        else if (filePath.endsWith(".cpp", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (filePath.endsWith(".h", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (filePath.endsWith(".hpp", Qt::CaseSensitive))
            (*info)["language"] = "c++";
        else if (filePath.endsWith(".c", Qt::CaseSensitive))
            (*info)["language"] = "c";
        else if (filePath.endsWith(".js", Qt::CaseSensitive))
            (*info)["language"] = "js";
        else if (filePath.endsWith(".java", Qt::CaseSensitive))
            (*info)["language"] = "java";
        else if (filePath.endsWith(".bat", Qt::CaseSensitive))
            (*info)["language"] = "bat";
        else if (filePath.endsWith(".sh", Qt::CaseSensitive))
            (*info)["language"] = "sh";
        else if (filePath.endsWith(".command", Qt::CaseSensitive))
            (*info)["language"] = "sh";
        else if (filePath.endsWith(".php", Qt::CaseSensitive))
            (*info)["language"] = "php";
        qDebug().noquote() << "  detected file: '" + filePath + "'";
        (*info)["good"] = "true";
        // qInfo() << "[outputinspector] found a good line with the following filename: " << filePath;
    }
    else
    {
        (*info)["good"] = "false";
        // qInfo() << "[outputinspector] found a bad line: " << originalLine;
    }
    if ((*info)["good"] == "true")
    {
        if (actualJump.length() > 0 && (*info)["master"] == "false")
        {
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

QString MainWindow::absPathOrSame(QString filePath)
{
    QString absFilePath;
    QString sCwd = QDir::currentPath(); // current() returns a QDir object
    // Don't use QDir::separator(), since this only is detectable on *nix & bsd-like OS:
    QDir cwDir = QDir(sCwd);
    QString setuptoolsTryPkgPath = QDir::cleanPath(sCwd + QDir::separator() + cwDir.dirName());
    if (this->m_Verbose)
        qInfo().noquote() << "setuptoolsTryPkgPath:" << setuptoolsTryPkgPath;
    absFilePath = filePath.startsWith("/", Qt::CaseInsensitive) ? filePath : (sCwd + "/" + filePath);
    if (!QFile(absFilePath).exists() && QDir(setuptoolsTryPkgPath).exists())
        absFilePath = QDir::cleanPath(setuptoolsTryPkgPath + QDir::separator() + filePath);
    return absFilePath;
}

void MainWindow::on_mainListWidget_itemDoubleClicked(QListWidgetItem* item)
{
    QString line = item->text();
    QString actualJump = item->data(ROLE_COLLECTED_FILE).toString(); // item->toolTip();
    QString actualJumpLine = item->data(ROLE_COLLECTED_LINE).toString(); // item->toolTip();
    if (actualJumpLine.length() > 0)
        line = actualJumpLine;
    bool ok = false;
    QString filePath = (item->data(ROLE_COLLECTED_FILE)).toString();
    QString absFilePath = filePath;
    QString errorMsg;
    if (filePath.length() > 0)
    {
        if (this->m_Verbose)
        {
            qInfo().noquote() << "clicked_file: '" + filePath + "'";
            qInfo().noquote() << "tooltip: '" + item->toolTip() + "'";
        }
        absFilePath = absPathOrSame(filePath);
        QString citedRowS = (item->data(ROLE_ROW)).toString();
        QString citedColS = (item->data(ROLE_COL)).toString();
        if (this->m_Verbose)
        {
            qInfo().noquote() << "citedRowS: '" + citedRowS + "'";
            qInfo().noquote() << "citedColS: '" + citedColS + "'";
        }
        int citedRow = citedRowS.toInt(&ok, 10);
        int citedCol = citedColS.toInt(&ok, 10);
        int xEditorOffset = this->settings->getInt("xEditorOffset");
        int yEditorOffset = this->settings->getInt("yEditorOffset");
        // region only for Kate <= 2
        citedRow += yEditorOffset;
        citedRowS.setNum(citedRow, 10);
        citedCol += xEditorOffset;
        citedColS.setNum(citedCol, 10);
        // endregion only for Kate <= 2
        if (this->m_CompensateForKateTabDifferences)
        {
            QFile qfileSource(absFilePath);
            QString line;
            int readCitedI = 0; /**< This is the current line number while the
                                     loop reads the entire cited file. */
            if (qfileSource.open(QFile::ReadOnly))   //| QFile::Translate
            {
                QTextStream qtextNow(&qfileSource);
                while (!qtextNow.atEnd())
                {
                    line = qtextNow.readLine(0); //does trim off newline characters
                    if (readCitedI == ((citedRow - yEditorOffset) - 1))
                    {
                        int tabCount = 0;
                        // TODO: Use regex for finding the tab.
                        for (int tryTabI = 0; tryTabI < line.length(); tryTabI++)
                        {
                            if (line.mid(tryTabI, 1) == "\t")
                                tabCount++;
                            else
                                break;
                        }
                        QString tabDebugMsg;
                        if (tabCount > 0)
                        {
                            tabDebugMsg.setNum(tabCount, 10);
                            tabDebugMsg = "tabs:" + tabDebugMsg;
                            // if subtracted 1 for kate 2, the 1st character after a line with 1 tab is currently citedCol==6,  otherwise it is 7
                            // if subtracted 1 for kate 2, the 2nd character after a line with 1 tab is currently citedCol==7,  otherwise it is 8
                            // if subtracted 1 for kate 2, the 1st character after a line with 2tabs is currently citedCol==12, otherwise it is 13
                            // if subtracted 1 for kate 2, the 2nd character after a line with 2tabs is currently citedCol==13, otherwise it is 14
                            if (this->m_KateMajorVer < 3)
                                citedCol -= xEditorOffset;
                            tabDebugMsg += "; citedColS-old:" + citedColS;
                            citedCol -= tabCount * (this->settings->getInt("CompilerTabWidth") - 1);
                            //citedCol+=xEditorOffset;
                            citedColS.setNum(citedCol, 10);
                            tabDebugMsg += "; citedCol-abs:" + citedColS;
                            // if above worked, then citedCol is now an absolute character (counting tabs as 1 character)
                            // if subtracted 1 for kate 2, the 1st character after a line with 1 tab has now become citedCol==1,  otherwise it is 2 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 2nd character after a line with 1 tab has now become citedCol==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 1st character after a line with 2tabs has now become citedCol==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            // if subtracted 1 for kate 2, the 2nd character after a line with 2tabs has now become citedCol==3,  otherwise it is 4 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            if (this->m_KateMajorVer < 3)
                            {
                                // Kate 2.5.9 reads a 'c' argument value of 0 as the beginning of the line and 1 as the first character after the leading tabs
                                if (citedCol < tabCount)
                                    citedCol = 0;
                                else
                                {
                                    // citedCol currently starts at 1 at the beginning of the line
                                    citedCol -= (tabCount);
                                    citedColS.setNum(citedCol, 10);
                                    tabDebugMsg += "; citedCol-StartAt1-rel-to-nontab:" + citedColS;
                                    // citedCol now starts at 1 starting from the first text after tabs
                                    int regeneratedCol = 1;
                                    tabDebugMsg += "; skips:";
                                    // This approximates how Kate 2 traverses tabs (the 'c' argument actually can't reach certain positions directly after the tabs):
                                    if (tabCount > 2)
                                        citedCol += tabCount - 2;
                                    for (int tryTabI = 1; tryTabI < citedCol; tryTabI++)
                                    {
                                        if (tryTabI <= (tabCount - 1) * this->settings->getInt("Kate2TabWidth") + 1)
                                        {
                                            if (tryTabI != 1 && (tryTabI - 1) % this->settings->getInt("Kate2TabWidth") == 0)
                                            {
                                                regeneratedCol++; // only add if it is 4,7,10,etc where addend is this->config->getInt("Kate2TabWidth") (1+this->config->getInt("Kate2TabWidth")*position)
                                                tabDebugMsg += "-";
                                            }
                                        }
                                        else
                                        {
                                            regeneratedCol++;
                                        }
                                    }
                                    citedCol = regeneratedCol; // +( (tabCount>3&&tabCount<6) ? tabCount : 0 );
                                    // end accounting for kate gibberish column translation
                                }
                            }
                            // else kate 3+, which handles tabs as absolute positions
                            citedColS.setNum(citedCol, 10);
                            tabDebugMsg += "; citedColS-new:" + citedColS;
                            if (this->m_EnableTabDebugMsg)
                                QMessageBox::information(this, "Output Inspector - Debug tab compensation", tabDebugMsg);
                        } // end if tabCount>0
                        break;
                    } // if correct line found
                    readCitedI++;
                } // while not at end of source file
                qfileSource.close();
            } // end if can open source file
            else
            {
                errorMsg = "Specified file '" + filePath + "' does not exist or is not accessible (if path looks right, try running from the location where it exists instead of '" + QDir::currentPath() + "')";
            }
        } // end if this->m_CompensateForKateTabDifferences
        // QString sArgs="-u "+absFilePath+" -l "+citedRowS+" -c "+citedColS;
        // QProcess qprocNow(this->config->getString("editor")+sArgs);
        // qprocNow
        if (QFile(absFilePath).exists())
        {
            QString commandMsg = this->settings->getString("editor");
            QStringList qslistArgs;
            // NOTE: -u is not needed at least as of kate 16.12.3 which does not create additional
            // instances of kate
            // qslistArgs.append("-u");
            // commandMsg+=" -u";
            // qslistArgs.append("\""+absFilePath+"\"");
            qslistArgs.append(absFilePath);
            commandMsg += " " + absFilePath;
            qslistArgs.append("--line"); // split into separate arg, otherwise geany complains that
            // it doesn't understand the arg "--line 1"
            qslistArgs.append(citedRowS);
            commandMsg += " --line " + citedRowS;
            // qslistArgs.append(citedRowS);
            qslistArgs.append("--column"); // NOTE: -c is column in kate, but alternate config dir
            // in geany, so use --column
            qslistArgs.append(citedColS); // NOTE: -c is column in kate, but alternate config dir
            // in geany, so use --column
            commandMsg += " --column " + citedColS;
            // qslistArgs.append(citedColS);
            // qWarning().noquote() << "qslistArgs: " << qslistArgs;
            QProcess::startDetached( this->settings->getString("editor"), qslistArgs);
            if (!QFile::exists(this->settings->getString("editor")))
            {
                // ok to run anyway for fault tolerance, since may be in system path
                QMessageBox::information(this, "Output Inspector - Configuration", this->settings->getString("editor") + " cannot be accessed.  Try setting the value after editor= in " + this->settings->fileName());
            }
            // if (this->m_Verbose)
            ui->statusBar->showMessage(commandMsg, 0);
            // system(sCmd);//stdlib
            // QMessageBox::information(this,"test",sCmd);
        }
        else
        {
            // errorMsg = "Specified file '" + absFilePath + "' does not exist (try a different line, or try running from the location where it exists instead of '" + QDir::currentPath() + "')";
            errorMsg = "[Output Inspector] No file exists here: '" + absFilePath + "'\n";
        }
    } // end if line is in proper format
    else
        errorMsg = "Could not detect error format\n";
    if (errorMsg.length() > 0)
    {
        if (filePath.length() > 0)
        {
            // qWarning().noquote() << errorMsg << " in '" + line + "':";
            std::map<QString, QString>* info = lineInfo(line, actualJump, actualJumpLine, false);
            QString infoS;
            for (auto it = info->begin(); it != info->end(); it++)
            {
                // qWarning().noquote() << "    " << it->first // key
                //         + ": '" + it->second + "'"; //value
                infoS += "; " + it->first + ": '" + it->second + "'";
            }
            qWarning().noquote() << "[outputinspector][error] " << errorMsg << " in the line:"
                                 << "; actualJump: " + item->data(this->ROLE_COLLECTED_FILE).toString()
                                 << "  actualJumpLine: " + item->data(this->ROLE_COLLECTED_LINE).toString()
                                 << infoS;
            //                      << "  info:";

            // for (auto const& it : (*info)) {  // >=C++11 only (use dot notation not -> below if using this)

            QMessageBox::information(this, "Output Inspector", errorMsg);
            // QMessageBox::information(this,"Output Inspector","'"+absFilePath+"' cannot be accessed (try a different line, or if this line's path looks right, try running from the location where it exists instead of '"+QDir::currentPath()+"')");
            // or pasting the entire line to 'Issues' link on web-based git repository
        }
        else
        {
            qInfo().noquote() << "[Output Inspector] No file was detected in line: '" + line + "'";
            qInfo().noquote() << "[Output Inspector] ERROR: '" + errorMsg + "'";
        }
    }
}

void MainWindow::readInput()
{
    int limit = 50;
    int count = 0;
    std::string line = " ";
    while (count < limit && !line.empty())
    {
        std::streamsize size = std::cin.rdbuf()->in_avail();
        if (size < 1)
        {
            // qInfo().noquote() << "OutputInspector: There is no input: got " << size;
            // Prevent waiting forever for a line.
            break;
        }
        std::getline(std::cin, line);
        if (!std::cin.eof())
        {
            // qInfo().noquote() << "OutputInspector: input is '" << line.c_str() << "'.";
            // this->addLine(QString("OutputInspector: input is: ") + QString::fromStdString(line), true);
            this->addLine(QString::fromStdString(line), true);
        }
        else
        {
            // qInfo().noquote() << "OutputInspector: input has ended.";
            // this->addLine("# OutputInspector: input has ended.", true);
            break;
        }
        count++;
    }

} // end MainWindow::on_mainListWidget_itemDoubleClicked
