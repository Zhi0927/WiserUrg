#include "Qt_urg.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Qt_urg w;
    w.show();
    return a.exec();
}
