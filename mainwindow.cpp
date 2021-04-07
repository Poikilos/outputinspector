#include "mainwindow.h"
// #include "ui_mainwindow.h"
#include "settings.h"

#include <iostream>//this is a trailing comment with no space before or after slashes (for clang-format test)
#include <vector>
#include <iosfwd>
// #include <cstddef>
#include <string>
#include <cassert>

using namespace std;


bool startswithCS(std::string haystack, std::string needle) {
    return haystack.rfind(needle, 0) == 0;
}

bool startswithCI(std::string haystack, std::string needle) {
    std::locale loc;
    return tolower(haystack, loc).rfind(tolower(needle, loc), 0) == 0;
}

bool endswithCS(std::string const &haystack, std::string const &needle) {
    if (haystack.length() >= needle.length()) {
        return (0 == haystack.compare(haystack.length()-needle.length(), needle.length(), needle));
    }
    else {
        return false;
    }
}

bool endswithCI(std::string const &haystack, std::string const &needle) {
    if (haystack.length() >= needle.length()) {
        std::locale loc;
        return (0 == tolower(haystack, loc).compare(haystack.length()-needle.length(), needle.length(), tolower(needle, loc)));
    }
    else {
        return false;
    }
}

int findCI(std::string haystack, std::string needle, int start) {
    std::locale loc;
    needle = needle;
    std::size_t found = tolower(haystack, loc).find(tolower(needle, loc), start);
    return (found != std::string::npos) ? found : -1;
}
int findCS(std::string haystack, std::string needle, int start) {
    std::locale loc;
    std::size_t found = haystack.find(needle, start);
    return (found != std::string::npos) ? found : -1;
}
bool inList(std::vector<std::string> haystack, string needle) {
    return std::find(haystack.begin(), haystack.end(), needle) != haystack.end();
}


string os_path_join(string p1, string p2) {
    return p1 + "/" + p2;
}

/*
std::string mid(std::string s, int start, int length) {
    return s.substr(start, length);
}
*/

string OIWidget::text() {
    return _text;
}

bool OITextStream::atEnd() {
    return true;
}

void OIListWidgetItem::setData(int role, string value) {
    this->_data[role] = value;
}


void OIListWidgetItem::setForeground(OIBrush brush) {
    this->_foreground = brush;
}

void OIListWidgetItem::setBackground(OIBrush brush) {
    this->_background = brush;
}

OIBrush::OIBrush(OIColor color)
{
    this->_color = color;
}

OIColor OIColor::fromRgb(int r, int g, int b)
{
    OIColor color;
    color.r = r;
    color.g = g;
    color.b = b;
    return color;
}

// endregion scripting

/**
 * This is a Functor for "Contains," such as for multi-needle searches
 * (see [single variable] initializer list in constructor for how haystack is
 * obtained)
 */
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
        return inList(haystack, needle);
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


template <class T>
bool contains_any(T haystack, std::list<T>& needles)
{
    // TODO: use unused parameter cs.
    return count_if(needles.begin(), needles.end(), bind(contains, haystack, std::placeholders::_1)) > 0;
}

string MainWindow::unmangledPath(string path)
{
    QRegularExpression literalDotsRE("\\.\\.\\.+"); /**< Match 2 dots followed by more. */
    QRegularExpressionMatch match = literalDotsRE.match(path);
    if (match.hasMatch()) {
        // int start = match.capturedStart(0);
        int end = match.capturedEnd(0);
        std::list<int> tryOffsets = std::list<int>();
        tryOffsets.push_back(-2);
        tryOffsets.push_back(1);
        tryOffsets.push_back(0);
        for (auto tryOffset : tryOffsets) {
            string tryPath = path.substr(end+tryOffset);
            if (QFile(tryPath).exists()) {
                if (this->_verbosity > 1) {
                    MainWindow::info(
                        "[outputinspector] transformed *.../dir into ../dir: \""
                        + tryPath + "\""
                    );
                }
                return tryPath;
            }
            else {
                if (this->_verbosity > 1) {
                    MainWindow::info(
                        string("[outputinspector] There is no \"")
                        + tryPath
                        + "\" in the current directory"
                        // + std::filesystem::current_path();
                    );
                }
            }
        }
    }
    else {
        if (this->_verbosity > 1) MainWindow::info("[outputinspector] There is no \"...\" in the cited path: \"" + path + "\"");
    }
    return path;
}

