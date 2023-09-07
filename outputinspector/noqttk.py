#!/usr/bin/env python3
'''
This module helps people migrate from PyQt to Tkinter, but the emphasis
is on Python code converted from C++ with Qt such as using SharpDevelop
4 (See <https://github.com/poikilos/pycodetool> for progress at
code conversion with and without SharpDevelop 4).

Functions and methods with "noqt" (and most or all members starting with
"_") are "shims" in the sense that they exist to replicate Qt behavior
but do not have corresponding symbols in Qt.
'''
from __future__ import print_function
import sys
import time
import threading
import os
import platform
import copy

if sys.version_info.major >= 3:
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
else:
    import Tkinter as tk
    import ttk
    import tkMessageBox as messagebox

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(MODULE_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)
    # ^ allows importing REPO_DIR
# print("[noqttk] loading", file=sys.stderr)
import outputinspector
outputinspector.ENABLE_GUI = True  # set True *only* in noqttk. Defaults to False.
import outputinspector.noqt as noqt
from outputinspector.noqt import (
    Qt,
    enum,
    QWidget,
    QListWidgetItem as QListWidgetItemCLI,
    QVariant,
    QTimer,
    connect,
    QListWidgetItem,
)

class QMainWindow(noqt.QMainWindow):
    """Grab all the ttk.Frame methods.

    The subclass will do Frame.__init__ if not a CLI subclass.
    """
    def __init__(self, *args, **kwargs):
        tk_kwargs = kwargs
        if "ui_file" in kwargs:
            tk_kwargs = copy.deepcopy(kwargs)
            del tk_kwargs['ui_file']
            echo0("ui_file is not used by ttk.Frame")
        # ttk.Frame.__init__(self, *args, **tk_kwargs)
        noqt.QMainWindow.__init__(self, *args, **kwargs)
        # ^ QMainWindow is already subclass of ttk.Frame

    def is_gui(self):
        return True
# class Qt:
#     lightGray = QColor.fromRgb(192, 192, 192)
#     darkGreen = QColor.fromRgb(0, 128, 0)
#     black = QColor.fromRgb(0, 0, 0)

verbosity = 0
max_verbosity = 2
TMP = "/tmp"
if platform.system() == "Windows":
    TMP = os.environ['TEMP']

def warn(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def echo0(*args, **kwargs):  # formerly error
    print(*args, file=sys.stderr, **kwargs)
    return True


def echo1(*args, **kwargs):  # formerly debug
    '''
    Only show the message if verbosity > 1 (See "set_verbosity").
    '''
    if verbosity > 1:
        print(*args, file=sys.stderr, **kwargs)


def echo2(*args, **kwargs):
    if verbosity < 2:
        return False
    print(*args, file=sys.stderr, **kwargs)
    return True
from outputinspector.best_timer import best_timer as second_timer

def noqt_tick():
    return int(second_timer() * 1000.0)


class QListView(tk.Listbox):
    def __init__(self, parent, **kwargs):
        tk.Listbox.__init__(self, **kwargs)
        self.parent = parent
        # self._items = []
        self.bind('<<ListboxSelect>>', self._on_items_selected)

    def addItem(self, qlistwidgetitem):
        prefix = "[noqttk addItem] "
        qlistwidgetitem.parent = self
        qlistwidgetitem.index = self.size()
        echo0("[noqttk addItem] calling append at %s" % qlistwidgetitem.index)
        self.append(qlistwidgetitem)  # calls the *fake* one def below not tk
        for key, value in qlistwidgetitem.queued_tk_args.items():
            self.itemconfig(qlistwidgetitem.index, {key: value})

    def append(self, item):
        index = self.size()
        self.insert(index, item)
        echo0("[noqttk append] inserted at %s" % index)
        if len(self._items) - 1 >= index:
            # A fake append from notk or nottk has run that already did self._items
            raise RuntimeError("non-tk widget is being used for noqttk")
        else:
            self._items.insert(index, item)

    def _on_items_selected(self, event):
        # print(event)
        # ^ Not very useful--just shows "<VirtualEvent event x=0 y=0>"
        # print(dir(event))
        # print("num={}".format(event.num))  # just says "??"
        # print("type={}".format(event.type))  # just says 35
        # print("state={}".format(event.state))  # just says 0
        for i in self.curselection():
            print("curselection()[{}]={}".format(i, self.get(i)))
            # KATE or geany
            proc = subprocess.Popen(cmd_parts)

no_listview_msg = (
    "This is noqt not qt, so you must first add the noqt.QListViewItem"
    " to o a noqt.QListView or"
    " manually set parent and index on each QListWidgetItem"
    " to the Listbox in order to use tk itemconfig to"
    " simulate QListWidgetItem behavior"
    " (parent={}, index={})."
)

class QColor:
    def __init__(self, color_tuple):
        self.color = color_tuple

    @staticmethod
    def fromRgb(r, g, b):
        return QColor((r, g, b))

    def toTkColor(self):
        # See <https://stackoverflow.com/a/3380739/4541104>
        # TODO: check bounds.
        return '#%02x%02x%02x' % self.color


class QBrush:
    def __init__(self, qcolor):
        self.qcolor = qcolor

    def toTkColor(self):
        return self.qcolor.toTkColor()

class NoQtMessage:
    def __init__(self, text, timeout):
        self.text = text
        self.timeout = timeout
        self.start = noqt_tick()


class QStatusBar(QWidget, ttk.Label):
    def __init__(self, *args, **kwargs):
        if len(args) < 1:
            raise ValueError("You must specify a parent.")
        self.var = tk.StringVar()
        kwargs['textvariable'] = self.var
        ttk.Label.__init__(self, *args, **kwargs)
        self._previous_text = None
        self._timer = None
        self._timeout = None

    def clearMessage(self, *args):
        if self._previous_text is not None:
            self.var.set(self._previous_text)
            self._previous_text = None

    def showMessage(self, *args):
        if len(args) < 1:
            raise ValueError("Specify text (and optionally timeout in milliseconds).")
        text = args[0]
        self._timeout = 0
        self._stop()
        if len(args) > 1:
            self._timeout = args[1]
        if self._previous_text is None:
            self._previous_text = self.var.get()
        # else never modify the permanent text
        self.var.set(text)
        if self._timeout > 0:
            self._timer = threading.Timer(0.1, self._on_timeout)
            self._timer.start()

    def _stop(self):
        if self._timer is not None:
            self._timer.cancel()
            self._timer = None

    def _on_timeout(self):
        self.clearMessage()

'''
class NoQtEvent:
    def __init__(self, fn, caller_obj=None):
        self.caller = caller_obj
        self.fn = fn


class NoQtEventHandler:
'''
