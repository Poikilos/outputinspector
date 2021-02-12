#!/bin/bash
BIN_FILE_NAME=outputinspector
PROJECT_NAME=outputinspector
SC_NAME=$PROJECT_NAME.desktop
CONF_NAME=outputinspector.conf
# NOTE: $SRC_BINS_PATH cannot have spaces (usages can't use quotes since * is used)
SRC_BINS_PATH=./package/bin
ICON="outputinspector-64.png"
echo
echo
echo
echo "Installing $PROJECT_NAME..."
echo
echo
printf "* looking for instances to terminate..."
killall $BIN_FILE_NAME

printf "* checking environment..."
if [ -z "$PREFIX" ]; then
    if [ "@$USER" == "@root" ]; then
        PREFIX=/usr/local
        echo "PREFIX=$PREFIX"
        echo "# ^ defaulted to this since no PREFIX and user is root"
    else
        PREFIX=~/.local
        mkdir -p ~/.local/bin
        echo "PREFIX=$PREFIX"
        echo "# ^ defaulted to this since no PREFIX and user is not root"
    fi
else
    echo "PREFIX=$PREFIX"
    echo "# ^ as specified by environment"
fi
PIXMAPS="$PREFIX/share/pixmaps"
APPLICATIONS="$PREFIX/share/applications"
DST_ICON_PATH="$PIXMAPS/$ICON"
DST_SC_PATH="$APPLICATIONS/$SC_NAME"

echo "# ^ export environment variable or use env before running to change,"
echo "# such as via:"
echo "  export PREFIX=/usr ./install"
BIN_DEST_DIR=$PREFIX/bin
#remember, project-specific info is also at end of this script (post-install notes)
BIN_CONFLICT_DIR=/usr/bin
if [ -f "$BIN_CONFLICT_DIR/$BIN_FILE_NAME" ]; then
    echo
    echo
    echo "You are installing the source version of $PROJECT_NAME, so you must first remove the packaged version of $PROJECT_NAME to avoid conflicting file '$BIN_CONFLICT_DIR/$BIN_FILE_NAME'--if you used an earlier version of $PROJECT_NAME, you may have this file from a installer that was not yet up to standards so before running install again as root you will have to run:"
    echo "su root"
    echo "rm -f /usr/bin/$BIN_FILE_NAME"
    echo
    echo
    exit 1
fi

uhoh="is in"
DEBUG_DIR="../build-$PROJECT_NAME-Desktop-Debug"
RELEASE_DIR="../build-$PROJECT_NAME-Desktop-Release"
if [ -f "outputinspector" ]; then
    #echo "ERROR: Nothing done for install since there is an ambiguous file named outputinspector here (it should instead be in '$RELEASE_DIR' or '$DEBUG_DIR')"
    #exit 1
    echo "WARNING: install found an ambiguous file named outputinspector here (it should instead be in '$RELEASE_DIR' or '$DEBUG_DIR')"
    sMove="no"
    #echo "Do you want to move it to Releases and try to continue (y/N)? "
    #read sMove
    sMove="yes"
    if [ "${sMove:0:1}" = "y" ]; then
        if [ ! -d "$RELEASE_DIR" ]; then
            mkdir -p "$RELEASE_DIR"
        fi
        mv -f outputinspector "$RELEASE_DIR/"
    else
        exit 2
    fi
fi
SOURCE_BIN_DIR="$RELEASE_DIR"
printf "* looking for a built binary..."
if [ ! -f "$RELEASE_DIR/$BIN_FILE_NAME" ]; then
    SOURCE_BIN_DIR="$DEBUG_DIR"
    if [ ! -f "$DEBUG_DIR/$BIN_FILE_NAME" ]; then
        if [ -f "./$BIN_FILE_NAME" ]; then
            SOURCE_BIN_DIR=.
            echo "Using only copy of $BIN_FILE_NAME which is in current directory '.'"
        else
            echo
            echo "Could not find '$DEBUG_DIR/$BIN_FILE_NAME' nor '$RELEASE_DIR/$BIN_FILE_NAME' so cannot install--$PROJECT_NAME.pro must be first built using Qt Creator 5, install.sh, or the binary must be built by other means and placed in one of those folders or the same folder as this script."
            echo
            echo
            exit 2
        fi
    else
        echo "WARNING: Found \"$DEBUG_DIR/$BIN_FILE_NAME\" (but not \"$RELEASE_DIR/$BIN_FILE_NAME\") so installing debug version"
    fi
else
    echo "FOUND (\"$RELEASE_DIR/$BIN_FILE_NAME\")"
fi

