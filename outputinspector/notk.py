"""
Usage:
import outputinspector.notk as tk

"""
import sys

END = "end"

def echo0(*args):
    print(*args, file=sys.stderr)

class Widget:
    def __init__(self, *args):
        self._parent = None
        if args:
            if len(args) > 1:
                raise ValueError(
                    "Expected parent or no args but got %s" % args
                )
            self._parent = args[0]

    def grid_columnconfigure(self, **kwargs):
        pass

    def pack(self, **kwargs):
        # TODO: something
        pass

class Label(Widget):
    def __init__(self, *args, **kwargs):
        for key, value in kwargs.items():
            # TODO: filter out bad ones
            setattr(self, key, value)
        if args:
            self.parent = args[0]
            if len(args) > 1:
                # If this looks familiar its also in
                # noqt (and maybe noqttk)
                raise ValueError(
                    "Expected either parent as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )

class Listbox(Widget):
    def __init__(self, *args, **kwargs):
        prefix = "[Listbox] "
        self._items = []
        Widget.__init__(self, *args)
        for key, value in kwargs.items():
            # TODO: Filter to args accepted by real tk.Listbox
            setattr(self, key, value)
        self._listbox_size = 0  # Avoid interfering with noqt _size
        if not hasattr(self, "size"):
            echo0(prefix+"size is in Tk (not qt) compatibility mode")
            self.size = self._tk_size
        else:
            # This occurs if noqt subclassed Listbox.
            echo0(prefix+"size is in Qt (not Tk) compatibility mode")
            self._tk_size = Listbox.size

    # def _tk_size(self):  # Avoid overriding noqt size()
    def size(self):
        return len(self._items)

    def bind(self, action, callback):
        """Bind a named action to a given callback
        Usage: bind("<<ListboxSelect>>", _on_items_clicked)
        """
        prefix = "[Listbox bind] "
        echo0(prefix+"Ignored bind %s since in console mode." % action)

    def append(self, item):
        # self.insert(self.size(), item)
        # ^ noqt size() may interfere, so:
        self.insert(Listbox.size(self), item)
    
    def insert(self, index, item):
        prefix = "[Listbox insert]"
        echo0(prefix+"into dummy CLI UI")
        self._items.insert(index, item)
    
    def itemconfig(self, index, options):
        # TODO: filter to attributes allowed on a Tk list item
        for key, value in options.items():
            setattr(self._items[index], key, value)


class StringVar:

    def __init__(self, *args, **kwargs):
        if args:
            self.root = args[0]
            if len(args) > 1:
                raise ValueError("expected only 1 arg (parent) or 0 args")
        self._value = kwargs.get('value')

    def get(self):
        return self._value

    def set(self, value):
        self._value = str(value)
