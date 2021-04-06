#!/bin/bash
# ^ bash is required for the "if" syntax used here.
if [ -z "$PREFIX" ]; then
    PREFIX="/usr"
fi
package_name=outputinspector
REPO_PATH="`pwd`"
mkdir -p $PREFIX || exit 1

if [ -z "$DEBUG" ]; then
    DEBUG=false
fi
RUN_DEBUG=false
for arg in "$@"
do
    if [ "@$arg" == "@--debug" ]; then
        DEBUG=true
    elif [ "@$arg" == "@--run-debug" ]; then
        DEBUG=true
        RUN_DEBUG=true
    fi
done
OPTION1="-O2"
OPTION2=
OPTION3=

OUT_BIN=build/$package_name

if [ "@$DEBUG" = "@true" ]; then
    OPTION1="-g"
    #OPTION2="-DQT_QML_DEBUG"
    OPTION3="-DDEBUG=true"
    OUT_BIN=build/debug/$package_name
    mkdir -p "build/debug"
    echo "* build:Debug (`pwd`/$OUT_BIN)"
else
    echo "* build:Release (`pwd`/$OUT_BIN)"
fi
SYSTEM_INCDIR=$PREFIX/include
#IRR_INCDIR=
#IRR_LIBDIR=
# FT2_INCDIR=$PREFIX/include/freetype2
# FT2_INCDIR=$PREFIX/include/freetype2
#FT2_LIBDIR=
OBJDIR="./build/tmp"

# ^ build is in .gitignore

if [ ! -d "$OBJDIR" ]; then
    mkdir -p "$OBJDIR"
fi

# only for pc file if exists for irrlicht:
#if [ -z "$PKG_CONFIG_PATH" ]; then
#    PKG_CONFIG_PATH=$IRR_PATH
#else
#    PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$IRR_PATH
#fi
#gcc -o build/$package_name main.cpp Debug.cpp Engine.cpp EventHandler.cpp settings.cpp  UserInterface.cpp  Utility.cpp  View.cpp $(pkg-config --libs --cflags irrlicht --cflags freetype2)
#^ can't find a pc file
# gcc -o build/$package_name main.cpp Debug.cpp Engine.cpp EventHandler.cpp settings.cpp  UserInterface.cpp  Utility.cpp  View.cpp

# based on qtcreator's build after clean (see contributing.md; some options are unclear):
g++ -c -pipe $OPTION1 $OPTION2 $OPTION3 -fPIC -I$REPO_PATH -o $OBJDIR/main.o $REPO_PATH/main.cpp
if [ $? -ne 0 ]; then echo "Error: building main failed."; exit 1; fi
g++ -c -pipe $OPTION1 $OPTION2 $OPTION3 -fPIC -I$REPO_PATH -o $OBJDIR/mainwindow.o $REPO_PATH/mainwindow.cpp
if [ $? -ne 0 ]; then echo "Error: building mainwindow failed."; exit 1; fi
g++ -c -pipe $OPTION1 $OPTION2 $OPTION3 -fPIC -I$REPO_PATH -o $OBJDIR/settings.o $REPO_PATH/settings.cpp
if [ $? -ne 0 ]; then echo "Error: building settings failed."; exit 1; fi
#-w: suppress warning
# -I.: include the current directory (suppresses errors when using include < instead of include "
#-pipe: "Use pipes rather than intermediate files."
#Options starting with -g, -f, -m, -O, -W, or --param are automatically
# passed on to the various sub-processes invoked by g++.  In order to pass
# other options on to these processes the -W<letter> options must be used.
if [ -f "$$OUT_BIN" ]; then
    mv "$OUT_BIN" "$OUT_BIN.BAK"
    if [ $? -ne 0 ]; then
        echo "Error: 'mv \"$OUT_BIN\" \"$OUT_BIN.BAK\"' failed.."
        exit 1
    fi
fi
g++  -o $OUT_BIN $OBJDIR/main.o $OBJDIR/mainwindow.o $OBJDIR/settings.o -lstdc++fs
if [ $? -ne 0 ]; then
    cat <<END
Error: Linking failed. Ensure you have installed:
- irrlicht-devel and its dependencies: mesa-libGL-devel (requires libglvnd-devel which requires libX11-devel) libXxf86vm-devel
- libXcursor-devel
- freetype-devel
END
else
    echo "* linking succeeded."
fi
if [ ! -f "$OUT_BIN" ]; then
    echo "Error: $OUT_BIN couldn't be built."
    exit 1
else
    echo "Building $OUT_BIN is complete."
fi

INSTALLED_BIN="$HOME/.local/bin/$package_name"

if [ "@$RUN_DEBUG" = "@true" ]; then
    if [ "@$DEBUG" = "@true" ]; then
        ./debug-and-show-tb.sh
    else
        echo "Error: you specified the run option but debug mode is not enabled so ./debug-and-show-tb.sh will not work as expected. Try:"
        echo "  $0 --debug --run-debug"
        exit 1
    fi
else
    echo "  (add the --run-debug option to run it automatically)"
fi
if [ "@$DEBUG" != "@true" ]; then
    if [ -f "$INSTALLED_BIN" ]; then
        echo "* updating $INSTALLED_BIN..."
        ./$OUT_BIN --test-and-exit
        if [ $? -eq 0 ]; then
            # if no errors occur, install it
            rm "$INSTALLED_BIN"
            cp -f "$OUT_BIN" "$INSTALLED_BIN"
            if [ $? -eq 0 ]; then
                echo "* installed $INSTALLED_BIN successfully."
            else
                echo "* FAILED to install $INSTALLED_BIN."
            fi
        else
            echo "* skipping install since './$OUT_BIN' failed."
            echo
            if [ "@$DEBUG" != "@true" ]; then
                echo "* Build with debugging symbols like:"
                echo "  $0 --debug"
                echo "  # then:"
            fi
            echo "  # (See ./debug-and-show-tb.sh)"
            # cat ./debug-and-show-tb.sh
            # echo "  gdb \"$OUT_BIN\""
        fi
    else
        echo "* skipping \"$INSTALLED_BIN\" update since it doesn't exist"
    fi
fi
echo "Done"
