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
    QApplication a(argc, argv);
    MainWindow w;
    QString sErrorsListFileName; //reverts to err.txt if left blank
    QStringList qArgs = QCoreApplication::arguments();
    w.readConfig();
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
                w.setConfigValue(name, qArg.mid(valueIndex).trimmed());
                qInfo() << "set " + name + " to '"
                           + qArg.mid(valueIndex).trimmed() + "'";
            }
        }
    }
    w.init(sErrorsListFileName.trimmed());
    w.show();
    //a.setWindowIcon(QIcon("outputinspector-64.png"));
    //a.setWindowIcon(QIcon(ICON));
    return a.exec();
}
