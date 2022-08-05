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

if sys.version_info.major >= 3:
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
else:
    import Tkinter as tk
    import ttk
    import tkMessageBox as messagebox


from outputinspector.best_timer import best_timer as second_timer


def noqt_tick():
    return int(second_timer() * 1000.0)


class QListView(tk.Listbox):
    def __init__(self, parent, **kwargs):
        tk.Listbox.__init__(self, **kwargs)
        self.parent = parent
        # self._items = []

    def addItem(self, qlistwidgetitem):
        qlistwidgetitem.parent = self
        qlistwidgetitem.index = self.size()
        self.append(qlistwidgetitem)
        for key, value in qlistwidgetitem.queued_tk_args.items():
            self.itemconfig(qlistwidgetitem.index, {key: value})

    def append(self, item):
        self.insert(self.size(), item)


no_listview_msg = (
    "This is noqt not qt, so you must first add the noqt.QListViewItem"
    " to o a noqt.QListView or"
    " manually set parent and index on each QListWidgetItem"
    " to the Listbox in order to use tk itemconfig to"
    " simulate QListWidgetItem behavior"
    " (parent={}, index={})."
)


class QListWidgetItem(tk.StringVar):
    '''
    Attributes:
    queued_tk_args -- These values are automatically set. They exist
        since Qt stores colors etc in the QListItem but Tk stores them
        in the Listbox (set via its itemconfig method). The issue is
        that Qt allows setting item options before the item is added to
        a list. To compensate, the noqt.QListView runs itemconfig for
        each key-value pair in queued_tk_args after the
        noqt.QListWidgetItem is added to a noqt.QListView.
    '''
    def __init__(self, *args, **kwargs):
        self.role = None
        if len(args) > 0:
            kwargs['value'] = args[0]
        if len(args) > 1:
            raise ValueError("Too many args")
        tk.StringVar.__init__(self, **kwargs)
        self.parent = None
        self.index = None
        self.queued_tk_args = {}

    def setData(self, role, value):
        self.role = role
        self.set(value)

    def setForeground(self, qbrush):
        if (self.parent is None) or (self.index is None):
            # raise RuntimeError(
            #     no_listview_msg.format(self.parent, self.index)
            # )
            self.queued_tk_args['fg'] = qbrush.toTkColor()
            return
        self.parent.itemconfig(self.index, {'fg': qbrush.toTkColor()})


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


class Qt:
    lightGray = QColor.fromRgb(192, 192, 192)
    darkGreen = QColor.fromRgb(0, 128, 0)
    black = QColor.fromRgb(0, 0, 0)


class QBrush:
    def __init__(self, qcolor):
        self.qcolor = qcolor

    def toTkColor(self):
        return self.qcolor.toTkColor()


class QVariant(tk.StringVar):
    def __init__(self, *args, **kwargs):
        if len(args) > 0:
            kwargs['value'] = args[0]
        if len(args) > 1:
            raise ValueError("Too many args")
        tk.StringVar.__init__(self, **kwargs)
        pass


class NoQtMessage:
    def __init__(self, text, timeout):
        self.text = text
        self.timeout = timeout
        self.start = noqt_tick()


class QStatusBar(ttk.Label):
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

def connect(sender, sig, receiver, slot):
    '''
    Sequential arguments:
    sender -- the sending object
    sig -- the sending object's event that occurs (implemented in noqt
        as a list of slots)
    receiver -- the handler object
    slot -- the handler
    '''
    sig.append(slot)

'''
class NoQtEvent:
    def __init__(self, fn, caller_obj=None):
        self.caller = caller_obj
        self.fn = fn


class NoQtEventHandler:
'''


class QTimer:
    '''
    Attributes:
    timeout -- It mimics the SLOTS feature of Qt in a much simpler way:
        It is just a list of signals (function handles).
    '''
    def __init__(self, parent):
        self.parent = parent
        self.timeout = []
        self._timer = None
        self._event = None
        self._interval = 0  # The default is 0 as per Qt 6 docs
        self._singleShot = False

    def setInterval(self, ms):
        self.stop()
        if not isinstance(ms, int):
            raise ValueError("setInterval only takes a milliseconds int")
        self._interval = ms

    def start(self):
        # See test_noqt.py for why to not use sched (cumbersome, needs
        # threads & blocking [by default] or polling if blocking=False).
        # The asyncio module & await aren't used since those aren't in
        # Python 2.
        self._timer = threading.Timer(self._interval, self._on_timeout)
        self._timer.start()

    def stop(self):
        if self._timer is not None:
            self._timer.cancel()
            self._timer = None
            self._interval = None

    def _on_timeout(self):
        echo2("* running _on_timeout")
        for slot in self.timeout:
            slot()
        if not self._singleShot:
            self.start()
            # threading.Timer can only be started once.
