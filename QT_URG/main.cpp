#include "QT_URG.h"
#include <QtWidgets/QApplication>
#include <QInputDialog>
#include "RegisterManager.hpp"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);

    RegisterManager* regManger = new RegisterManager();
    if (!regManger->checkActivate()) {
        QMessageBox::warning(NULL, QObject::tr("Oops"), QObject::tr("Illegal Device!\nPlease check that you have activated via the WiserUrg Register Tool."));

        QString itemText = QInputDialog::getText(0, QObject::tr("ActivateCode"), QObject::tr("please input key:"));
        if (!regManger->getEncode().isNull() && itemText == regManger->getEncode()) {
            regManger->setActivateCodetoRegedit(itemText);
            QMessageBox::warning(NULL, QObject::tr("Success"), QObject::tr("Activate successfuly"));
        }
        else {
            QMessageBox::warning(NULL, QObject::tr("Fail"), QObject::tr("Activate Failed"));
            return -1;
        }
        delete regManger;
    }

    QT_URG w;
    w.show();
    return a.exec();
}
