# Output Inspector
Output Inspector is a parser for parser output that makes errors clickable so you can get back to your software's source code.


## Purpose
The main function of Output Inspector is to tell Kate to go
to the file and location of warnings/errors such as:
```
foo.js: line 1, col 10, reason
```
or
```
foo.cs(1,10): reason
```

Your source file should not have any unsaved changes in any other program at the time (it is ok if in Kate, but saving first and using your parser on that version is recommended for accuracy).
Other than jshint output, Output Inspector has only been tested on mcs [mono compiler] output, but may work for any C# compiler and will work for any compiler or other tool using the formatting above.
The parsing is not fault-tolerant at this time, especially for the first type of formatting.
Output of jshint is expected unless the second formatting is used by your parser (such as mcs).


## Install
* Right-click the downloaded zip file, then click Extract Here
* open in QT Creator 5
* then push the F7 key.  When it is finished compiling, exit.
* open a terminal
    * cd to the directory where you extracted outputinspector such as:
      ```
      # if you don't have sudo installed or are not a sudoer,
      # `su root` before attempting install below
      cd outputinspector
      # or
      cd outputinspector-master
      ```
      ```
      sudo ./install
      # or if you don't have sudo installed and are root, just `./install`
      ```


## Use
* make sure kate is installed (run install script again if wasn't when install script ran--it recreates the config based on detecting kate's location if you enter y for yes)
* For cs files, you have to run outputinspector from the location of the cs files you are compiling, and your compiler error output has to be redirected to err.txt.
  example:
  ```
  mcs AssemblyInfo.cs MainForm.cs 2>err.txt
  outputinspector &
  ```
  (a space then & sign after outputinspector makes it not prevent continued use of console, however this is not recommended or else you may forget its open and 
  If these instructions have been followed, and your compiler errors are in err.txt in the same folder, specified with lines starting with:
  Filename.ext(row,col): error
  Then Output Inspector should work when you double-click on the error.
* jshint instructions (jshint package helps check js files)
  ```
  jshint > err.txt
  outputinspector &
  ```
  If these instructions have been followed, and your compiler errors are in err.txt in the same folder, specified with lines starting with:
  
  Then Output Inspector should work when you double-click on the error.


## Overview of jshint
Usually from nodejs-jshint package, jshint is a linting and/or hinting tool for javascript (especially node.js) which is considered a successor to jslinter. Here is the timeline:
 * Kate-plugins project (original source of jslinter and other plugins) is no longer maintained since merged with Kate.
 * Kate removed jslinter, and some say the kde team got complaints that jslinter was too opinionated
 * jshint and possibly other js linting/hinting tools were created to fill the gap left by jslinter
 * Kate-plugins can still be installed but is either difficult or impossible to get working (only via `python2 -m pip install Kate-plugins` as trying to use python3 such as via pip directly if python3 is the default python, you will only get errors regarding python2 style code that remains in Kate-plugins), but how to install it and where to put the plugins is unclear. For example, creating a symlink as instructed by the project doesn't work even if the extra slash is removed after the closing parenthesis of the kde4-config output. Searching all files in '/' with max file size 2048000bytes using DeepFileFind for the phrase Replicode does not yield any non-binary files or folders that look like plugin folders (only results in mo, docbook, pmapc, pmap, qm, so files, and the config file for DeepFileFind itself where search history is saved). The so file found is:
`/usr/lib/qt/plugins/ktexteditor/katereplicodeplugin.so`
 * How to install in userspace remains unclear, but perhaps jslinter could be placed there.
 * However, one should note that after installing the package via the python2 command above:
   * there are no binaries from the Kate-plugins project, only python and python-related files, in:
     * /usr/lib/python2.7/site-packages/
       * /usr/lib/python2.7/site-packages/kate_plugins/
       * /usr/lib/python2.7/site-packages/Kate_plugins-0.2.3-py2.7.egg-info
       * /usr/lib/python2.7/site-packages/pyjslint-0.3.3-py2.7.egg-info/
         * or any of their subfolders.
     * a filename search for jslint in /usr/lib/python2.7/site-packages yields no binaries or files other than those in the folders above

    
## Configure
* Remember to edit /etc/outputinspector.conf specifying the kate binary (i.e. include the line kate=/usr/bin/kate such as for Fedora 25 or kate=/usr/lib/kde4/bin/kate for Ubuntu Hardy or appropriate command).
* Tab handling:  If you are using mcs 1.2.6 or other compiler that reads tabs as 6 spaces, and you are using Kate 2 with the default tab width of 8 or are using Kate 3, you don't have to change anything.  Otherwise:
* Set CompilerTabWidth and Kate2TabWidth in /etc/outputinspector.conf -- kate 3.0.0+ is handled automatically, but CompilerTabWidth may have to be set.  If the compiler treats tabs as 1 character, make sure you set CompilerTabWidth=1 -- as of 1.2.6, mcs counts tabs as 6 spaces (outputinspector default).

  
## Changes
* (2017-11-09) has mechanism to detect and show fatal errors (with an explanation that your tool recorded them before outputinspector started, to avoid any confusion)
* (2017-11-09) drastically improved install script (detects and warns if can't manipulate existing install in any way, and installs to /usr/local/bin instead of /usr/bin)
  * detects and stops going if already /usr/bin/outputinspector
    (should be /usr/local/bin) and gives repair instructions (see install file echo statements for more details)
  * detects newer version (whether Debug or Release)
  * if neither Debug nor Release is present (in Qt Creator default build folders), uses outputinspector binary in working directory if present, otherwise shows error and exits
    * if doesn't find any source binary, gives instructions on how to proceed (also, instructions for recompiling on success now correctly state to use Qt Creator instead of QtDevelop)
* (2017-11-09) tries to detect and convert jshint output to mcs output:
  * example output of running 'mcs etc/foo.cs 1>out.txt 2>err.txt  if you have mcs installed (the c# compiler which normally comes with the mono package, or can also be from a .NET Framework SDK or other C# development tool)':
    ```
    etc/foo.cs(10,24): error CS0103: The name `Path' does not exist in the current context
    Compilation failed: 1 error(s), 0 warnings
    ```
    or with line 10 commented:
    ```
    etc/foo.cs(11,20): warning CS0219: The variable `uhoh' is assigned but its value is never used
    Compilation succeeded - 1 warning(s)
    ```
    or if file doesn't exist:
    ```
    error CS2001: Source file `etc/foo.cs' could not be found
    ```
  * jshint example output (result of running jshint etc/foo.js if you have jshint installed):
    ```
    etc/foo.js: line 2, col 9, Use '!==' to compare with 'null'.

    1 error
    ```

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

## Planned Features
* paste from an online code quality tool

## Known Issues
* should have user-space config generated at startup if doesn't exist (instead of requiring install script to run as root and generate /etc/outputinspector.conf)
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
