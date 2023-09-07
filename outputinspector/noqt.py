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
# import time
import threading
import os
import platform
import inspect
import copy

import xml.etree.ElementTree as ET

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(MODULE_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)
    # ^ allows importing REPO_DIR
# print("[noqt] loading", file=sys.stderr)
TK_ONLY_ATTRIBUTE = "_setup"

import outputinspector


if outputinspector.ENABLE_GUI:
    print("[noqt] detected GUI mode (import noqttk for GUI)",
          file=sys.stderr)
    if sys.version_info.major >= 3:
        import tkinter as tk
        from tkinter import ttk
        from tkinter import messagebox
    else:
        import Tkinter as tk
        import ttk
        import tkMessageBox as messagebox
else:
    print("[noqt] detected CLI mode", file=sys.stderr)
    import outputinspector.notk as tk
    import outputinspector.nottk as ttk
    from outputinspector.notkinter import messagebox

verbosity = 0
max_verbosity = 2
TMP = "/tmp"
if platform.system() == "Windows":
    TMP = os.environ['TEMP']

def warn(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def echo0(*args, **kwargs):  # formerly error
    print(*args, file=sys.stderr, **kwargs)
    return True


def echo1(*args, **kwargs):  # formerly debug
    '''
    Only show the message if verbosity > 1 (See "set_verbosity").
    '''
    if verbosity > 1:
        print(*args, file=sys.stderr, **kwargs)


def echo2(*args, **kwargs):
    if verbosity < 2:
        return False
    print(*args, file=sys.stderr, **kwargs)
    return True
from outputinspector.best_timer import best_timer as second_timer


def noqt_tick():
    return int(second_timer() * 1000.0)

class QLayoutItem(object):
    def __init__(self, *args):
        self._parent = None
        self._alignment = None
        if args:
            if not isinstance(args[0], Alignment):
                raise TypeError("Expected Alignment but got %s"
                                % type(args[0]).__name__)
            if len(args) > 1:
                # If this looks familiar its also in
                # QWidget and nottk (and maybe notk)
                raise ValueError(
                    "Expected either Alignment as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )
            self._alignment = args[0]
        self._children = []

    def isEmpty(self):
        return not self._children


class QLayout(QLayoutItem, ttk.Frame):
    """This is a basic layout widget.

    NOTE: Tk documentation says to avoid using "tk.Widget"
      (only use subclasses).
    """
    def __init__(self, *args, f=None):
        self._layouts = []
        self._children = []
        if args:
            if len(args) > 1:
                # If this looks familiar its also in
                # QWidget and nottk (and maybe notk)
                raise ValueError(
                    "Expected either parent as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )
            self._parent = args[0]
        ttk.Frame.__init__(
            self,
            self._parent,
        )

    def addChildLayout(self, layout):
        self._layouts.append(layout)

    def addChildWidget(self, widget):
        self._children.append(widget)


class QBoxLayout(QLayout):

    def addLayout(self, layout):
        self._layouts.append(layout)

    def insertLayout(self, index, layout, stretch=0):
        # TODO: implement stretch
        self._layouts.insert(index, layout)


class QHBoxLayout(QBoxLayout):
    pass


class QSize(object):
    def __init__(self, *args):
        if args:
            if len(args) != 2:
                raise ValueError("Requires 2 or 0 args")
            self.size = [args[0], args[1]]
            return
        self.size = [0, 0]


class QWidget(ttk.Frame):
    """
    For full operation this:
    - should be inherited by all widgets.
    - should have all necessary properties as listed at
      <https://doc.qt.io/qt-6/qwidget.html>

    (Do not use ttk.Widget--it isn't supposed to be instantiated)

    Args:
        f (Optional[WindowFlags]): reserved (here for compatibility)
    """
    def __init__(self, *args, f=None):
        self._children = []
        self._layout = None
        self._size = QSize()
        if args:
            if len(args) > 1:
                # If this looks familiar its also in
                # QLayout and nottk (and maybe notk)
                raise ValueError(
                    "Expected either parent as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )
            self._parent = args[0]
        else:
            raise ValueError("QWidget without parent")
        ttk.Frame.__init__(self, self._parent)

    def setLayout(self, layout):
        self._layout = layout

    def parentWidget(self):
        return self._parent

    def size(self):
        return self._size


class QMenuBar(QWidget):
    pass
    # def __init__(self, *args):
    #     QWidget.__init__(self, *args)


class QToolBar(QWidget):
    pass
    # def __init__(self, *args):
    #     QWidget.__init__(self, *args)


class QFrame(QWidget):
    pass


class QAbstractScrollArea(QFrame):
    pass


class QAbstractItemView(QAbstractScrollArea):
    pass


class QListView(QAbstractItemView, tk.Listbox):
    def __init__(self, *args, **kwargs):
        self._items = []
        QAbstractItemView.__init__(self, *args)  # set _size etc
        if args and hasattr(args[0], "grid_columnconfigure"):
            # It is really tk (or notk or nottk)
            tk.Listbox.__init__(self, args[0], **kwargs)
            # ^ sets _parent (get with parentWidget())
        else:
            tk.Listbox.__init__(self, **kwargs)
            # ^ sets _parent (get with parentWidget())
        self.bind('<<ListboxSelect>>', self._on_items_selected)

    def _on_items_selected(self, event):
        # print(event)
        # ^ Not very useful--just shows "<VirtualEvent event x=0 y=0>"
        # print(dir(event))
        # print("num={}".format(event.num))  # just says "??"
        # print("type={}".format(event.type))  # just says 35
        # print("state={}".format(event.state))  # just says 0
        for i in self.curselection():
            print("curselection()[{}]={}".format(i, self.get(i)))

    def selectedItems(self):
        return [self._items[i] for i in self.curselection()]
        # ^ uh oh, this gets strings
        return [self.get(i) for i in self.curselection()]
        # ^ curselection is the tk-like solution, and the
        #   output is Qt-like (object list in the case of QListView)


class QListWidget(QListView):
    def addItem(self, qlistwidgetitem):
        """Add a QListWidgetItem to the QListWidget (inherits QListView but adds addItem)
        Mimic Qt C++
        """
        prefix = "[noqt addItem] "
        qlistwidgetitem._parent = self
        # qlistwidgetitem.index = self._tk_size(self)  # maybe fancy this up better??
        qlistwidgetitem.index = tk.Listbox.size(self)
        # self.append(qlistwidgetitem)
        echo2(prefix+"adding a %s from %s: %s" % (
            type(qlistwidgetitem).__name__,
            os.path.basename(inspect.getfile(type(qlistwidgetitem))),
            qlistwidgetitem.get(),
        ))

        # TODO: (?) Make something better than Listbox for QtListBox so it has
        #   true widgets inside of it instead of objects with __str__ (Listbox
        #   only uses items as str-like objects).
        #   - For now the real bind call is in _window_init
        # qlistwidgetitem.bind(
        #     "<Double-Button-1>",
        #     self.on_mainListWidget_itemDoubleClicked,
        # )
        if len(self._items) - 1 >= qlistwidgetitem.index:
            # A fake append from notk or nottk has run that already did self._items
            raise RuntimeError("non-tk widget is being used for noqt")
        else:
            self.insert(tk.END, qlistwidgetitem)  # This is the *real* tk so must do fake Qt-like op:
            self._items.insert(qlistwidgetitem.index, qlistwidgetitem)
        for key, value in qlistwidgetitem.queued_tk_args.items():
            # tk.Listbox-like:
            self.itemconfig(qlistwidgetitem.index, {key: value})



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
    Usually in Tkinter, there is only one StringVar for a listbox:
    """
    self.var = tk.StringVar(self._values)
    Listbox(..., listvariable=self.var)
    """
    However, to make each list item able to be manipulated directly,
    NoQt makes a Qt-like object for each list item. The item must
    behave like a string, so the __str__ function must be overridden
    (otherwise the listbox will show a series of internal IDs such as:
    PY_VAR1, PY_VAR2, etc., one for each line).

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
        prefix = "[QListWidgetItem __init__] "
        if hasattr(self, "_tclCommands"):
            echo0("Using GUI-like QListWidgetItem\n")
        # else imported notk as tk as expected in noqt (only noqttk uses
        #   the *real* tk.
        # QWidget.__init__(self, *args, **kwargs)
        # ^ sets self._parent
        self.roles = {}
        if len(args) > 0:
            echo1(prefix+"passed %s: %s" % (type(args[0]).__name__, args[0]))
            kwargs['value'] = args[0]
        tk_args = copy.deepcopy(args)  # to be safe though tuple has no copy()
        tk_args = list(tk_args)  # convert from tuple
        value = None
        if len(tk_args) > 1:
            if isinstance(args[1], str):
                value = tk_args[1]
                del tk_args[1]
        if len(tk_args) > 0:
            if isinstance(args[0], str):
                value = tk_args[0]
                del tk_args[0]

        # Optionally, first arg is parent (but only for tk.StringVar,
        #   which doesn't need the string?)
        if len(args) > 2:
            raise ValueError("Too many args")

        tk.StringVar.__init__(self, *tk_args, **kwargs)
        if value is not None:
            self.set(value)
        echo1(prefix+"initialized to %s" % self.get())
        # ^ will raise an exception if MainWindow (or tk.Tk) not initialized
        self.parent = None  # custom (may differ from tkinter _parent)
        self.index = None
        self.queued_tk_args = {}

    def __repr__(self):
        return self.get()

    def __str__(self):
        # Required so it works as a list item in tk.Listbox
        return self.get()

    def setData(self, role, var):
        prefix = "[noqttk QListWidgetItem setData] "
        if hasattr(var, "get"):
            # Such as QVariant
            value = var.get()
        else:
            value = var
        self.roles[role] = value
        if role == Qt.DisplayRole:
            self.set(value)


    def data(self, role):
        var = self.roles[role]
        if hasattr(var, "get"):
            # Such as QVariant
            value = var.get()
        else:
            # Create a QVariant so client code can call .toString() on it:
            value = QVariant()
            value.set(var)
        return value

    def text(self):
        return self.get()

    def setForeground(self, qbrush):
        if (self.parent is None) or (self.index is None):
            # raise RuntimeError(
            #     no_listview_msg.format(self.parent, self.index)
            # )
            self.queued_tk_args['fg'] = qbrush.toTkColor()
            return
        self.parent.itemconfig(self.index, {'fg': qbrush.toTkColor()})
        # or (naming is inconsistent):
        # self.parent.itemconfig(0, foreground="purple")
        # self.parent.itemconfig(2, bg='green')


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


class enum(object):
    """Better than Enum because type of value is known when subclassed.

    For example, a method can require Alignment instead of int or enum,
    since only Alignment, not just any Enum, has the correct values.

    Args:
        value (Optional[int]): Set the value of the specific enum constant (Each
            enum subtype has its own set). Defaults to 0 or each successive call
            adds 1.
    """
    next_roles = {}
    _typeNames = {}

    def get_typeNames(self):
        return enum._typeNames[type(self).__name__][self.value]

    def set_typeNames(self, value):
        enum._typeNames[type(self).__name__][self.value] = value

    typeNames = property(get_typeNames, set_typeNames)

    def __init__(self, *args, typeNames=None):
        cls = type(self)
        className = cls.__name__
        if className == "enum":
            raise TypeError(
                "You must subclass enum so each cls has its own typeNames"
            )

        if className not in enum._typeNames:
            enum._typeNames[className] = {}
            enum.next_roles[className] = 0

        if args:
            self.value = args[0]
            if len(args) > 1:
                # If this looks familiar its also in
                # QWidget and nottk (and maybe notk)
                raise ValueError(
                    "Expected either value as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )
        else:
            # cls.next_role doesn't work (backtracks)
            while enum.next_roles[className] in enum._typeNames[className]:
                enum.next_roles[className] += 1

            self.value = enum.next_roles[className]

        if self.value in enum._typeNames[className]:
            raise ValueError("%s is already used." % self.value)

        if typeNames is None:
            typeNames = []
        # self.typeNames[self.value] = typeNames
        # type(self).get_typeNames(self)[self.value] = typeNames
        # ^ same for every subclass, still... :( so:
        if not isinstance(typeNames, (list, tuple)):
            raise TypeError("list or tuple is required")

        enum._typeNames[className][self.value] = typeNames

        enum.next_roles[className] += 1

    def get(self):
        return self.value

    def set(self, value):
        self.value = value

    def __eq__(self, other):
        if isinstance(other, int):
            return self.value == other
        return self.value == other.value


class flags(enum):
    def __init__(self, *args, typeNames=None):
        cls = type(self)
        className = cls.__name__
        if className == "enum":
            raise TypeError(
                "You must subclass enum so each cls has its own typeNames"
            )

        if className not in flags._typeNames:
            flags._typeNames[className] = {}
            flags.next_roles[className] = 1

        if args:
            self.value = args[0]
            if len(args) > 1:
                # If this looks familiar its also in
                # QWidget and nottk (and maybe notk)
                raise ValueError(
                    "Expected either value as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )
        else:
            # cls.next_role doesn't work (backtracks)
            while flags.next_roles[className] in flags._typeNames[className]:
                flags.next_roles[className] *= 2

            self.value = flags.next_roles[className]

        if self.value in flags._typeNames[className]:
            raise ValueError("%s is already used." % self.value)

        if typeNames is None:
            typeNames = []
        # self.typeNames[self.value] = typeNames
        # type(self).get_typeNames(self)[self.value] = typeNames
        # ^ same for every subclass, still... :( so:
        if not isinstance(typeNames, (list, tuple)):
            raise TypeError("list or tuple is required")

        flags._typeNames[className][self.value] = typeNames

        flags.next_roles[className] *= 2

    def __or__(self, other):
        if isinstance(other, int):
            return self.value | other
        return self.value | other.value

    def __ror__(self, other):
        """Define right "or" to make "or" commutative."""
        if isinstance(other, int):
            return other | self.value
        return other.value | self.value


class Alignment(flags):  # mimic Qt::Alignment
    pass
    """
    typeNames = {}

    next_role = 0
    def __init__(self, *args, typeNames=None):
        # Pass self so subclass has separate roles!
        enum.__init__(self, *args, typeNames=typeNames)
    """


class ItemDataRole(enum):  # mimic Qt::ItemDataRole
    pass
    """
    Ugh...even this doesn't make a separate dict for
    each. It gets reset each time an instance is made.

    _typeNames = {}
    def get_typeNames(self):
        return type(self)._typeNames

    def set_typeNames(self, value):
        type(self)._typeNames = value

    typeNames = property(get_typeNames, set_typeNames)

    next_role = 0
    def __init__(self, *args, typeNames=None):
        # Pass self so subclass has separate roles!
        enum.__init__(self, *args, typeNames=typeNames)
    """

class ItemFlag(enum):
    pass

class CheckState(enum):
    pass


class SortOrder(enum):
    pass


class SizeMode(enum):
    pass


class SizeHint(enum):
    pass

class CaseSensitivity(enum):
    pass


class Qt:
    """Mimic Qt constants.

    Each enum set is typed, to simplify generation and allow adding type
    checking in Python.

    https://doc.qt.io/qt-6/qt.html

    Attributes:
        AlignAbsolute (Alignment): Absolute alignment side in RTL languages.
        AlignCenter (Alignment): Center on both dimensions.
        CaseSensitive (CaseSensitivity): Set case sensitivity.
    """
    lightGray = QColor.fromRgb(192, 192, 192)
    darkGreen = QColor.fromRgb(0, 128, 0)
    black = QColor.fromRgb(0, 0, 0)
    AlignLeft = Alignment()
    assert(AlignLeft.value == 1)
    assert(flags.next_roles["Alignment"] == 2)
    AlignRight = Alignment()
    assert(AlignRight.value == 2)
    AlignHCenter = Alignment()
    assert(AlignHCenter.value == 4)  # flags is exponential (enum is not)
    AlignJustify = Alignment(8)
    AlignAbsolute = Alignment(0x0010)
    AlignTop = Alignment(0x0020)
    AlignBottom = Alignment(0x0040)
    AlignVCenter = Alignment(0x0080)
    AlignBaseline = Alignment(0x0100)
    AlignLeading = AlignLeft
    AlignTrailing = AlignRight
    AlignHorizontal_Mask = Alignment(AlignLeft | AlignRight | AlignHCenter | AlignJustify | AlignAbsolute)
    AlignVertical_Mask = Alignment(AlignTop | AlignBottom | AlignVCenter | AlignBaseline)
    AlignCenter = Alignment(AlignVCenter | AlignHCenter)
    assert(AlignCenter == 4+128)

    CaseInsensitive = CaseSensitivity()
    CaseSensitive = CaseSensitivity()
    assert(CaseSensitive.value == 1)


    # General purpose roles:
    DisplayRole = ItemDataRole(typeNames=["QString"])
    assert(DisplayRole.value == 0)
    DecorationRole = ItemDataRole(1, typeNames=["QColor", "QIcon", "QPixmap"])
    EditRole = ItemDataRole(2, typeNames=["QString"])
    assert(DecorationRole.typeNames == ["QColor", "QIcon", "QPixmap"])
    # TODO: ^ Why does this it change to EditRole's? Even commented
    #   code for DecorationRole doesn't fix it. So try:
    # assert(enum._typeNames['DecorationRole'] == ["QColor", "QIcon", "QPixmap"])
    ToolTipRole = ItemDataRole(3, typeNames=["QString"])
    StatusTipRole = ItemDataRole(4, typeNames=["QString"])
    WhatsThisRole = ItemDataRole(5, typeNames=["QString"])
    SizeHintRole = ItemDataRole(13, typeNames=["QSize"])

    # appearance and meta data roles:
    FontRole = ItemDataRole(6, typeNames=["QFont"])
    TextAlignmentRole = ItemDataRole(7, typeNames=["Alignment"])
    BackgroundRole = ItemDataRole(8, typeNames=["QBrush"])
    ForegroundRole = ItemDataRole(9, typeNames=["QBrush"])
    CheckStateRole = ItemDataRole(10, typeNames=["Checkstate"])
    InitialSortOrderRole = ItemDataRole(14, typeNames=["SortOrder"])

    # Accessibility roles:
    AccessibleTextRole = ItemDataRole(11)
    AccessibleDescriptionRole = ItemDataRole(12)

    # User roles:
    UserRole = ItemDataRole(0x0100)  # This & up can be used for application-specific purposes

    NoItemFlags = ItemFlag(0)
    ItemIsSelectable = ItemFlag(1)
    ItemIsEditable = ItemFlag(2)
    ItemIsDragEnabled = ItemFlag(4)
    ItemIsDropEnabled = ItemFlag(8)
    ItemIsUserCheckable = ItemFlag(16)
    ItemIsEnabled = ItemFlag(32)
    ItemIsAutoTristate = ItemFlag(64)
    ItemNeverHasChildren = ItemFlag(128)
    ItemIsUserTristate = ItemFlag(256)

    Unchecked = CheckState(0)
    PartiallyChecked = CheckState(1)
    Checked = CheckState(2)

    AscendingOrder = SortOrder(0)
    DescendingOrder = SortOrder(1)

    AbsoluteSize = SizeMode(0)
    RelativeSize = SizeMode(1)

    MinimumSize = SizeHint(0)
    PreferredSize = SizeHint(1)
    MaximumSize = SizeHint(2)
    MinimumDescent = SizeHint(3)


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

    def toString(self):
        return self.get()


class NoQtMessage:
    def __init__(self, text, timeout):
        self.text = text
        self.timeout = timeout
        self.start = noqt_tick()


class QStatusBar(QWidget, ttk.Label):
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
    Args:
        sender: the sending object
        sig: the sending object's event that occurs (implemented in noqt
            as a list of slots)
        receiver: the handler object
        slot: the handler
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
    Use the connect( function, which in noqt for Python, just adds the given
    signal (just a function in noqt) to the slot (just a list below: timeout)


    Attributes:
    timeout -- It mimics the SLOTS feature of Qt in a much simpler way:
        It is just a list of slots (function references in this case)
        for the timeout signal which is now a concept rather than a
        construct (_on_timeout runs each function in the timeout list).
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
        self._timer = threading.Timer(self._interval/1000.0, self._on_timeout)
        # ^ Qt interval is in ms (Python threading.Timer is in seconds)!
        self._timer.start()

    def stop(self):
        if self._timer is not None:
            self._timer.cancel()
            self._timer = None
            self._interval = None

    def _on_timeout(self):
        prefix = "[QTimer _on_timeout] "
        echo2(prefix+"running")
        for slot in self.timeout:
            slot()
        if not self._singleShot:
            self.start()
            # threading.Timer can only be started once.


def _ui_subtree(ui, parentWidget, parentNode, ui_file, indent=""):
    """
    Args:
        ui (QWidget): Every widget will be added as a member to this,
            even if outside of central widget or nested inside of
            a layout.
        parentWidget: (any QWidget or QLayout subclass)
        parentNode: The xml node to traverse.
        ui_file (string): Path to ui file, only for traceable errors
    """
    raise NotImplementedError(
        "This works but is cumbersome (too many subclasses and"
        " subtrees for noqqttk (as opposed to noqt with notk)"
        " to work well)"
    )
    count = 0
    prefix = "[outputinspector noqt _ui_subtree] " + indent
    more_args = {}
    for node in parentNode:
        if node.tag in ("widget", "layout", "item"):
            if node.tag == "item":
                className = "QLayout"
                varName = None
                thisType = QLayout
                # QLayout because QLayoutItem has no
                #   addChildLayout nor addChildWidget
            else:
                className = node.attrib["class"]
                varName = node.attrib["name"]
                thisType = globals()[className]
                if className == "QMainWindow":
                    raise RuntimeError(
                        "QMainWindow should be the highest parentWidget"
                        " under ui file's XML root but it was a child."
                    )
                elif className == "QListWidget":  # "QListView"
                    scrollbar = tk.Scrollbar(parentWidget, orient="vertical");
                    more_args['yscrollcommand'] = scrollbar.set
                if className not in globals():
                    raise NotImplementedError(className)
            echo2(prefix+"[%s] %s" % (node.tag, className))
            subObj = thisType(
                parentWidget,
                **more_args,
            )
            # ^ Should never call QMainWindow constructor--that was
            #   already constructed (or constructor called this and
            #   it is incomplete)
            # ^ QLayoutItem only takes Alignment or nothing (don't
            #   use it anyway, because it has no addChild* methods)

            subObj.name = varName
            if node.tag == "widget":
                echo2(prefix+"  parent adding %s = %s()  # %s"
                      % (varName, className, node.tag))
                if hasattr(parentWidget, "addChildWidget"):
                    parentWidget.addChildWidget(subObj)
                else:
                    # It is a layout and instead has:
                    parentWidget.addWidget(subObj)
            else:
                echo2(prefix+"  parent adding %s = %s()  # %s"
                      % (varName, className, node.tag))
                if hasattr(parentWidget, "addChildLayout"):
                    parentWidget.addChildLayout(subObj)
                elif hasattr(parentWidget, "addLayout"):
                    # It is a layout and instead has:
                    parentWidget.addLayout(subObj)
                else:
                    # It is a Widget and instead has:
                    parentWidget.setLayout(subObj)
            if varName is not None:
                setattr(parentWidget, varName, subObj)
                if node.tag == "widget":
                    echo2(prefix+"  +Adding %s = %s()  # %s to ui"
                          % (varName, className, node.tag))
                    setattr(ui, varName, subObj)
                    # ^ widget must always be a member of ui even if
                    #   in a layout (which can in turn be inside
                    #   QWidget centralWidget)
                else:
                    echo2(prefix+"  *not* adding %s = %s()  # %s to ui"
                          % (varName, className, node.tag))
            else:
                echo2(prefix+"  *not* adding %s = %s()  # %s to ui"
                        % (varName, className, node.tag))

            sub_count = _ui_subtree(ui, subObj, node, ui_file,
                                indent=indent+"  ")
            count += 1
            if sub_count == 0:
                echo2(prefix+"[/ leaf] done %s = %s()  # %s"
                      % (varName, className, node.tag))
            else:
                echo2(prefix+"[/ children=%s] done %s = %s()  # %s"
                      % (sub_count, varName, className, node.tag))
        elif node.tag == "property":
            name = node.attrib.get('name')
            echo1(prefix+"NotImplemented: %s  # %s in %s"
                  % (node.tag, name, ui_file))
        else:
            parentName = parentNode.attrib.get('name')
            if parentName is None:
                parentName = "a " + parentNode.tag
            echo1(prefix+'Unknown tag "%s"'
                  ' under %s in %s'
                  % (node.tag, parentName, ui_file))
    return count


def _ui_loader(self, ui, ui_file):
    """
    To behave like Qt, _ui (ui in Qt) must be equivalent to the centralWidget of
    the ui file, even though _ui is inside of QMainWindow and the centralWidget
    of the ui file is QMainWindow (usually or always (?)).

    Due to this function the code does *not* have statements like:
    - _ui.mainListWidget = QListWidget
    - self.statusBar = QStatusBar(self)

    (Instead, the class named in the ui file is used, to imitate Qt.)
    """
    prefix = "[outputinspector noqt _ui_loader] "
    echo1(prefix+"Loading %s in CLI mode." % ui_file)
    xmltree = ET.parse(ui_file)
    xmlroot = xmltree.getroot()  # such as <ui version="4.0">
    self.className = type(self).__name__
    for node in xmlroot:
        if node.tag == "class":
            self.className = node.text
        elif node.tag == "widget":
            _ui_subtree(ui, self, node, ui_file)
        else:
            echo1(prefix+"Unknown tag %s under %s in %s"
                  % (node.tag, self.className, ui_file))


QMainWindow_callerName = None


class QMainWindow(QWidget):
    def __init__(self, *args, ui_file=None):
        global QMainWindow_callerName
        prefix = "[QMainWindow] "
        # callerName = inspect.stack()[1][3]
        # ^ callerName is useless, just "__init__"
        callerName = type(self).__name__
        message = prefix+"%s triggered init" % callerName
        if QMainWindow_callerName is not None:
            raise RuntimeError(
                message+" (already done by %s)" % QMainWindow_callerName
            )
        else:
            echo1(message)
        QMainWindow_callerName = callerName
        del message
        echo1(prefix+"initializing")
        QWidget.__init__(self, *args)
        # if not hasattr(self, "is_gui") or not self.is_gui():
        # FIXME: Why isn't this ever False even when run from MainWindow?
        import outputinspector
        # if not outputinspector.ENABLE_GUI:
        if not hasattr(self, "is_gui") or not self.is_gui():
            import outputinspector.nottk
            echo0(prefix+"using CLI frame for %s" % type(self).__name__)
            outputinspector.nottk.Frame.__init__(self, *args)
        else:
            if not hasattr(self, TK_ONLY_ATTRIBUTE):
                # ^ ok since "_setup", should *not* be in notk's, only tk's.
                # This should *only* be true if subclass
                #   is also a subclass of ttk.Frame.

                # for some reason it is missing even though the
                #   correct subclass is being used--must be since
                #   the ttk.Frame class is incompete at this stage, so pass
                pass
                """
                raise RuntimeError(
                    prefix+"%s is using non-Tk subclass as GUI frame for %s"
                    % (outputinspector.caller_info_str(),
                       type(self).__name__)
                )
                """
            # Only if it is a *real* tk.Widget subclass!
            #   The fake one (non-subclassed OutputInspector
            #   adds this method.)
            ttk.Frame.__init__(self, *args)
            # NOTE: ^ still, this will only help in subclass, since
            #   this class doesn't have ttk.Frame methods!
        if ui_file is None:
            raise ValueError(
                prefix+"In this Python shim (noqt.py),"
                " the ui_file keyword argument is required."
            )

        if args:
            if not hasattr(args[0], "parentWidget"):
                # ^ Every QWidget subclass has the parentWidget method
                #   (This could check for any other QWidget attribute
                #   or method, but not in OutputInspector because it
                #   has a CLI mode so a subclass of OutputInspector
                #   and QMainWindow is not always used--but
                #   OutputInspector has the parentWidget method).
                echo0(
                    "Warning: Expected 1 parent widget"
                    " (or no sequential args)"
                    " but got a(n) %s."
                    % type(args[0]).__name__
                )
        if not isinstance(ui_file, str):
            # We must be making self
            return
            # raise TypeError("Expected string (ui file path) but got a(n) %s"
            #                 % type(ui_file).__name__)
        if not os.path.isfile(ui_file):
            raise FileNotFoundError(ui_file)
        self._ui = QWidget(self)  # centralWidget type is QWidget
        # but _ui is a special container that also contains
        # widgets outside of centralWidget such as
        # a QStatusBar
        _ui_loader(self, self._ui, ui_file)
        # ^ To behave like Qt, _ui (ui in Qt) must be
        # equivalent to the centralWidget of the ui file.

    def is_gui(self):
        return False  # override and return False in subclass in noqttk
