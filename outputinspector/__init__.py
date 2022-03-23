#!/usr/bin/env python
import sys
import os

from outputinspector.reporting import (
    warn,
    error,
    debug,
    critical,
    fatal,
    pinfo,
)

from settings import Settings

try:
    from tkinter import messagebox
except ImportError:
    # Python 2
    import tkMessageBox as messagebox

# QMessageBox.information becomes messagebox.showinfo

#class OutputInspectorSleepThread : public QThread
#public:
#    static void msleep(unsigned long msecs)
#        QThread.msleep(msecs)



# endregion scripting

def contains_any(haystack, needles):
    # cs = Qt.CaseSensitive
    for needle in needles:
        if needle in haystack:
            return True
    return False


COLLECT_REUSE = "REUSE"
'''*< The target in the analyzed output should be also used as the jump
location for the following lines. '''
STACK_LOWER = "LOWER"
'''*< The code reference is further down in the call stack, it is
probably not pointing to the relevant code.'''
TOKEN_FILE = 0
'''*< linelinedef[TOKEN_FILE] is the opener for the file path (blank if
the file path starts at the begginning of the line). '''
TOKEN_PARAM_A = 1
'''*< linelinedef[TOKEN_PARAM_A] is the first coordinate token (blank
if none, as grep-- -n is automatically added if you use the included
ogrep script). '''
TOKEN_PARAM_B = 2
'''*< linelinedef[TOKEN_PARAM_B] is the second coordinate delimiter
(blank if no column). '''
TOKEN_END_PARAMS = 3;
'''*< linelinedef[TOKEN_END_PARAMS] ParamsEnder (what is after last
coord). '''
PARSE_COLLECT = 4;
'''*< linelinedef[PARSE_COLLECT] determines the mode for connecting
lines. For possible values and their behaviors, the documentation for
COLLECT_REUSE (or future COLLECT_* constants). '''
PARSE_STACK = 5
'''*< linelinedef[PARSE_STACK] flags a pattern as being for a
callstack, such as to connect it to a previous error (see documentation
for STACK_LOWER or for any later-added STACK_* constants). '''
PARSE_DESCRIPTION = 5
'''*< linelinedef[PARSE_STACK] describes the parser mode (linedef) in a
human-readable way.'''
PARSE_PARTS_COUNT = 7
ROLE_COLLECTED_FILE = Qt.UserRole
ROLE_ROW = Qt.UserRole + 1
ROLE_COL = Qt.UserRole + 2
ROLE_LOWER = Qt.UserRole + 3
ROLE_COLLECTED_LINE = Qt.UserRole + 4
ROLE_DETAILS = Qt.UserRole + 5

