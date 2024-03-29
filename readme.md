# Output Inspector

Output Inspector makes errors clickable so you can get to the issue in
your software's source code instantly. Get the popular feature included
in many IDEs independently of an IDE. Output Inspector will even look
for TODOs in comments for various source code files cited by the log!
Output Inspector parses your parser output.

![Screenshot](doc/images/screenshot.jpg)

On Linux, you can examine output in near-realtime (or non-realtime on
Windows) using pipes instead of a log file. You can use the included
"passthrough-outputinspector" script to automatically run
outputinspector in Geany when you press the execute button. You can
even use the included `ogrep` instead of grep then double click on a
result to go to the line in the matching file! One of many
possibilities is redirecting the output of many style checker commands
to one file, then seeing the code for your entire project just like in
a popular IDE. This becomes possible for any scenario now, so you do
not have to work with an IDE you don't like for some language you are
using if all you want is fast code navigation during debugging.


## Primary Features
* Double-click an error to go to where the error occurs in your code.
* Color code lines in your output (red: error; orange: warning; yellow:
  issue in installed library used [if in site-packages]; black:
  formatting marks; gray: unrecognized information)
  * Detect flags in your output (such as `Warning` or `Error`).
* Detect flags in files cited by your output: `TODO` or `FIXME` (in
  inline comments). Predicing inline comment marks is possible when
  the file uses one of the following extensions: `py`, `pyw`, `sh`, `c`,
  `h`, `cpp`, `hpp`, `js`, `java`, `php`, `bat`, `command`.
* Install passthrough-outputinspector for use in IDEs (see Usage).


