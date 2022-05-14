#ifndef ZHI_QT_URGRUN_H_
#define ZHI_QT_URGRUN_H_

#include "EthernetConnector.hpp"
#include "SCIP_library.hpp"
#include "ObjectDetector.hpp"
#include "MouseSimulator.hpp"

#include <QtWidgets/QWidget>
#include "ui_QT_URG.h"

class QT_URG : public QWidget
{
    Q_OBJECT

public:
    QT_URG(QWidget *parent = Q_NULLPTR);
    ~QT_URG();

private:
    void drawRect(const Rect& rect, QColor color = Qt::red);
    void drawLabel(QPointer<QCPItemText>& item, const float x, const float y, const QString& text, int fontsize = 16,  QColor color = Qt::black);
    void InitFunc();
    void setData();
    void clearData();

    void Mainloop();

private slots:
    void ConnectTcp_Button();
    void DisconnectTcp_Button();
    void ConnectTcp02_Button();
    void DisconnectTcp02_Button();

    void setConstraintRegion_Button();
    void setParm_Buttom();
    void useOffset(bool checkstate);
    void useTouchEvent(bool checkstate);

    void DrawMain();

public:
    QColor distanceColor01          = Qt::blue;
    QColor distanceColor02          = Qt::magenta;

    QColor objectColor              = Qt::green;
    QColor objectPointColor         = Qt::red;

    QColor processedObjectColor     = Qt::cyan;
                                    
    QColor RectColor                = Qt::red;
    QColor LabelColor               = Qt::black;

private:
    Ui::Qt_urgClass* ui;

    std::unique_ptr<ObjectDetector>     UrgDetector;
    std::unique_ptr<MouseSimulator>		UrgMouse;
    std::unique_ptr<EthernetConnector>	UrgNet01;
    std::unique_ptr<EthernetConnector>	UrgNet02;
    std::mutex                          distance_guard;

    std::vector<long>					Origindistance01;
    std::vector<long>					Origindistance02;
    std::vector<long>					Previewdistance01;
    std::vector<long>					Previewdistance02;
    int                                 Scanstep = 0;
    bool                                RegionInverse = false; //part1 left, part2 right

    QVector<double> PointX01,  PointY01;
    QVector<double> PointX02,  PointY02;
    QVector<double> RawObjX,   RawObjY;
    QVector<double> PosObjX,   PosObjY;
    QVector<double> ObjPointX, ObjPointY;

    QPointer<QCPItemRect> RectItem      = nullptr;
    QPointer<QCPItemText> LabelItem01   = nullptr;
    QPointer<QCPItemText> LabelItem02   = nullptr;
};
#endif
