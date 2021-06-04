#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qmenubar.h>
#include <qapplication.h>
#include <qimage.h>
#include <qtimer.h>

#include "canvas.hxx"

#include <stdlib.h>
#include <TApplication.h>
#include <TSystem.h>

int main(int argc, char** argv)
{
    TApplication rootapp("Simple Qt ROOT Application", &argc, argv);
    QApplication app(argc,argv);
    QMainCanvas m(0);

    m.resize(m.sizeHint());
    m.setWindowTitle("Qt Example - Canvas");
    m.setGeometry( 100, 100, 700, 500 );
    m.show();

    QObject::connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );

    return app.exec();
}


