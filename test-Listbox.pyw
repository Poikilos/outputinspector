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
