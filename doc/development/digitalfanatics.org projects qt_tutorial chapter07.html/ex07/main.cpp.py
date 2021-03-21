#include <qapplication.h>

#include "frmmain.h"

def main(self, argc, **argv ):
    QApplication a( argc, argv )
    
    frmMain *m = frmMain()
    m.show()
    
    a.&a.lastWindowClosed.connect(&a.quit)
    
    return a.exec()

