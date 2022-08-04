#!/usr/bin/env python3
from __future__ import print_function
import os
import sys

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

try:
    import outputinspector
except ImportError as ex:
    if (("No module named 'outputinspector'" in str(ex))  # Python 3
            or ("No module named outputinspector" in str(ex))):  # Python 2
        sys.path.insert(0, REPO_DIR)
    else:
        raise ex

from outputinspector import (
    OutputInspector,
    pinfo,
)


class MainWindow(OutputInspector, ttk.Frame):

    def __init__(self, root):
        OutputInspector.__init__(self)
        self.root = root
        if root is not None:
            self._window_init(root)
        else:
            # Use console mode.
            pinfo("")
            pinfo("Output Inspector")
            pinfo("----------------")

    def _window_init(self, parent):
        # self.bV = tk.StringVar()
        ttk.Frame.__init__(self, parent)
        self.pack(fill=tk.BOTH, expand=True)
        self.mainListWidget = tk.Listbox(  # There is no ttk Listbox
            self,
        )
        #   textvariable=self.bV,
        self.mainListWidget.pack()

    def showinfo(self, title, msg):
        messagebox.showinfo(title, msg)

    def showerror(self, title, msg):
        messagebox.showerror(title, msg)
