#include <QtGui/QApplication>
#include "configurator.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Configurator w;
    w.show();
    
    return a.exec();
}
