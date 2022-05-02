#ifndef HKY_QT_URGRUN_H_
#define HKY_QT_URGRUN_H_

#include "URGObjectDetector.hpp"
#include <QtWidgets/QWidget>
#include "ui_Qt_urg.h"

class Qt_urg : public QWidget
{
    Q_OBJECT
public:
    Qt_urg(QWidget* parent = Q_NULLPTR);
    ~Qt_urg();

    void drawRect(const Rect& rect, QColor color = Qt::red);
    void setPlottemplate();
    void plot();

private slots: 
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();
    void DrawMain();

public:
    QColor distanceColor = Qt::blue;
    QColor objectColor = Qt::green;
    QColor processedObjectColor = Qt::cyan;

private:
    Ui::Qt_urgClass* ui;
    std::unique_ptr <URGSensorObjectDetector> m_urgdetector;
    bool isdetect = false;

    QVector<double> RayX, RayY;
    QVector<double> RawObjX, RawObjY;
    QVector<double> PosObjX, PosObjY;
};
#endif
