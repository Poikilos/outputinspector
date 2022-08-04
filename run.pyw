#!/usr/bin/env python3
from __future__ import print_function
import sys
if sys.version_info.major >= 3:
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
else:
    import Tkinter as tk
    import ttk
    import tkMessageBox as messagebox

from outputinspector.mainwindow import (
    MainWindow,
)

from outputinspector import (
    pinfo,
    echo0,  # error,
    echo1,  # debug,
)

root = None
window = None


def main():
    global root
    global window
    root = tk.Tk()
    root.title("Output Inspector")
    # app.setOrganizationDomain("poikilos.org")
    window = MainWindow(root)
    sErrorsListFileName = ""  # reverts to err.txt if left blank
    qArgs = []
    i = 0
    # start at 1 since qArgs[0] is self:
    while i < len(qArgs):
        i += 1
        qArg = qArgs[i]
        if not qArg.startswith("--"):
            sErrorsListFileName = qArg
        else:
            signIndex = qArg.find("=")
            if signIndex > -1:
                valueIndex = signIndex + 1
                nameLen = signIndex - 2
                name = qArg[2:signIndex]
                value = qArg[valueIndex:].strip()
                window.settings.setValue(name, value)
                pinfo("set {} to {}"
                      "".format(name, value))
    window.init(sErrorsListFileName.strip())
    root.mainloop()
    # (Urban & Murach, 2016, p. 515)
    session.stop()
    if session.save():
        print("Save completed.")
    else:
        print("Save failed.")
    # TODO: app.setWindowIcon(QIcon("outputinspector-64.png"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
