#ifndef HKY_QT_URGRUN_H_
#define HKY_QT_URGRUN_H_

#include "URGObjectDetector.hpp"
#include <QtWidgets/QWidget>
#include "ui_QT_URG.h"

class QT_URG : public QWidget
{
    Q_OBJECT
public:
    QT_URG(QWidget *parent = Q_NULLPTR);
    ~QT_URG();

    void drawRect(const Rect& rect, QColor color = Qt::red);
    void setPlottemplate();
    void plot();

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void setConstraintRegion_Button();
    void setParm_Buttom();
    void smatoggle(bool checkstate);
    void useOffset(bool checkstate);

    void DrawMain();

public:
    QColor distanceColor = Qt::blue;
    QColor objectColor = Qt::green;
    QColor processedObjectColor = Qt::cyan;

private:
    Ui::Qt_urgClass* ui;
    std::unique_ptr <URGSensorObjectDetector> m_urgdetector;
    bool isdetect = false;

    QVector<double> PointX, PointY;
    QVector<double> RawObjX, RawObjY;
    QVector<double> PosObjX, PosObjY;
    //QVector<double> LineX, LineY;

    QPointer<QCPItemRect> xRectItem;
};
#endif
