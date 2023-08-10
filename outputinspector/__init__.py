#!/usr/bin/env python3
from __future__ import print_function
import sys
import os
import platform
import time
import re
from subprocess import Popen, PIPE

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(MODULE_DIR)
my_path = os.path.realpath(__file__)
if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)

from outputinspector.noqttk import (
    QListWidgetItem,
    QVariant,
    QBrush,
    QColor,
    Qt,
    QTimer,
    connect,
    # ALL of these have to be redefined in become_non_gui
    #   if Tk is not initialized.
)
QMainWindow = None  # Only necessary for subclasses.
verbosity = 2
max_verbosity = 2
TMP = "/tmp"
if platform.system() == "Windows":
    TMP = os.environ['TEMP']

def warn(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def echo0(*args, **kwargs):  # formerly error
    print(*args, file=sys.stderr, **kwargs)
    return True


def echo1(*args, **kwargs):  # formerly debug
    '''
    Only show the message if verbosity > 1 (See "set_verbosity").
    '''
    if verbosity > 1:
        print(*args, file=sys.stderr, **kwargs)


def echo2(*args, **kwargs):
    if verbosity < 2:
        return False
    print(*args, file=sys.stderr, **kwargs)
    return True


def critical(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def fatal(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def pinfo(*args, **kwargs):
    '''
    Print info.
    '''
    print(*args, file=sys.stderr, **kwargs)


def set_verbosity(v):
    '''
    Set verbosity to 1 for verbose messages and 2 for debug messages.
    '''
    verbosities = [True, False] + list(range(max_verbosity+1))
    if v not in verbosities:
        raise ValueError(
            "{} is not valid. Verbosity should be one of: {}."
            "".format(v, verbosities)
        )
    verbosity = v


from outputinspector.settings import Settings

profile = None
AppsData = None
if platform.system() == "Windows":
    profile = os.environ['USERPROFILE']
    AppsData = os.environ['APPDATA']
elif platform.system() == "Darwin":
    profile = os.environ['HOME']
    AppsData = os.path.join(profile, "Library", "Preferences")
else:
    # "Linux" or other
    profile = os.environ['HOME']
    AppsData = os.path.join(profile, ".config")

myAppData = os.path.join(AppsData, "outputinspector")


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
'''*< linedef[TOKEN_FILE] is the opener for the file path (blank if
the file path starts at the begginning of the line). '''
TOKEN_PARAM_A = 1
'''*< linedef[TOKEN_PARAM_A] is the first coordinate token (blank
if none, as grep-- -n is automatically added if you use the included
ogrep script). '''
TOKEN_PARAM_B = 2
'''*< linedef[TOKEN_PARAM_B] is the second coordinate delimiter
(blank if no column). '''
TOKEN_END_PARAMS = 3
'''*< linedef[TOKEN_END_PARAMS] ParamsEnder (what is after last
coord). '''
PARSE_COLLECT = 4
'''*< linedef[PARSE_COLLECT] determines the mode for connecting
lines. For possible values and their behaviors, the documentation for
COLLECT_REUSE (or future COLLECT_* constants). '''
PARSE_STACK = 5
'''*< linedef[PARSE_STACK] flags a pattern as being for a
callstack, such as to connect it to a previous error (see documentation
for STACK_LOWER or for any later-added STACK_* constants). '''
PARSE_DESCRIPTION = 5
'''*< linedef[PARSE_STACK] describes the parser mode (linedef) in a
human-readable way.'''
PARSE_PARTS_COUNT = 7
UserRole = 1
ROLE_COLLECTED_FILE = UserRole
ROLE_ROW = UserRole + 1
ROLE_COL = UserRole + 2
ROLE_LOWER = UserRole + 3
ROLE_COLLECTED_LINE = UserRole + 4
ROLE_DETAILS = UserRole + 5
QMainWindow = None
def become_non_gui():
    prefix = "[outputinspector] "
    echo0(prefix+"Switching to non-gui mode."
          " Make a subclass that inherits OutputInspector and tk.Frame"
          " to use a GUI.")
    global QListWidgetItem
    global QVariant
    global QBrush
    global QColor
    global Qt
    global QTimer
    global connect
    import outputinspector.noqt as noqt
    QListWidgetItem = noqt.QListWidgetItem
    QVariant = noqt.QVariant
    QBrush = noqt.QBrush
    QColor = noqt.QColor
    Qt = noqt.Qt
    QTimer = noqt.QTimer
    connect = noqt.connect
    global QMainWindow
    QMainWindow = noqt.QMainWindow


class OutputInspector:
    """Parse output from various compilers and linters.
    
    This class has some methods that are redundant to noqt and noqttk.
    To allow either to be used, this class must behave like Widget but
    avoid inheriting it so this class works as a secondary superclass
    where primary superclass is either MainWindow which is already a
    widget, usually a notqtk widget that is not CLI-compatible like
    OutputInspector is if used directly instead of as a superclass.
    """

    def addChildWidget(self, widget):
        if not hasattr(self, '_children'):
            # Try to allow use while the class is still incomplete
            #   (before __init__ is finished)
            self._children = []
        self._children.append(widget)

    def addChildLayout(self, layout):
        if not hasattr(self, '_layouts'):
            # Try to allow use while the class is still incomplete
            #   (before __init__ is finished)
            self._layouts = []
        self._layouts.append(layout)

    def __init__(self):
        prefix = "[OutputInspector] "
        self._ui = None
        # self.name = "inspector"
        # leave out name to prevent _ui_subtree from constructing one
        def parentWidget(self):
            class UhOh:
                pass
            # Behave as a QWidget, but only to this extent.
            #   This must be done here since the ui loader
            #   will check for this during this constructor and
            #   the class is still incomplete.
            return UhOh()
        setattr(self, 'parentWidget', parentWidget)
        echo0("* outputinspector init...")
        if type(self).__name__ == "OutputInspector":
            echo0(prefix+"Switching to non-gui operation.")
            become_non_gui()
            # Also, run QMainWindow.__init__ later
            #   since this is not a subclass.
            #   *see bottom* of __init__.
        
        # public:
        # static QString unmangledPath(QString path)

        self.m_DebugBadHints = True
        self.sInternalFlags = []
        self.sSectionBreakFlags = []
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
        self.lineCount = 0  # TODO: eliminate this?
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

        # void pushWarnings();'''*< Push warnings to the GUI. '''

        self.inTimer = None  # *< Read standard input lines regularly

        self.iErrors = 0
        self.iWarnings = 0
        self.iTODOs = 0
        self.m_Files = []

        #endif # MAINWINDOW_H

        # : QMainWindow(parent)
        # , self._ui(new Ui.MainWindow)
        # self._ui.setupUi(self)
        # configDir = QStandardPaths.StandardLocation(QStandardPaths.ConfigLocation())
        # QStandardPaths.StandardLocation(QStandardPaths.HomeLocation)
        # ^ same as QDir.homePath()
        # See <https:#stackoverflow.com/questions/32525196/
        #   how-to-get-a-settings-storage-path-in-a-cross-platform-way-in-qt>
        filePath = os.path.join(myAppData, "settings.txt")
        # TODO: fill self in or remove it (and the comments).
        # if os.path.isfile(filePath):
        if not os.path.isfile(filePath):
            pass
            # if f.open(QIODevice.WriteOnly | QIODevice.Truncate):
            #     f.write("Hello, World")
        else:
            pass

        echo0("* outputinspector init...settings...")
        # pinfo("Creating settings: {}".format(filePath))
        self.settings = Settings(filePath)
        pinfo("[outputinspector] used the settings file \"{}\""
              "".format(self.settings.fileName()))
        # if os.path.isfile("/etc/outputinspector.conf"):
        #     self.config = Settings("/etc/outputinspector.conf")
        # elif os.path.isfile("/etc/outputinspector.conf"):
        #     self.config = Settings("/etc/outputinspector.conf")

        self.settings.setIfMissing("Kate2TabWidth", 8)
        self.settings.setIfMissing("CompilerTabWidth", 6)
        self.settings.setIfMissing("ShowWarningsLast", False)
        # TODO: Implement ShowWarningsLast (but ignore it and behave as if it were
        # False if there is anything in stdin).
        self.settings.setIfMissing("FindTODOs", True)
        if self.settings.contains("kate"):
            changed = self.settings.setIfMissing("editor", self.settings.getString("kate"))
            self.settings.remove("kate")
            self.settings.sync()
            if changed:
                pinfo(
                    "[outputinspector] transferred the old setting"
                    " 'kate={}' to 'editor={}'."
                    "".format(self.settings.getString("kate"),
                              self.settings.getString("editor"))
                )
            else:
                pinfo(
                    "[outputinspector] ignored the deprecated setting"
                    " 'kate={}' in favor of 'editor={}'."
                    "".format(self.settings.getString("kate"),
                              self.settings.getString("editor"))
                )

        self.settings.setIfMissing("editor", "/usr/bin/geany")
        # echo0("* outputinspector init...done")



        # def init(self, errorsListFileName):
        '''
        formats with "\n" at end must be AFTER other single-param formats that have
        same TOKEN_FILE and PARSE_PARAM_A, because "\n" is forced
        (which would leave extra stuff at the end if there are more tokenings)
        '''
        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = "  File "
        linedef[TOKEN_PARAM_A] = ", line "
        linedef[TOKEN_PARAM_B] = ")"
        linedef[TOKEN_END_PARAMS] = ""
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Nose error"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = "  File "
        linedef[TOKEN_PARAM_A] = ", line "
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = ","
        linedef[PARSE_COLLECT] = COLLECT_REUSE
        linedef[PARSE_STACK] = STACK_LOWER
        linedef[PARSE_DESCRIPTION] = "Nose lower traceback"
        self.enclosures.append(linedef)

        linedef = []
        for i in range(PARSE_PARTS_COUNT):
            linedef.append("")
        linedef[TOKEN_FILE] = "ERROR: Failure: SyntaxError (invalid syntax ("
        linedef[TOKEN_PARAM_A] = ", line "
        linedef[TOKEN_PARAM_B] = ""
        linedef[TOKEN_END_PARAMS] = ")"
        linedef[PARSE_COLLECT] = ""
        linedef[PARSE_STACK] = ""
        linedef[PARSE_DESCRIPTION] = "Nose syntax error"
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
        self.brushes["Regular"] = QBrush(Qt.black)

        echo0("* initialized brushes")

        self.sInternalFlags.append("/site-packages/")
        internalS = "/usr/lib/python2.7/site-packages/nose/importer.py"
        assert(contains_any(internalS, self.sInternalFlags))

        self.sSectionBreakFlags.append("--------")
        breakS = "---------------------"
        assert(contains_any(breakS, self.sSectionBreakFlags))

        echo1("debug stream is active.")
        # pinfo("pinfo stream is active.")
        # warn("warn stream is active.")
        # critical("critical stream is active."
        # fatal("fatal stream is active."
        if self.m_Verbose:
            pinfo("lists:")
        # self for loop has the brace on the next line (for clang-format test):
        for itList in self.enclosures:
            # pinfo("  -")
            # for i in range(len(itList)):
            #     pinfo("    - {}".format(itList[i]))
            #
            assert(len(itList) >= PARSE_PARTS_COUNT)
            # pinfo("  items.size(): {}".format(itList.size()))
            if self.m_Verbose:
                pinfo("  items: ['" + "', '".join(itList) + "']")
        if type(self).__name__ == "OutputInspector":
            QMainWindow.__init__(
                self,
                ui_file=os.path.join(REPO_DIR, "mainwindow.ui")
            )
            # ^ set self._ui etc.
        return

    @staticmethod
    def unmangledPath(path):
        """Replace elipsis (3 or more dots) with the missing parts
        if a similar file can be found.
        """
        # a.k.a. remove_elipsis
        # QRegularExpression literalDotsRE("\\.\\.\\.+") '''*< Match 2 dots followed by more. '''
        # FIXME: make below act like above
        literalDotsRE = re.compile("\\.\\.\\.+")  #*< Match 2 dots followed by more.
        match = literalDotsRE.match(path)
        # TODO: capturedEnd technically uses the last element in:
        '''
        matches = list(match.finditer())
        if matches is not None:
            match = matches[-1]
        '''
        verbose = False  # This is manually set to True for debug only.
        if match is not None:
            # start = match.capturedStart(0)
            # echo0("match={}".format(match))
            end = match.end(0)
            tryOffsets = []
            tryOffsets.append(-2)
            tryOffsets.append(1)
            tryOffsets.append(0)
            for tryOffset in tryOffsets:
                tryPath = path[end+tryOffset:]
                if os.path.isfile(tryPath):
                    # TODO: ^ originally exists(). Is isfile always ok?
                    if verbose:
                        pinfo("[outputinspector] transformed *.../dir"
                              " into ../dir: \"{}\""
                              "".format(tryPath))

                    return tryPath

                else:
                    if verbose:
                        pinfo("[outputinspector] There is no \"{}\""
                              " in the current directory (\"{}\")"
                              "".format(tryPath, os.getcwd()))
        else:
            if (verbose):
                pinfo("[outputinspector] There is no \"...\""
                      " in the cited path: \"{}\""
                      "".format(path))

        return path

    def showinfo(self, title, msg):
        '''
        Override this if your subclass provides a GUI.
        '''
        pinfo("[{}] {}".format(title, msg))

    def showerror(self, title, msg):
        '''
        Override this if your subclass provides a GUI.
        '''
        echo0("[{}] {}".format(title, msg))

    def isFatalSourceError(self, line):
        '''
        @brief Check whether your parser reports that your code has a fatal error.
        @param line stderr output from your parser
        @return
        '''
        return (
            ("Can't open" in line)  # jshint couldn't find a source file
            or ("Too many errors" in line) # jshint already showed the error for self line, can't display more errors
            or ("could not be found" in line) # mcs couldn't find a source file
            or ("compilation failed" in line) # mcs couldn't compile the sources
        )

    def __del__(self):
        del self.inTimer
        del self.settings
        # del self._ui

    def init(self, errorsListFileName):
        if not (self.settings.contains("xEditorOffset") or self.settings.contains("yEditorOffset")):
            self.CompensateForEditorVersion()
        output_names = ["err.txt", "out.txt"]
        if (errorsListFileName is None) or (len(errorsListFileName) == 0):
            echo0("* detecting any of {}...".format(output_names))
            tryPath = "debug.txt"
            if os.path.isfile(tryPath):
                errorsListFileName = tryPath
                pinfo("[outputinspector] detected \"{}\"...examining..."
                      "".format(tryPath))

            else:
                errorsListFileName = "err.txt"
        self.lineCount = 0
        self.load_stdin_or_file(errorsListFileName)
        self.inTimer = QTimer(self)
        self.inTimer.setInterval(500);  # milliseconds
        connect(self.inTimer, self.inTimer.timeout, self, self.readInput)
        self.inTimer.start()
        # end init

    def has_stdin(self):
        return False
        # TODO: finish this (return True and await lines if any stdin)
        '''
        lineCount = 0
        if std.cin.rdbuf().in_avail() > 1:
            # TODO: fix self--self never happens (Issue #16)
            pinfo("[outputinspector] detected standard input (such as"
                  " from a console pipe)...skipping \"{}\"..."
                  "".format(errorsListFileName))
        '''

    def load_stdin_or_file(self, errorsListFileName):
        self.lineCount = 0
        # QFile qfileTest(errorsListFileName)
        # self.m_ToDoFlags.append("TODO")
        # self.m_ToDoFlags.append("FIXME")
        # cutToDoCount = 2
        # self._ui.mainListWidget is a QListWidget
        # setCentralWidget(self._ui.mainListWidget)
        # self._ui.mainListWidget.setSizePolicy(QSizePolicy.)
        # OutputInspectorSleepThread.msleep(150); # wait for stdin (doesn't work)
        if self.has_stdin():
            return True
        if not os.path.isfile(errorsListFileName):
            # if std.cin.rdbuf().in_avail() < 1:
            title = "Output Inspector - Help"
            msg = my_path + ": Output Inspector cannot read the output file due to permissions or other read error (tried \"./" + errorsListFileName + "\")."
            self.showinfo(title, msg)
            # self.addLine(title + ":" + msg, True)

        echo0('* reading "{}"'.format(errorsListFileName))
        with open(errorsListFileName, 'r') as qtextNow:
            for rawL in qtextNow:
                line = rawL.rstrip()
                self.addLine(line, False)
            # end while not at end of file named errorsListFileName
            self.processAllAddedLines()

        # end if could open file named errorsListFileName
        # else:
        #     if std.cin.rdbuf().in_avail() < 1:
        #         my_path = QCoreApplication.applicationFilePath()
        #         title = "Output Inspector - Help"
        #         msg = my_path + ": Output Inspector cannot find the output file to process (tried \"./" + errorsListFileName + "\")."
        #         self.showinfo(title, msg)
        #         # self.addLine(title + ":" + msg, True)

    def processAllAddedLines(self):
        sNumErrors = str(self.iErrors)
        sNumWarnings = str(self.iWarnings)
        sNumTODOs = str(self.iTODOs)
        self.pushWarnings()
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
                pinfo("Closing since no errors...")
                # QCoreApplication.quit(); # doesn't work
                # aptr.exit(); # doesn't work (QApplication*)
                # aptr.quit(); # doesn't work
                # aptr.closeAllWindows(); # doesn't work
                # if the event loop is not running, function (QCoreApplication.exit()) does nothing
                sys.exit(1)

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
        self.lineCount += 1  # TODO: eliminate this?

        self.m_LineCount += 1
        originalLine = line
        self.m_MasterLine = line
        info = {}  # values are strings
        if len(line) > 0:
            if len(line.strip()) > 0:
                self.m_NonBlankLineCount += 1
            if self.isFatalSourceError(line):
                self._ui.mainListWidget.addItem(QListWidgetItem(line + " <your compiler (or other tool) recorded self fatal or summary error before outputinspector ran>", self._ui.mainListWidget))

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

                if info["master"] == "True":
                    self.m_ActualJump = info["file"]
                    self.m_ActualJumpLine = line
                    self.m_ActualJumpRow = info["row"]
                    self.m_ActualJumpColumn = info["column"]
                    self.m_IsJumpLower = (info["lower"] == "True")
                    echo1("(master) set actualJump to '{}'"
                          "".format(self.m_ActualJump))
                else:
                    echo1("(not master) line: '{}'"
                          "".format(line))

                isWarning = False
                sColorPrefix = "Error"
                if len(self.m_ActualJump) > 0:
                    self.m_MasterLine = self.m_ActualJumpLine

                if self.m_Warning in self.m_MasterLine:
                    isWarning = True
                    sColorPrefix = "Warning"

                # do not specify self._ui.mainListWidget on new, will be added automatically
                lwi = QListWidgetItem(line)
                if len(self.m_ActualJumpRow) > 0:
                    lwi.setData(ROLE_ROW, QVariant(self.m_ActualJumpRow))
                    lwi.setData(ROLE_COL, QVariant(self.m_ActualJumpColumn))
                else:
                    lwi.setData(ROLE_ROW, QVariant(info["row"]))
                    lwi.setData(ROLE_COL, QVariant(info["column"]))

                if len(self.m_ActualJump) > 0:
                    lwi.setData(ROLE_COLLECTED_FILE, QVariant(self.m_ActualJump))
                    if info["lower"] == "True":
                        lwi.setForeground(self.brushes["TracebackNotTop"])
                    elif info["good"] == "True":
                        lwi.setForeground(self.brushes[sColorPrefix])
                    else:
                        lwi.setForeground(self.brushes[sColorPrefix + "Details"])
                else:
                    lwi.setData(ROLE_COLLECTED_FILE, QVariant(info["file"]))
                    if info["good"] == "True":
                        lwi.setForeground(self.brushes[sColorPrefix])
                    else:
                        lwi.setForeground(self.brushes["Unusable"])

                if contains_any(self.m_MasterLine, self.sInternalFlags):
                    lwi.setForeground(self.brushes["Internal"])

                lwi.setData(ROLE_COLLECTED_LINE, QVariant(self.m_MasterLine))
                lwi.setData(ROLE_DETAILS, QVariant(line != self.m_MasterLine))
                lwi.setData(ROLE_LOWER, QVariant(info["lower"]))
                if info["good"] == "True":
                    if isWarning:
                        self.iWarnings += 1
                    else:
                        self.iErrors += 1

                if self.settings.getBool("ShowWarningsLast") and isWarning:
                    self.lwiWarnings.append(lwi)
                else:
                    self._ui.mainListWidget.addItem(lwi)

                sTargetLanguage = info["language"]

                if len(sTargetLanguage) > 0:
                    if sTargetLanguage == "python" or sTargetLanguage == "sh":
                        self.m_CommentToken = "#"
                    elif sTargetLanguage in ["c++", "c", "php", "js", "java"]:
                        self.m_CommentToken = "#"
                    elif sTargetLanguage == "bat":
                        self.m_CommentToken = "rem "

                # if ((is_jshint and info["file"].endswith(".js"))
                #         or self.m_Error in line):
                #  # ^ TODO?:
                #  #   if (is_jshint or "previous error" not in line):
                #  #       self.iErrors += 1
                #    # if self.config.getBool("ShowWarningsLast")):
                #    #     self.m_Errors.append(line:
                #

                if self.settings.getBool("FindTODOs"):
                    if info["good"] == "True":
                        sFileX = ""  # = unmangledPath(info["file"])
                        sFileX = absPathOrSame(sFileX)
                        # =line[0:line.find("(")]
                        if sFileX not in self.m_Files:
                            self.m_Files.append(sFileX)
                            if self.m_Verbose:
                                echo1("outputinspector trying to open"
                                      " '{}'...".format(sFileX))
                            # if not qfileSource.open(QFile.ReadOnly):
                            if not os.path.isfile(sFileX):
                                warn("[outputinspector] did not scan a"
                                     " file that is cited by the log"
                                     " but that is not present: '{}'"
                                     "".format(sFileX))
                                return
                            with open(sFileX, 'r') as qtextSource:
                                iSourceLineFindToDo = 0
                                for rawL in qtextSource:
                                    sSourceLine = rawL.rstrip()
                                    iSourceLineFindToDo += 1
                                    # ^ Increment self now since the
                                    #   compiler's line numbering starts
                                    #   with 1.
                                    iToDoFound = -1
                                    iCommentFound = sSourceLine.find(self.m_CommentToken, 0)
                                    if iCommentFound > -1:
                                        for i in range(len(self.m_ToDoFlags)):
                                            iToDoFound = sSourceLine.find(self.m_ToDoFlags[i], iCommentFound + 1)
                                            if iToDoFound > -1:
                                                break

                                    if iToDoFound > -1:
                                        sNumLine = str(iSourceLineFindToDo)
                                        processedCol = iToDoFound
                                        for citedI in range(len(sSourceLine)):
                                            if sSourceLine[citedI:citedI+1] == "\t":
                                                processedCol += (self.settings.getInt("CompilerTabWidth") - 1)
                                            else:
                                                break

                                        processedCol += 1
                                        # ^ start numbering at 1 to
                                        #   mimic compiler
                                        processedCol += 2
                                        # ^ +2 to start after slashes
                                        sNumPos = str(processedCol)
                                        sLineToDo = sFileX + "(" + sNumLine + "," + sNumPos + ") " + sSourceLine[iToDoFound:]
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

                                    # end while not at end of source file
                                if self.m_Verbose:
                                    echo1("outputinspector finished"
                                          " reading sourcecode")
                                if self.m_Verbose:
                                    echo1(
                                        "(processed {} line(s))"
                                        "".format(iSourceLineFindToDo)
                                    )
                                qfileSource.close()
                                # end with open sourcecode

                            # end if list does not already contain self
                            # file
                        # end if found filename ender
                    elif self.m_Verbose:
                        echo1("[outputinspector] WARNING:"
                              " filename ender in {}"
                              "".format(line))
                    # end if getIniBool("FindTODOs")
                else:
                    echo1("[outputinspector] [debug]"
                          " getIniBool(\"FindTODOs\") off"
                          " so parsing has been skipped in: {}"
                          "".format(line))
                # end if a regular line (not fatal, formatting)
            # end if length>0 (after trim using 0 option for readLine)
        if enablePush:
            self.pushWarnings()

    def CompensateForEditorVersion(self):
        isFound = False
        sVersionArgs = []
        sFileTemp = os.path.join(TMP, "outputinspector.using.kate.version.tmp")
        sVersionArgs.append("--version")
        sVersionArgs.append(" > " + sFileTemp)
        # QProcess.execute(IniString("editor"), sVersionArgs)
        os.system(self.settings.getString("editor") + " --version > "
                  + sFileTemp)
        # OutputInspectorSleepThread.msleep(125)
        time.sleep(.125)

        line = None
        with open(sFileTemp, 'r') as qtextNow:  # as qfileTmp
            # detect Kate version using output of Kate command above
            sKateOpener = "Kate: "
            for rawL in qtextNow:
                line = rawL.rstrip()
                if self.m_Verbose:
                    self.showinfo(
                        "Output Inspector - Finding Kate version...",
                        line
                    )
                if line.startswith(sKateOpener, Qt.CaseInsensitive):
                    iDot = line.find(".")
                    if iDot > -1:
                        isFound = True
                        self.m_KateMajorVer = int(line[6:iDot])
            # end if could open temp file
        sRevisionMajor = str(self.m_KateMajorVer)
        if self.m_Verbose:
            vMsg = "Could not detect Kate version."
            if isFound:
                vMsg = "Detected Kate " + sRevisionMajor
            self.showinfo("Output Inspector - Kate Version", vMsg)
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
        if len(self.lwiWarnings) > 0:
            for it in self.lwiWarnings:
                self._ui.mainListWidget.addItem(it)

            del self.lwiWarnings[:]

    def getLineInfo(self, line, actualJump, actualJumpLine,
                    isPrevCallPrevLine):
        info = {}
        self.lineInfo(info, line, actualJump, actualJumpLine,
                      isPrevCallPrevLine)
        return info

    def lineInfo(self, info, originalLine, actualJump, actualJumpLine,
                 isPrevCallPrevLine):
        '''
        Sequential arguments:
        info -- a dictionary to modify
        '''
        info["file"] = ""
        # ^ same as info["file"]
        info["row"] = ""
        info["line"] = originalLine
        info["column"] = ""
        info["language"] = ""
        # ^ only if language can be detected from self line
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
        # nonDigitRE = re.compile("\\D")
        # nOrZRE = re.compile("\\d*")
        # ^ a digit (\d), or more times (*)
        numOrMoreRE = re.compile("\\d+")
        # ^ a digit (\d), or more times (+)
        if self.m_VerboseParsing:
            pinfo("`{}`:".format(originalLine))

        for itList in self.enclosures:
            if ((len(itList[TOKEN_FILE]) == 0)
                    or (itList[TOKEN_FILE] in line)):
                fileToken = itList[TOKEN_FILE]
                if self.m_VerboseParsing:
                    if len(fileToken) > 0:
                        pinfo("  looking for fileToken '{}'"
                              "".format(fileToken))

                paramAToken = itList[TOKEN_PARAM_A]
                paramBToken = itList[TOKEN_PARAM_B]
                # ^ coordinate delimiter (blank if no column)
                endParamsToken = itList[TOKEN_END_PARAMS]
                # ^ what is after last coord ("\n" if line ends)
                if len(fileToken) != 0:
                    fileTokenI = line.find(fileToken)
                else:
                    fileTokenI = 0; # if file path at begining of line
                if fileTokenI > -1:
                    if self.m_VerboseParsing:
                        pinfo("  has '{}' @ {}>= START"
                              "".format(fileToken, fileTokenI))

                    if len(paramAToken) > 0:
                        paramATokenI = line.find(
                            paramAToken,
                            fileTokenI + len(fileToken),
                        )
                        if paramATokenI >= 0:
                            # if not numOrMoreRE.match(line[paramATokenI+len(paramAToken)]):
                            if not numOrMoreRE.match(line[paramATokenI]):
                                # Don't allow the opener if the next
                                # character is not a digit.
                                # - `not` works since None if no match
                                paramATokenI = -1
                    elif len(endParamsToken) > 0:
                        paramATokenI = line.find(endParamsToken)
                        if paramATokenI < 0:
                            paramATokenI = len(line)
                    else:
                        paramATokenI = len(line)
                        # paramAToken = "<forced token=\""
                        # + paramAToken.replace("\"", "\\\"") + "\">"

                    if paramATokenI > -1:
                        if self.m_VerboseParsing:
                            pinfo("    has pre-ParamA '{}' @{}"
                                  " (after {}-long file token at {}"
                                  " ending at {})"
                                  "".format(paramAToken, paramATokenI,
                                            len(fileToken), fileTokenI,
                                            fileTokenI+len(fileToken)))
                            # ^ such as ', line '

                        paramAI = paramATokenI + len(paramAToken)
                        if len(paramBToken) > 0:
                            # There should be no B if there is no A,
                            # failing in that case is OK.
                            paramBTokenI = line.find(paramBToken,
                                                     paramAI)
                        else:
                            paramBTokenI = paramAI
                            # paramBToken = "<forced token=\""
                            #     + paramBToken.replace("\"", "\\\"")
                            #     + "\">"

                        if paramBTokenI > -1:
                            if self.m_VerboseParsing:
                                pinfo("      has pre-ParamB token '{}'"
                                      " @{} at or after ParamA token"
                                      " ending at {}"
                                      "".format(paramBToken,
                                                paramBTokenI,
                                                (paramATokenI
                                                 + len(paramAToken))))

                            # if paramBToken != itList[TOKEN_PARAM_B]:
                            #    paramBToken = ""
                            #    # ^ since may be used to locate next
                            #    #   value
                            if len(paramBToken) > 0:
                                paramBI = (paramBTokenI
                                           + len(paramBToken))
                            else:
                                paramBI = paramBTokenI
                            if len(endParamsToken) == 0:
                                endParamsTokenI = paramBI
                                if self.m_VerboseParsing:
                                    pinfo("  using paramBI for"
                                          " endParamsTokenI: {}"
                                          "".format(paramBI))
                            elif endParamsToken != "\n":
                                endParamsTokenI = line.find(
                                    endParamsToken,
                                    paramBI
                                )
                            else:
                                endParamsTokenI = len(line)
                                # endParamsToken = "<forced token=\""
                                # + endParamsToken.replace("\"", "\\\"")
                                # .replace("\n", "\\n") + "\">"

                            if endParamsTokenI > -1:
                                if len(paramBToken) == 0:
                                    paramBTokenI = endParamsTokenI
                                    # so paramAI can be calculated
                                    # correctly if ends at
                                    # endParamsTokenI
                                    paramBI = endParamsTokenI

                                if (itList[PARSE_COLLECT]
                                        == COLLECT_REUSE):
                                    info["master"] = "True"
                                if itList[PARSE_STACK] == STACK_LOWER:
                                    info["lower"] = "True"
                                if self.m_VerboseParsing:
                                    endParamsTokenEnc = \
                                        endParamsToken.replace("\n",
                                                               "\\n")
                                    pinfo(
                                        "        has post-params '{}'"
                                        " ending@{}>={}={}+{} in '{}'"
                                        "".format(
                                            endParamsTokenEnc,
                                            endParamsTokenI,
                                            (paramBTokenI
                                             + len(paramBToken)),
                                            paramBTokenI,
                                            len(paramBToken),
                                            line
                                        )
                                    )

                                if (endParamsToken
                                        != itList[TOKEN_END_PARAMS]):
                                    # since could be used for more stuff
                                    # after 2 params in future versions,
                                    # length should be 0 if not found
                                    # but forced:
                                    endParamsToken = ""

                                fileI = fileTokenI + len(fileToken)
                                break
                            elif self.m_VerboseParsing:
                                pinfo(
                                    "        no post-params '{}' >="
                                    " {} in '{}'"
                                    "".format(endParamsToken,
                                              (paramBTokenI
                                               + len(paramBToken),
                                              line))
                                )
                        elif self.m_VerboseParsing:
                            pinfo(
                                "      no pre-ParamB '{}' >= {} in '{}'"
                                "".format(paramBToken,
                                          (paramATokenI
                                           + len(paramAToken)),
                                          line)
                            )
                    elif self.m_VerboseParsing:
                        pinfo("    no pre-paramA '{}' >= {}"
                              "".format(paramAToken,
                                        (fileTokenI + len(fileToken))))
                elif self.m_VerboseParsing:
                    pinfo("  no pre-File '{}' >= START"
                          "".format(fileToken))

        # pinfo("fileTokenI: {}".format(fileTokenI))
        # pinfo("paramATokenI: {}".format(paramATokenI))
        # pinfo("paramBTokenI: {}".format(paramBTokenI))
        # pinfo("endParamsTokenI: {}".format(endParamsTokenI))
        # pinfo("fileToken: {}".format(fileToken))
        # pinfo("paramAToken: {}".format(paramAToken))
        # pinfo("paramBToken: {}".format(paramBToken))
        # pinfo("endParamsToken: {}".format(endParamsToken))
        if fileI >= 0 and (paramATokenI > fileI or endParamsToken > fileI):
            # Even if closer is not present,
            # endParamsTokenI is set to len IF applicable to self-
            # enclosure
            filePath = ""
            if paramATokenI > fileI:
                filePath = line[fileI:paramATokenI]
            else:
                filePath = line[fileI:endParamsTokenI]

            filePath = filePath.strip()
            if len(filePath) >= 2:
                if ((filePath.startswith('"')
                     and filePath.endswith('"'))
                    or (filePath.startswith('\'')
                        and filePath.endswith('\''))):
                    filePath = filePath[1:-1]

            echo1("[outputinspector] [debug]"
                  " file path before unmangling: \"{}\""
                  "".format(filePath))
            filePath = OutputInspector.unmangledPath(filePath)
            info["file"] = filePath
            info["row"] = line[paramAI:paramBTokenI]
            if len(paramBToken) > 0:
                info["column"] = line[paramBI:endParamsTokenI]
            else:
                info["column"] = ""
            if self.m_VerboseParsing:
                pinfo("        file '{}'"
                      "".format(line[fileI:paramATokenI]))
            # if self.m_VerboseParsing:
            #     pinfo("        row '{}'"
            #           "".format(line[paramAI:paramBTokenI]))
            if self.m_VerboseParsing:
                pinfo("        row '{}'"
                      "".format(info["row"]))
            if self.m_VerboseParsing:
                pinfo("          length {}-{}"
                      "".format(paramBTokenI, paramAI))
            # if self.m_VerboseParsing:
            #     pinfo("        col '{}'"
            #           "".format(line[paramBI:endParamsTokenI]))
            if self.m_VerboseParsing:
                pinfo("        col '{}'".format(info["column"]))
            if self.m_VerboseParsing:
                pinfo("          length {}-{}".format(endParamsTokenI,
                                                      paramBI))
            filePathLower = filePath.lower()
            if filePathLower.endswith(".py"):
                info["language"] = "python"
            elif filePathLower.endswith(".pyw"):
                info["language"] = "python"
            elif filePathLower.endswith(".cpp"):
                info["language"] = "c++"
            elif filePathLower.endswith(".h"):
                info["language"] = "c++"
            elif filePathLower.endswith(".hpp"):
                info["language"] = "c++"
            elif filePathLower.endswith(".c"):
                info["language"] = "c"
            elif filePathLower.endswith(".js"):
                info["language"] = "js"
            elif filePathLower.endswith(".java"):
                info["language"] = "java"
            elif filePathLower.endswith(".bat"):
                info["language"] = "bat"
            elif filePathLower.endswith(".sh"):
                info["language"] = "sh"
            elif filePathLower.endswith(".command"):
                info["language"] = "sh"
            elif filePathLower.endswith(".php"):
                info["language"] = "php"
            echo1("  detected file: '{}'"
                  "".format(filePath))
            info["good"] = "True"
            # pinfo("[outputinspector] found a good line"
            #       " with the following filename: {}".format(filePath))

        else:
            info["good"] = "False"
            # pinfo("[outputinspector] found a bad line: {}"
            #       "".format(originalLine))

        if info["good"] == "True":
            if (len(actualJump) > 0) and (info["master"] == "False"):
                echo1("INFO: nosetests output was detected, the line is"
                      " not first line of a known multi-line error"
                      " format, flagging as details (must be"
                      " a sample line of code or something).")
                info["good"] = "False"
                # TODO: ^ possibly eliminate self for fault tolerance
                # (different styles in same output)
                info["details"] = "True"
                info["file"] = ""




    def absPathOrSame(self, filePath):
        # absFilePath = os.path.abspath(filePath)
        absFilePath = ""
        sCwd = os.getcwd()  # current() returns a QDir object
        setuptoolsTryPkgPath = os.path.join(sCwd, os.path.split(sCwd)[1])
        # ^ See if it is in a deeper directory created by setuptools
        # FIXME: ^ Whether this code is correct is unclear--document
        # a use case??
        # - If should use filePath, remove the setuptoolsTryPkgPath join
        #   call below (do not add the filename when generating
        #   absFilePath if filename already part of
        #   setuptoolsTryPkgPath).
        if self.m_Verbose:
            pinfo("setuptoolsTryPkgPath: {}"
                  "".format(setuptoolsTryPkgPath))
        absFilePath = filePath
        if not filePath.startswith("/"):
            absFilePath = os.path.join(sCwd, filePath)
        if not os.path.exists(absFilePath) and os.path.exists(setuptoolsTryPkgPath):
            absFilePath = os.path.join(setuptoolsTryPkgPath, filePath)
        if not os.path.exists(absFilePath):
            pinfo("- absFilePath doesn't exist: {}".format(absFilePath))
        return absFilePath


    def on_mainListWidget_itemDoubleClicked(self, item):
        line = item.text()
        actualJump = item.data(ROLE_COLLECTED_FILE).toString();
        # item.toolTip()
        actualJumpLine = item.data(ROLE_COLLECTED_LINE).toString();
        # item.toolTip()
        if len(actualJumpLine) > 0:
            line = actualJumpLine
        filePath = (item.data(ROLE_COLLECTED_FILE)).toString()
        absFilePath = filePath
        errorMsg = ""
        if len(filePath) > 0:
            if self.m_Verbose:
                pinfo("clicked_file: '{}'".format(filePath))
                pinfo("tooltip: '{}'".format(item.toolTip()))

            absFilePath = self.absPathOrSame(filePath)
            citedRowS = (item.data(ROLE_ROW)).toString()
            citedColS = (item.data(ROLE_COL)).toString()
            if self.m_Verbose:
                pinfo("citedRowS: '{}'".format(citedRowS))
                pinfo("citedColS: '{}'".format(citedColS))

            citedRow = int(citedRowS)
            citedCol = int(citedColS)
            xEditorOffset = self.settings.getInt("xEditorOffset")
            yEditorOffset = self.settings.getInt("yEditorOffset")
            # region only for Kate <= 2
            citedRow += yEditorOffset
            citedRowS = str(citedRow)
            citedCol += xEditorOffset
            citedColS = str(citedCol)
            # endregion only for Kate <= 2
            if self.m_CompensateForKateTabDifferences:
                readCitedI = 0
                '''*< This is the current line number while the loop
                reads the entire cited file. '''
                with open(absFilePath, 'r') as qtextNow:
                    #| QFile.Translate
                    for rawL in qtextNow:
                        line = rawL.rstrip()
                        if readCitedI == ((citedRow - yEditorOffset) - 1):
                            tabCount = 0
                            # TODO: Use regex for finding the tab.
                            for tryTabI in range(len(line)):
                                if line[tryTabI] == "\t":
                                    tabCount += 1
                                else:
                                    break

                            tabDebugMsg = ""
                            if tabCount > 0:
                                tabDebugMsg = str(tabCount)
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
                                citedColS = str(citedCol)
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
                                        citedColS = str(citedCol)
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
                                                    regeneratedCol += 1
                                                    '''^ only add if it
                                                    is 4,7,10, where
                                                    addend is
                                                    self.config.getInt("Kate2TabWidth")
                                                    (1+self.config.getInt("Kate2TabWidth")*position)'''
                                                    tabDebugMsg += "-"


                                            else:
                                                regeneratedCol += 1

                                        citedCol = regeneratedCol
                                        # + ( (tabCount>3andtabCount<6)
                                        #     ? tabCount : 0 )
                                        # end accounting for kate#
                                        # gibberish column translation

                                # else kate 3+, handles tabs as absolute
                                # positions
                                citedColS = str(citedCol)
                                tabDebugMsg += ("; citedColS-new:"
                                                + citedColS)
                                if self.m_EnableTabDebugMsg:
                                    self.showinfo(
                                        ("Output Inspector"
                                         " - Debug tab compensation"),
                                        tabDebugMsg
                                    )
                            # end if tabCount>0
                            break
                        # if correct line found
                        readCitedI += 1
                    # while not at end of source file
                    # qfileSource.close()
                # end if can open source file

                # else:
                '''
                    errorMsg = ("Specified file '{}' does not exist or"
                                " is not accessible (if path looks"
                                " right, running from the location"
                                " where it exists instead of '{}')"
                                "".format(filePath, os.getcwd()))
                '''

                # end if self.m_CompensateForKateTabDifferences
            # sArgs="-u "+absFilePath+" -l "+citedRowS+" -c "+citedColS
            # QProcess qprocNow(self.config.getString("editor")+sArgs)
            # qprocNow
            if os.path.isfile(absFilePath):
                commandMsg = self.settings.getString("editor")
                qslistArgs = []  # editor subprocess argument strings
                # NOTE: -u is not needed at least as of kate 16.12.3
                # which does not create additional
                # instances of kate
                # qslistArgs.append("-u")
                # commandMsg+=" -u"
                # qslistArgs.append("\""+absFilePath+"\"")
                qslistArgs.append(absFilePath)
                commandMsg += " " + absFilePath
                qslistArgs.append("--line")
                # ^ split into separate arg, geany complains that
                # it doesn't understand the arg "--line 1"
                qslistArgs.append(citedRowS)
                commandMsg += " --line " + citedRowS
                # qslistArgs.append(citedRowS)
                qslistArgs.append("--column")
                # ^ NOTE: -c is column in kate, alternate config dir
                # in geany, use --column
                qslistArgs.append(citedColS)
                # ^ NOTE: -c is column in kate, alternate config dir
                # in geany, use --column
                commandMsg += " --column " + citedColS
                # qslistArgs.append(citedColS)
                # warn("qslistArgs: {}".format(qslistArgs))
                # TODO: (?) process = Popen(['cat', 'example.py'],
                #                           stdout=PIPE, stderr=PIPE)
                # stdout, stderr = process.communicate()
                # print(stdout)
                process = Popen([self.settings.getString("editor")]
                                 + qslistArgs)
                '''
                "...use poll() to check if the child process has
                terminated, or use wait() to wait for it to terminate."
                -Adam Rosenfield Mar 11, 2009 at 22:09 comment on
                <https://stackoverflow.com/a/636570/4541104>
                '''
                if not os.path.isfile(self.settings.getString("editor")):
                    # ok to run anyway for fault tolerance, may be in
                    # system path
                    self.showinfo("Output Inspector - Configuration",
                                  (self.settings.getString("editor")
                                   + " cannot be accessed.  Try setting"
                                   " the value editor = in "
                                   + self.settings.fileName()))

                # if self.m_Verbose:
                self._ui.statusBar.showMessage(commandMsg, 0)
                # os.system(sCmd)  # stdlib
                # self.showinfo("test", sCmd)

            else:
                # errorMsg = ("Specified file '" + absFilePath
                #             + "' does not exist (try a different"
                #             " line, try running from the location"
                #             " where it exists instead of '"
                #             + os.getcwd() + "')")
                errorMsg = ("[Output Inspector] No file exists here:"
                            " '{}'\n".format(absFilePath))
        # end if line is in proper format
        else:
            errorMsg = "Could not detect error format\n"
        if len(errorMsg) > 0:
            if len(filePath) > 0:
                # warn(errorMsg + " in '" + line + "':")
                info = self.getLineInfo(line, actualJump,
                                        actualJumpLine, False)
                infoS = ""
                for k, v in info.items():
                    # warn("    " + it.first # key
                    #         + ": '" + it.second + "'")  # value
                    infoS += "; " + k + ": '" + v + "'"

                warn("[outputinspector][error] {} in the line:"
                     "; actualJump: {}  actualJumpLine: {} {}"
                     "".format(errorMsg,
                               item.data(self.ROLE_COLLECTED_FILE),
                               item.data(self.ROLE_COLLECTED_LINE),
                               infoS))
                #    + "  info:")

                self.showinfo("Output Inspector", errorMsg)
                # self.showinfo("Output Inspector",
                #               "'{}' cannot be accessed (try a"
                #               " different line, if self line's path"
                #               " looks right, running from the"
                #               " location where it exists instead of"
                #               " '{}')"
                #               "".format(absFilePath, os.getcwd()))
                # or pasting the entire line to 'Issues' link on
                # web-based git repository
            else:
                pinfo("[Output Inspector] No file was detected in line:"
                      " '{}'".format(line))
                pinfo("[Output Inspector] ERROR: '{}'".format(errorMsg))

    def readInput(self):
        limit = 50
        count = 0
        line = " "
        while (count < limit and not line.empty()):
            size = std.cin.rdbuf().in_avail()
            if size < 1:
                # pinfo("OutputInspector: There is no input: got "
                #       "".format(size))
                # Prevent waiting forever for a line.
                break

            std.getline(std.cin, line)
            if not std.cin.eof():
                # pinfo("OutputInspector: input is '{}'."
                #       "".format(line))
                # self.addLine("OutputInspector: input is: {}"
                #              "".format(line), True)
                self.addLine(line, True)

            else:
                # pinfo("OutputInspector: input has ended.")
                # self.addLine("# OutputInspector: input has ended.",
                #              True)
                break

            count += 1

def main():
    prefix = "[main (testing only, import instead)] "
    inspector = OutputInspector()
    echo0(prefix+"created a test OutputInspector")


if __name__ == "__main__":
    sys.exit(main())