class MainWindow(ttk.Frame):


    def __init__(self, root):
        # public:
        # static QString unmangledPath(QString path)

        self.m_DebugBadHints = True
        self.sInternalFlags = []
        self.sSectionBreakFlags = ""
        self.enclosures = []
        self.brushes = {}

        # explicit MainWindow(QWidget *parent = 0)
        # ~MainWindow()
        #void addLine(QString, bool)
        #void init(QString)
        #bool isFatalSourceError(QString)
        #{} lineInfo( QString line, actualJump, actualJumpLine, isPrevCallPrevLine)
        #void lineInfo(std.map<QString, info, sLineOriginal, actualJump, actualJumpLine, isPrevCallPrevLine)
        # QString absPathOrSame(QString filePath)
        self.settings = None
        self.m_EnableTabDebugMsg = False
        self.m_CompensateForKateTabDifferences = True
        self.m_KateMajorVer = 0
        '''*< Use 2 to represent the 2.5.9 (kde3
        version); and 3 for 3.0.3 (kde4 version),
        etc. '''
        # ifdef QT_DEBUG
        #     self.m_Verbose = True
        #     self.m_VerboseParsing = True; '''*< Enable line-by-line parser output '''
        # else:
        self.m_Verbose = False
        self.m_VerboseParsing = False
        # endif
        # private slots:
        # void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item)
        # void readInput()

        # private:
        # self._ui = None
        # void CompensateForEditorVersion()

        self.m_ToDoFlags = ["TODO","FIXME"]
        self.m_Error = "Error"
        self.m_Warning = "Warning"
        self.m_CommentToken = "#"

        self.lwiWarnings = []  # QListWidgetItem*[]
        self.lwiToDos = []  # QListWidgetItem*[]
        self.m_LineCount = 0
        self.m_NonBlankLineCount = 0
        self.m_ActualJump = ""
        '''*< Store the jump in case the file & line# are on
        a different line than the error, as with
        nosetests. '''
        self.m_ActualJumpLine = ""
        self.m_IsJumpLower = True


        self.m_MasterLine = ""
        self.m_ActualJumpRow = ""
        self.m_ActualJumpColumn = ""

        #void pushWarnings();'''*< Push warnings to the GUI. '''

        self.inTimer = None  # *< Read standard input lines regularly

        self.iErrors = 0
        self.iWarnings = 0
        self.iTODOs = 0
        self.m_Files = []

        #endif # MAINWINDOW_H
        self._window_init(root)



    '''*
     * This is a Functor for "Contains," such as for multi-needle searches
     * (see [single variable] initializer list in constructor for how haystack is
     * obtained)
    template <class T>
    class ContainsF
        T haystack

    public:
        ContainsF(T val)
            : haystack(val)

        bool operator()(T needle)
            return haystack.contains(needle)
     '''



    '''*
     * This version uses the functor to allow using count_if with a
     * param that is determined later (by the functor using the param).
    template <class T>
    def containsAnyF(self, haystack, needles):
        return count_if(needles.begin(), needles.end(), ContainsF<T>(haystack)) > 0; # FIXME: Test containsAnyF


    template <class T>
    def contains(self, haystack, needle):
        return haystack.contains(needle)
    '''

    '''
    template <class T>
    def qcontains(self, haystack, needle, cs = Qt.CaseSensitive):
        return haystack.contains(needle, cs)


    template <class T>
    def qcontains_any(self, haystack, needles, cs = Qt.CaseSensitive):
        return count_if(needles.begin(), needles.end(), bind(qcontains<T>, haystack, std.placeholders._1, cs)) > 0
    '''

    @staticmethod
    def unmangledPath(self, path):
        QRegularExpression literalDotsRE("\\.\\.\\.+"); '''*< Match 2 dots followed by more. '''
        match = literalDotsRE.match(path)
        verbose = False  # This is manually set to True for debug only.
        if match.hasMatch():
            # start = match.capturedStart(0)
            end = match.capturedEnd(0)
            QList<int> tryOffsets = QList<int>()
            tryOffsets.append(-2)
            tryOffsets.append(1)
            tryOffsets.append(0)
            for tryOffset in tryOffsets:
                tryPath = path.mid(end+tryOffset)
                if QFile(tryPath).exists():
                    if verbose:
                        pinfo("[outputinspector] transformed *.../dir"
                              " into ../dir: \"{}\""
                              "".format(tryPath))

                    return tryPath

                else:
                    if verbose:
                        pinfo("[outputinspector] There is no \""
                              + tryPath
                              + "\" in the current directory (\""
                              + os.getcwd()
                              + "\")")




        else:
            if (verbose):
                pinfo("[outputinspector] There is no \"...\" in the cited path: \"" + path + "\"")

        return path


    def _window_init(self, root):
        # : QMainWindow(parent)
        # , self._ui(new Ui.MainWindow)
        # self._ui.setupUi(self)
        # configDir = QStandardPaths.StandardLocation(QStandardPaths.ConfigLocation())
        # QStandardPaths.StandardLocation(QStandardPaths.HomeLocation)
        # ^ same as QDir.homePath()
        # See <https:#stackoverflow.com/questions/32525196/
        #   how-to-get-a-settings-storage-path-in-a-cross-platform-way-in-qt>
        path = QStandardPaths.writableLocation(QStandardPaths.AppDataLocation)
        if path.isEmpty()) qFatal("Cannot determine settings storage location":
        QDir d{path
        if not d.exists():
            d.mkpath(d.absolutePath())

        filePath = QDir.cleanPath(d.absolutePath() + QDir.separator() + "settings.txt")
        QFile f{filePath
        #/ TODO: fill self in or remove it (and the comments).
        if d.exists():
            if not f.exists():
                # if f.open(QIODevice.WriteOnly | QIODevice.Truncate):
                #     f.write("Hello, World")

            else:



        # qInfo().noquote().nospace() << "Creating settings: " << filePath
        self.settings = Settings(filePath)
        qInfo().noquote().nospace()
                << "[outputinspector]"
                << " used the settings file \"" << self.settings.fileName() << "\""
        # if QFile("/etc/outputinspector.conf").exists():    #     self.config = Settings("/etc/outputinspector.conf")
        #
        # elif QFile("/etc/outputinspector.conf").exists():    #     self.config = Settings("/etc/outputinspector.conf")
        #
        self.settings.setIfMissing("Kate2TabWidth", 8)
        self.settings.setIfMissing("CompilerTabWidth", 6)
        self.settings.setIfMissing("ShowWarningsLast", False)
        #/ TODO: Implement ShowWarningsLast (but ignore it and behave as if it were
        #/ False if there is anything in stdin).
        self.settings.setIfMissing("FindTODOs", True)
        if self.settings.contains("kate"):
            changed = self.settings.setIfMissing("editor", self.settings.getString("kate"))
            self.settings.remove("kate")
            self.settings.sync()
            if changed:
                qInfo().noquote().nospace()
                        << "[outputinspector]" << " transferred the old setting"
                        << " 'kate=" << self.settings.getString("kate")
                        << "' to 'editor=" << self.settings.getString("editor")
                        << "'."
            else:
                qInfo().noquote().nospace()
                        << "[outputinspector]" << " ignored the deprecated setting"
                        << " 'kate=" << self.settings.getString("kate")
                        << "' in favor of 'editor="
                        << self.settings.getString("editor")
                        << "'."

        self.settings.setIfMissing("editor", "/usr/bin/geany")


    def init(self, errorsListFileName)
        '''
        formats with "\n" at end must be AFTER other single-param formats that have
        same TOKEN_FILE and PARSE_PARAM_A, because "\n" is forced
        (which would leave extra stuff at the end if there are more tokenings)
        '''
        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linelinedef.append("")
        linelinedef[TOKEN_FILE] = "  File "
        linelinedef[TOKEN_PARAM_A] = ", line "
        linelinedef[TOKEN_PARAM_B] = ")"
        linelinedef[TOKEN_END_PARAMS] = ""
        linelinedef[PARSE_COLLECT] = ""
        linelinedef[PARSE_STACK] = ""
        linelinedef[PARSE_DESCRIPTION] = "Nose error"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linelinedef.append("")
        linelinedef[TOKEN_FILE] = "  File "
        linelinedef[TOKEN_PARAM_A] = ", line "
        linelinedef[TOKEN_PARAM_B] = ""
        linelinedef[TOKEN_END_PARAMS] = ","
        linelinedef[PARSE_COLLECT] = COLLECT_REUSE
        linelinedef[PARSE_STACK] = STACK_LOWER
        linelinedef[PARSE_DESCRIPTION] = "Nose lower traceback"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linelinedef.append("")
        linelinedef[TOKEN_FILE] = "ERROR: Failure: SyntaxError (invalid syntax ("
        linelinedef[TOKEN_PARAM_A] = ", line "
        linelinedef[TOKEN_PARAM_B] = ""
        linelinedef[TOKEN_END_PARAMS] = ")"
        linelinedef[PARSE_COLLECT] = ""
        linelinedef[PARSE_STACK] = ""
        linelinedef[PARSE_DESCRIPTION] = "Nose syntax error"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = "  File "
        linedef[TOKEN_PARAM_A] = ", line "
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = "\n"
        linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Nose upper traceback"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = "ERROR[Main]:"
        linedef[TOKEN_PARAM_A] = ":"
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = ":"
        # linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Minetest Lua traceback"
        self.enclosures.append(linedef)

        # An example of jshint output is the entire next comment:
        # functions.js: line 32, 26, Use '!==' to compare with 'null'.
        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = ""
        linedef[TOKEN_PARAM_A] = ": line "
        linedef[TOKEN_PARAM_B] = ", col "
        linedef[TOKEN_END_PARAMS] = ", "
        # linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "hint from jshint"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        # TODO: change to "WARNING\[Server\].* accessed at " (requires:
        # implementing regex)
        linedef[TOKEN_FILE] = " accessed at "
        linedef[TOKEN_PARAM_A] = ":"
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = "\n"
        # linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Minetest access warning"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        #/ TODO: change to "WARNING\[Server\].* accessed at " (requires:
        # implementing regex)
        linedef[TOKEN_FILE] = " inside a function at "
        linedef[TOKEN_PARAM_A] = ":"
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = "."
        # linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Minetest warning 'inside a function'"
        self.enclosures.append(linedef)

        linedef = []; '''*< This is a fallback definition that applies
                              to various parsers.
                              Simpler definitions must be attempted in
                              order from most to least complex to avoid
                              False positives (Now there are even
                              simpler ones after self one). '''

        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = ""
        linedef[TOKEN_PARAM_A] = "("
        linedef[TOKEN_PARAM_B] = ","
        linedef[TOKEN_END_PARAMS] = ")"
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "generic ('path(row,*col)')"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = ""
        linedef[TOKEN_PARAM_A] = ":"
        linedef[TOKEN_PARAM_B] = ":"
        linedef[TOKEN_END_PARAMS] = ":"
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "pycodestyle-like"
        self.enclosures.append(linedef)

        linedef = []
        # -n option for grep shows line # like:
        # <filename>:<number>:
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = ""
        linedef[TOKEN_PARAM_A] = ":"
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = ":"
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "grep -n result"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = ""
        linedef[TOKEN_PARAM_A] = ""
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = ":"
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "grep-like result (path then colon)"
        self.enclosures.append(linedef)

        self.brushes["TracebackNotTop"] = QBrush(QColor.fromRgb(128, 60, 0))
        self.brushes["Unusable"] = QBrush(Qt.lightGray)
        self.brushes["Internal"] = QBrush(QColor.fromRgb(192, 192, 100))
        self.brushes["Warning"] = QBrush(QColor.fromRgb(192, 120, 80))
        self.brushes["WarningDetails"] = QBrush(QColor.fromRgb(255, 180, 120))
        self.brushes["Error"] = QBrush(QColor.fromRgb(80, 0, 0))
        self.brushes["ErrorDetails"] = QBrush(QColor.fromRgb(160, 80, 80))
        self.brushes["ToDo"] = QBrush(Qt.darkGreen)
        self.brushes["Default"] = QBrush(Qt.black)

        self.sInternalFlags.append("/site-packages/")
        assert(contains_any("/usr/lib/python2.7/site-packages/nose/importer.py", self.sInternalFlags))

        self.sSectionBreakFlags.append("--------")
        assert(contains_any("---------------------", self.sSectionBreakFlags))

        qDebug().noquote() << "qDebug() stream is active."
        # qInfo().noquote() << "qInfo() stream is active."
        # qWarning().noquote() << "qWarning() stream is active."
        # qCritical().noquote() << "qCritical() stream is active."
        # qFatal().noquote() << "qFatal() stream is active."
        if self.m_Verbose:
            qInfo().noquote() << "lists:"
        # self for loop has the brace on the next line (for clang-format test):
        for itList in self.enclosures:
            # qInfo().noquote() << "  -"
            # for i in range(len(itList)): qInfo().noquote() << "    - " << itList[i]
            #
            assert(len(itList) >= PARSE_PARTS_COUNT); # qInfo().noquote() << "  items.size(): " << itList.size()
            if self.m_Verbose:
                qInfo().noquote() << "  items: ['" + "', '".join(itList) + "']"

    def isFatalSourceError(self, line):
    '''*
     * @brief Check whether your parser reports that your code has a fatal error.
     * @param line stderr output from your parser
     * @return
     '''
        return (
                   line.indexOf("Can't open", 0, Qt.CaseInsensitive) > -1 #jshint could not find a source file
                   or line.indexOf("Too many errors", 0, Qt.CaseInsensitive) > -1 #jshint already showed the error for self line, can't display more errors
                   or line.indexOf("could not be found", 0, Qt.CaseInsensitive) > -1 #mcs could not find a source file
                   or line.indexOf("compilation failed", 0, Qt.CaseInsensitive) > -1 #mcs could not compile the sources
               )


    def __del__(self):
        del self.inTimer
        del self.settings
        # del self._ui

    def init(self, errorsListFileName):
        if not (self.settings.contains("xEditorOffset") or self.settings.contains("yEditorOffset")):
            self.CompensateForEditorVersion()
        if errorsListFileName.length() == 0:
            tryPath = "debug.txt"
            if QFile(tryPath).exists():
                errorsListFileName = tryPath
                qInfo().noquote().nospace() << "[outputinspector]"
                                            << " detected \"" << tryPath << "\"...examining..."

            else:
                errorsListFileName = "err.txt"

        # QTextStream err(stderr);  # avoid quotes and escapes caused by qWarning().noquote() being derived from qDebug()--alternative to qWarning().noquote().noquote()<<

        QFile qfileTest(errorsListFileName)
        # self.m_ToDoFlags.append("TODO")
        # self.m_ToDoFlags.append("FIXME")
        # cutToDoCount = 2
        # self._ui.mainListWidget is a QListWidget
        # setCentralWidget(self._ui.mainListWidget)
        # self._ui.mainListWidget.setSizePolicy(QSizePolicy.)
        # OutputInspectorSleepThread.msleep(150); # wait for stdin (doesn't work)
        if std.cin.rdbuf().in_avail() > 1:
            # TODO: fix self--self never happens (Issue #16)
            qInfo().noquote().nospace() << "[outputinspector]"
                                        << " detected standard input (such as from a console pipe)"
                                        << "...skipping \"" << errorsListFileName << "\"..."

        elif qfileTest.exists():
            if (qfileTest.open(QFile.ReadOnly))   #| QFile.Translate
                QTextStream qtextNow(&qfileTest)
                while (not qtextNow.atEnd())
                    # NOTE: Readline trims off newline characters.
                    self.addLine(qtextNow.readLine(0), False)
                } # end while not at end of file named errorsListFileName
                qfileTest.close()
                QString sNumErrors
                sNumErrors = str(self.iErrors)
                QString sNumWarnings
                sNumWarnings = str(self.iWarnings)
                QString sNumTODOs
                sNumTODOs = str(self.iTODOs)
                pushWarnings()
                if self.settings.getBool("FindTODOs"):
                    for it in self.lwiToDos:
                        self._ui.mainListWidget.addItem(it)


                if not self.settings.getBool("ExitOnNoErrors"):
                    if self.m_LineCount == 0:
                        lwiNew = QListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 lines in file")
                        lwiNew.setForeground(self.brushes["Default"])
                        self._ui.mainListWidget.addItem(lwiNew)

                    elif self.m_NonBlankLineCount == 0:
                        lwiNew = QListWidgetItem("#" + errorsListFileName + ": INFO (generated by outputinspector) 0 non-blank lines in file")
                        lwiNew.setForeground(self.brushes["Default"])
                        self._ui.mainListWidget.addItem(lwiNew)


                # else hide errors since will exit anyway if no errors
                sMsg = "Errors: " + sNumErrors + "; Warnings:" + sNumWarnings
                if self.settings.getBool("FindTODOs"):
                    sMsg += "; TODOs:" + sNumTODOs
                self._ui.statusBar.showMessage(sMsg, 0)
                if self.settings.getBool("ExitOnNoErrors"):
                    if self.iErrors < 1:
                        qInfo() << "Closing since no errors..."
                        # QCoreApplication.quit(); # doesn't work
                        # aptr.exit(); # doesn't work (QApplication*)
                        # aptr.quit(); # doesn't work
                        # aptr.closeAllWindows(); # doesn't work
                        # if the event loop is not running, function (QCoreApplication.exit()) does nothing
                        exit(EXIT_FAILURE)



            else:
                if std.cin.rdbuf().in_avail() < 1:
                    my_path = QCoreApplication.applicationFilePath()
                    title = "Output Inspector - Help"
                    msg = my_path + ": Output Inspector cannot read the output file due to permissions or other read error (tried \"./" + errorsListFileName + "\")."
                    messagebox.showinfo(self, title, msg)
                    # self.addLine(title + ":" + msg, True)


        } # end if could open file named errorsListFileName
        else:
            if std.cin.rdbuf().in_avail() < 1:
                my_path = QCoreApplication.applicationFilePath()
                title = "Output Inspector - Help"
                msg = my_path + ": Output Inspector cannot find the output file to process (tried \"./" + errorsListFileName + "\")."
                messagebox.showinfo(self, title, msg)
                # self.addLine(title + ":" + msg, True)


        self.inTimer = QTimer(self)
        self.inTimer.setInterval(500);  # milliseconds
        self.inTimer.timeout.connect(self.readInput)
        self.inTimer.start()
    } # end init

    def addLine(self, line, enablePush):
        '''
        This method will add or collect a line. This method sets some related private
        variables for the purpose of connecting a line (such as a callstack line)
        to a previous line.

        @brief Add a line.
        @param line a line from standard output or error from a program
        @param enablePush Push the line to the GUI right away (This is best for
               when reading information from standard input).
        '''
        self.m_LineCount += 1
        originalLine = line
        self.m_MasterLine = line
        # TODO: debug performance of and delete
        info = {}  # string, string
        if line.length() > 0:
            if line.trimmed().length() > 0:
                self.m_NonBlankLineCount += 1
            if self.isFatalSourceError(line):
                self._ui.mainListWidget.addItem(new QListWidgetItem(line + " <your compiler (or other tool) recorded self fatal or summary error before outputinspector ran>", self._ui.mainListWidget))

            elif contains_any(line, self.sSectionBreakFlags):
                self.m_ActualJump = ""
                self.m_ActualJumpLine = ""
                self.m_ActualJumpRow = ""
                self.m_ActualJumpColumn = ""
                lwi = QListWidgetItem(line)
                lwi.setForeground(self.brushes["Regular"])
                self._ui.mainListWidget.addItem(lwi)

            else:
                # lineInfo does the actual parsing:
                self.lineInfo(info, line, self.m_ActualJump, self.m_ActualJumpLine, True)

                if info.at("master") == "True":
                    self.m_ActualJump = info.at("file")
                    self.m_ActualJumpLine = line
                    self.m_ActualJumpRow = info.at("row")
                    self.m_ActualJumpColumn = info.at("column")
                    self.m_IsJumpLower = (info.at("lower") == "True")
                    qDebug().noquote() << "(master) set actualJump to '" + self.m_ActualJump + "'"

                else:
                    qDebug().noquote() << "(not master) line: '" + line + "'"

                isWarning = False
                sColorPrefix = "Error"
                if self.m_ActualJump.length() > 0:
                    self.m_MasterLine = self.m_ActualJumpLine

                if self.m_MasterLine.indexOf(self.m_Warning, 0, Qt.CaseInsensitive) > -1:
                    isWarning = True
                    sColorPrefix = "Warning"

                # do not specify self._ui.mainListWidget on new, will be added automatically
                lwi = QListWidgetItem(line)
                if self.m_ActualJumpRow.length() > 0:
                    lwi.setData(ROLE_ROW, QVariant(self.m_ActualJumpRow))
                    lwi.setData(ROLE_COL, QVariant(self.m_ActualJumpColumn))

                else:
                    lwi.setData(ROLE_ROW, QVariant(info.at("row")))
                    lwi.setData(ROLE_COL, QVariant(info.at("column")))

                if self.m_ActualJump.length() > 0:
                    lwi.setData(ROLE_COLLECTED_FILE, QVariant(self.m_ActualJump))
                    if info.at("lower") == "True":
                        lwi.setForeground(self.brushes["TracebackNotTop"])
                    elif info.at("good") == "True":
                        lwi.setForeground(self.brushes[sColorPrefix])
                    else:
                        lwi.setForeground(self.brushes[sColorPrefix + "Details"])

                else:
                    lwi.setData(ROLE_COLLECTED_FILE, QVariant(info.at("file")))
                    if info.at("good") == "True":
                        lwi.setForeground(self.brushes[sColorPrefix])
                    else:
                        lwi.setForeground(self.brushes["Unusable"])

                if contains_any(self.m_MasterLine, self.sInternalFlags):
                    lwi.setForeground(self.brushes["Internal"])

                lwi.setData(ROLE_COLLECTED_LINE, QVariant(self.m_MasterLine))
                lwi.setData(ROLE_DETAILS, QVariant(line != self.m_MasterLine))
                lwi.setData(ROLE_LOWER, QVariant(info.at("lower")))
                if info.at("good") == "True":
                    if isWarning:
                        self.iWarnings += 1
                    else:
                        self.iErrors += 1

                if self.settings.getBool("ShowWarningsLast") and isWarning:
                    self.lwiWarnings.append(lwi)
                else:
                    self._ui.mainListWidget.addItem(lwi)

                sTargetLanguage = info["language"]

                if sTargetLanguage.length() > 0:
                    if sTargetLanguage == "python" or sTargetLanguage == "sh":
                        self.m_CommentToken = "#"

                    elif (sTargetLanguage == "c++" or sTargetLanguage == "c" or sTargetLanguage == "php"
                             or sTargetLanguage == "js" or sTargetLanguage == "java")
                        self.m_CommentToken = "#"

                    elif sTargetLanguage == "bat":
                        self.m_CommentToken = "rem "



                #if (is_jshint and info["file"].endsWith(".js")) or line.indexOf(self.m_Error, 0, Qt.CaseInsensitive) > -1:            #  # TODO?: if (is_jshintor line.indexOf("previous error",0,Qt.CaseInsensitive)<0) self.iErrors += 1
                #  # if self.config.getBool("ShowWarningsLast")) self.m_Errors.append(line:
                #

                if self.settings.getBool("FindTODOs"):
                    if info.at("good") == "True":
                        QString sFileX;# = unmangledPath(info.at("file"))
                        sFileX = absPathOrSame(sFileX); # =line.mid(0,line.indexOf("("))
                        if not self.m_Files.contains(sFileX, Qt.CaseSensitive):
                            self.m_Files.append(sFileX)
                            QFile qfileSource(sFileX)
                            if self.m_Verbose:
                                qDebug() << "outputinspector trying to open '" + sFileX + "'..."
                            # if not qfileSource.open(QFile.ReadOnly):                        #
                            if qfileSource.open(QFile.ReadOnly):
                                QTextStream qtextSource(&qfileSource)
                                iSourceLineFindToDo = 0
                                while (not qtextSource.atEnd())
                                    sSourceLine = qtextSource.readLine(0)
                                    iSourceLineFindToDo++; # Increment self now since the compiler's line numbering starts with 1.
                                    iToDoFound = -1
                                    iCommentFound = sSourceLine.indexOf(self.m_CommentToken, 0, Qt.CaseInsensitive)
                                    if iCommentFound > -1:
                                        for i in range(len(self.m_ToDoFlags)):
                                            iToDoFound = sSourceLine.indexOf(self.m_ToDoFlags[i], iCommentFound + 1, Qt.CaseInsensitive)
                                            if iToDoFound > -1:
                                                break


                                    if iToDoFound > -1:
                                        QString sNumLine
                                        sNumLine.setNum(iSourceLineFindToDo, 10)
                                        QString sNumPos
                                        processedCol = iToDoFound
                                        for citedI in range(len(sSourceLine)):
                                            if sSourceLine.mid(citedI, 1) == "\t":
                                                processedCol += (self.settings.getInt("CompilerTabWidth") - 1)
                                            else:
                                                break

                                        processedCol += 1; # start numbering at 1 to mimic compiler
                                        processedCol += 2; # +2 to start after slashes
                                        sNumPos.setNum(processedCol, 10)
                                        sLineToDo = sFileX + "(" + sNumLine + "," + sNumPos + ") " + sSourceLine.mid(iToDoFound)
                                        lwi = QListWidgetItem(sLineToDo)
                                        lwi.setData(ROLE_ROW, QVariant(sNumLine))
                                        lwi.setData(ROLE_COL, QVariant(sNumPos))
                                        lwi.setData(ROLE_COLLECTED_FILE, QVariant(sFileX))
                                        lwi.setData(ROLE_LOWER, QVariant("False"))
                                        lwi.setData(ROLE_COLLECTED_LINE, QVariant(sLineToDo))
                                        lwi.setData(ROLE_DETAILS, QVariant("False"))
                                        if contains_any(self.m_MasterLine, self.sInternalFlags):
                                            lwi.setForeground(self.brushes["Internal"])
                                        else:
                                            lwi.setForeground(self.brushes["ToDo"])

                                        self.lwiToDos.append(lwi)
                                        self.iTODOs += 1

                                } # end while not at end of source file
                                if self.m_Verbose:
                                    qDebug() << "outputinspector finished reading sourcecode"
                                if self.m_Verbose:
                                    qDebug() << "(processed " << iSourceLineFindToDo << " line(s))"
                                qfileSource.close()
                            } # end if could open sourcecode
                            else:
                                qWarning().noquote() << "[outputinspector] did not scan a file that is cited by the log but that is not present: '" + sFileX + "'"

                        } # end if list does not already contain self file
                    } # end if found filename ender
                    elif self.m_Verbose:
                        qDebug() << "[outputinspector] WARNING: filename ender in " + line
                } # end if getIniBool("FindTODOs")
                else:
                    qDebug() << "[outputinspector] WARNING: getIniBool(\"FindTODOs\") off so skipped parsing " + line
            } # end if a regular line (not fatal, formatting)
        } # end if length>0 (after trim using 0 option for readLine)
        if enablePush:
            self.pushWarnings()

        delete info


    def CompensateForEditorVersion(self):
        isFound = False
        QStringList sVersionArgs
        sFileTemp = "/tmp/outputinspector.using.kate.version.tmp"
        sVersionArgs.append("--version")
        sVersionArgs.append(" > " + sFileTemp)
        # QProcess.execute(IniString("editor"), sVersionArgs)
        system((char*)QString(self.settings.getString("editor") + " --version > " + sFileTemp).toLocal8Bit().data())
        OutputInspectorSleepThread.msleep(125)

        QFile qfileTmp(sFileTemp)
        QString line
        if (qfileTmp.open(QFile.ReadOnly))   # | QFile.Translate
            # detect Kate version using output of Kate command above
            QTextStream qtextNow(&qfileTmp)
            sKateOpener = "Kate: "
            while (not qtextNow.atEnd())
                line = qtextNow.readLine(0); # does trim off newline characters
                if self.m_Verbose:
                    messagebox.showinfo(self, "Output Inspector - Finding Kate version...", line)
                if line.startsWith(sKateOpener, Qt.CaseInsensitive):
                    iDot = line.indexOf(".", 0)
                    if iDot > -1:
                        bool ok
                        isFound = True
                        self.m_KateMajorVer = QString(line.mid(6, iDot - 6)).toInt(&ok, 10)



            qfileTmp.close()
        } # end if could open temp file
        QString sRevisionMajor
        sRevisionMajor.setNum(self.m_KateMajorVer, 10)
        if self.m_Verbose:
            messagebox.showinfo(self, "Output Inspector - Kate Version", isFound ? ("Detected Kate " + sRevisionMajor) : "Could not detect Kate version.")
        if self.m_KateMajorVer > 2:
            self.settings.setValue("xEditorOffset", 0)
            self.settings.setValue("yEditorOffset", 0)

        else:
            self.settings.setValue("xEditorOffset", 0)
            self.settings.setValue("yEditorOffset", 0)
            # NOTE: The values are no longer necessary.
            # self.config.setValue("xEditorOffset", -1)
            # self.config.setValue("yEditorOffset", -1)



    def pushWarnings(self):
        if self.lwiWarnings.length() > 0:
            for it in self.lwiWarnings:
                self._ui.mainListWidget.addItem(*it)

            self.lwiWarnings.clear()



    def getLineInfo(self, line, actualJump, actualJumpLine, isPrevCallPrevLine)
        info = {}
        self.lineInfo(info, line, actualJump, actualJumpLine, isPrevCallPrevLine)
        return info


    def lineInfo(self, info, originalLine, actualJump, actualJumpLine, isPrevCallPrevLine):
        '''
        Sequential arguments:
        info -- a dictionary to modify
        '''
        info["file"] = ""; # same as info.at("file")
        info["row"] = ""
        info["line"] = originalLine
        info["column"] = ""
        info["language"] = ""; # only if language can be detected from self line
        info["good"] = "False"
        info["lower"] = "False"
        info["master"] = "False"
        info["color"] = "Default"
        line = originalLine

        fileToken = ""
        paramAToken = ""
        paramBToken = ""
        endParamsToken = ""
        fileTokenI = -1
        fileI = -1
        paramATokenI = -1
        paramAI = -1
        paramBTokenI = -1
        paramBI = -1
        endParamsTokenI = -1
        QRegExp nonDigitRE("\\D")
        QRegExp nOrZRE("\\d*"); # a digit (\d), or more times (*)
        QRegExp numOrMoreRE("\\d+"); # a digit (\d), or more times (+)
        if self.m_VerboseParsing:
            qInfo().noquote() << "`" + originalLine + "`:"

        for itList in self.enclosures:
            if ((itList[TOKEN_FILE]).length() == 0) or line.contains(itList[TOKEN_FILE]):
                fileToken = itList[TOKEN_FILE]
                if self.m_VerboseParsing:
                    if fileToken.length() > 0:
                        qInfo().noquote() << "  looking for fileToken '" + fileToken + "'"

                paramAToken = itList[TOKEN_PARAM_A]
                paramBToken = itList[TOKEN_PARAM_B]; # coordinate delimiter (blank if no column)
                endParamsToken = itList[TOKEN_END_PARAMS]; # what is after last coord ("\n" if line ends)
                if fileToken.length() != 0:
                    fileTokenI = line.indexOf(fileToken)
                else:
                    fileTokenI = 0; # if file path at begining of line
                if fileTokenI > -1:
                    if self.m_VerboseParsing:
                        qInfo().noquote() << "  has '" + fileToken + "' @ " << fileTokenI
                                          << ">= START"


                    if paramAToken.length() > 0:
                        paramATokenI = line.indexOf(
                                           paramAToken, fileTokenI + fileToken.length()
                                       )
                        if paramATokenI >=0:
                            if not line.mid(paramATokenI+paramAToken.length(), 1).contains(numOrMoreRE):
                                # Don't allow the opener if the next character is
                                # not a digit.
                                paramATokenI = -1



                    elif endParamsToken.length() > 0:
                        paramATokenI = line.indexOf(endParamsToken)
                        if paramATokenI < 0:
                            paramATokenI = line.length()


                    else:
                        paramATokenI = line.length()
                        # paramAToken = "<forced token=\"" + paramAToken.replace("\"", "\\\"") + "\">"

                    if paramATokenI > -1:
                        if self.m_VerboseParsing:
                            qInfo().noquote() << "    has pre-ParamA '" + paramAToken + "' @"
                                              << paramATokenI << " (after " << fileToken.length() << "-long file token at "
                                              << fileTokenI
                                              << " ending at " << (fileTokenI + fileToken.length())
                                              << ")"; # such as ', line '

                        paramAI = paramATokenI + paramAToken.length()
                        if paramBToken.length() > 0:
                            # There should be no B if there is no A, failing
                            # in that case is OK.
                            paramBTokenI = line.indexOf(paramBToken, paramAI)

                        else:
                            paramBTokenI = paramAI
                            # paramBToken = "<forced token=\"" + paramBToken.replace("\"", "\\\"") + "\">"

                        if paramBTokenI > -1:
                            if self.m_VerboseParsing:
                                qInfo().noquote() << "      has pre-ParamB token '" + paramBToken + "' @"
                                                  << paramBTokenI << " at or after ParamA token ending at"
                                                  << (paramATokenI + paramAToken.length())

                            # if paramBToken != itList[TOKEN_PARAM_B]:
                            #    paramBToken = ""; # since may be used to locate next value
                            if paramBToken.length() > 0:
                                paramBI = paramBTokenI + paramBToken.length()
                            else:
                                paramBI = paramBTokenI
                            if endParamsToken.length() == 0:
                                endParamsTokenI = paramBI
                                if self.m_VerboseParsing:
                                    qInfo().noquote() << "  using paramBI for endParamsTokenI: " << paramBI


                            elif endParamsToken != "\n":
                                endParamsTokenI = line.indexOf(endParamsToken, paramBI)

                            else:
                                endParamsTokenI = line.length()
                                # endParamsToken = "<forced token=\"" + endParamsToken.replace("\"", "\\\"").replace("\n", "\\n") + "\">"

                            if endParamsTokenI > -1:
                                if paramBToken.length() == 0:
                                    paramBTokenI = endParamsTokenI; # so paramAI can be calculated correctly if ends at endParamsTokenI
                                    paramBI = endParamsTokenI

                                if itList[PARSE_COLLECT] == COLLECT_REUSE:
                                    info["master"] = "True"
                                if itList[PARSE_STACK] == STACK_LOWER:
                                    info["lower"] = "True"
                                if self.m_VerboseParsing:
                                    qInfo().noquote() << "        has post-params '" + endParamsToken.replace("\n", "\\n") + "' ending@"
                                                      << endParamsTokenI << ">=" << (paramBTokenI + paramBToken.length()) << "="
                                                      << paramBTokenI << "+" << paramBToken.length() << "in '" + line + "'"

                                if endParamsToken != itList[TOKEN_END_PARAMS]:
                                    # since could be used for more stuff after 2 params in future versions,
                                    # length should be 0 if not found but forced:
                                    endParamsToken = ""

                                fileI = fileTokenI + fileToken.length()
                                break

                            elif self.m_VerboseParsing:
                                qInfo().noquote() << "        no post-params '" + endParamsToken + "' >="
                                                  << (paramBTokenI + paramBToken.length()) << "in '" + line + "'"


                        elif self.m_VerboseParsing:
                            qInfo().noquote() << "      no pre-ParamB '" + paramBToken + "' >="
                                              << (paramATokenI + paramAToken.length()) << "in '" + line + "'"


                    elif self.m_VerboseParsing:
                        qInfo().noquote() << "    no pre-paramA '" + paramAToken + "' >="
                                          << (fileTokenI + fileToken.length())


                elif self.m_VerboseParsing:
                    qInfo().noquote() << "  no pre-File '" + fileToken + "' >= START"

        # qInfo().noquote() << "fileTokenI: " << fileTokenI
        # qInfo().noquote() << "paramATokenI: " << paramATokenI
        # qInfo().noquote() << "paramBTokenI: " << paramBTokenI
        # qInfo().noquote() << "endParamsTokenI: " << endParamsTokenI
        # qInfo().noquote() << "fileToken: " << fileToken
        # qInfo().noquote() << "paramAToken: " << paramAToken
        # qInfo().noquote() << "paramBToken: " << paramBToken
        # qInfo().noquote() << "endParamsToken: " << endParamsToken
        if fileI >= 0 and (paramATokenI > fileI or endParamsToken > fileI):
            # Even if closer is not present,
            # endParamsTokenI is set to length() IF applicable to self enclosure

            QString filePath
            if paramATokenI > fileI:
                filePath = line.mid(fileI, paramATokenI - fileI)
            else:
                filePath = line.mid(fileI, endParamsTokenI - fileI)

            filePath = filePath.trimmed()
            if filePath.length() >= 2:
                if (filePath.startsWith('"') and filePath.endsWith('"')) or (filePath.startsWith('\'') and filePath.endsWith('\'')):
                    filePath = filePath.mid(1, filePath.length() - 2)


            qDebug() << "[outputinspector][debug] file path before unmangling: \"" << filePath << "\""
            filePath = unmangledPath(filePath)
            info["file"] = filePath
            info["row"] = line.mid(paramAI, paramBTokenI - paramAI)
            if paramBToken.length() > 0:
                info["column"] = line.mid(paramBI, endParamsTokenI - paramBI)
            else:
                info["column"] = ""
            if (self.m_VerboseParsing) qInfo().noquote() << "        file '" + line.mid(fileI, paramATokenI - fileI) + "'"
            # if (self.m_VerboseParsing) qInfo().noquote() << "        row '" + line.mid(paramAI, paramBTokenI - paramAI) + "'"
            if (self.m_VerboseParsing) qInfo().noquote() << "        row '" + info["row"] + "'"
            if (self.m_VerboseParsing) qInfo().noquote() << "          length " << paramBTokenI << "-" << paramAI
            #if (self.m_VerboseParsing) qInfo().noquote() << "        col '" + line.mid(paramBI, endParamsTokenI - paramBI) + "'"
            if (self.m_VerboseParsing) qInfo().noquote() << "        col '" + info["column"] + "'"
            if (self.m_VerboseParsing) qInfo().noquote() << "          length " << endParamsTokenI << "-" << paramBI

            if filePath.endsWith(".py", Qt.CaseSensitive):
                info["language"] = "python"
            elif filePath.endsWith(".pyw", Qt.CaseSensitive):
                info["language"] = "python"
            elif filePath.endsWith(".cpp", Qt.CaseSensitive):
                info["language"] = "c++"
            elif filePath.endsWith(".h", Qt.CaseSensitive):
                info["language"] = "c++"
            elif filePath.endsWith(".hpp", Qt.CaseSensitive):
                info["language"] = "c++"
            elif filePath.endsWith(".c", Qt.CaseSensitive):
                info["language"] = "c"
            elif filePath.endsWith(".js", Qt.CaseSensitive):
                info["language"] = "js"
            elif filePath.endsWith(".java", Qt.CaseSensitive):
                info["language"] = "java"
            elif filePath.endsWith(".bat", Qt.CaseSensitive):
                info["language"] = "bat"
            elif filePath.endsWith(".sh", Qt.CaseSensitive):
                info["language"] = "sh"
            elif filePath.endsWith(".command", Qt.CaseSensitive):
                info["language"] = "sh"
            elif filePath.endsWith(".php", Qt.CaseSensitive):
                info["language"] = "php"
            qDebug().noquote() << "  detected file: '" + filePath + "'"
            info["good"] = "True"
            # qInfo() << "[outputinspector] found a good line with the following filename: " << filePath

        else:
            info["good"] = "False"
            # qInfo() << "[outputinspector] found a bad line: " << originalLine

        if info["good"] == "True":
            if actualJump.length() > 0 and info["master"] == "False":
                qDebug() << "INFO: nosetests output was detected, the line is not first"
                         << "line of a known multi-line error format, flagging as"
                         << "details (must be a sample line of code or something)."
                info["good"] = "False"; # TODO: possibly eliminate self for fault tolerance
                # (different styles in same output)
                info["details"] = "True"
                info["file"] = ""




    def absPathOrSame(self, filePath):
        absFilePath = ""
        sCwd = os.getcwd()  # current() returns a QDir object
        # Don't use QDir.separator(), self only is detectable on *nix & bsd-like OS:
        cwDir = QDir(sCwd)
        setuptoolsTryPkgPath = QDir.cleanPath(sCwd + QDir.separator() + cwDir.dirName())
        if self.m_Verbose:
            qInfo().noquote() << "setuptoolsTryPkgPath:" << setuptoolsTryPkgPath
        absFilePath = filePath.startsWith("/", Qt.CaseInsensitive) ? filePath : (sCwd + "/" + filePath)
        if not QFile(absFilePath).exists() and QDir(setuptoolsTryPkgPath).exists():
            absFilePath = QDir.cleanPath(setuptoolsTryPkgPath + QDir.separator() + filePath)
        return absFilePath


    def on_mainListWidget_itemDoubleClicked(self, item):
        line = item.text()
        actualJump = item.data(ROLE_COLLECTED_FILE).toString(); # item.toolTip()
        actualJumpLine = item.data(ROLE_COLLECTED_LINE).toString(); # item.toolTip()
        if actualJumpLine.length() > 0:
            line = actualJumpLine
        ok = False
        filePath = (item.data(ROLE_COLLECTED_FILE)).toString()
        absFilePath = filePath
        QString errorMsg
        if filePath.length() > 0:
            if self.m_Verbose:
                qInfo().noquote() << "clicked_file: '" + filePath + "'"
                qInfo().noquote() << "tooltip: '" + item.toolTip() + "'"

            absFilePath = self.absPathOrSame(filePath)
            citedRowS = (item.data(ROLE_ROW)).toString()
            citedColS = (item.data(ROLE_COL)).toString()
            if self.m_Verbose:
                qInfo().noquote() << "citedRowS: '" + citedRowS + "'"
                qInfo().noquote() << "citedColS: '" + citedColS + "'"

            citedRow = citedRowS.toInt(&ok, 10)
            citedCol = citedColS.toInt(&ok, 10)
            xEditorOffset = self.settings.getInt("xEditorOffset")
            yEditorOffset = self.settings.getInt("yEditorOffset")
            # region only for Kate <= 2
            citedRow += yEditorOffset
            citedRowS.setNum(citedRow, 10)
            citedCol += xEditorOffset
            citedColS.setNum(citedCol, 10)
            # endregion only for Kate <= 2
            if self.m_CompensateForKateTabDifferences:
                QFile qfileSource(absFilePath)
                QString line
                readCitedI = 0; '''*< This is the current line number while the
                                         loop reads the entire cited file. '''
                if (qfileSource.open(QFile.ReadOnly))   #| QFile.Translate
                    QTextStream qtextNow(&qfileSource)
                    while (not qtextNow.atEnd())
                        line = qtextNow.readLine(0); #does trim off newline characters
                        if readCitedI == ((citedRow - yEditorOffset) - 1):
                            tabCount = 0
                            # TODO: Use regex for finding the tab.
                            for tryTabI in range(len(line)):
                                if line.mid(tryTabI, 1) == "\t":
                                    tabCount += 1
                                else:
                                    break

                            QString tabDebugMsg
                            if tabCount > 0:
                                tabDebugMsg.setNum(tabCount, 10)
                                tabDebugMsg = "tabs:" + tabDebugMsg
                                # if subtracted 1 for kate 2, 1st character after a line with 1 tab is currently citedCol==6, it is 7
                                # if subtracted 1 for kate 2, 2nd character after a line with 1 tab is currently citedCol==7, it is 8
                                # if subtracted 1 for kate 2, 1st character after a line with 2tabs is currently citedCol==12, it is 13
                                # if subtracted 1 for kate 2, 2nd character after a line with 2tabs is currently citedCol==13, it is 14
                                if self.m_KateMajorVer < 3:
                                    citedCol -= xEditorOffset
                                tabDebugMsg += "; citedColS-old:" + citedColS
                                citedCol -= tabCount * (self.settings.getInt("CompilerTabWidth") - 1)
                                #citedCol+=xEditorOffset
                                citedColS.setNum(citedCol, 10)
                                tabDebugMsg += "; citedCol-abs:" + citedColS
                                # if above worked, citedCol is now an absolute character (counting tabs as 1 character)
                                # if subtracted 1 for kate 2, 1st character after a line with 1 tab has now become citedCol==1, it is 2 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                                # if subtracted 1 for kate 2, 2nd character after a line with 1 tab has now become citedCol==2, it is 3 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                                # if subtracted 1 for kate 2, 1st character after a line with 2tabs has now become citedCol==2, it is 3 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                                # if subtracted 1 for kate 2, 2nd character after a line with 2tabs has now become citedCol==3, it is 4 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                                if self.m_KateMajorVer < 3:
                                    # Kate 2.5.9 reads a 'c' argument value of 0 as the beginning of the line and 1 as the first character after the leading tabs
                                    if citedCol < tabCount:
                                        citedCol = 0
                                    else:
                                        # citedCol currently starts at 1 at the beginning of the line
                                        citedCol -= (tabCount)
                                        citedColS.setNum(citedCol, 10)
                                        tabDebugMsg += "; citedCol-StartAt1-rel-to-nontab:" + citedColS
                                        # citedCol now starts at 1 starting from the first text after tabs
                                        regeneratedCol = 1
                                        tabDebugMsg += "; skips:"
                                        # This approximates how Kate 2 traverses tabs (the 'c' argument actually can't reach certain positions directly after the tabs):
                                        if tabCount > 2:
                                            citedCol += tabCount - 2
                                        for tryTabI in range(citedCol):
                                            if tryTabI <= (tabCount - 1) * self.settings.getInt("Kate2TabWidth") + 1:
                                                if tryTabI != 1 and (tryTabI - 1) % self.settings.getInt("Kate2TabWidth") == 0:
                                                    regeneratedCol++; # only add if it is 4,7,10, where addend is self.config.getInt("Kate2TabWidth") (1+self.config.getInt("Kate2TabWidth")*position)
                                                    tabDebugMsg += "-"


                                            else:
                                                regeneratedCol += 1


                                        citedCol = regeneratedCol; # +( (tabCount>3andtabCount<6) ? tabCount : 0 )
                                        # end accounting for kate gibberish column translation


                                # else kate 3+, handles tabs as absolute positions
                                citedColS.setNum(citedCol, 10)
                                tabDebugMsg += "; citedColS-new:" + citedColS
                                if self.m_EnableTabDebugMsg:
                                    messagebox.showinfo(self, "Output Inspector - Debug tab compensation", tabDebugMsg)
                            } # end if tabCount>0
                            break
                        } # if correct line found
                        readCitedI += 1
                    } # while not at end of source file
                    qfileSource.close()
                } # end if can open source file
                else:
                    errorMsg = "Specified file '" + filePath + "' does not exist or is not accessible (if path looks right, running from the location where it exists instead of '" + os.getcwd() + "')"

            } # end if self.m_CompensateForKateTabDifferences
            # QString sArgs="-u "+absFilePath+" -l "+citedRowS+" -c "+citedColS
            # QProcess qprocNow(self.config.getString("editor")+sArgs)
            # qprocNow
            if QFile(absFilePath).exists():
                commandMsg = self.settings.getString("editor")
                QStringList qslistArgs
                # NOTE: -u is not needed at least as of kate 16.12.3 which does not create additional
                # instances of kate
                # qslistArgs.append("-u")
                # commandMsg+=" -u"
                # qslistArgs.append("\""+absFilePath+"\"")
                qslistArgs.append(absFilePath)
                commandMsg += " " + absFilePath
                qslistArgs.append("--line"); # split into separate arg, geany complains that
                # it doesn't understand the arg "--line 1"
                qslistArgs.append(citedRowS)
                commandMsg += " --line " + citedRowS
                # qslistArgs.append(citedRowS)
                qslistArgs.append("--column"); # NOTE: -c is column in kate, alternate config dir
                # in geany, use --column
                qslistArgs.append(citedColS); # NOTE: -c is column in kate, alternate config dir
                # in geany, use --column
                commandMsg += " --column " + citedColS
                # qslistArgs.append(citedColS)
                # qWarning().noquote() << "qslistArgs: " << qslistArgs
                QProcess.startDetached( self.settings.getString("editor"), qslistArgs)
                if not QFile.exists(self.settings.getString("editor")):
                    # ok to run anyway for fault tolerance, may be in system path
                    messagebox.showinfo(self, "Output Inspector - Configuration", self.settings.getString("editor") + " cannot be accessed.  Try setting the value editor = in " + self.settings.fileName())

                # if self.m_Verbose:
                self._ui.statusBar.showMessage(commandMsg, 0)
                # system(sCmd);#stdlib
                # messagebox.showinfo(self,"test",sCmd)

            else:
                # errorMsg = "Specified file '" + absFilePath + "' does not exist (try a different line, try running from the location where it exists instead of '" + os.getcwd() + "')"
                errorMsg = "[Output Inspector] No file exists here: '" + absFilePath + "'\n"

        } # end if line is in proper format
        else:
            errorMsg = "Could not detect error format\n"
        if errorMsg.length() > 0:
            if filePath.length() > 0:
                # qWarning().noquote() << errorMsg << " in '" + line + "':"
                info = self.getLineInfo(line, actualJump, actualJumpLine, False)
                QString infoS
                for k, v in info.items():
                    # qWarning().noquote() << "    " << it.first # key
                    #         + ": '" + it.second + "'"; #value
                    infoS += "; " + k + ": '" + v + "'"

                warn("[outputinspector][error] " + errorMsg + " in the line:"
                     + "; actualJump: " + item.data(self.ROLE_COLLECTED_FILE).toString()
                     + "  actualJumpLine: " + item.data(self.ROLE_COLLECTED_LINE).toString()
                     + infoS)
                #    + "  info:"

                messagebox.showinfo(self, "Output Inspector", errorMsg)
                # messagebox.showinfo(self,"Output Inspector","'"+absFilePath+"' cannot be accessed (try a different line, if self line's path looks right, running from the location where it exists instead of '"+os.getcwd()+"')")
                # or pasting the entire line to 'Issues' link on web-based git repository
            else:
                qInfo().noquote() << "[Output Inspector] No file was detected in line: '" + line + "'"
                qInfo().noquote() << "[Output Inspector] ERROR: '" + errorMsg + "'"

    def readInput(self):
        limit = 50
        count = 0
        line = " "
        while (count < limit and not line.empty())
            size = std.cin.rdbuf().in_avail()
            if size < 1:
                # qInfo().noquote() << "OutputInspector: There is no input: got " << size
                # Prevent waiting forever for a line.
                break

            std.getline(std.cin, line)
            if not std.cin.eof():
                # qInfo().noquote() << "OutputInspector: input is '" << line.c_str() << "'."
                # self.addLine(QString("OutputInspector: input is: ") + line, True)
                self.addLine(line, True)

            else:
                # qInfo().noquote() << "OutputInspector: input has ended."
                # self.addLine("# OutputInspector: input has ended.", True)
                break

            count += 1
