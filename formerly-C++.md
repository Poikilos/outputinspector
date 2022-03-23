I converted the C++ version to Python starting Mar 2021.
`cpp2python` didn't work well. It only converted formatting.


### Installing seasnake

The official instructions don't work (See "Stuff that didn't work").

Seasnake requires sealang, which is archived on GitHub. Use ptrstr's fork which uses an updated version of clang:
<https://github.com/ptrstr/sealang>. It requires "LLVM 11.0 (with clang)"

(instructions below are partly from the faulty instructions further down)
```
sudo apt-get install zlib1g-dev libncurses5-dev -y
#export LLVM_HOME=/usr/lib/llvm-6.0
#export LD_LIBRARY_PATH=$LLVM_HOME/lib
# ^ fix missing LLVM or LLVM_HOME error with pip install
#sudo apt install libclang1-6.0 clang-6.0
#sudo apt-get install libclang-6.0-dev clang-6.0-dev -y
# ^ avoid clang 6.0 due to doc/development/clang-6.0-error.txt
sudo apt remove libclang1-6.0 clang-6.0 libclang-6.0-dev clang-6.0-dev -y
sudo apt autoremove -y
# ^ remove llvm 6.0 etc
# sudo apt install libclang1-8 clang-8 libclang-8-dev -y -t buster-backports
# ^ fix missing clang/AST/OperationKinds.h error with pip install
# export LLVM_HOME=/usr/lib/llvm-8
# export LD_LIBRARY_PATH=$LLVM_HOME/lib
# ^ Set exports as per the readme to fix missing LLVM or LLVM_HOME error with pip install
# python3 -m pip install --user https://github.com/ptrstr/sealang/archive/refs/heads/master.zip
# To avoid /doc/development/clang-8-error.txt, try clang 11 as recommended.
sudo apt remove libclang1-8 clang-8 libclang-8-dev -y
# as per <https://stackoverflow.com/questions/66223241/how-to-install-clang-11-on-debian>:
cat | sudo tee -a /etc/apt/sources.list <<END

# LLVM 11 as per <https://stackoverflow.com/questions/66223241/how-to-install-clang-11-on-debian> such as for <https://github.com/ptrstr/sealang>:
deb http://apt.llvm.org/buster/ llvm-toolchain-buster main
deb-src http://apt.llvm.org/buster/ llvm-toolchain-buster main
deb http://apt.llvm.org/buster/ llvm-toolchain-buster-10 main
deb-src http://apt.llvm.org/buster/ llvm-toolchain-buster-10 main
deb http://apt.llvm.org/buster/ llvm-toolchain-buster-11 main
deb-src http://apt.llvm.org/buster/ llvm-toolchain-buster-11 main
END


wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -


sudo apt-get update
sudo apt-get install -y clang-11
export CMAKE_C_COMPILER=clang-11
export CMAKE_CXX_COMPILER=clang++-11
# ^ as per <https://stackoverflow.com/a/66227796/4541104>

sudo apt-get install -y libclang1-11 libclang-11-dev
# ^ avoid doc/development/clang-11-error.txt

export LLVM_HOME=/usr/lib/llvm-11
export LD_LIBRARY_PATH=$LLVM_HOME/lib

python3 -m pip install --user https://github.com/ptrstr/sealang/archive/refs/heads/master.zip

```

### Stuff that didn't work

