#include "mainwindow.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void info(string msg) {
    cerr << "INFO: " << msg << endl;
}

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
    for (int i=0; i<qArgs.size(); i++) {
        string qArg = qArgs[i];
        if (!startswithCS(qArg, "--")) {
            sErrorsListFileName = qArg;
        }
        else {
            int signIndex = findCS(qArg, "=", 0);
            if (signIndex>-1) {
                int valueIndex = signIndex + 1;
                string name = qArg.substr(2, signIndex-2);
                window.settings->setValue(name, strip(qArg.substr(valueIndex)));
                info("set " + name + " to '"
                     + strip(qArg.substr(valueIndex)) + "'");
            }
        }
    }
    window.init(strip(sErrorsListFileName));
    // TODO: window.show();
    //app.setWindowIcon(QIcon("outputinspector-64.png"));
    //app.setWindowIcon(QIcon(ICON));
    // TODO: return app.exec();
}
