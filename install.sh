#!/bin/bash
BIN_FILE_NAME=outputinspector
PROJECT_NAME=outputinspector
CONF_NAME=outputinspector.conf
# NOTE: $SRC_BINS_PATH cannot have spaces (usages can't use quotes since * is used)
SRC_BINS_PATH=./package/bin
echo
echo
echo
echo "Installing $PROJECT_NAME..."
echo
echo
echo "looking for instances to terminate..."
killall $BIN_FILE_NAME

if [ -z "$PREFIX" ]; then
    if [ "@$USER" == "@root" ]; then
        PREFIX=/usr/local
    else
        PREFIX=~/.local
        mkdir -p ~/.local/bin
    fi
fi

echo "Using prefix $PREFIX (export environment variable or use env before running to change)"
echo "such as:"
echo "  env PREFIX=/usr ./install"
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
    echo "WARNING: Found '$DEBUG_DIR/$BIN_FILE_NAME' (but not '$RELEASE_DIR/$BIN_FILE_NAME') so installing debug version"
  fi
else
  echo "Found '$RELEASE_DIR/$BIN_FILE_NAME'"
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

echo

was_there=false
if [ -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
  was_there=true
  rm -f "$BIN_DEST_DIR/$BIN_FILE_NAME"
  if [ -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
    echo
    echo
    echo "ERROR: install could not remove existing '$BIN_DEST_DIR/$BIN_FILE_NAME' since you don't have root permission (you must run this install script with root permissions)."
    echo
    echo
    exit 3
  fi
fi

cp -f "$BIN_FILE_PATH" "$BIN_DEST_DIR/"
if [ ! -f "$BIN_DEST_DIR/$BIN_FILE_NAME" ]; then
  echo
  echo
  #this error is always accurate since program would have exited already if file were already there before the cp command and couldn't be deleted:
  echo "ERROR: install could not create '$BIN_DEST_DIR/$BIN_FILE_NAME' since you don't have root permission (you must run this install script with root permissions)."
  echo
  echo
  exit 4
fi
echo

sCreate="no"
sExistNote="changed"
outputconf="/etc/$CONF_NAME"
if [ -f $outputconf ]; then
  sCreate="n"
  # echo "Do you want to create a new $outputconf[y/N]?"
else
  sExistNote="created"
  sCreate="y"
  # echo "Do you want to create a new $outputconf[Y/n]?"
fi


# read sCreate

if [ "${sCreate:0:1}" = "y" ]; then
echo
echo
echo "Resetting $outputconf"
#rm $outputconf
kate_path=$(command -v kate)
if [ -f "$kate_path" ]; then
  echo "kate=$kate_path" > $outputconf
else
  end_error="WARNING: path to kate could not be found. Kate must be installed in order for this program to work."
  echo $end_error
  echo "kate=/usr/bin/kate" > $outputconf
fi
#echo "kate=/usr/lib/kde4/bin/kate" >> $outputconf
#echo "xEditorOffset=-1" >> $outputconf
#echo "yEditorOffset=-1" >> $outputconf
echo "ExitOnNoErrors=no" >> $outputconf
echo "FindTODOs=yes" >> $outputconf
echo "Kate2TabWidth=8" >> $outputconf
echo "CompilerTabWidth=6" >> $outputconf
echo "ShowWarningsLast=yes" >> $outputconf
echo
echo "(you need root permissions to use this file)"
echo
echo "Result (to change, edit $outputconf):"
chmod a+rw $outputconf
cat $outputconf
echo
echo
else
echo "$outputconf not $sExistNote"
fi

if [ ! -f "$BIN_FILE_PATH" ]; then
  echo "$BIN_FILE_PATH not found!"
else
   echo "found $BIN_FILE_PATH"
fi

if [ -f "$BIN_DEST_FILE_PATH" ]; then
  echo "$BIN_FILE_NAME $uhoh $BIN_DEST_FILE_PATH"
  chmod a+x "$BIN_DEST_FILE_PATH"
fi

if [ -d "$SRC_BINS_PATH" ]; then
  chmod a+x $SRC_BINS_PATH/*
  cp $SRC_BINS_PATH/* $BIN_DEST_DIR/
fi
# else
#   echo "WARNING: missing '$SRC_BINS_PATH'"

echo
echo "TROUBLESHOOTING"
echo "If you have trouble with a release binary, open"
echo ".pro file in Qt Creator and build $BIN_FILE_NAME"
echo
echo "USAGE AFTER SUCCESSFUL INSTALL"
echo "In terminal, in project folder run something like:"
echo "     mcs AssemblyInfo.cs MainForm.cs 2>err.txt"
echo "     $BIN_FILE_NAME"
echo "     or"
echo "     jshint foo.js > err.txt"
echo "     $BIN_FILE_NAME"
echo " or your compiler or jshint output has"
echo " to be otherwise redirected to err.txt,"
echo " then run with that working directory:"
echo "     $BIN_FILE_NAME"
echo " from the same folder as the source code files and err.txt."
echo
echo "Then $BIN_FILE_NAME will automatically get working directory and"
echo " send the absolute path to kate upon double-clicking a line"
echo " from the error log using the outputinspector window."
echo
echo "DEVELOPMENT: err.txt (which is default, or file specified as"
echo " first param of outputinspector)"
echo " must be formatted exactly like one of these"
echo " (the purpose of $BIN_FILE_NAME is to tell Kate to go"
echo " to the file and location of warnings/errors such as):"
echo "   foo.js: line 32, col 26, reason"
echo "     or"
echo "   foo.cs(10,24): reason"
if [ ! -z "$end_error" ]; then
  #echo "WARNING: path to kate could not be found--install it or set the kate variable in $outputconf to full path of another binary which accepts -l <line> and -c <column> parameters."
  echo "$end_error"
else
  #NOTE: this is accurate since would have exited already if couldn't edit
  echo "Successfully installed."
  echo "NOTE: you can also use:"
  echo "  kate=/usr/bin/geany"
  echo "  # in '/etc/outputinspector.conf'"
  echo "  # (outputinspector knows how to tell Geany which line"
  echo "  # and column for jumping to line similar or same way as Kate)"
  echo "See above for other information."
fi
echo
echo
