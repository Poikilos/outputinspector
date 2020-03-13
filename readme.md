# Output Inspector
Output Inspector is a "parser for parser output" that makes errors clickable so
you can get to the issue in your software's source code instantly.
After using your compiler/linting tool, you can now double-click an error or
warning to jump (using Kate or Geany line,col jump feature) to the file and line
of code having the issue (see Usage below).


## Install
### Web Install
#### Linux
Set a RELEASE to a different release tag you want:
```
RELEASE=1.3.0
mkdir -p ~/.local/bin
wget -O ~/.local/bin/outputinspector https://github.com/poikilos/outputinspector/releases/download/$RELEASE/outputinspector
wget -O ~/.local/bin/ogrep https://github.com/poikilos/outputinspector/raw/master/package/bin/ogrep
```


## Features
* Jumps to source line if you double-click error
* Color codes lines in your output (red: error; orange: warning; yellow: issue in installed library used [if in site-packages]; black: formatting marks; gray: unrecognized information)
  * Detects flags in your output: "Warning"
* Detects flags in files cited by your output: `TODO` or `FIXME` in inline comments
  * inline comment mark is determined from file extension: py, pyw, sh, c, h, cpp, hpp, js, java, php, bat, command
* Installs passthrough-outputinspector for use in IDEs (see Usage)


