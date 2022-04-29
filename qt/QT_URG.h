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

    void DrawMain();
    void plot();
    void drawRect(const Rect& rect, QColor color = Qt::red);
    void drawLine(const vector3& from, const vector3& to, QColor color = Qt::green);

    QColor distanceColor        = Qt::green;
    QColor strengthColor        = Qt::yellow;
    QColor objectColor          = Qt::red;
    QColor processedObjectColor = Qt::cyan;

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();

private:
    Ui::QT_URGClass* ui;
    //std::unique_ptr <URGSensorObjectDetector> m_urgdetector;
    URGSensorObjectDetector m_urgdetector;
    bool isdetect = false;
};
