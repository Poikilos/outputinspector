"""
Usage:
import outputinspector.nottk as ttk

There is no ttk Listbox. Use notk for that.
"""
import outputinspector.notk

class Widget(outputinspector.notk.Widget):
    pass
    # def grid_columnconfigure(**kwargs):  # proves to noqt (or noqttk) it's Tk

class Label(outputinspector.notk.Label):
    pass

# class Frame(outputinspector.notk.Frame):
#     pass