## Usage
Before first use, make sure kate or geany package is installed (run
install script again if wasn't when install script ran--it recreates
the config based on detecting kate's location if you enter y for yes).

### Piping
You can pipe output from another program to get near-realtime (depending
on your OS's implementation of piping) results without using files.

To get error output, run: `... 2>&1 >/dev/null | outputinspector`;
otherwise, simply run: `... | outputinspector`
(where "..." is your program).

### GUI-based use
For automatic usage on Linux, create a build command in Geany:
* "Build," "Set Build Commands"
* Set Execute (or an empty box under "Independent commands") to:\
  `passthrough-outputinspector python3 "%f"`\
  (if you set the Execute command, the gear button will run
  passthrough-outputinspector and then the button will turn into a stop button
  and be able to stop both passthrough-outputinspector and outputinspector)
  * You can send up to 7 additional params as passthrough-outputinspector
    checks for that many. Example:\
    `passthrough-outputinspector python3 "%f" --ExitOnNoErrors=no`

### Specific Uses
* For py file linting: you can use pycodestyle (tested with pycodestyle-3
command--package may be named python3-pycodestyle in your Linux distro):
```
pycodestyle-3 > err.txt
outputinspector &
```
* For cs files, you have to run outputinspector from the location of the cs
  files you are compiling, and your compiler error output has to be redirected
  to err.txt.\
  example:
  ```
  mcs AssemblyInfo.cs MainForm.cs 2>err.txt
  outputinspector &
  ```
  (a space then & sign after outputinspector makes it not prevent continued use
  of console, however this is not recommended or else you may forget its open
  and if these instructions have been followed, and your compiler errors are in
  err.txt in the same folder, specified with lines starting with:
  ```
  Filename.ext(row,col): error
  ```
  Then Output Inspector should work when you double-click on the error.
* for nose tests:
```
nosetests 1>out.txt 2>err.txt
outputinspector &
```
* jshint instructions (jshint package helps check js files)
  ```
  jshint > err.txt
  outputinspector &
  ```
  If these instructions have been followed, and your compiler errors are in
  err.txt in the same folder, with lines starting the sourcecode filename as per
  one of the output formats (see 'Formats' section above),
  then Output Inspector should work when you double-click on the error.
* `outputinspector err.txt &` can be added to the end of your linting or build
  script, allowing you to immediately go to the correct source file and line
  containing the error!\
  Your compiler or linter output can be redirected to any file, then the file
  can be specified as the first parameter of outputinspector (default is
  err.txt). You can also run outputinspector manually if you have a file
  containing linter/compiler output.
* Your source file should not have any unsaved changes in any other program at
  the time (it is ok if in Kate, but saving first and using your parser on that
  version is recommended for accuracy).
Other than jshint output, Output Inspector has only been tested on `mcs` (mono
compiler) output, but may work for any C# compiler and will work for any
compiler or other tool using the formatting above.\
The parsing is not fault-tolerant at this time, especially for the first type of
formatting.\
Output of jshint is expected unless the second formatting is used by your parser
(such as `mcs`).
* OPTIONAL: To use Geany, set: `kate=/usr/bin/geany` in
  '/etc/outputinspector.conf'" (outputinspector knows how to tell Geany which
  line and column for jumping to line by using args compatible with both Geany
  and Kate)"
* You can set any of the ini options as command line options (case sensitive).
  For a list of settings, see "/etc/outputinspector.conf" after install, or the
  included "etc/outputinspector.example.conf"\
  example:
  `outputinspector --ExitOnNoErrors=yes` (or `true` or `on` or `1`)


#### Overview of jshint
Usually from nodejs-jshint package, jshint is a linting and/or hinting tool for
javascript (especially node.js) which is considered a successor to jslinter.
Here is the timeline:
* Kate-plugins project (original source of jslinter and other plugins) is no
  longer maintained since merged with Kate.
* Kate removed jslinter, and some say the kde team got complaints that jslinter
  was too opinionated
* jshint and possibly other js linting/hinting tools were created to fill the
  gap left by jslinter
* Kate-plugins can still be installed but is either difficult or impossible to
  get working (only via `python2 -m pip install Kate-plugins` as trying to use
  python3 such as via pip directly if python3 is the default python, you will
  only get errors regarding python2 style code that remains in Kate-plugins),
  but how to install it and where to put the plugins is unclear. For example,
  creating a symlink as instructed by the project doesn't work even if the extra
  slash is removed after the closing parenthesis of the kde4-config output.
  Searching all files in '/' with max file size 2048000bytes using DeepFileFind
  for the phrase Replicode does not yield any non-binary files or folders that
  look like plugin folders (only results in mo, docbook, pmapc, pmap, qm, so
  files, and the config file for DeepFileFind itself where search history is
  saved). The so file found is:\
  `/usr/lib/qt/plugins/ktexteditor/katereplicodeplugin.so`
* How to install in userspace remains unclear, but perhaps jslinter could be
  placed there.
* However, one should note that after installing the package via the python2
  command above:
  * there are no binaries from the Kate-plugins project, only python and
    python-related files, in:
    * /usr/lib/python2.7/site-packages/
      * /usr/lib/python2.7/site-packages/kate_plugins/
      * /usr/lib/python2.7/site-packages/Kate_plugins-0.2.3-py2.7.egg-info
      * /usr/lib/python2.7/site-packages/pyjslint-0.3.3-py2.7.egg-info/
        * or any of their subfolders.
    * a filename search for jslint in /usr/lib/python2.7/site-packages yields no
      binaries or files other than those in the folders above


## Backward Compatibility
* Remember to edit /etc/outputinspector.conf specifying the kate binary (i.e.
  include the line `kate=/usr/bin/kate` such as for Fedora 25 or
  `kate=/usr/lib/kde4/bin/kate` or appropriate command for older linux distro
  such as Ubuntu Hardy).
* Tab handling:  If you are using `mcs` 1.2.6 or other compiler that reads tabs
  as 6 spaces, and you are using Kate 2 with the default tab width of 8 or are
  using Kate 3, you don't have to change anything.  Otherwise:
  * Set CompilerTabWidth and Kate2TabWidth in /etc/outputinspector.conf -- kate
    3.0.0+ is handled automatically, but CompilerTabWidth may have to be set.
    If the compiler treats tabs as 1 character, make sure you set
    `CompilerTabWidth=1` -- as of 1.2.6, `mcs` counts tabs as 6 spaces
    (outputinspector default).


## Changes
see CHANGELOG.md


## Planned Features
* paste (such as from an online code quality tool)
* allow reading from standard input


## Developer Notes
### Compiling
* Right-click the downloaded zip file, then click Extract Here
* open in QT Creator 5
* then push the F7 key.  When it is finished compiling, exit.
* open a terminal
    * cd to the directory where you extracted outputinspector such as:
      ```
      # If you want to install to /usr/local,
      # then do `su root` before attempting install below (or use
      # sudo ./install.sh instead).
      # If you are not named root, the default PREFIX is ~/.local
      # (you can also set the PREFIX environment variable manually).
      cd outputinspector
      # or
      cd outputinspector-master
      ```
      ```
      ./install.sh
      ```

### coding style
cd to project dir, then
```bash
clang-format -style=file mainwindow.cpp > mainwindow.cpp.tmp
meld mainwindow.cpp mainwindow.cpp.tmp
```

#### clang first-time setup
```bash
dnf -y install clang  # includes clang-format
dnf -y install meld
```

#### deciding on coding style
"$HOME/ownCloud/Documents/Programming/coding style/1.Coding Style poikilos.md" on main poikilos' computer

~~## Notes that only applied to qt4 version~~
~~* Compile First (if desired, but there is a binary built on Ubuntu Hardy):~~
~~* Build using qdevelop  (formerly required: qt4 libqt4-dev qdevelop):~~
~~`qdevelop outputinspector.pro`~~
~~* Install:~~
~~    Requires: qt4 libqt4 libqt4-gui libqt4-core~~