MainWindow::MainWindow()
{
    ui->setupUi(this);
    // QDir configDir = QStandardPaths::StandardLocation(QStandardPaths::ConfigLocation());
    // QStandardPaths::StandardLocation(QStandardPaths::HomeLocation)
    // ^ same as QDir::homePath()
    // See <https://stackoverflow.com/questions/32525196/
    //   how-to-get-a-settings-storage-path-in-a-cross-platform-way-in-qt>
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) cerr << "FATAL: Cannot determine settings storage location" << endl;
    QDir d{path};
    if (!d.exists()) {
        d.mkpath(d.absolutePath());
    }
    string filePath = QDir::cleanPath(d.absolutePath() + QDir::separator() + "settings.txt");
    QFile f{filePath};
    /// TODO: fill this in or remove it (and the comments).
    if (d.exists()) {
        if (!f.exists()) {
            // if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            //     f.write("Hello, World");
        }
        else {

        }
    }
    // MainWindow::info("Creating settings: " + this->filePath());
    this->settings = new Settings(filePath);
    MainWindow::info(
        "[outputinspector] used the settings file \"" + this->settings->fileName() + "\""
    );
    // if (QFile("/etc/outputinspector.conf").exists()) {
    //     this->config = new Settings("/etc/outputinspector.conf");
    // }
    // else if (QFile("/etc/outputinspector.conf").exists()) {
    //     this->config = new Settings("/etc/outputinspector.conf");
    // }
    this->settings->setIfMissing("Kate2TabWidth", "8");
    this->settings->setIfMissing("CompilerTabWidth", "6");
    this->settings->setIfMissing("ShowWarningsLast", "false");
    /// TODO: Implement ShowWarningsLast (but ignore it and behave as if it were
    /// false if there is anything in stdin).
    this->settings->setIfMissing("FindTODOs", "true");
    if (this->settings->contains("kate")) {
        bool changed = this->settings->setIfMissing("editor", this->settings->getString("kate"));
        this->settings->remove("kate");
        this->settings->sync();
        if (changed)
            MainWindow::info(
                "[outputinspector] transferred the old setting"
                + string(" 'kate=") + this->settings->getString("kate")
                + "' to 'editor=" + this->settings->getString("editor")
                + "'."
            );
        else
            MainWindow::info(
                "[outputinspector] ignored the deprecated setting"
                + string(" 'kate=") + this->settings->getString("kate")
                + "' in favor of 'editor="
                + this->settings->getString("editor")
                + "'."
            );
    }
    this->settings->setIfMissing("editor", "/usr/bin/geany");


    // init(errorsListFileName);
    // formats with "\n" at end must be AFTER other single-param formats that have
    // same TOKEN_FILE and PARSE_PARAM_A, because "\n" is forced
    // (which would leave extra stuff at the end if there are more tokenings)
    {
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def; /**< This is a fallback definition that applies
                              to various parsers.
                              Simpler definitions must be attempted in
                              order from most to least complex to avoid
                              false positives (Now there are even
                              simpler ones after this one). */

        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        // -n option for grep shows line # like:
        // <filename>:<number>:
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
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
        std::vector<std::string> def;
        for (int i = 0; i < PARSE_PARTS_COUNT; i++)
            def.push_back("");
        def[TOKEN_FILE] = "";
        def[TOKEN_PARAM_A] = "";
        def[TOKEN_PARAM_B] = "";
        def[TOKEN_END_PARAMS] = ":";
        def[PARSE_COLLECT] = "";
        def[PARSE_STACK] = "";
        def[PARSE_DESCRIPTION] = "grep-like result (path then colon)";
        enclosures.push_back(def);
    }
    brushes["TracebackNotTop"] = OIBrush(OIColor::fromRgb(128, 60, 0));
    brushes["Unusable"] = OIBrush(OI_lightGray);
    brushes["Internal"] = OIBrush(OIColor::fromRgb(192, 192, 100));
    brushes["Warning"] = OIBrush(OIColor::fromRgb(192, 120, 80));
    brushes["WarningDetails"] = OIBrush(OIColor::fromRgb(255, 180, 120));
    brushes["Error"] = OIBrush(OIColor::fromRgb(80, 0, 0));
    brushes["ErrorDetails"] = OIBrush(OIColor::fromRgb(160, 80, 80));
    brushes["ToDo"] = OIBrush(OI_darkGreen);
    brushes["Default"] = OIBrush(OI_black);

    sInternalFlags.push_back("/site-packages/");
    assert(contains_any<string>("/usr/lib/python2.7/site-packages/nose/importer.py", sInternalFlags));

    sSectionBreakFlags.push_back("--------");
    assert(contains_any<string>("---------------------", sSectionBreakFlags));

    MainWindow::debug("* The debug stream is active.");
    // qInfo().noquote() << "qInfo() stream is active.";
    // qWarning().noquote() << "qWarning() stream is active.";
    // qCritical().noquote() << "qCritical() stream is active.";
    // qFatal().noquote() << "qFatal() stream is active.";
    if (this->_verbosity)
        MainWindow::info("lists:");
    // this for loop has the brace on the next line (for clang-format test):
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++) {
        // qInfo().noquote() << "  -";
        // for (int i=0; i<(*itList).length(); i++) {
        //     qInfo().noquote() << "    - " << (*itList)[i];
        // }
        assert(itList->size() >= PARSE_PARTS_COUNT); // qInfo().noquote() << "  items->size(): " << itList->size();
        /*
        if (this->_verbosity) {
            MainWindow::info(string("  items: ['") + (*itList).join("', '") + string("']"));
        }
        */
    }
}

/**
 * @brief Check whether your parser reports that your code has a fatal error.
 * @param line stderr output from your parser
 * @return
 */
bool MainWindow::isFatalSourceError(string line)
{
    return (
        findCI(line, "Can't open", 0) > -1 //jshint could not find a source file
        || findCI(line, "Too many errors", 0) > -1 //jshint already showed the error for this line, but can't display more errors
        || findCI(line, "could not be found", 0) > -1 //mcs could not find a source file
        || findCI(line, "compilation failed", 0) > -1 //mcs could not compile the sources
    );
}

