#include "Qt_urg.h"

Qt_urg::Qt_urg(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::Qt_urgClass)
{
    ui->setupUi(this);
    isdetect = false;

    setPlottemplate();

    // Plot
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    ui->plot->setPlottingHint(QCP::phFastPolylines);

    // Button
    connect(ui->Connect_Button, SIGNAL(clicked()), this, SLOT(ConnectTcp_Button()));
    connect(ui->Disconnect_Button, SIGNAL(clicked()), this, SLOT(DisconnectTcp_Button()));
    connect(ui->SetCR_Button, SIGNAL(clicked()), this, SLOT(setConstraintRegion_Button()));

    QTimer* dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(DrawMain()));
    dataTimer->start();

    std::cout << "Initial QT successful!" << std::endl;
}

Qt_urg::~Qt_urg() {
    delete ui;
}


void Qt_urg::DrawMain() {
    if (isdetect) {
        m_urgdetector->mainloop();
        const std::vector<long>& croppedDistances           = m_urgdetector->GetcroppedDistances();
        const std::vector<vector3>& directions              = m_urgdetector->GetDirection();
        const std::vector<RawObject>& rawObjectList         = m_urgdetector->GetRawObjectList();
        const std::vector<ProcessedObject>& detectedObjects = m_urgdetector->GetDetectObjects();


        if (ui->DrawRay->isChecked() && !croppedDistances.empty()) {
            for (int i = 0; i < croppedDistances.size(); i++) {
                vector3 result = directions[i] * croppedDistances[i];
                RayX.append(static_cast<double>(result.x));
                RayY.append(static_cast<double>(result.y));
            }         
        }
        
        if (ui->DrawObject->isChecked() && !rawObjectList.empty()) {
            for (int i = 0; i < rawObjectList.size(); i++) {
                auto obj = rawObjectList[i];

                if (obj.idList.size() == 0 || obj.distList.size() == 0)  return;

                vector3 dir = directions[obj.medianId()];
                long dist = obj.medianDist();

                auto rawpos = obj.getPosition();
                RawObjX.append(static_cast<double>(rawpos.x));
                RawObjY.append(static_cast<double>(rawpos.y));
            }          
        }

        if (ui->DrawProObject->isChecked()) {
            for (auto pObj : detectedObjects) {
                auto propos = pObj.getPosition();

                PosObjX.append(static_cast<double>(propos.x));
                PosObjY.append(static_cast<double>(propos.y));
            }
        }
    }

    plot();
}

void Qt_urg::setPlottemplate() {
    ui->plot->addGraph();
    ui->plot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssPlus, QPen(distanceColor, 1), distanceColor, 2));
    ui->plot->graph(0)->setLineStyle(QCPGraph::LineStyle::lsNone);

    ui->plot->addGraph();
    ui->plot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(objectColor, 2), objectColor, 15));
    ui->plot->graph(1)->setLineStyle(QCPGraph::LineStyle::lsNone);
    ui->plot->addGraph();
    ui->plot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ScatterShape::ssSquare, QPen(processedObjectColor, 2), processedObjectColor, 15));
    ui->plot->graph(2)->setLineStyle(QCPGraph::LineStyle::lsNone);
}

void Qt_urg::plot() {
    ui->plot->graph(0)->setData(RayX, RayY, true);
    ui->plot->graph(1)->setData(RawObjX, RawObjY, true);
    ui->plot->graph(2)->setData(PosObjX, PosObjY, true);

    ui->plot->replot();
    RayX.clear();
    RayY.clear();
    RawObjX.clear();
    RawObjY.clear();
    PosObjX.clear();
    PosObjY.clear();
}

void Qt_urg::drawRect(const Rect& rect, QColor color) {
    QCPItemRect* xRectItem = new QCPItemRect(ui->plot);

    xRectItem->setVisible(true);
    xRectItem->setPen(QPen(color));
    xRectItem->setBrush(QBrush(Qt::NoBrush));

    xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->topLeft->setCoords(rect.xmin, rect.ymin);

    xRectItem->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->bottomRight->setCoords(rect.xmin + rect.width, rect.ymin + rect.height);
}

void Qt_urg::ConnectTcp_Button() {
    if (!isdetect) {
        std::cout << "start Tcp..." << std::endl;

        m_urgdetector.reset(new URGSensorObjectDetector(ui->IP_Input->text().toStdString(), ui->Port_number_input->value()));
        m_urgdetector->start();
        isdetect = true;

        Rect rect = m_urgdetector->detectAreaRect();
        drawRect(rect, Qt::red);
    }
}

void Qt_urg::DisconnectTcp_Button() {
    if (isdetect) {
        isdetect = false;
        m_urgdetector.reset(nullptr);
    }
}

void Qt_urg::setConstraintRegion_Button() {
    m_urgdetector->setConstraintWH(0, 0, ui->CR_Right_Input->value(), ui->CR_Left_Input->value());
    //m_urgdetector->CalculateDistanceConstrainList();
}
