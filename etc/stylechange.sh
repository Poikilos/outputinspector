#!/bin/sh
# the path actually used for the operation:
echo
fmt="$1"
src_path="mainwindow.cpp"
echo "1st param (coding format based on): $fmt"
echo "2nd param (source file [default: $src_path]): $2"
if [ ! -z "$2" ]; then
  src_path="$2"
fi
if [ ! -f "$src_path" ]; then
  if [ -f "../$src_path" ]; then
    cd ..
    echo "operating on source file $src_path"
  fi
else
  echo "operating on source file $src_path"
fi
if [ ! -f "$src_path" ]; then
  echo "Source file $src_path was not found. You may be in the wrong directory, so quitting."
  echo
  echo
  exit 1
fi
if [ ! -f "`command -v clang-format`" ]; then
  install_bin="apt-get"
  if [ -f "command -v apt" ]; then install_bin="apt"; fi
  if [ -f "command -v yum" ]; then install_bin="yum"; fi
  if [ -f "command -v dnf" ]; then install_bin="dnf"; fi
  echo "You must first install clang-format command such as:"
  echo "  $install_bin install clang"
  echo
  echo
  exit 2
fi
if [ -z "$fmt" ]; then
  echo "You must specify a coding style (clang format) such as:"
  clang-format --help | grep LLVM
  echo
  echo
  exit 3
fi

# the temporary config file (plain text, no extension)

default_fmt_path="../style-$fmt.clang-format"
alt_fmt_path="../style-$fmt-customized.clang-format"
fmt_path="$default_fmt_path"
custom_flag=""
if [ -f "$alt_fmt_path" ]; then
  fmt_path="$alt_fmt_path"
  custom_flag="-customized"
fi

# the new cpp file
dest_name="$src_path.$fmt$custom_flag.cpp"
dest_path="../$dest_name"

if [ ! -f "$fmt_path" ]; then
  echo "Missing '$fmt_path'."
  echo "First create a sample file"
  echo "  or generate it like:"
  echo
  echo "    cd `pwd`"
  echo "    clang-format -style=$fmt -dump-config > $alt_fmt_path"
  echo "    # or (filename also detected by this script)"
  echo "    # clang-format -style=$fmt -dump-config > $default_fmt_path"
  echo "    # -style=$fmt can be used to generate default $fmt style file"
  echo "    # -style=file: uses .clang-format if already in current directory."
  echo
  echo "Then run this file and it will overwrite '`pwd`/.clang-format' file and '$dest_path'"
  #echo "(actually moved to ../ so overwrites if there)"
  echo
  echo
  exit 4
else
  echo "Using $fmt_path (to overwrite `pwd`/.clang-format)"
fi

echo "In '`pwd`'..."

# (warn on create/overwrite .clang-format)
if [ -f ".clang-format" ]; then
  echo "Overwriting existing '.clang-format'..."
else
  echo "Creating new '.clang-format'..."
fi
# load existing format file named $fmt_path
cp -f "$fmt_path" ./.clang-format

# use -style=file to use customized .clang-format
# in current directory created by user apart from this script
# (as instructed above if missing):
if [ -f "$dest_path" ]; then
  echo "Overwriting existing '$dest_path'..."
else
  echo "Creating new '$dest_path'.."
fi
echo "clang-format -style=file mainwindow.cpp > $dest_path"
clang-format -style=file mainwindow.cpp > $dest_path
if [ ! -f "../$dest_name" ]; then
  mv "$dest_path" ../
fi
echo
echo
