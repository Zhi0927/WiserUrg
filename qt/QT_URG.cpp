#include "Qt_urg.h"

Qt_urg::Qt_urg(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::Qt_urgClass)
{
    ui->setupUi(this);
    isdetect = false;

    // Plot
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    ui->plot->setPlottingHint(QCP::phFastPolylines);

    //ui->plot->addGraph();
    //ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    //ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    //rayX.reserve(1081);
    //rayY.reserve(1081);

    for (int i = 0; i < 1081; i++) {
        ui->plot->addGraph();
    }
    rayX.resize(2);
    rayY.resize(2);
    rayX[0] = 0;
    rayY[0] = 0;

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
    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0;
    static double lastPointKey = 0;

    //-----------------------------------------------------------------------------------//
    if (isdetect) {
        m_urgdetector->mainloop();

        auto croppedDistances = m_urgdetector->GetcroppedDistances();
        auto directions = m_urgdetector->GetDirection();

        //for (int i = 0; i < croppedDistances.size(); i++) {
        //    vector3 result = directions[i] * croppedDistances[i];
        //    rayX.append(static_cast<double>(result.x));
        //    rayY.append(static_cast<double>(result.y));
        //}
        //ui->plot->graph(0)->setData(rayX, rayY, true);
        //ui->plot->replot();
        //rayX.clear();
        //rayY.clear();

        for (int i = 0; i < croppedDistances.size(); i++) {
            vector3 result = directions[i] * croppedDistances[i];
            rayX[1] = static_cast<double>(result.x);
            rayY[1] = static_cast<double>(result.y);
            ui->plot->graph(i)->setData(rayX, rayY, true);
        }
        ui->plot->replot();
    }
    //-----------------------------------------------------------------------------------//
    lastPointKey = key;

    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 2)
    {
        ui->FPS_Label->setText(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
            .arg(ui->plot->graph(0)->data()->size()));
        lastFpsKey = key;
        frameCount = 0;
    }


    //auto croppedDistances = m_urgdetector->GetcroppedDistances();
    //auto directions = m_urgdetector->GetDirection();
    //auto rawObjectList = m_urgdetector->GetRawObjectList();
    //auto detectedObjects = m_urgdetector->GetDetectObjects();

    //if (ui->debugDrawDistance->isChecked() && !croppedDistances.empty()) {
    //    for (int i = 0; i < croppedDistances.size(); i++) {
    //        vector3 dir = directions[i];
    //        long dist = croppedDistances[i];

    //        drawLine(vector3(0, 0, 0), dir * dist, distanceColor);
    //        std::cout << "debugDrawDistance->draw line" << std::endl;
    //    }
    //}

    //if (rawObjectList.empty()){
    //    return;
    //}

    //for (int i = 0; i < rawObjectList.size(); i++) {
    //    auto obj = rawObjectList[i];

        //if (obj.m_idList.size() == 0 || obj.m_idList.size() == 0) {
        //    return;
        //}

    //    vector3 dir = directions[obj.medianId()];
    //    long dist = obj.medianDist();

    //    if (ui->drawObjectRays->isChecked()) {
    //        for (int j = 0; j < obj.m_idList.size(); j++) {
    //            auto& myDir = directions[obj.m_idList[j]];
    //            drawLine(vector3(0, 0, 0), myDir * obj.m_distList[j], objectColor);
    //            std::cout << "drawObjectRays-> draw line" << std::endl;
    //        }
    //    }

    //    if (ui->drawObjectCenterRay->isChecked()) {
    //        drawLine(vector3(0, 0, 0), dir * dist, Qt::blue);
    //        std::cout << "drawObjectCenterRay-> draw line" << std::endl;
    //    }
    //    if (ui->drawObject->isChecked()) {
    //        auto pos = obj.getPosition();
    //        drawRect(Rect(pos.x - 50, pos.y - 50, 100, 100));
    //    }
    //}

    //if (ui->drawProcessedObject->isChecked()) {
    //    for (auto pObj : detectedObjects) {
    //        auto pos = pObj.getPosition();

    //        QCPItemText* textLabel = new QCPItemText(ui->plot);
    //        textLabel->setPositionAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
    //        textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    //        textLabel->position->setCoords(pos.x, pos.y);

    //        textLabel->setText(QString::fromStdString(pos.ToString()));
    //        textLabel->setFont(QFont(font().family(), 30));
    //        textLabel->setPen(QPen(processedObjectColor));
    //    }
    //}

    //if (ui->drawRunningLine->isChecked()) {
    //    for (int i = 1; i < croppedDistances.size(); i++) {
    //        drawLine(vector3(i, m_urgdetector->m_detectRectHeight + croppedDistances[i], 0), vector3(i - 1, m_urgdetector->m_detectRectHeight + croppedDistances[i - 1], 0), Qt::blue);
    //    }
    //}
}

void Qt_urg::plot() {
    ui->plot->graph(0)->setData(rayX, rayY);
    ui->plot->replot();
    //ui->plot->update();
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

void Qt_urg::drawLine(const vector3& from, const vector3& to, QColor color) {
    QCPItemLine* line = new QCPItemLine(ui->plot);
    line->setPen(QPen(color));
    line->start->setCoords(from.x, from.y);
    line->end->setCoords(to.x, to.y);
}

void Qt_urg::ConnectTcp_Button() {
    if (!isdetect) {
        std::cout << "start Tcp..." << std::endl;

        m_urgdetector.reset(new URGSensorObjectDetector(ui->IP_Input->text().toStdString(), ui->Port_number_input->value()));
        m_urgdetector->setrecalculateConstrainAreaEveryFrame(ui->recalculateConstrainAreaEveryFrame->isChecked());
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
    m_urgdetector->setConstraintWH(ui->CR_Right_Input->value(), ui->CR_Left_Input->value());
}
