#include <qapplication.h>

#include "frmmain.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    
    frmMain *m = new frmMain();
    m->show();
    
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    
    return a.exec();
}
