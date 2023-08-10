"""
Usage:
from outputinspector.notkinter import messagebox

"""
import sys


def echo0(*args):
    print(*args, file=sys.stderr)


class messagebox:
    @staticmethod
    def show(title, msg):
        print("%s: %s" % (title, msg))
