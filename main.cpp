#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //a.setWindowIcon(QIcon("outputinspector-64.png"));
    //a.setWindowIcon(QIcon(ICON));
    return a.exec();
}
