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

TESTS_DIR = os.path.dirname(os.path.realpath(__file__))
TEST_DATA_DIR = os.path.join(TESTS_DIR, "data")
TEST_DEBUG_PATH = os.path.join(TEST_DATA_DIR, "debug.txt")
if not os.path.isdir(TEST_DATA_DIR):
    raise FileNotFoundError(TEST_DATA_DIR)
if not os.path.isfile(TEST_DEBUG_PATH):
    raise FileNotFoundError(TEST_DEBUG_PATH)
REPO_DIR = os.path.dirname(TESTS_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)

from outputinspector import (
    # OutputInspector,
    mainwindow,
)

os.chdir(TEST_DATA_DIR)  # Since mainwindow.main looks for err.txt in current dir

HOME = None
if platform.system() == "Windows":
    HOME = os.environ['USERPROFILE']
else:
    HOME = os.environ['HOME']

class TestMainWindow(unittest.TestCase):
    def test_main(self):
        import outputinspector
        outputinspector.ENABLE_GUI = True
        if len(sys.argv) > 1:
            # if not sys.argv[1].endswith(".txt"):
            print("Warning: removing bad args: %s" % sys.argv[1:], file=sys.stderr)
            sys.argv = sys.argv[:1]
        try:
            mainwindow.main()
        except UnicodeDecodeError as ex:
            print("Bad text file. Bad argument in %s?" % sys.argv[1:], file=sys.stderr)
            raise


if __name__ == "__main__":
    testcase = TestMainWindow()
    count = 0
    for name in dir(testcase):
        if name.startswith("test"):
            count += 1
            fn = getattr(testcase, name)
            fn()  # Look at def test_* for the code if tracebacks start here
    if count > 0:
        print("All {} test(s) passed".format(count))
    sys.exit(0)