## How to Use
Before first use, make sure kate or geany package is installed (run
install script again if wasn't when install script ran--it recreates
the config based on detecting kate's location if you enter `y` for yes).

## Planned features
### Piping
Pipe output from another program to get near-realtime (depending
on your OS's implementation of piping) results without using files.

To get error output, run: `... 2>&1 >/dev/null | outputinspector`;
otherwise, simply run: `... | outputinspector`
(where "..." is your program).

### GUI-based use
For automatic usage on Linux, create a build command in Geany:
* Place passthrough-outputinspector in ~/.local/bin
* In a terminal, run `echo "PATH=$PATH:$HOME/.local/bin" >> ~/.bashrc`
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

#### C++
##### Backtracing
(Parsing output such as in tests/data/gdb-backtrace.txt using the Python version of outputinspector is not available at least until the python branch becomes the default branch)
1. Compile your program with the option to add debugging symbols.
2. In a Terminal, set `MY_EXE` to the path to your debug executable.
3. In the same terminal run the following:
```
gdb "$MY_EXE" --eval-command=run --eval-command=bt --batch  >& err.txt
outputinspector &
```

#### Python linting
For py file linting: you can use pycodestyle (tested with pycodestyle-3
command--package may be named python3-pycodestyle in your Linux distro):
```
pycodestyle-3 > err.txt
outputinspector &
```

#### C#
For cs files, you have to run outputinspector from the location of the cs
files you are compiling, and your compiler error output has to be redirected
to err.txt.\
Example:
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

#### Python Nose Tests
```
nosetests 1>out.txt 2>err.txt
outputinspector &
```

#### "JavaScript" (Node.js or ECMAScript) linting using jshint
The jshint package helps check js files by providing the jshint command.
If you need to check an entire project at once, append them all to the
same file. You can automatically use err.txt, or specify a file:
```
echo "" > err.txt # erase it first
jshint app.js >> err.txt
jshint config.js >> err.txt
outputinspector &
```

or

```
echo "" > issues.txt # erase it first
jshint app.js >> issues.txt
jshint config.js >> issues.txt
outputinspector issues.txt &
```

Your source file should not have any unsaved changes in any other program at
the time (it is ok if in Geany or Kate--whichever outputinspector is
using--but saving first and using your parser on that
version is recommended for accuracy).

Other than jshint output, Output Inspector has only been tested on `mcs` (mono
compiler) output, but may work for any C# compiler and will work for any
compiler or other tool using the formatting above.

The parsing is not fault-tolerant at this time, especially for the first type of
formatting.

Output of jshint is expected unless the second formatting is used by your parser
(such as `mcs`).
* OPTIONAL: To use Geany, set: `editor=/usr/bin/geany` in
  '$HOME/.local/share/outputinspector/settings.txt"
  (outputinspector knows how to tell Geany which
  line and column for jumping to line by using args compatible with both
  Geany and Kate)"
* You can set any of the ini options as command line options (case sensitive).
  For a list of settings, see
  "$HOME/.local/share/outputinspector/settings.txt" after running then
  closing the program, or see "etc/outputinspector.example.conf"\
  - Here is an example (which overrides a setting from the settings
    file):\
    `outputinspector --ExitOnNoErrors=yes` (or `true` or `on` or `1`)


##### Overview of jshint
Usually from the nodejs-jshint package, jshint is a linting and/or
hinting tool for javascript (especially node.js) which is considered a
successor to jslinter. Here is the timeline:
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
    * `/usr/lib/python2.7/site-packages/`
      * `/usr/lib/python2.7/site-packages/kate_plugins/`
      * `/usr/lib/python2.7/site-packages/Kate_plugins-0.2.3-py2.7.egg-info`
      * `/usr/lib/python2.7/site-packages/pyjslint-0.3.3-py2.7.egg-info/`
      * or any of their subfolders.
    * A filename search for jslint in `/usr/lib/python2.7/site-packages`
      yields no binaries or files other than those in the folders above.

#### Minetest Lua tracebacks
Output Inspector un-mangles paths with an ellipsis!
```
2020-03-13 03:15:17: ERROR[Main]: ServerError: AsyncErr: environment_Step: Runtime error from mod 'unified_foods' in callback environment_Step(): ...../gameshunger.lua:342: attempt to compare number with nil
2020-03-13 03:15:17: ERROR[Main]:       ...../games/ENLIVEN/mods/coderfood/unified_foods/hunger.lua:342: in function <...../games/ENLIVEN/mods/coderfood/unif
```
`*.../dir` becomes `../dir`, resulting in a path such as:
`../games/ENLIVEN/mods/coderfood/unified_foods/hunger.lua` (this will
work if you run outputinspector from the same directory as the program
you ran. If the path exists in `.` then the path will also be
transformed correctly to something like
`games/ENLIVEN/mods/coderfood/unified_foods/hunger.lua`.


## Changes
See [changelog.md](changelog.md).

The program was formerly maintained in C++ but now in Python.

See [formerly-C++.md](formerly-C++.md).

### Backward Compatibility
* Remember to edit `$HOME/.local/share/outputinspector/settings.txt`
  specifying the kate binary (i.e.
  include the line `kate=/usr/bin/kate` such as for Fedora 25 or
  `editor=/usr/lib/kde4/bin/kate` or appropriate command for older linux distro
  such as Ubuntu Hardy). You can also use Geany's path if it exists.
* Tab handling:  If you are using `mcs` 1.2.6 or other compiler that reads tabs
  as 6 spaces, and you are using Kate 2 with the default tab width of 8 or are
  using Kate 3, you don't have to change anything.  Otherwise:
  * Set CompilerTabWidth and Kate2TabWidth in
    `$HOME/.local/share/outputinspector/settings.txt` -- kate
    3.0.0+ is handled automatically, but CompilerTabWidth may have to be set.
    If the compiler treats tabs as 1 character, make sure you set
    `CompilerTabWidth=1` -- as of 1.2.6, `mcs` counts tabs as 6 spaces
    (outputinspector default).
* If the `editor` setting is not present but the `kate` setting is, the
  program will copy the setting to the new `editor` variable in the conf
  file.

## References
- Urban, M., & Murach, J. (2016). Murach’s Python Programming
  [VitalSource Bookshelf]. Retrieved from
  https://bookshelf.vitalsource.com/#/books/9781943872152-
