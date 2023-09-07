#!/usr/bin/env python3
from __future__ import print_function
import os
import sys
import inspect

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
# print("[mainwindow] loading", file=sys.stderr)

# if __name__ == __main__:
#     sys.path.insert(0, REPO_DIR)

try:
    import outputinspector
except ImportError as ex:
    if (("No module named 'outputinspector'" in str(ex))  # Python 3
            or ("No module named outputinspector" in str(ex))):  # Python 2
        sys.path.insert(0, REPO_DIR)
        import outputinspector
    else:
        raise ex
outputinspector.ENABLE_GUI = True  # mainwindow is GUI subclass so force GUI

from outputinspector import (
    OutputInspector,
    pinfo,
    echo0,  # error,
    echo1,  # debug,
)

if outputinspector.ENABLE_GUI:
    echo0("[mainwindow] Using the GUI version of QMainWindow")
    from outputinspector.noqttk import (
        # QListView,
        # QListWidget,
        QStatusBar,
        QMainWindow,
    )
else:
    echo0("[mainwindow] Using the CLI version of QMainWindow")
    from outputinspector.noqt import (
        # QListView,
        # QListWidget,
        QStatusBar,
        QMainWindow,
    )

from outputinspector.noqt import (
    # QListView,
    QListWidget,
)
class WidgetCollection(object):
    pass


class MainWindow(OutputInspector, ttk.Frame):  # ttk.Frame

    def __init__(self, root):
        prefix = "[MainWindow] "
        OutputInspector.__init__(
            self,
            # Should *not* do any GUI stuff. GUI subclass (MainWindow) should.
            # ui_file=os.path.join(REPO_DIR, "mainwindow.ui"),
            # ^ ui_file is already done by OutputInspector
        )

        # QMainWindow.__init__(  # already done by OutputInspector
        #     self,
        #     ui_file=os.path.join(REPO_DIR, "mainwindow.ui"),
        # )
        # FIXME: formerly, OutputInspector would detect if it is a QMainWindow subclass
        #   OutputInspector to detect this case and not  in
        #   this case (may call noqt's instead of noqttk's)
        echo0("Using QMainWindow from %s" % inspect.getfile(QMainWindow))
        # QMainWindow.__init__(
        #     self,
        #     root,
        #     ui_file=os.path.join(REPO_DIR, "mainwindow.ui"),
        # )
        # ^ set self._ui etc.
        echo1(prefix+"initializing")
        # ttk.Frame.__init__(self)
        self.root = root
        if root is not None:
            # TODO: eliminate this? Old way of detecting GUI mode
            # self._window_init(root)
            # wait until ttk.Frame class is complete:
            # root.after(10, self._window_init_timed)
            # ^ too late. self.load_stdin_or_file runs first. so:
            self._window_init(self.root)
        else:
            # Use console mode.
            pinfo("")
            pinfo("Output Inspector")
            pinfo("----------------")

    def _window_init_timed(self):
        self._window_init(self.root)

    def _window_init(self, parent):
        # self.bV = tk.StringVar()
        ttk.Frame.__init__(self, parent)
        self.pack(
            side=tk.TOP,
            fill=tk.BOTH,
            expand=True,
            anchor=tk.N,  # n, ne, e, se, s, sw, w, nw, or center
        )
        self._ui = WidgetCollection()  # ignore Qt ui file
        # self._ui = self  # REMOVED since loses ones added by noqt ui loader
        # ^ _ui is only for graphical mode (must be set after
        #   OutputInspector.__init__(self))
        # textvariable = self.bV,

        # See _ui_loader
        # Which *parent* is used in constructor supersedes pack order to
        #   determine order if nesting varies for widgets packed!
        #   - Constructor is called by ui file parser! Unless:
        scrollbar = tk.Scrollbar(root, orient="vertical")
        self._ui.mainListWidget = QListWidget(
            self,
            yscrollcommand=scrollbar.set,
        )
        # self._ui.mainListWidget = QListView(self)  # instead set in ui by noqt
        self._ui.mainListWidget.pack(
            side=tk.TOP,
            fill=tk.BOTH,
            expand=True,
            anchor=tk.N,
        )
        # lb = tk.Listbox(root, width=50, height=20, yscrollcommand=scrollbar.set)
        # self.mainListWidget.pack()
        self._ui.mainListWidget.bind(
            "<Double-Button-1>",
            self.on_mainListWidget_itemDoubleClicked,
        )

        # from mainwindow.ui:
        self._ui.statusBar = QStatusBar(self)
        self._ui.statusBar.pack(side=tk.BOTTOM, fill=tk.X)  # self.statusBar.pack
        # self.root.geometry("1200x400")
        self.root.minsize(1200,32)
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
    sErrorsListFileName = None  # reverts to err.txt if left blank
    i = 0
    # start at 1 since args[0] is self:
    for i in range(1, len(sys.argv)):
        arg = sys.argv[i]
        if not arg.startswith("--"):
            if sErrorsListFileName is not None:
                echo0("Error: Specify only one log file.")
                # return 1  # commented so GUI isn't prevented from loading
                # TODO: store startup errors and show them.
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
    if sErrorsListFileName is not None:
        sErrorsListFileName = sErrorsListFileName.strip()
    window.init(sErrorsListFileName)
    root.mainloop()
    return 0

if __name__ == "__main__":
    sys.exit(main())