MainWindow::~MainWindow()
{
    // delete this->inTimer;
    delete this->settings;
    delete ui;
}
void MainWindow::init(string errorsListFileName)
{
    if (!(this->settings->contains("xEditorOffset") || this->settings->contains("yEditorOffset")))
        CompensateForEditorVersion();
    if (errorsListFileName.length() == 0) {
        string tryPath = "debug.txt";
        if (QFile(tryPath).exists()) {
            errorsListFileName = tryPath;
            MainWindow::info("[outputinspector] detected \"" + tryPath + "\"...examining...");
        }
        else
            errorsListFileName = "err.txt";
    }
    // QTextStream err(stderr);  // avoid quotes and escapes caused by qWarning().noquote() being derived from qDebug()--alternative to qWarning().noquote().noquote()<<

    QFile qfileTest(errorsListFileName);
    // this->m_ToDoFlags.push_back("TODO");
    // this->m_ToDoFlags.push_back("FIXME");
    // int cutToDoCount = 2;
    // ui->mainListWidget is a std::listWidget
    // setCentralWidget(ui->mainListWidget);
    // ui->mainListWidget->setSizePolicy(QSizePolicy::)
    // OutputInspectorSleepThread::msleep(150); // wait for stdin (doesn't work)
    if (std::cin.rdbuf()->in_avail() > 1) {
        // TODO: fix this--this never happens (Issue #16)
        MainWindow::info(string("[outputinspector]")
                   + string(" detected standard input (such as from a console pipe)")
                   + "...skipping \"" + errorsListFileName + "\"...");
    }
    else if (qfileTest.exists()) {
        if (qfileTest.open(QFile::ReadOnly)) { //| QFile::Translate
            QTextStream qtextNow(&qfileTest);
            while (!qtextNow.atEnd()) {
                // NOTE: Readline trims off newline characters.
                this->addLine(qtextNow.readLine(0), false);
            } // end while not at end of file named errorsListFileName
            qfileTest.close();
            string sNumErrors;
            sNumErrors.setNum(iErrors, 10);
            string sNumWarnings;
            sNumWarnings.setNum(iWarnings, 10);
            string sNumTODOs;
            sNumTODOs.setNum(iTODOs, 10);
            pushWarnings();
            if (this->settings->getBool("FindTODOs")) {
                for (auto it = this->lwiToDos.begin(); it != this->lwiToDos.end(); ++it) {
                    ui->mainListWidget->addItem(*it);
                }
            }
            if (!this->settings->getBool("ExitOnNoErrors")) {
                if (this->m_LineCount == 0) {
                    OIListWidgetItem* lwiNew = new OIListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                } else if (this->m_NonBlankLineCount == 0) {
                    OIListWidgetItem* lwiNew = new OIListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 non-blank lines in file");
                    lwiNew->setForeground(brushes["Default"]);
                    ui->mainListWidget->addItem(lwiNew);
                }
            }
            // else hide errors since will exit anyway if no errors
            string sMsg = "Errors: " + sNumErrors + "; Warnings:" + sNumWarnings;
            if (this->settings->getBool("FindTODOs"))
                sMsg += "; TODOs:" + sNumTODOs;
            ui->statusBar->showMessage(sMsg, 0);
            if (this->settings->getBool("ExitOnNoErrors")) {
                if (iErrors<1) {
                    MainWindow::info("Closing since no errors...");
                    // QCoreApplication::quit(); // doesn't work
                    // aptr->exit(); // doesn't work (QApplication*)
                    // aptr->quit(); // doesn't work
                    // aptr->closeAllWindows(); // doesn't work
                    // if the event loop is not running, this function (QCoreApplication::exit()) does nothing
                    exit(EXIT_FAILURE);
                }
            }
        } else {
            if (std::cin.rdbuf()->in_avail() < 1) {
                string my_path = QCoreApplication::applicationFilePath();
                string title = "Output Inspector - Help";
                string msg = my_path + ": Output Inspector cannot read the output file due to permissions or other read error (tried \"./" + errorsListFileName + "\").";
                QMessageBox::information(this, title, msg);
                // this->addLine(title + ":" + msg, true);
            }
        }
    } // end if could open file named errorsListFileName
    else {
        if (std::cin.rdbuf()->in_avail() < 1) {
            string my_path = QCoreApplication::applicationFilePath();
            string title = "Output Inspector - Help";
            string msg = my_path + ": Output Inspector cannot find the output file to process (tried \"./" + errorsListFileName + "\").";
            QMessageBox::information(this, title, msg);
            // this->addLine(title + ":" + msg, true);
        }
    }
    // this->inTimer = new QTimer(this);
    // this->inTimer->setInterval(500);  // milliseconds
    // TODO: read standard input (in case on right of pipe): connect(this->inTimer, SIGNAL(timeout()), this, SLOT(readInput()));
    // this->inTimer->start();
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
void MainWindow::addLine(string line, bool enablePush)
{
    this->m_LineCount++;
    string originalLine = line;
    this->m_MasterLine = line;
    // TODO: debug performance of new and delete
    std::map<string, string>* info = new std::map<string, string>;
    if (line.length() > 0) {
        if (strip(line).length() > 0)
            this->m_NonBlankLineCount++;
        if (isFatalSourceError(line)) {
            ui->mainListWidget->addItem(new OIListWidgetItem(line + " <your compiler (or other tool) recorded this fatal or summary error before outputinspector ran>", ui->mainListWidget));
        } else if (contains_any<string>(line, sSectionBreakFlags)) {
            this->m_ActualJump = "";
            this->m_ActualJumpLine = "";
            this->m_ActualJumpRow = "";
            this->m_ActualJumpColumn = "";
            OIListWidgetItem* lwi = new OIListWidgetItem(line);
            lwi->setForeground(brushes["Regular"]);
            ui->mainListWidget->addItem(lwi);
        } else {
            // lineInfo does the actual parsing:
            lineInfoByRef(info, line, this->m_ActualJump, this->m_ActualJumpLine, true);

            if (info->at("master") == "true") {
                this->m_ActualJump = info->at("file");
                this->m_ActualJumpLine = line;
                this->m_ActualJumpRow = info->at("row");
                this->m_ActualJumpColumn = info->at("column");
                this->m_IsJumpLower = (info->at("lower") == "true");
                MainWindow::debug("(master) set actualJump to '" + this->m_ActualJump + "'");
            } else {
                MainWindow::debug("(not master) line: '" + line + "'");
            }
            bool isWarning = false;
            string sColorPrefix = "Error";
            if (this->m_ActualJump.length() > 0) {
                this->m_MasterLine = this->m_ActualJumpLine;
            }
            if (findCI(this->m_MasterLine, this->m_Warning, 0) > -1) {
                isWarning = true;
                sColorPrefix = "Warning";
            }
            // do not specify ui->mainListWidget on new, otherwise will be added automatically
            OIListWidgetItem* lwi = new OIListWidgetItem(line);
            if (this->m_ActualJumpRow.length() > 0) {
                lwi->setData(ROLE_ROW, this->m_ActualJumpRow);
                lwi->setData(ROLE_COL, this->m_ActualJumpColumn);
            } else {
                lwi->setData(ROLE_ROW, info->at("row"));
                lwi->setData(ROLE_COL, info->at("column"));
            }
            if (this->m_ActualJump.length() > 0) {
                lwi->setData(ROLE_COLLECTED_FILE, this->m_ActualJump);
                if (info->at("lower") == "true")
                    lwi->setForeground(brushes["TracebackNotTop"]);
                else if (info->at("good") == "true")
                    lwi->setForeground(brushes[sColorPrefix]);
                else
                    lwi->setForeground(brushes[sColorPrefix + "Details"]);
            } else {
                lwi->setData(ROLE_COLLECTED_FILE, info->at("file"));
                if (info->at("good") == "true")
                    lwi->setForeground(brushes[sColorPrefix]);
                else
                    lwi->setForeground(brushes["Unusable"]);
            }
            if (contains_any<string>(this->m_MasterLine, this->sInternalFlags)) {
                lwi->setForeground(brushes["Internal"]);
            }
            lwi->setData(ROLE_COLLECTED_LINE, this->m_MasterLine);
            lwi->setData(ROLE_DETAILS, (line != this->m_MasterLine)? "true" : "false");
            lwi->setData(ROLE_LOWER, info->at("lower"));
            if (info->at("good") == "true") {
                if (isWarning)
                    iWarnings++;
                else
                    iErrors++;
            }
            if (this->settings->getBool("ShowWarningsLast") && isWarning)
                this->lwiWarnings.push_back(lwi);
            else
                ui->mainListWidget->addItem(lwi);

            string sTargetLanguage = (*info)["language"];

            if (sTargetLanguage.length() > 0) {
                if (sTargetLanguage == "python" || sTargetLanguage == "sh") {
                    this->m_CommentToken = "#";
                } else if (sTargetLanguage == "c++" || sTargetLanguage == "c" || sTargetLanguage == "php"
                    || sTargetLanguage == "js" || sTargetLanguage == "java") {
                    this->m_CommentToken = "//";
                } else if (sTargetLanguage == "bat") {
                    this->m_CommentToken = "rem ";
                }
            }

            //if ((is_jshint && endswithCI((*info)["file"],".js")) || findCI(line, this->m_Error, 0) > -1) {
            //  // TODO?: if (is_jshint|| findCI(line, "previous error", 0)<0) iErrors++;
            //  // if (this->config->getBool("ShowWarningsLast")) this->m_Errors.push_back(line);
            //}

            if (this->settings->getBool("FindTODOs")) {
                if (info->at("good") == "true") {
                    string sFileX;// = unmangledPath(info->at("file"));
                    sFileX = absPathOrSame(sFileX); // =line.substr(0,findCI(line, "(", 0));
                    if (!inList(this->m_Files, sFileX)) {
                        this->m_Files.push_back(sFileX);
                        QFile qfileSource(sFileX);
                        if (this->_verbosity)
                            MainWindow::debug("outputinspector trying to open '" + sFileX + "'...");
                        // if (!qfileSource.open(QFile::ReadOnly)) {
                        // }
                        if (true) { // (qfileSource.open(QFile::ReadOnly)) {
                            // QTextStream qtextSource(&qfileSource);
                            OITextStream qtextSource;
                            int iSourceLineFindToDo = 0;
                            while (!qtextSource.atEnd()) {
                                string sSourceLine = qtextSource.readLine(0);
                                iSourceLineFindToDo++; // Increment this now since the compiler's line numbering starts with 1.
                                int iToDoFound = -1;
                                int iCommentFound = findCI(sSourceLine, this->m_CommentToken, 0);
                                if (iCommentFound > -1) {
                                    for (int i=0; i<this->m_ToDoFlags.size(); i++) {
                                        iToDoFound = findCI(sSourceLine, this->m_ToDoFlags[i], iCommentFound + 1);
                                        if (iToDoFound > -1)
                                            break;
                                    }
                                }
                                if (iToDoFound > -1) {
                                    string sNumLine;
                                    sNumLine.setNum(iSourceLineFindToDo, 10);
                                    string sNumPos;
                                    int processedCol = iToDoFound;
                                    for (int citedI = 0; citedI < sSourceLine.length(); citedI++) {
                                        if (sSourceLine.substr(citedI, 1) == "\t")
                                            processedCol += (this->settings->getInt("CompilerTabWidth") - 1);
                                        else
                                            break;
                                    }
                                    processedCol += 1; // start numbering at 1 to mimic compiler
                                    processedCol += 2; // +2 to start after slashes
                                    sNumPos.setNum(processedCol, 10);
                                    string sLineToDo = sFileX + "(" + sNumLine + "," + sNumPos + ") " + sSourceLine.substr(iToDoFound);
                                    OIListWidgetItem* lwi = new OIListWidgetItem(sLineToDo);
                                    lwi->setData(ROLE_ROW, sNumLine);
                                    lwi->setData(ROLE_COL, sNumPos);
                                    lwi->setData(ROLE_COLLECTED_FILE, sFileX);
                                    lwi->setData(ROLE_LOWER, "false");
                                    lwi->setData(ROLE_COLLECTED_LINE, sLineToDo);
                                    lwi->setData(ROLE_DETAILS, "false");
                                    if (contains_any<string>(this->m_MasterLine, this->sInternalFlags)) {
                                        lwi->setForeground(brushes["Internal"]);
                                    } else {
                                        lwi->setForeground(brushes["ToDo"]);
                                    }
                                    this->lwiToDos.push_back(lwi);
                                    iTODOs++;
                                }
                            } // end while not at end of source file
                            if (this->_verbosity)
                                MainWindow::debug("outputinspector finished reading sourcecode");
                            if (this->_verbosity)
                                MainWindow::debug("(processed " << iSourceLineFindToDo << " line(s))");
                            qfileSource.close();
                        } // end if could open sourcecode
                        else {
                            qWarning().noquote() << "[outputinspector] did not scan a file that is cited by the log but that is not present: '" + sFileX + "'";
                        }
                    } // end if list does not already contain this file
                } // end if found filename ender
                else if (this->_verbosity)
                    MainWindow::debug("[outputinspector] WARNING: filename ender in " + line);
            } // end if getIniBool("FindTODOs")
            else
                MainWindow::debug("[outputinspector] WARNING: getIniBool(\"FindTODOs\") off so skipped parsing " + line);
        } // end if a regular line (not fatal, not formatting)
    } // end if length>0 (after trim using 0 option for readLine)
    if (enablePush) {
        this->pushWarnings();
    }
    delete info;
}

void MainWindow::CompensateForEditorVersion()
{
    bool isFound = false;
    std::vector<std::string> sVersionArgs;
    string sFileTemp = "/tmp/outputinspector.using.kate.version.tmp";
    sVersionArgs.push_back("--version");
    sVersionArgs.push_back(" > " + sFileTemp);
    // QProcess::execute(IniString("editor"), sVersionArgs);
    system((char*)string(this->settings->getString("editor") + " --version > " + sFileTemp).toLocal8Bit().data());
    OutputInspectorSleepThread::msleep(125);

    QFile qfileTmp(sFileTemp);
    string line;
    if (qfileTmp.open(QFile::ReadOnly)) { // | QFile::Translate
        // detect Kate version using output of Kate command above
        QTextStream qtextNow(&qfileTmp);
        string sKateOpener = "Kate: ";
        while (!qtextNow.atEnd()) {
            line = qtextNow.readLine(0); // does trim off newline characters
            if (this->_verbosity)
                QMessageBox::information(this, "Output Inspector - Finding Kate version...", line);
            if (startswithCI(line, sKateOpener)) {
                int iDot = findCS(line, ".", 0);
                if (iDot > -1) {
                    bool ok;
                    isFound = true;
                    this->m_KateMajorVer = string(line.substr(6, iDot - 6)).toInt(&ok, 10);
                }
            }
        }
        qfileTmp.close();
    } // end if could open temp file
    string sRevisionMajor;
    sRevisionMajor.setNum(this->m_KateMajorVer, 10);
    if (this->_verbosity)
        QMessageBox::information(this, "Output Inspector - Kate Version", isFound ? ("Detected Kate " + sRevisionMajor) : "Could not detect Kate version.");
    if (this->m_KateMajorVer > 2) {
        this->settings->setValue("xEditorOffset", 0);
        this->settings->setValue("yEditorOffset", 0);
    } else {
        this->settings->setValue("xEditorOffset", 0);
        this->settings->setValue("yEditorOffset", 0);
        // NOTE: The values are no longer necessary.
        // this->config->setValue("xEditorOffset", -1);
        // this->config->setValue("yEditorOffset", -1);
    }
}

void MainWindow::pushWarnings()
{
    if (this->lwiWarnings.length() > 0) {
        for (auto it = this->lwiWarnings.begin(); it != this->lwiWarnings.end(); ++it) {
            ui->mainListWidget->addItem(*it);
        }
        this->lwiWarnings.clear();
    }
}

std::map<string, string>* MainWindow::lineInfo(const string line, string actualJump, const string actualJumpLine, bool isPrevCallPrevLine)
{
    std::map<string, string>* info = new std::map<string, string>();
    lineInfo(info, line, actualJump, actualJumpLine, isPrevCallPrevLine);
    return info;
}

void MainWindow::lineInfoByRef(std::map<string, string>* info, const string originalLine, const string actualJump, const string actualJumpLine, bool isPrevCallPrevLine)
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
    string line = originalLine;

    string fileToken;
    string paramAToken;
    string paramBToken;
    string endParamsToken;
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
    if (this->_verbosityParsing) {
        MainWindow::info("`" + originalLine + "`:");
    }
    for (auto itList = enclosures.begin(); itList != enclosures.end(); itList++) {
        if ((((*itList)[TOKEN_FILE]).length() == 0) || line.contains((*itList)[TOKEN_FILE])) {
            fileToken = (*itList)[TOKEN_FILE];
            if (this->_verbosityParsing) {
                if (fileToken.length() > 0)
                    MainWindow::info("  looking for fileToken '" + fileToken + "'");
                }
            paramAToken = (*itList)[TOKEN_PARAM_A];
            paramBToken = (*itList)[TOKEN_PARAM_B]; // coordinate delimiter (blank if no column)
            endParamsToken = (*itList)[TOKEN_END_PARAMS]; // what is after last coord ("\n" if line ends)
            if (fileToken.length() != 0)
                fileTokenI = findCS(line, fileToken);
            else
                fileTokenI = 0; // if file path at begining of line
            if (fileTokenI > -1) {
                if (this->_verbosityParsing) {
                    MainWindow::info("  has '" + fileToken + "' @ " + to_string(fileTokenI)
                               + ">= START");
                }

                if (paramAToken.length() > 0) {
                    paramATokenI = findCS(line
                        paramAToken, fileTokenI + fileToken.length()
                    );
                    if (paramATokenI >=0) {
                        if (!line.substr(paramATokenI+paramAToken.length(), 1).contains(numOrMoreRE)) {
                            // Don't allow the opener if the next character is
                            // not a digit.
                            paramATokenI = -1;
                        }
                    }
                } else if (endParamsToken.length() > 0) {
                    paramATokenI = findCS(line, endParamsToken);
                    if (paramATokenI < 0) {
                        paramATokenI = line.length();
                    }
                } else {
                    paramATokenI = line.length();
                    // paramAToken = "<forced token=\"" + paramAToken.replace("\"", "\\\"") + "\">";
                }
                if (paramATokenI > -1) {
                    if (this->_verbosityParsing) {
                        MainWindow::info("    has pre-ParamA '" + paramAToken + "' @"
                                   + to_string(paramATokenI) << " (after " << to_string(fileToken.length()) << "-long file token at "
                                   + to_string(fileTokenI)
                                   + " ending at " << to_string((fileTokenI + fileToken.length()))
                                   + ")"); // such as ', line '
                    }
                    paramAI = paramATokenI + paramAToken.length();
                    if (paramBToken.length() > 0) {
                        // There should be no B if there is no A, so failing
                        // in that case is OK.
                        paramBTokenI = findCS(line, paramBToken, paramAI);
                    }
                    else {
                        paramBTokenI = paramAI;
                        // paramBToken = "<forced token=\"" + paramBToken.replace("\"", "\\\"") + "\">";
                    }
                    if (paramBTokenI > -1) {
                        if (this->_verbosityParsing) {
                            MainWindow::info("      has pre-ParamB token '" + paramBToken + "' @"
                                       + paramBTokenI + " at or after ParamA token ending at"
                                       + to_string(paramATokenI + paramAToken.length()));
                        }
                        // if (paramBToken != (*itList)[TOKEN_PARAM_B])
                        //    paramBToken = ""; // since may be used to locate next value
                        if (paramBToken.length() > 0)
                            paramBI = paramBTokenI + paramBToken.length();
                        else
                            paramBI = paramBTokenI;
                        if (endParamsToken.length() == 0) {
                            endParamsTokenI = paramBI;
                            if (this->_verbosityParsing) {
                                MainWindow::info("  using paramBI for endParamsTokenI: " + to_string(paramBI));
                            }
                        } else if (endParamsToken != "\n") {
                            endParamsTokenI = findCS(line, endParamsToken, paramBI);
                        } else {
                            endParamsTokenI = line.length();
                            // endParamsToken = "<forced token=\"" + endParamsToken.replace("\"", "\\\"").replace("\n", "\\n") + "\">";
                        }
                        if (endParamsTokenI > -1) {
                            if (paramBToken.length() == 0) {
                                paramBTokenI = endParamsTokenI; // so paramAI can be calculated correctly if ends at endParamsTokenI
                                paramBI = endParamsTokenI;
                            }
                            if ((*itList)[PARSE_COLLECT] == COLLECT_REUSE)
                                (*info)["master"] = "true";
                            if ((*itList)[PARSE_STACK] == STACK_LOWER)
                                (*info)["lower"] = "true";
                            if (this->_verbosityParsing) {
                                MainWindow::info("        has post-params '" + endParamsToken.replace("\n", "\\n") + "' ending@"
                                           + to_string(endParamsTokenI) + ">=" << to_string(paramBTokenI + paramBToken.length()) + "="
                                           + to_string(paramBTokenI) + "+" << to_string(paramBToken.length()) + "in '" + line + "'");
                            }
                            if (endParamsToken != (*itList)[TOKEN_END_PARAMS]) {
                                // since could be used for more stuff after 2 params in future versions,
                                // length should be 0 if not found but forced:
                                endParamsToken = "";
                            }
                            fileI = fileTokenI + fileToken.length();
                            break;
                        } else if (this->_verbosityParsing) {
                            MainWindow::info("        no post-params '" + endParamsToken + "' >="
                                       + to_string(paramBTokenI + paramBToken.length()) << "in '" + line + "'");
                        }
                    } else if (this->_verbosityParsing) {
                        MainWindow::info("      no pre-ParamB '" + paramBToken + "' >="
                                   + to_string(paramATokenI + paramAToken.length()) + "in '" + line + "'");
                    }
                } else if (this->_verbosityParsing) {
                    MainWindow::info("    no pre-paramA '" + paramAToken + "' >="
                               + to_string(fileTokenI + fileToken.length()));
                }
            } else if (this->_verbosityParsing) {
                MainWindow::info("  no pre-File '" + fileToken + "' >= START");
            }
        }
    }

    // MainWindow::info("fileTokenI: "+ to_string(fileTokenI));
    // MainWindow::info("paramATokenI: " + to_string(paramATokenI));
    // MainWindow::info("paramBTokenI: " + to_string(paramBTokenI));
    // MainWindow::info("endParamsTokenI: " + to_string(endParamsTokenI);
    // MainWindow::info("fileToken: " + fileToken);
    // MainWindow::info("paramAToken: " + paramAToken);
    // MainWindow::info("paramBToken: " + paramBToken);
    // MainWindow::info("endParamsToken: " + endParamsToken);
    if (fileI >= 0 && (paramATokenI > fileI || endParamsToken > fileI)) {
        // Even if closer is not present,
        // endParamsTokenI is set to length() IF applicable to this enclosure

        string filePath;
        if (paramATokenI > fileI)
            filePath = line.substr(fileI, paramATokenI - fileI);
        else
            filePath = line.substr(fileI, endParamsTokenI - fileI);

        filePath = strip(filePath);
        if (filePath.length() >= 2) {
            if ((startswithCS(filePath, '"') && endswithCS(filePath, '"')) || (startswithCS(filePath, '\'') && endswithCS(filePath, '\''))) {
                filePath = filePath.substr(1, filePath.length() - 2);
            }
        }
        MainWindow::debug("[outputinspector][debug] file path before unmangling: \"" + filePath + "\"");
        filePath = unmangledPath(filePath);
        (*info)["file"] = filePath;
        (*info)["row"] = line.substr(paramAI, paramBTokenI - paramAI);
        if (paramBToken.length() > 0)
            (*info)["column"] = line.substr(paramBI, endParamsTokenI - paramBI);
        else
            (*info)["column"] = "";
        if (this->_verbosityParsing) MainWindow::info("        file '" + line.substr(fileI, paramATokenI - fileI) + "'");
        // if (this->_verbosityParsing) MainWindow::info("        row '" + line.substr(paramAI, paramBTokenI - paramAI) + "'");
        if (this->_verbosityParsing) MainWindow::info("        row '" + to_string((*info)["row"]) + "'");
        if (this->_verbosityParsing) MainWindow::info("          length " + to_string(paramBTokenI) + "-" to_string(paramAI);
        //if (this->_verbosityParsing) qInfo().noquote() << "        col '" + line.substr(paramBI, endParamsTokenI - paramBI) + "'";
        if (this->_verbosityParsing) MainWindow::info("        col '" + to_string((*info)["column"]) + "'");
        if (this->_verbosityParsing) MainWindow::info("          length " + to_string(endParamsTokenI) + "-" + to_string(paramBI));

        if (endswithCS(filePath, ".py"))
            (*info)["language"] = "python";
        else if (endswithCS(filePath, ".pyw"))
            (*info)["language"] = "python";
        else if (endswithCS(filePath, ".cpp"))
            (*info)["language"] = "c++";
        else if (endswithCS(filePath, ".h"))
            (*info)["language"] = "c++";
        else if (endswithCS(filePath, ".hpp"))
            (*info)["language"] = "c++";
        else if (endswithCS(filePath, ".c"))
            (*info)["language"] = "c";
        else if (endswithCS(filePath, ".js"))
            (*info)["language"] = "js";
        else if (endswithCS(filePath, ".java"))
            (*info)["language"] = "java";
        else if (endswithCS(filePath, ".bat"))
            (*info)["language"] = "bat";
        else if (endswithCS(filePath, ".sh"))
            (*info)["language"] = "sh";
        else if (endswithCS(filePath, ".command"))
            (*info)["language"] = "sh";
        else if (endswithCS(filePath, ".php"))
            (*info)["language"] = "php";
        MainWindow::debug("  detected file: '" + filePath + "'");
        (*info)["good"] = "true";
        // MainWindow::info("[outputinspector] found a good line with the following filename: " + this->filePath());
    }
    else {
        (*info)["good"] = "false";
        // MainWindow::info("[outputinspector] found a bad line: " + originalLine);
    }
    if ((*info)["good"] == "true") {
        if (actualJump.length() > 0 && (*info)["master"] == "false") {
            MainWindow::debug(string("INFO: nosetests output was detected, but the line is not first")
                        + string("line of a known multi-line error format, so flagging as")
                        + "details (must be a sample line of code or something).");
            (*info)["good"] = "false"; // TODO: possibly eliminate this for fault tolerance
                                       // (different styles in same output)
            (*info)["details"] = "true";
            (*info)["file"] = "";
        }
    }
}

void MainWindow::debug(string msg) {
    if (this->_verbosity > 1) {
        cerr << msg << endl;
    }
}
void MainWindow::info(string msg) {
    cerr << "INFO: " << msg << endl;
}
void MainWindow::warn(string msg) {
    cerr << "WARNING: " << msg << endl;
}

string MainWindow::absPathOrSame(string filePath)
{
    string absFilePath;
    string sCwd = QDir::currentPath(); // current() returns a QDir object
    // Don't use QDir::separator(), since this only is detectable on *nix & bsd-like OS:
    QDir cwDir = QDir(sCwd);
    string setuptoolsTryPkgPath = QDir::cleanPath(sCwd + QDir::separator() + cwDir.dirName());
    if (this->_verbosity)
        MainWindow::info("setuptoolsTryPkgPath:" + setuptoolsTryPkgPath);
    absFilePath = startswithCS(filePath, "/") ? filePath : (sCwd + "/" + filePath);
    if (!QFile(absFilePath).exists() && QDir(setuptoolsTryPkgPath).exists())
        absFilePath = QDir::cleanPath(setuptoolsTryPkgPath + QDir::separator() + filePath);
    return absFilePath;
}

void MainWindow::on_mainListWidget_itemDoubleClicked(OIWidget* item)
{
    string line = item->text();
    string actualJump = item->data(ROLE_COLLECTED_FILE).toString(); // item->toolTip();
    string actualJumpLine = item->data(ROLE_COLLECTED_LINE).toString(); // item->toolTip();
    if (actualJumpLine.length() > 0)
        line = actualJumpLine;
    bool ok = false;
    string filePath = (item->data(ROLE_COLLECTED_FILE)).toString();
    string absFilePath = filePath;
    string errorMsg;
    if (filePath.length() > 0) {
        if (this->_verbosity) {
            MainWindow::info("clicked_file: '" + filePath + "'");
            MainWindow::info("tooltip: '" + item->toolTip() + "'");
        }
        absFilePath = absPathOrSame(filePath);
        string citedRowS = (item->data(ROLE_ROW)).toString();
        string citedColS = (item->data(ROLE_COL)).toString();
        if (this->_verbosity) {
            MainWindow::info("citedRowS: '" + citedRowS + "'");
            MainWindow::info("citedColS: '" + citedColS + "'");
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
        if (this->m_CompensateForKateTabDifferences) {
            QFile qfileSource(absFilePath);
            string line;
            int readCitedI = 0; /**< This is the current line number while the
                                     loop reads the entire cited file. */
            if (qfileSource.open(QFile::ReadOnly)) { //| QFile::Translate
                QTextStream qtextNow(&qfileSource);
                while (!qtextNow.atEnd()) {
                    line = qtextNow.readLine(0); //does trim off newline characters
                    if (readCitedI == ((citedRow - yEditorOffset) - 1)) {
                        int tabCount = 0;
                        // TODO: Use regex for finding the tab.
                        for (int tryTabI = 0; tryTabI < line.length(); tryTabI++) {
                            if (line.substr(tryTabI, 1) == "\t")
                                tabCount++;
                            else
                                break;
                        }
                        string tabDebugMsg;
                        if (tabCount > 0) {
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
                            if (this->m_KateMajorVer < 3) {
                                // Kate 2.5.9 reads a 'c' argument value of 0 as the beginning of the line and 1 as the first character after the leading tabs
                                if (citedCol < tabCount)
                                    citedCol = 0;
                                else {
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
                                    for (int tryTabI = 1; tryTabI < citedCol; tryTabI++) {
                                        if (tryTabI <= (tabCount - 1) * this->settings->getInt("Kate2TabWidth") + 1) {
                                            if (tryTabI != 1 && (tryTabI - 1) % this->settings->getInt("Kate2TabWidth") == 0) {
                                                regeneratedCol++; // only add if it is 4,7,10,etc where addend is this->config->getInt("Kate2TabWidth") (1+this->config->getInt("Kate2TabWidth")*position)
                                                tabDebugMsg += "-";
                                            }
                                        } else {
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
            else {
                errorMsg = "Specified file '" + filePath + "' does not exist or is not accessible (if path looks right, try running from the location where it exists instead of '" + QDir::currentPath() + "')";
            }
        } // end if this->m_CompensateForKateTabDifferences
        // string sArgs="-u "+absFilePath+" -l "+citedRowS+" -c "+citedColS;
        // QProcess qprocNow(this->config->getString("editor")+sArgs);
        // qprocNow
        if (QFile(absFilePath).exists()) {
            string commandMsg = this->settings->getString("editor");
            std::vector<std::string> qslistArgs;
            // NOTE: -u is not needed at least as of kate 16.12.3 which does not create additional
            // instances of kate
            // qslistArgs.push_back("-u");
            // commandMsg+=" -u";
            // qslistArgs.push_back("\""+absFilePath+"\"");
            qslistArgs.push_back(absFilePath);
            commandMsg += " " + absFilePath;
            qslistArgs.push_back("--line"); // split into separate arg, otherwise geany complains that
                                         // it doesn't understand the arg "--line 1"
            qslistArgs.push_back(citedRowS);
            commandMsg += " --line " + citedRowS;
            // qslistArgs.push_back(citedRowS);
            qslistArgs.push_back("--column"); // NOTE: -c is column in kate, but alternate config dir
                                           // in geany, so use --column
            qslistArgs.push_back(citedColS); // NOTE: -c is column in kate, but alternate config dir
                                             // in geany, so use --column
            commandMsg += " --column " + citedColS;
            // qslistArgs.push_back(citedColS);
            // qWarning().noquote() << "qslistArgs: " << qslistArgs;
            QProcess::startDetached( this->settings->getString("editor"), qslistArgs);
            if (!QFile::exists(this->settings->getString("editor"))) {
                // ok to run anyway for fault tolerance, since may be in system path
                QMessageBox::information(this, "Output Inspector - Configuration", this->settings->getString("editor") + " cannot be accessed.  Try setting the value after editor= in " + this->settings->fileName());
            }
            // if (this->_verbosity)
            ui->statusBar->showMessage(commandMsg, 0);
            // system(sCmd);//stdlib
            // QMessageBox::information(this,"test",sCmd);
        } else {
            // errorMsg = "Specified file '" + absFilePath + "' does not exist (try a different line, or try running from the location where it exists instead of '" + QDir::currentPath() + "')";
            errorMsg = "[Output Inspector] No file exists here: '" + absFilePath + "'\n";
        }
    } // end if line is in proper format
    else
        errorMsg = "Could not detect error format\n";
    if (errorMsg.length() > 0) {
        if (filePath.length() > 0) {
            // qWarning().noquote() << errorMsg << " in '" + line + "':";
            std::map<string, string>* info = lineInfo(line, actualJump, actualJumpLine, false);
            string infoS;
            for (auto it = info->begin(); it != info->end(); it++) {
                // qWarning().noquote() << "    " << it->first // key
                //         + ": '" + it->second + "'"; //value
                infoS += "; " + it->first + ": '" + it->second + "'";
            }
            MainWindow::warn("[outputinspector][error] " << errorMsg << " in the line:"
                       + "; actualJump: " + item->data(this->ROLE_COLLECTED_FILE).toString()
                       + "  actualJumpLine: " + item->data(this->ROLE_COLLECTED_LINE).toString()
                       + infoS;
            //                      << "  info:";

            // for (auto const& it : (*info)) {  // >=C++11 only (use dot notation not -> below if using this)

            QMessageBox::information(this, "Output Inspector", errorMsg);
            // QMessageBox::information(this,"Output Inspector","'"+absFilePath+"' cannot be accessed (try a different line, or if this line's path looks right, try running from the location where it exists instead of '"+QDir::currentPath()+"')");
            // or pasting the entire line to 'Issues' link on web-based git repository
        } else {
            MainWindow::info("[Output Inspector] No file was detected in line: '" + line + "'");
            MainWindow::info("[Output Inspector] ERROR: '" + errorMsg + "'");
        }
    }
}

void MainWindow::readInput()
{
    int limit = 50;
    int count = 0;
    std::string line = " ";
    while (count < limit && !line.empty()) {
        std::streamsize size = std::cin.rdbuf()->in_avail();
        if (size < 1) {
            // qInfo().noquote() << "OutputInspector: There is no input: got " << size;
            // Prevent waiting forever for a line.
            break;
        }
        std::getline(std::cin, line);
        if (!std::cin.eof()) {
            // qInfo().noquote() << "OutputInspector: input is '" << line.c_str() << "'.";
            // this->addLine(string("OutputInspector: input is: ") + string::fromStdString(line), true);
            this->addLine(string::fromStdString(line), true);
        }
        else {
            // qInfo().noquote() << "OutputInspector: input has ended.";
            // this->addLine("# OutputInspector: input has ended.", true);
            break;
        }
        count++;
    }

} // end MainWindow::on_mainListWidget_itemDoubleClicked
