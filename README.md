# Output Inspector
Output Inspector is a helper app for Kate which allows you to double-click to jump to the file and line of code at which your compiler found errors (for mono or any C# or other compiler using "filename(line,position): error" notation).

## Purpose:
Allows you to double-click lines of your compiler output, telling kate to jump to the appropriate file, line and position in the code.

## Install
* Right-click the downloaded zip file, then click Extract Here
```
cd outputinspector
```
* open in QT Creator 5
* then push the F7 key.  When it is finished compiling, exit.
* open a terminal
    * cd to the directory where you extracted outputinspector i.e. cd outputinspector-2008-10-08-qt4
    ```
    sudo ./install (OR: su root && ./install)
    ```
    * If you have never installed outputinspector before you must run:
    ```
    sudo ./install-conf (OR: su root && ./install)
    ```
    (this will reset to default settings if outputinspector was already installed)

## Configure
* Remember to edit /etc/outputinspector.conf specifying the kate binary (i.e. include the line kate=/usr/bin/kate such as for Fedora 25 or kate=/usr/lib/kde4/bin/kate for Ubuntu Hardy or appropriate command).
* Tab handling:  If you are using mcs 1.2.6 or other compiler that reads tabs as 6 spaces, and you are using Kate 2 with the default tab width of 8 or are using Kate 3, you don't have to change anything.  Otherwise:
* Set CompilerTabWidth and Kate2TabWidth in /etc/outputinspector.conf -- kate 3.0.0+ is handled automatically, but CompilerTabWidth may have to be set.  If the compiler treats tabs as 1 character, make sure you set CompilerTabWidth=1 -- as of 1.2.6, mcs counts tabs as 6 spaces (outputinspector default).

## Use
You have to run outputinspector from the location of the cs files you are compiling, and your compiler error output has to be redirected to err.txt.
example:
```
mcs AssemblyInfo.cs MainForm.cs 2>err.txt
outputinspector
```
If these instructions have been followed, and your compiler errors are in err.txt in the same folder, specified with lines starting with:
Filename.ext(row,col): error
Then Output Inspector should work when you double-click on the error.

## Changes
* (2017-03-25) migrated from qt4 to qt5
    * changes to old code (had MainWindow, listMain, menubar, and statusbar):
        * changed menubar to menuBar
        * changed statusbar to statusBar
        * changed listMain (in old code) to ui->mainListWidget (renamed in new code from listWidget which was present by default for new widget form)
        * deprecated manually resizing list widget (in favor of sizePolicy Expanding [and default aka MAX_INT maximumSize] for both vertical and horizontal)
    * changes to new code (had MainWindow, centralWIdget, menuBar, mainToolBar and statusBar all by default for new widget form):
        * nothing renamed
        * created a List Widget (QListWidget, and Item-Based Widget) named mainListWidget
        * right-click mainListWidget in form designer, go to slot, pasted content of QListWidget::itemDoubleClicked from old qt4

* (2008-10-qt4) fixes and improvements
(for configurable settings, edit the variable name in /etc/outputinspector.conf)
    * Opens source files and looks for TODO comments (FindTODOs=yes in conf file)
    * Allows auto-close if no errors (ExitOnNoErrors=yes in conf file)
    * Shows count of errors and warnings (and TODOs unless specified -- see above) in status bar
    * Fixed problem caused by Kate line & row args starting with zero (configurable: by default xEditorOffset=-1 and xEditorOffset-1)
    * Kate is no longer linked as a child process of outputinspector
    * Compensates for different tab handling between Kate 3, and mcs, and attempts to work around Kate 2 tab traversal glitches related to the column command line parameter (see README)
    * Known Issues with this sf.net release:
        * At least in Kate 3.0.3, "kate -u" becomes ineffective when Kate 2 is open at the same time, so more copies of Kate 3 open.

* (2008-09-30) qt4 (Initial release):
    * opens err.txt if in the same folder as outputinspector (see README under "Usage")
    * allows you to double-click on lines of code, then tells kate to navigate to that file and location, as long as the file (or relative path) is in the same folder as outputinspector and err.txt.
    * kate (at least 2.5.9) does not go to exact code location since -l and -c args start at origin (0,0)
    * Known issues with this sf.net release:
        * kate 2.5.x, Kate 3.0.x, and mcs all have different ways of counting tabs, so column numbering is not exact.

## Known Issues
* Move OutputInspectorSleepThread to new h and cpp
* Move declarations from cpp to h file (keep initialization in cpp file); possibly move to class
* Encapsulate settings functions; possibly use an existing class from qt or other source
* Doesn't add to error count for lines that contain string "previous error"
* Shows errors first (including lines that contain "previous error") (ShowWarningsLast=yes in conf file)
* Counts errors, warnings, and ToDos correctly thanks to correct indexOf (QString not const char*) syntax for case-insensitivity
* application icon doesn't work (see comments in main.cpp for setting via the QApplication object named 'a')
* allow param for compiler output text file name (instead of only err.txt in current directory)
* allow param for directory containing source files to which error output refers

## Notes that only applied to qt4 version
* Compile First (if desired, but there is a binary built on Ubuntu Hardy):
* Build using qdevelop  (formerly required: qt4 libqt4-dev qdevelop):
```
qdevelop outputinspector.pro
```
* Install:
	(Requires: qt4 libqt4 libqt4-gui libqt4-core)
