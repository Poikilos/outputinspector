#!/usr/bin/env python
from outputinspector.mainwindow import MainWindow

def main(self, argc, *argv[]):
    std.cin.sync_with_stdio(False); '''*< Stop in_avail from always being 0
                                          (See <https:#stackoverflow.com/
                                            questions/17474252/
        why-does-in-avail-output-zero-even-if-the-stream-has-some-char) '''
    QApplication app(argc, argv)
    # app.setOrganizationDomain("poikilos.org")
    app.setApplicationName("outputinspector")
    MainWindow window
    QString sErrorsListFileName; #reverts to err.txt if left blank
    qArgs = QCoreApplication.arguments()
    # start at 1 since qArgs[0] is self:
    for (int i=1; i<qArgs.length(); i++)
        qArg = qArgs[i]
        if not qArg.startsWith("--"):
            sErrorsListFileName = qArg

        else:
            signIndex = qArg.indexOf("=")
            if signIndex>-1:
                valueIndex = signIndex + 1
                name = qArg.mid(2, signIndex-2)
                window.settings.setValue(name, qArg.mid(valueIndex).trimmed())
                qInfo() << "set " + name + " to '"
                        + qArg.mid(valueIndex).trimmed() + "'"



    window.init(sErrorsListFileName.trimmed())
    window.show()
    #app.setWindowIcon(QIcon("outputinspector-64.png"))
    #app.setWindowIcon(QIcon(ICON))
    return app.exec()

