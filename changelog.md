# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [git] - 2022-05-15
### Fixed
- Rename "dist" to "usr" so it isn't excluded by .gitignore.

## [new new-python branch in git] - 2020-03-21
### Changed
(Change to Python. So far, only change punctuation only using cpp2python.py from <https://github.com/andreikop/cpp2python> so far)
- `astyle --style=ansi *.cpp *.h`
- `python3 cpp2python.py ~/git/outputinspector-ansi-pre-python`
- combine h.py and cpp.py files.
- rename main.cpp.py run.pyw
- rename mainwindow.py outputinspector/__init__.py
- rename etc to projects and move other files out of there to:
  - doc/development
  - tests/data
  - and other places
- rename package to dist

### Remove
- files related to Qt



## [git] - 2020-11-25
### Add
- xcf and ico versions of the icon
- optional static build script as per
  <https://wiki.qt.io/Building_a_static_Qt_for_Windows_using_MinGW>

### Fixed
- Change icon to ico to allow a Windows exe to build.
  - Add it as the Windows icon in the pro file.
  - Change the window icon to in the UI file using the Designer view.


## [git] - 2020-03-27
### Added
- Detect debug.txt in current directory if present.


## [2.0.0] - 2020-03-13
### Added
- Show a warning regarding deprecated settings (show what value was
  used).

### Changed
- Document the new settings file path correctly (in documents and
  message(s) shown at run-time).
- Rename variable from `kate` to `editor` as of
  https://github.com/poikilos/outputinspector/commit/d19bd0074bfb633c4ce68225a37c1509fb255fab
- Copy the old `kate` variable to the new `editor` variable.
- Rename `config` object to `settings`.
- Clarify names of transient variables.
- Improve formatting in readme.md.

### Removed
- Delete previously-commented code in DoubleClicked event.


## [2.0.0] - 2020-03-13
### Added
- a description (`PARSE_DESCRIPTION`) in the syntax `def`
- `unmangledPath` (remove ellipsis!) resolves #5

### Changed
- Place the configuration into a separate class (See settings.h and
  settings.cpp)--resolves #9.
- Move globals into classes.
- Rename variables.
- Move jshint parsing to a unified parsing def.
- Change settings path from `/etc/outputinspector.conf` to
  `$HOME/.local/share/outputinspector/settings.txt`


## [2.0.0] - 2020-03-13
### Changed
- Reduce double-click warnings (such as for missing file) to one line.
- Prepend "[outputinspector]" to stdout/stderr (such as warnings).


## [1.3.0] - 2020-03-13
### Changed
- Move old known issues from readme.md to the issue tracker.
  - The "allow param for directory" issue is now a part of issue #5
  - Any not added to the issue tracker were already resolved.


## [1.3.0] - 2020-03-13
### Added
- Accept standard input such as by piping from another program!
  - resolves #2 (enhancement)

### Changed
- If any standard input is waiting, change dialog behavior. Instead of
  showing dialog boxes that interrupt init (in the case of a missing
  input file), show lines in the window.


## [1.3.0] - 2020-03-13
### Added
- Run the script as a user not named "root" to install to use the
  prefix ~/.local (if the `PREFIX` environment variable is not
  specified)


## [1.2.0] - 2020-03-12
### Added
- Handle Minetest Lua Warnings.
- Add Qt-style doxygen strings for constants pointing to parsing markers
  and their values (resolves #3).


## [1.1.0] - 2020-03-12
### Added
- Handle Minetest Lua tracebacks (such as from debug.txt or stderr).
  - This feature requires the fix below.
  - :edit: This is improved now since issue #5 is closed
    (enhancement: un-mangle a path containing an ellipsis)!

### Fixed
- Handle `PARSE_MARKER_FILE` value not at the start of a line.


## [1.1.0] - 2020-03-11
### Added
- Allow simpler syntax while avoiding false positives by only allowing
  the param opener when it is followed by a digit.
  - Detect grep output syntax.
  - Detect nose test output syntax without hard-coded `split(":")`.


## [1.1.0] - 2019-12-27
### Changed
- Fix pycodestyle support.
- Improve changelog formatting and grammar.


## [1.1.0] - 2019-01-11
### Changed
- Make parsing modes into modular, expandable datasets.
- Use clang-format for coding style.


## [1.1.0] - 2018-01-11
### Added
- Support nosetests output.


## [1.1.0] - 2018-08-15
### Changed
- Warn if input file is missing or has only blank lines.
- Allow a param to specify output (continue to use err.txt as default).


## [git] - 2017-11-09
### Added
- Detect and show fatal errors (with an explanation that your tool
  recorded them before outputinspector started, to avoid any confusion)


## [git] - 2017-11-09
### Changed
- Drastically improve install script (detect and warn if it can't
  manipulate existing install in any way, and installs to /usr/local/bin
  instead of /usr/bin).
  - Detect and stop installing if /usr/bin/outputinspector exists,
    (should be /usr/local/bin) and show repair instructions (see install
    file echo statements for more details).
  - Detect newer version (whether Debug or Release).
  - If neither Debug nor Release is present (in Qt Creator default build
    folders), use outputinspector binary in working directory if
    present, otherwise show error and exit.
    - If no source binary is found, give instructions on how to proceed
      (also, instructions for recompiling on success now correctly state
      to use Qt Creator instead of QtDevelop).


## [git] - 2017-11-09
### Added
- Try to detect and convert jshint output to mcs output:
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
- Change old code (had MainWindow, listMain, menubar, and statusbar):
  - Change menubar to menuBar.
  - Change statusbar to statusBar.
  - Change listMain (in old code) to ui->mainListWidget (rename in new
    code from listWidget which was present by default for new widget
    form).
  - Deprecate manually resizing list widget (in favor of sizePolicy
    Expanding [and default aka MAX_INT maximumSize] for both vertical
    and horizontal).
- Change new code (had MainWindow, centralWIdget, menuBar, mainToolBar
  and statusBar all by default for new widget form; don't rename
  anything):
  - Create a List Widget (QListWidget, and Item-Based Widget) named
    mainListWidget.
  - Right-click mainListWidget in form designer, go to slot, paste
    content of QListWidget::itemDoubleClicked from old qt4 version.


## [git] - 2017-03-25
### Changed
- Move from sourceforge to GitHub.


## [Unreleased] - 2008-10-qt4
- Known Issues with this sf.net release:
  - At least in Kate 3.0.3, "kate -u" becomes ineffective when Kate 2 is
    open at the same time, so more copies of Kate 3 open.

### Changed
(for configurable settings, edit the variable name in /etc/outputinspector.conf)
- Open source files and look for TODO comments (FindTODOs=yes in conf
  file).
- Allow auto-close if no errors occur (ExitOnNoErrors=yes in conf file).
- Show count of errors and warnings (and TODOs unless specified -- see
  above) in status bar.
- Fix problem caused by Kate line & row args starting with zero
  (configurable: by default xEditorOffset=-1 and xEditorOffset-1).
- Kate is no longer linked as a child process of outputinspector.
- Compensate for different tab handling between Kate 3 and mcs, and
  attempt to work around Kate 2 tab traversal glitches related to the
  column command line parameter (see README.md).


## [qt4 (Initial release)] - 2008-09-30
- Known issues with this sf.net release:
  - kate 2.5.x, Kate 3.0.x, and mcs all have different ways of counting
    tabs, so column numbering is not exact.

### Changed
- Open err.txt if in the same folder as outputinspector (see README
  under "Usage").
- If user double-clicks a line of code, tell kate to navigate to that
  file and location, as long as the file (or relative path) is in the
  same folder as outputinspector and err.txt.

### Added
- Handle kate (at least 2.5.9) as it does not go to the exact code
  location since -l and -c args start at origin (0,0).