[rkantos commented on Jan 27, 2019](https://github.com/mitsuhiko/pipsi/issues/134#issuecomment-457951662) says to first install sealang manually using the llvm env install instructions at:

<https://github.com/pybee/sealang>

However, `sudo apt-get install libclang-3.6 clang-3.6 -y` says:
```
Note, selecting 'librust-clang-sys-0+clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-0.27.0+gte-clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-0.27+gte-clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-0+gte-clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys+clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-0.27.0+clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-0.27+clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys+gte-clang-3-6-dev' for regex 'clang-3.6'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys+gte-clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys+clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0+clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0+gte-clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0.27+clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0.27+gte-clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0.27.0+clang-3-6-dev'
Note, selecting 'librust-clang-sys-dev' instead of 'librust-clang-sys-0.27.0+gte-clang-3-6-dev'
E: Unable to locate package libclang-3.6
E: Couldn't find any package by glob 'libclang-3.6'
E: Couldn't find any package by regex 'libclang-3.6'
```

so:
```
sudo apt-get install libclang clang
```

can't fint libclang.

`sudo apt search clang` has only much later versions:

```
libclang-6.0-dev/stable 1:6.0.1-10 amd64
  clang library - Development package

libclang-7-dev/stable 1:7.0.1-8+deb10u2 amd64
  Clang library - Development package

libclang-8-dev/buster-backports 1:8.0.1-3~bpo10+1 amd64
  Clang library - Development package

libclang-common-6.0-dev/stable 1:6.0.1-10 amd64
  clang library - Common development package

libclang-common-7-dev/stable 1:7.0.1-8+deb10u2 amd64
  Clang library - Common development package

libclang-common-8-dev/buster-backports 1:8.0.1-3~bpo10+1 amd64
  Clang library - Common development package

libclang-dev/stable 1:7.0-47 amd64
  clang library - Development package

libclang-perl/stable 0.09-4+b9 amd64
  Perl bindings to the Clang compiler's indexing interface

libclang1/stable 1:7.0-47 amd64
  C, C++ and Objective-C compiler (LLVM based)

libclang1-6.0/stable 1:6.0.1-10 amd64
  C interface to the clang library

libclang1-7/stable 1:7.0.1-8+deb10u2 amd64
  C interface to the Clang library

libclang1-8/buster-backports 1:8.0.1-3~bpo10+1 amd64
  C interface to the Clang library
```

so try:

```
sudo apt install libclang1-6.0 clang-6.0
```

Then continue the instructions (but change version to 6.0):
```
sudo apt-get install zlib1g-dev libncurses5-dev -y
export LLVM_HOME=/usr/lib/llvm-6.0
export LD_LIBRARY_PATH=$LLVM_HOME/lib
pip install --user sealang
```

result:
```
x86_64-linux-gnu-gcc -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fno-strict-aliasing -Wdate-time -D_FORTIFY_SOURCE=2 -g -fdebug-prefix-map=/build/python2.7-2.7.16=. -fstack-protector-strong -Wformat -Werror=format-security -fPIC -I/usr/include/python2.7 -c sealang/sealang.cpp -o build/temp.linux-x86_64-2.7/sealang/sealang.o -I/usr/lib/llvm-6.0/include -std=c++0x -fuse-ld=gold -Wl,--no-keep-files-mapped -Wl,--no-map-whole-files -fPIC -fvisibility-inlines-hidden -Werror=date-time -std=c++11 -Wall -W -Wno-unused-parameter -Wwrite-strings -Wcast-qual -Wno-missing-field-initializers -pedantic -Wno-long-long -Wno-maybe-uninitialized -Wdelete-non-virtual-dtor -Wno-comment -ffunction-sections -fdata-sections -O2 -DNDEBUG -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
    cc1plus: warning: command line option ‘-Wstrict-prototypes’ is valid for C/ObjC but not for C++
    In file included from sealang/sealang.cpp:1:
    sealang/sealang.h:6:10: fatal error: clang/AST/OperationKinds.h: No such file or directory
     #include "clang/AST/OperationKinds.h"
              ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
    compilation terminated.
    error: command 'x86_64-linux-gnu-gcc' failed with exit status 1

    ----------------------------------------
Command "/usr/bin/python -u -c "import setuptools, tokenize;__file__='/tmp/pip-install-0tB9Sb/sealang/setup.py';f=getattr(tokenize, 'open', open)(__file__);code=f.read().replace('\r\n', '\n');f.close();exec(compile(code, __file__, 'exec'))" install --record /tmp/pip-record-w6UTnY/install-record.txt --single-version-externally-managed --compile --user --prefix=" failed with error code 1 in /tmp/pip-install-0tB9Sb/sealang/

```

So try it on Ubuntu 18.04.5 LTS (Bionic Beaver):
```
sudo apt-get install libclang-3.6 clang-3.6
```

It still fails:
```
Note, selecting 'python-clang-3.6' for regex 'clang-3.6'
E: Unable to locate package libclang-3.6
E: Couldn't find any package by glob 'libclang-3.6'
E: Couldn't find any package by regex 'libclang-3.6'
```

Note that <https://github.com/pybee/sealang> only mentions Trusty and Xenial.


```
cp -R ~/git/outputinspector-qt ~/git/outputinspector-py
sudo apt install python-pip
# pip install seasnake
pip install --user https://github.com/pybee/sealang/archive/refs/heads/master.zip
mkdir -p ~/tmp
cd ~/tmp
seasnake -s ~/git/outputinspector-py
```

#### Problems attempted to solve above:

seasnake won't install via:

- `python3 -m pip install --user seasnake` says:
```
ERROR: Could not find a version that satisfies the requirement seasnake (from versions: none)
ERROR: No matching distribution found for seasnake
WARNING: You are using pip version 20.0.2; however, version 21.0.1 is available.
You should consider upgrading via the '/usr/bin/python3 -m pip install --upgrade pip' command.
```

- `python -m pip install --user seasnake` (Python 2 since using Debian 10) says:

```
Collecting seasnake
  Cache entry deserialization failed, entry ignored
  Could not find a version that satisfies the requirement seasnake (from versions: )
No matching distribution found for seasnake
```

- `python3 -m pip install --user https://github.com/pybee/seasnake/archive/refs/heads/master.zip`

says:
```
Collecting https://github.com/pybee/seasnake/archive/refs/heads/master.zip
  Downloading https://github.com/pybee/seasnake/archive/refs/heads/master.zip
     | 53 kB 4.3 MB/s
Collecting sealang
  Downloading sealang-3.9.dev259750.tar.gz (49 kB)
     |████████████████████████████████| 49 kB 651 kB/s
ERROR: Files/directories not found in /tmp/pip-install-jbh9m_vs/sealang/pip-egg-info

```

- `python -m pip install --user https://github.com/pybee/seasnake/archive/refs/heads/master.zip`
  (Python 2 since using Debian 10)
  says:
  ```
Collecting https://github.com/pybee/seasnake/archive/refs/heads/master.zip
  Cache entry deserialization failed, entry ignored
  Downloading https://github.com/pybee/seasnake/archive/refs/heads/master.zip
     | 112kB 4.9MB/s
Collecting sealang (from seasnake==0.0.0)
  Cache entry deserialization failed, entry ignored
  Cache entry deserialization failed, entry ignored
  Downloading https://files.pythonhosted.org/packages/50/f6/dbae5a114494f2d7dc4a3a410f8bed187ef9a30856d3e10fb60ef7a2ce79/sealang-3.9.dev259750.tar.gz (49kB)
    100% |████████████████████████████████| 51kB 2.4MB/s
Files/directories not found in /tmp/pip-install-g_VeZR/sealang/pip-egg-info
```


## Developer Notes
### Compiling
* Ensure that qt is installed, not just Qt designer
  - On Linux: install a package such as `qt-devel` on Fedora
    (and possibly `qt5-devel` if it is not installed automatically,
    which should pull in `qt5-qtbase-devel`) or `qtbase5-dev` on Debian
    or Ubuntu as per Henk van der Laak's Nov 20, 2014 answer on
    <https://askubuntu.com/questions/508503/whats-the-development-package-for-qt5-in-14-04>.
    Also, possibly for other projects but not this one,
    `qtdeclarative5-dev` as per AlexGreg's  Aug 8, 2014 answer there.
  - On Windows: Go to Apps & Features, Qt, Change, and check the latest
    Qt (it will install required components such as MinGW compiler).
    - Static building (`windows-build-qt-static.ps1`) is not working
      until
      [Issue #20](https://github.com/poikilos/outputinspector/issues/20)
      is resolved (but feel free to try it on your system--you may have
      to modify it to download the version of Qt matching your Qt
      Creator installation (See doc\development\windeployqt_exe.png
      for examples using Qt 5.15.2).
* Open "Qt Creator"
* Right-click the downloaded zip file, then click Extract Here
* Open the `outputinspector.pro` in QT Creator.
* Push the F7 key.  When it is finished compiling, exit.
* Install
  * Windows:
    The Windows build (even regular not static) isn't working right now
    since it only runs when you push "Run" not when you double click it
    and instead shows an error--until [Issue
    #21](https://github.com/poikilos/outputinspector/issues/21)
    is resolved.
    - Tried: Find your version of windeployqt.exe that matches your
      configuration, and run it on the built exe.
      For example, if you are using MinGW 64-bit and you made a release
      build , run:
      `C:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe C:\Users\Jatlivecom\GitHub\build-outputinspector-Desktop_Qt_5_15_2_MinGW_64_bit-Release\release\outputinspector.exe`
  * Linux: Open a terminal and enter the following:
```
# If you are not named root, the default PREFIX is ~/.local
# (you can also set the PREFIX environment variable manually).
cd outputinspector
# or
cd outputinspector-master
# If you want to install to /usr/local,
# then do `su root` before attempting install below (or use
# sudo ./install.sh instead).
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
