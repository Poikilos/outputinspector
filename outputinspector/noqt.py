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
import time
import threading
import os
import platform
import inspect

import xml.etree.ElementTree as ET

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
REPO_DIR = os.path.dirname(MODULE_DIR)

if __name__ == "__main__":
    sys.path.insert(0, REPO_DIR)
    # ^ allows importing REPO_DIR

ENABLE_GUI = True
for argi in range(len(sys.argv)):
    arg = sys.argv[argi]
    if arg == "--cli":
        ENABLE_GUI = False

if ENABLE_GUI:
    print("[noqt] using GUI mode", file=sys.stderr)
    if sys.version_info.major >= 3:
        import tkinter as tk
        from tkinter import ttk
        from tkinter import messagebox
    else:
        import Tkinter as tk
        import ttk
        import tkMessageBox as messagebox
else:
    print("[noqt] using CLI mode", file=sys.stderr)
    import outputinspector.notk as tk
    import outputinspector.nottk as ttk
    from outputinspector.notkinter import messagebox

verbosity = 2
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

    def isEmpty():
        return not self._children


class QLayout(QLayoutItem):
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


class QWidget(object):
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
        QAbstractItemView.__init__(self)  # set _size etc
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
        echo0(prefix+"adding a %s from %s: %s" % (
            type(qlistwidgetitem).__name__,
            os.path.basename(inspect.getfile(type(qlistwidgetitem))),
            qlistwidgetitem.get(),
        ))
        self.insert(tk.END, qlistwidgetitem)
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


class QListWidgetItem(tk.StringVar):  # TODO: also inherit from QWidget?
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
        echo0("WARNING: Using GUI-like QListWidgetItem in CLI\n")
        QWidget.__init__(self, *args, **kwargs)
        # ^ sets self._parent
        self.roles = []
        if len(args) > 0:
            kwargs['value'] = args[0]
        if len(args) > 1:
            raise ValueError("Too many args")
        if args:
            tk.StringVar.__init__(self, args[0], **kwargs)
        else:
            tk.StringVar.__init__(self, **kwargs)
        # ^ will raise an exception if MainWindow (or tk.Tk) not initialized
        self.index = None
        self.queued_tk_args = {}

    def __repr__(self):
        return self.get()

    def setData(self, role, var):

        if hasattr(var, "get"):
            # Such as QVariant
            value = var.get()
        else:
            value = var
        

    def setForeground(self, qbrush):
        if (self._parent is None) or (self.index is None):
            # ^ self._parent should be set by QWidget constructor
            #   which this subclass's constructor should call.  
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
    """This is an enum that is better than Enum because type of value
    is known when subclassed.
    """
    next_value = 0
    used_values = set()  # or {value,}  # Pythonic init like dict but no pairs
    types = []
    def __init__(self, *args, types=None):
        cls = type(self)
        if args:
            self.value = args[0]
            cls.used_values.add(self.value)
            if len(args) > 1:
                # If this looks familiar its also in
                # QWidget and nottk (and maybe notk)
                raise ValueError(
                    "Expected either value as 1st arg or no args but"
                    " got more sequential args: %s"
                    % (args[1:])
                )

            if args[0] in cls.used_values:
                raise ValueError("%s is already used." % args[0])
            return
        while cls.next_value in cls.used_values:
            cls.next_value += 1


        self.value = cls.next_value
        cls.used_values.add(self.value)

        if types:
            if not isinstance(types, (list, tuple)):
                raise TypeError("list or tuple is required")
            cls.types = types

        cls.next_value += 1

class Alignment(enum):  # mimic Qt::Alignment
    pass

class ItemDataRole(enum):  # mimic Qt::ItemDataRole
    pass

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

class Qt:
    lightGray = QColor.fromRgb(192, 192, 192)
    darkGreen = QColor.fromRgb(0, 128, 0)
    black = QColor.fromRgb(0, 0, 0)
    assert(Alignment.next_value == 0)
    AlignLeft = Alignment()
    assert(Alignment.next_value != 0)
    AlignRight = Alignment()
    assert(AlignLeft.value != AlignRight.value)
    AlignBottom = Alignment()
    AlignTop = Alignment()
    AlignCenter = Alignment()
    AlignHCenter = Alignment()
    AlignVCenter = Alignment()
    # AlignJustify etc. were in Qt4.

    # General purpose roles:
    DisplayRole = ItemDataRole(types=["QString"])
    echo0("DisplayRole.value = %s" % DisplayRole.value)
    assert(DisplayRole.value == 0)
    DecorationRole = ItemDataRole(1, types=["QColor", "QIcon", "QPixmap"])
    EditRole = ItemDataRole(2, types=["QString"])
    assert(DecorationRole.types == ["QColor", "QIcon", "QPixmap"])
    ToolTipRole = ItemDataRole(3, types=["QString"])
    StatusTipRole = ItemDataRole(4, types=["QString"])
    WhatsThisRole = ItemDataRole(5, types=["QString"])
    DisplayRole = ItemDataRole(6, types=["QString"])
    SizeHintRole = ItemDataRole(13, types=["QSize"])

    # appearance and meta data roles:
    FontRole = ItemDataRole(6, types=["QFont"])
    TextAlignmentRole = ItemDataRole(7, types=["Alignment"])
    BackgroundRole = ItemDataRole(8, types=["QBrush"])
    ForegroundRole = ItemDataRole(9, types=["QBrush"])
    CheckStateRole = ItemDataRole(10, types=["Checkstate"])
    InitialSortOrderRole = ItemDataRole(14, types=["SortOrder"])

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


