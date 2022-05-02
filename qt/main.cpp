#include "QT_URG.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QT_URG w;
    w.setWindowTitle("IMD");
    w.show();
    return a.exec();
}
