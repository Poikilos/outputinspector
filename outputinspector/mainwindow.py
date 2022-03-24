#!/usr/bin/env python3

try:
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
except ImportError:
    # Python 2
    import Tkinter as tk
    import ttk
    import tkMessageBox as messagebox

from outputinspector import OutputInspector

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
        self.mainListWidget = ttk.ListBox(
            self,
        )
        #   textvariable=self.bV,
        self.mainListWidget.pack()

    def showinfo(self, title, msg):
        messagebox.showinfo(title, msg)

    def showerror(self, title, msg):
        messagebox.showerror(title, msg)