#if both exist, do not use one that is -ot (older than) other one
if [ -f "$RELEASE_DIR/$BIN_FILE_NAME" ]; then
    if [ -f "$DEBUG_DIR/$BIN_FILE_NAME" ]; then
        if [ "$DEBUG_DIR/$BIN_FILE_NAME" -ot "$RELEASE_DIR/$BIN_FILE_NAME" ]; then
            SOURCE_BIN_DIR="$RELEASE_DIR"
            echo "Using newer install source in '$RELEASE_DIR'"
        else
            echo "WARNING: Using newer install source in '$DEBUG_DIR'"
        fi
    fi
fi
BIN_FILE_PATH=$SOURCE_BIN_DIR/$BIN_FILE_NAME
#cp -f ./bin/$BIN_FILE_NAME $BIN_DEST_DIR/
BIN_DEST_FILE_PATH=$BIN_DEST_DIR/$BIN_FILE_NAME

if [ -f "$BIN_DEST_FILE_PATH" ]; then
    uhoh="was already installed: "
fi

was_there=false
if [ -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
    was_there=true
elif [ -L "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
    echo "* detected symlink $BIN_DEST_DIR/$BIN_FILE_NAME"
    was_there=true
else
    echo "* adding new $BIN_DEST_DIR/$BIN_FILE_NAME..."
fi
if [ "@$was_there" = "@true" ]; then
    printf "* removing old $BIN_DEST_DIR/$BIN_FILE_NAME..."
    rm -f "$BIN_DEST_DIR/$BIN_FILE_NAME"
    if [ -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
        echo "FAILED"
        echo
        echo "ERROR: install could not remove existing '$BIN_DEST_DIR/$BIN_FILE_NAME'"
        echo
        echo
        exit 3
    else
        echo "OK"
    fi
fi
printf "* installing the binary to $BIN_DEST_DIR/..."
cp -f "$BIN_FILE_PATH" "$BIN_DEST_DIR/"
if [ ! -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
    echo
    echo
    #this error is always accurate since program would have exited already if file were already there before the cp command and couldn't be deleted:
    echo "ERROR: install could not create '$BIN_DEST_DIR/$BIN_FILE_NAME'. via 'cp -f \"$BIN_FILE_PATH\" \"$BIN_DEST_DIR/\"'"
    echo
    echo
    exit 4
else
    echo "OK"
fi

if [ -z "$RESET_CONF" ]; then
    RESET_CONF="no"
    sExistNote="changed"
    outputconf="/etc/$CONF_NAME"
    if [ -f $outputconf ]; then
        RESET_CONF="n"
        # echo "Do you want to create a new $outputconf[y/N]?"
    else
        sExistNote="created"
        RESET_CONF="y"
        # echo "Do you want to create a new $outputconf[Y/n]?"
    fi
fi

# read RESET_CONF
printf "* detecting $outputconf..."
# if [ "${RESET_CONF:0:1}" = "y" ]; then
if [ "@$RESET_CONF" = "@y" ] || [ "@$RESET_CONF" = "@yes" ]; then
    echo
    echo
    echo "FOUND"
    echo "  * resetting $outputconf..."
    # rm $outputconf
    kate_path=$(command -v kate)
    geany_path=$(command -v geany)
    if [ -f "$kate_path" ]; then
        echo "kate=$kate_path" > $outputconf
    elif [ -f "$geany_path" ]; then
        echo "kate=$geany_path" > $outputconf
    else
        end_error="WARNING: path to kate could not be found. Kate or Geany must be installed in order for this program to work."
        echo $end_error
        echo "kate=/usr/bin/kate" > $outputconf
    fi
    # echo "kate=/usr/lib/kde4/bin/kate" >> $outputconf
    # echo "xEditorOffset=-1" >> $outputconf
    # echo "yEditorOffset=-1" >> $outputconf
    echo "ExitOnNoErrors=no" >> $outputconf
    echo "FindTODOs=yes" >> $outputconf
    echo "Kate2TabWidth=8" >> $outputconf
    echo "CompilerTabWidth=6" >> $outputconf
    echo "ShowWarningsLast=yes" >> $outputconf
    echo
    echo "    The owner of this file is: `stat -c '%U' install.sh`"
    echo
    echo "  * result (to change, edit $outputconf):"
    chmod a+rw $outputconf
    cat $outputconf
    echo
    echo
else
    echo "$outputconf not $sExistNote since it exists (and RESET_CONF is neither 'yes' nor 'y')"
    # ^ such as /etc/outputinspector.conf not changed
fi

printf "* verifying binary..."
if [ ! -f "$BIN_FILE_PATH" ]; then
    echo "ERROR: \"$BIN_FILE_PATH\" not found!"
else
    echo "OK (found \"$BIN_FILE_PATH\")"
fi

printf "* checking for existing $BIN_DEST_FILE_PATH..."

if [ -f "$BIN_DEST_FILE_PATH" ]; then
    printf "$BIN_FILE_NAME $uhoh $BIN_DEST_FILE_PATH..."
    chmod a+x "$BIN_DEST_FILE_PATH"
    if [ $? -ne 0 ]; then
        echo "FAILED ('chmod a+x \"$BIN_DEST_FILE_PATH\"')"
    else
        echo "OK"
    fi
else
    echo "OK ($BIN_DEST_FILE_PATH does not exist--this is not a problem)"
fi

printf "* checking for $SRC_BINS_PATH..."
if [ -d "$SRC_BINS_PATH" ]; then
    printf "setting permissions..."
    chmod a+x $SRC_BINS_PATH/*
    if [ $? -ne 0 ]; then
        echo "FAILED ('chmod a+x $SRC_BINS_PATH/*' in `pwd`)"
    else
        echo "OK"
    fi
    printf "  * copying to $BIN_DEST_DIR/..."
    cp $SRC_BINS_PATH/* $BIN_DEST_DIR/
    if [ $? -ne 0 ]; then
        echo "FAILED ('cp $SRC_BINS_PATH/* $BIN_DEST_DIR/' in `pwd`)"
    else
        echo "OK"
    fi
fi
# else
#     echo "WARNING: missing '$SRC_BINS_PATH'"

printf "* creating \"$PIXMAPS\"..."
mkdir -p "$PIXMAPS"
if [ $? -ne 0 ]; then
    echo "'mkdir -p \"$PIXMAPS\"' failed."
    exit 1
else
    echo "OK"
fi
cp "$ICON" "$DST_ICON_PATH"
if [ $? -ne 0 ]; then
    echo "'cp \"$ICON\" \"$DST_ICON_PATH\"' failed."
    exit 1
fi

# TMP_ICON="/tmp/$ICON"
# printf "* writing icon $TMP_ICON..."
# cat > "$TMP_ICON" <<END

printf "* writing icon $DST_SC_PATH..."
cat > "$DST_SC_PATH" <<END
[Desktop Entry]
Name=Output Inspector
Exec=outputinspector %f
Type=Application
Icon=$DST_ICON_PATH
END
#Terminal=true
if [ $? -ne 0 ]; then
    echo "'writing \"$DST_SC_PATH\"' failed."
    exit 1
else
    echo "OK"
fi
printf "* setting as executable..."
chmod +x "$DST_SC_PATH"
if [ $? -ne 0 ]; then
    echo "'chmod +x \"$DST_SC_PATH\"' failed."
    exit 1
else
    echo "  OK"
fi
echo "* installing icon..."
export XDG_UTILS_DEBUG_LEVEL=999
xdg-desktop-icon install --novendor "$DST_SC_PATH"
# ^ --novendor: install even if name doesn't start with tld.domain.
if [ $? -ne 0 ]; then
    echo "'xdg-desktop-icon install --novendor \"$DST_SC_PATH\"' failed."
    exit 1
else
    echo "  OK"
fi
echo "* installing ogrep..."
cp "./package/bin/ogrep" "$HOME/.local/bin/ogrep"
if [ $? -ne 0 ]; then
    echo "'cp \"./package/bin/ogrep\" \"$HOME/.local/bin/ogrep\"' failed."
    exit 1
else
    echo "  OK"
fi

cat <<END


TROUBLESHOOTING
If you have trouble with a release binary, open
.pro file in Qt Creator and build $BIN_FILE_NAME

USAGE AFTER SUCCESSFUL INSTALL
In terminal, in project folder run something like:
     mcs AssemblyInfo.cs MainForm.cs 2>err.txt
     $BIN_FILE_NAME
     or
     jshint foo.js > err.txt
     $BIN_FILE_NAME
 or your compiler or jshint output has
 to be otherwise redirected to err.txt,
 then run with that working directory:
     $BIN_FILE_NAME
 from the same folder as the source code files and err.txt.

Then $BIN_FILE_NAME will automatically get working directory and
 send the absolute path to kate upon double-clicking a line
 from the error log using the outputinspector window.

DEVELOPMENT: err.txt (which is default, or file specified as
 first param of outputinspector)
 must be formatted exactly like one of these
 (the purpose of $BIN_FILE_NAME is to tell Kate to go
 to the file and location of warnings/errors such as):
   foo.js: line 32, col 26, reason
     or
   foo.cs(10,24): reason
END
if [ ! -z "$end_error" ]; then
    # echo "WARNING: path to kate could not be found--install it or set the kate variable in $outputconf to full path of another binary which accepts -l <line> and -c <column> parameters."
    echo "$end_error"
else
    # NOTE: this is accurate since would have exited already if couldn't edit
    cat <<END
The $BIN_FILE_NAME is successfully installed at $BIN_DEST_DIR.
NOTE: you can also use:
  editor=/usr/bin/geany
  # or editor=/usr/bin/kate
  # in '\$HOME/.local/share/outputinspector/settings.txt'
  # (outputinspector knows how to tell Geany which line
  # and column for setting your cursor position like with Kate)
See above for other information.
END
fi
echo
echo
