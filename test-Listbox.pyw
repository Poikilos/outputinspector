#!/usr/bin/env python3
from __future__ import print_function
import sys

if sys.version_info.major >= 3:
    print("\nUsing Python %s" % sys.version, file=sys.stderr)
    # sys.path.insert(0, "/usr/lib/python3.10")  # even this doesn't prevent
    # VSCode linux "ModuleNotFoundError: No module named 'tkinter'"
    # even though with or without that, the path above is in sys.path
    # and has tkinter in it :(
    print("PYTHON=%s" % (sys.executable))
    print("PYTHONPATH=%s" % (sys.path))
    import tkinter as tk
    from tkinter import ttk
    from tkinter import messagebox
else:
    # Python 2
    print("\nUsing Python 2", file=sys.stderr)
    import Tkinter as tk
    import ttk
    import tkMessageBox as messagebox

from outputinspector import OutputInspector

class MainWindow(ttk.Frame):  # (OutputInspector, tk.Tk)

    def __init__(self, root):
        # tk.Tk.__init__(self)
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
        self.mainListWidget = tk.Listbox(
            self,
        )
        #   textvariable=self.bV,
        # Avoid
        # "AttributeError: 'tkinter.ttk' has no attribute 'Listbox'"
        self.mainListWidget.pack()

    def showinfo(self, title, msg):
        messagebox.showinfo(title, msg)

    def showerror(self, title, msg):
        messagebox.showerror(title, msg)


root = None
window = None


def main():
    global root
    global window
    root = tk.Tk()
    root.title("Listbox Test Application")
    # app.setOrganizationDomain("poikilos.org")
    window = MainWindow(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    main()
