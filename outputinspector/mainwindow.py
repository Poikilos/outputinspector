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

root = None
window = None

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
    echo0,  # error,
    echo1,  # debug,
)

from outputinspector.noqt import (
    QListView,
    QStatusBar,
    QMainWindow,
)


class MainWindow(OutputInspector, QMainWindow):  # ttk.Frame

    def __init__(self, root):
        prefix = "[MainWindow] "
        OutputInspector.__init__(self)
        QMainWindow.__init__(
            self,
            ui_file=os.path.join(REPO_DIR, "mainwindow.ui"),
        )
        # ^ OutputInspector should detect if it is not an
        #   OutputInspector to detect this case and not call that in
        #   this case.
        echo0(prefix+"initializing")
        # ttk.Frame.__init__(self)
        self.root = root
        if root is not None:
            # TODO: eliminate this? Old way of detecting GUI mode
            self._window_init(root)
        else:
            # Use console mode.
            pinfo("")
            pinfo("Output Inspector")
            pinfo("----------------")

    def _window_init(self, parent):
        # self.bV = tk.StringVar()
        # ttk.Frame.__init__(self, parent)
        self.pack(fill=tk.BOTH, expand=True)

        # self.mainListWidget = QListView(self)  # instead set in ui by noqt
        # self._ui = self  # REMOVED since loses ones added by noqt ui loader
        # ^ _ui is only for graphical mode (must be set after
        #   OutputInspector.__init__(self))
        # textvariable = self.bV,
        self._ui.mainListWidget.pack()  # self.mainListWidget.pack()
        # from mainwindow.ui:
        # self.statusBar = QStatusBar(self)
        self._ui.statusBar.pack(side=tk.BOTTOM, fill=tk.Y)  # self.statusBar.pack
        self.root.geometry("1200x400")
        # ^ TODO: use noqt to call geometry from ui file


    def showinfo(self, title, msg):
        messagebox.showinfo(title, msg)

    def showerror(self, title, msg):
        messagebox.showerror(title, msg)


def main():
    global root
    global window
    root = tk.Tk()
    root.title("Output Inspector")
    # TODO: app.setWindowIcon(QIcon("outputinspector-64.png"))
    # app.setOrganizationDomain("poikilos.org")
    window = MainWindow(root)
    sErrorsListFileName = ""  # reverts to err.txt if left blank
    i = 0
    # start at 1 since args[0] is self:
    for i in range(1, len(sys.argv)):
        arg = sys.argv[i]
        if not arg.startswith("--"):
            sErrorsListFileName = arg
        else:
            signIndex = arg.find("=")
            if signIndex > -1:
                valueIndex = signIndex + 1
                nameLen = signIndex - 2
                name = arg[2:signIndex]
                value = arg[valueIndex:].strip()
                window.settings.setValue(name, value)
                pinfo("set {} to {}"
                      "".format(name, value))
    window.init(sErrorsListFileName.strip())
    root.mainloop()
    return 0

if __name__ == "__main__":
    sys.exit(main())
