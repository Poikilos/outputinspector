#include "mainwindow.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    std::cin.sync_with_stdio(false); /**< Stop in_avail from always being 0
                                          (See <https://stackoverflow.com/
                                            questions/17474252/
        why-does-in-avail-output-zero-even-if-the-stream-has-some-char) */
    // QApplication app(argc, argv);
    // app.setOrganizationDomain("poikilos.org");
    // app.setApplicationName("outputinspector");
    MainWindow window;
    string sErrorsListFileName; //reverts to err.txt if left blank
    std::vector<std::string> qArgs(argv + 1, argv + argc);
    // ^ See https://stackoverflow.com/questions/6361606/save-argv-to-vector-or-string
    // start at 1 since qArgs[0] is self:
    for (int i=0; i<qArgs.length(); i++) {
        string qArg = qArgs[i];
        if (!qArg.startsWith("--")) {
            sErrorsListFileName = qArg;
        }
        else {
            int signIndex = qArg.indexOf("=");
            if (signIndex>-1) {
                int valueIndex = signIndex + 1;
                string name = qArg.mid(2, signIndex-2);
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
    // return app.exec();
}