class NoQtMessage:
    def __init__(self, text, timeout):
        self.text = text
        self.timeout = timeout
        self.start = noqt_tick()


class QStatusBar(ttk.Label):
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
    Sequential arguments:
    sender -- the sending object
    sig -- the sending object's event that occurs (implemented in noqt
        as a list of slots)
    receiver -- the handler object
    slot -- the handler
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
        self._timer = threading.Timer(self._interval, self._on_timeout)
        self._timer.start()

    def stop(self):
        if self._timer is not None:
            self._timer.cancel()
            self._timer = None
            self._interval = None

    def _on_timeout(self):
        echo2("* running _on_timeout")
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
    count = 0
    prefix = "[outputinspector noqt _ui_subtree] " + indent
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
                if className not in globals():
                    raise NotImplementedError(className)
            echo0(prefix+"[%s] %s" % (node.tag, className))
            subObj = thisType(parentWidget)
            # ^ Should never call QMainWindow constructor--that was
            #   already constructed (or constructor called this and
            #   it is incomplete)
            # ^ QLayoutItem only takes Alignment or nothing (don't
            #   use it anyway, because it has no addChild* methods)
            
            subObj.name = varName
            if node.tag == "widget":
                echo0(prefix+"  parent adding %s = %s()  # %s"
                        % (varName, className, node.tag))
                if hasattr(parentWidget, "addChildWidget"):
                    parentWidget.addChildWidget(subObj)
                else:
                    # It is a layout and instead has:
                    parentWidget.addWidget(subObj)
            else:
                echo0(prefix+"  parent adding %s = %s()  # %s"
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
                    echo0(prefix+"  +Adding %s = %s()  # %s to ui"
                          % (varName, className, node.tag))
                    setattr(ui, varName, subObj)
                    # ^ widget must always be a member of ui even if
                    #   in a layout (which can in turn be inside
                    #   QWidget centralWidget)
                else:
                    echo0(prefix+"  *not* adding %s = %s()  # %s to ui"
                          % (varName, className, node.tag))
            else:
                echo0(prefix+"  *not* adding %s = %s()  # %s to ui"
                        % (varName, className, node.tag))

            sub_count = _ui_subtree(ui, subObj, node, ui_file,
                                indent=indent+"  ")
            count += 1
            if sub_count == 0:
                echo0(prefix+"[/ leaf] done %s = %s()  # %s"
                      % (varName, className, node.tag))
            else:
                echo0(prefix+"[/ children=%s] done %s = %s()  # %s"
                      % (sub_count, varName, className, node.tag))
        elif node.tag == "property":
            name = node.attrib.get('name')
            echo0(prefix+"NotImplemented: %s  # %s in %s"
                  % (node.tag, name, ui_file))
        else:
            parentName = parentNode.attrib.get('name')
            if parentName is None:
                parentName = "a " + parentNode.tag
            echo0(prefix+'Unknown tag "%s"'
                  ' under %s in %s'
                  % (node.tag, parentName, ui_file))
    return count

def _ui_loader(self, ui, ui_file):
    """
    To behave like Qt, _ui (ui in Qt) must be equivalent to the centralWidget of
    the ui file, even though _ui is inside of QMainWindow and the centralWidget
    of the ui file is QMainWindow (usually or always (?)).
    """
    prefix = "[outputinspector noqt _ui_loader] "
    echo0(prefix+"Loading %s in no-GUI mode." % ui_file)
    xmltree = ET.parse(ui_file)
    xmlroot = xmltree.getroot()  # such as <ui version="4.0">
    self.className = type(self).__name__
    for node in xmlroot:
        if node.tag == "class":
            self.className = node.text
        elif node.tag == "widget":
            _ui_subtree(ui, self, node, ui_file)
        else:
            echo0(prefix+"Unknown tag %s under %s in %s"
                  % (node.tag, self.className, ui_file))



class QMainWindow(QWidget, ttk.Frame):
    def __init__(self, *args, ui_file=None):
        prefix = "[QMainWindow] "
        echo0(prefix+"initializing")
        ttk.Frame.__init__(self)
        QWidget.__init__(self, *args)
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
                raise TypeError("Expected 1 parent widget"
                                " (or no sequential args)"
                                " but got a(n) %s."
                                % type(args[0]).__name__)
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
        