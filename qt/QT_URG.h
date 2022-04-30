#pragma once

#include "URGSensorObjectDetector.hpp"
#include <QtWidgets/QWidget>
#include "ui_QT_URG.h"

class QT_URG : public QWidget
{
    Q_OBJECT

public:
    QT_URG(QWidget *parent = Q_NULLPTR);
    ~QT_URG();

    QColor distanceColor        = Qt::green;
    QColor strengthColor        = Qt::yellow;
    QColor objectColor          = Qt::red;
    QColor processedObjectColor = Qt::cyan;

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();

    void drawRect(const Rect& rect, QColor color = Qt::red);
    void drawLine(const vector3& from, const vector3& to, QColor color = Qt::green);
    void DrawMain();
    void plot();

private:
    Ui::QT_URGClass* ui;
    std::unique_ptr <URGSensorObjectDetector> m_urgdetector;
    std::unique_ptr<std::thread> m_mainthread;

    bool isdetect = false;
};
