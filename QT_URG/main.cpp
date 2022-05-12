#include "QT_URG.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QT_URG w;
    w.setWindowTitle("WiserZhi URG");
    w.setGeometry(QRect(0, 0, 900, 625));
    w.show();
    return a.exec();
}
