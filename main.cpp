#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QString sErrorsListFileName; //reverts to err.txt if left blank
    QStringList qArgs = QCoreApplication::arguments();
    if (qArgs.length()>1) {
        sErrorsListFileName = qArgs[1]; //0 is self
    }
    w.init(sErrorsListFileName);
    w.show();
    //a.setWindowIcon(QIcon("outputinspector-64.png"));
    //a.setWindowIcon(QIcon(ICON));
    return a.exec();
}
