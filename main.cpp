#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    std::cin.sync_with_stdio(false); /**< Stop in_avail from always being 0
                                          (See <https://stackoverflow.com/
                                            questions/17474252/
        why-does-in-avail-output-zero-even-if-the-stream-has-some-char) */
    QApplication app(argc, argv);
    // app.setOrganizationDomain("poikilos.org");
    app.setApplicationName("outputinspector");
    MainWindow window;
    QString sErrorsListFileName; //reverts to err.txt if left blank
    QStringList qArgs = QCoreApplication::arguments();
    // start at 1 since qArgs[0] is self:
    for (int i=1; i<qArgs.length(); i++) {
        QString qArg = qArgs[i];
        if (!qArg.startsWith("--")) {
            sErrorsListFileName = qArg;
        }
        else {
            int signIndex = qArg.indexOf("=");
            if (signIndex>-1) {
                int valueIndex = signIndex + 1;
                QString name = qArg.mid(2, signIndex-2);
                window.settings->setValue(name, qArg.mid(valueIndex).trimmed());
                qInfo() << "set " + name + " to '"
                           + qArg.mid(valueIndex).trimmed() + "'";
            }
        }
    }
    window.init(sErrorsListFileName.trimmed());
    window.show();
    //app.setWindowIcon(QIcon("outputinspector-64.png"));
    //app.setWindowIcon(QIcon(ICON));
    return app.exec();
}
