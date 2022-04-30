#pragma once

#include "URGSensorObjectDetector.hpp"
#include <QtWidgets/QWidget>
#include "ui_Qt_urg.h"


class Qt_urg : public QWidget
{
    Q_OBJECT
public:
    Qt_urg(QWidget* parent = Q_NULLPTR);
    ~Qt_urg();

    void drawRect(const Rect& rect, QColor color = Qt::red);
    void drawLine(const vector3& from, const vector3& to, QColor color = Qt::green);
    void plot();

    QColor distanceColor = Qt::green;
    QColor strengthColor = Qt::yellow;
    QColor objectColor = Qt::red;
    QColor processedObjectColor = Qt::cyan;

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();
    void DrawMain();

private:
    Ui::Qt_urgClass* ui;
    std::unique_ptr <URGSensorObjectDetector> m_urgdetector;
    bool isdetect = false;

    QVector<double> rayX;
    QVector<double> rayY;
};
