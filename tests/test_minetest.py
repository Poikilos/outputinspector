from __future__ import print_function
import sched
import sys
import unittest
import time
import os
import threading
import platform

from pprint import pformat

if sys.version_info.major < 3:
    FileNotFoundError = IOError
    ModuleNotFoundError = ImportError

TEST_SUBMODULE_DIR = os.path.dirname(os.path.realpath(__file__))
TEST_DATA_DIR = os.path.join(TEST_SUBMODULE_DIR, "data")
TEST_DEBUG_PATH = os.path.join(TEST_DATA_DIR, "debug.txt")
if not os.path.isdir(TEST_DATA_DIR):
    raise FileNotFoundError(TEST_DATA_DIR)
if not os.path.isfile(TEST_DEBUG_PATH):
    raise FileNotFoundError(TEST_DEBUG_PATH)
MODULE_DIR = os.path.dirname(TEST_SUBMODULE_DIR)
REPO_DIR = os.path.dirname(MODULE_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)

from outputinspector import OutputInspector

HOME = None
if platform.system() == "Windows":
    HOME = os.environ['USERPROFILE']
else:
    HOME = os.environ['HOME']

mt_share_path = None
default_basedir = os.path.join(HOME, "minetest")
# ^ default if None found is not same as preferred one (first in list):
try_dirs = [
    os.path.join(HOME, "minetest-rsync"),
    default_basedir,
]
if platform.system() == "Windows":
    del try_dirs[:]
    try_dirs.append(os.path.join("C:", "Games", "minetest"))
    try_dirs.append(os.path.join(HOME, "minetest"))

for try_dir in try_dirs:
    if os.path.isdir(try_dir):
        mt_share_path = try_dir
        break

if mt_share_path is not None:
    print("chdir %s (to find real files after removing elipses)"
          % pformat(mt_share_path))
    os.chdir(mt_share_path)
else:
    print("%s was not found (only dummy files should be found after"
          " removing elipses)." % pformat(default_basedir))
    mt_share_path = default_basedir
    os.chdir(TEST_DATA_DIR)


class TestMinetest(unittest.TestCase):
    def __init__(self):
        unittest.TestCase.__init__(self)
        self.inspector = OutputInspector()

    def test_file_read(self):
        self.inspector.init(TEST_DEBUG_PATH)


if __name__ == "__main__":
    testcase = TestMinetest()
    count = 0
    for name in dir(testcase):
        if name.startswith("test"):
            count += 1
            fn = getattr(testcase, name)
            fn()  # Look at def test_* for the code if tracebacks start here
    if count > 0:
        print("All {} test(s) passed".format(count))

