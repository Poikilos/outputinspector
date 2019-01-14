# Changelog


## [Unreleased] - 2019-01-11
### Changed
- make parsing modes into modular, expandable datasets
- use clang-format for coding style


## [Unreleased] - 2018-01-11
### Added
- support for nosetests output


## [Unreleased] - 2018-08-15
### Changed
- Show warning for missing input file or file with only blank lines
- allow param to specify output (continue to use err.txt as default)


## [Unreleased] - 2017-11-09
### Added
- mechanism to detect and show fatal errors (with an explanation that your tool recorded them before outputinspector started, to avoid any confusion)


## [Unreleased] - 2017-11-09
### Changed
- drastically improved install script (detects and warns if can't manipulate existing install in any way, and installs to /usr/local/bin instead of /usr/bin)
  - detects and stops going if already /usr/bin/outputinspector
    (should be /usr/local/bin) and gives repair instructions (see install file echo statements for more details)
  - detects newer version (whether Debug or Release)
  - if neither Debug nor Release is present (in Qt Creator default build folders), uses outputinspector binary in working directory if present, otherwise shows error and exits
    - if doesn't find any source binary, gives instructions on how to proceed (also, instructions for recompiling on success now correctly state to use Qt Creator instead of QtDevelop)


## [Unreleased] - 2017-11-09
### Added
- tries to detect and convert jshint output to mcs output:
  - example output of running 'mcs etc/foo.cs 1>out.txt 2>err.txt  if you have mcs installed (the c# compiler which normally comes with the mono package, or can also be from a .NET Framework SDK or other C# development tool)':
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
  - jshint example output (result of running jshint etc/foo.js if you have jshint installed):
    ```
    etc/foo.js: line 2, col 9, Use '!==' to compare with 'null'.

    1 error
    ```


## [first qt5 version] - 2017-03-25
### Changed
- changes to old code (had MainWindow, listMain, menubar, and statusbar):
  - changed menubar to menuBar
  - changed statusbar to statusBar
  - changed listMain (in old code) to ui->mainListWidget (renamed in new code from listWidget which was present by default for new widget form)
  - deprecated manually resizing list widget (in favor of sizePolicy Expanding [and default aka MAX_INT maximumSize] for both vertical and horizontal)
- changes to new code (had MainWindow, centralWIdget, menuBar, mainToolBar and statusBar all by default for new widget form):
  - nothing renamed
  - created a List Widget (QListWidget, and Item-Based Widget) named mainListWidget
  - right-click mainListWidget in form designer, go to slot, pasted content of QListWidget::itemDoubleClicked from old qt4


## [Unreleased] - 2017-03-25
### Changed
- moved from sourceforge to GitHub


## [Unreleased] - 2008-10-qt4
- Known Issues with this sf.net release:
  - At least in Kate 3.0.3, "kate -u" becomes ineffective when Kate 2 is open at the same time, so more copies of Kate 3 open.

### Changed
(for configurable settings, edit the variable name in /etc/outputinspector.conf)
- Opens source files and looks for TODO comments (FindTODOs=yes in conf file)
- Allows auto-close if no errors (ExitOnNoErrors=yes in conf file)
- Shows count of errors and warnings (and TODOs unless specified -- see above) in status bar
- Fixed problem caused by Kate line & row args starting with zero (configurable: by default xEditorOffset=-1 and xEditorOffset-1)
- Kate is no longer linked as a child process of outputinspector
- Compensates for different tab handling between Kate 3, and mcs, and attempts to work around Kate 2 tab traversal glitches related to the column command line parameter (see README)


## [qt4 (Initial release)] - 2008-09-30
- Known issues with this sf.net release:
  - kate 2.5.x, Kate 3.0.x, and mcs all have different ways of counting tabs, so column numbering is not exact.

### Changed
- opens err.txt if in the same folder as outputinspector (see README under "Usage")
- allows you to double-click on lines of code, then tells kate to navigate to that file and location, as long as the file (or relative path) is in the same folder as outputinspector and err.txt.

### Added
- kate (at least 2.5.9) does not go to exact code location since -l and -c args start at origin (0,0)
 
