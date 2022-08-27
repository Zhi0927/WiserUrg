
#ifndef ConfigMgr_H
#define ConfigMgr_H

#include <QString>
#include <QWidget>
#include <QSettings>
#include <QCoreApplication>
#include "ui_QT_URG.h"

class ConfigManager
{
public:
    static ConfigManager* Instance() {
        static ConfigManager singleton;
        return &singleton;
    }

    void SaveWindowSize(QWidget* pWnd);
    void LoadWindowSize(QWidget* pWnd);
    void SaveParameter(Ui::Qt_urgClass* ui);
    void LoadParameter(Ui::Qt_urgClass* ui);

private:
    ConfigManager();

public:
    QString mIniPath;
};

#endif
