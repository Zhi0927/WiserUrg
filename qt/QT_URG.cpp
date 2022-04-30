#include "QT_URG.h"
#include<QDebug>

QT_URG::QT_URG(QWidget *parent)
    :   QWidget(parent),
        ui(new Ui::QT_URGClass)
        //m_urgdetector("192.168.0.10", 10940)
        
{
    ui->setupUi(this);
    isdetect = false;

    // Plot
    ui->plot->setInteraction(QCP::iRangeDrag, true);
    ui->plot->setInteraction(QCP::iRangeZoom, true);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    ui->plot->setPlottingHint(QCP::phFastPolylines);
    //ui->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    //ui->plot->graph(0)->setLineStyle(QCPGraph::lsNone);

    // Button
    connect(ui->Connect_Button, SIGNAL(clicked()), this, SLOT(ConnectTcp_Button()));
    connect(ui->Disconnect_Button, SIGNAL(clicked()), this, SLOT(DisconnectTcp_Button()));
    connect(ui->SetCR_Button, SIGNAL(clicked()), this, SLOT(setConstraintRegion_Button()));

    std::cout << "Initial QT successful!" << std::endl;
}

QT_URG::~QT_URG() {
    delete ui;
}

void QT_URG::DrawMain() {

    auto croppedDistances   = m_urgdetector->GetcroppedDistances();
    auto directions         = m_urgdetector->GetDirection();
    auto rawObjectList      = m_urgdetector->GetRawObjectList();
    auto detectedObjects    = m_urgdetector->GetDetectObjects();
    
    switch (m_urgdetector->m_distanceCroppingMethod)
    {
        case URGSensorObjectDetector::DistanceCroppingMethod::RADIUS:
            // Draw Circle
            break;
        case URGSensorObjectDetector::DistanceCroppingMethod::RECT:
            Rect rect = m_urgdetector->detectAreaRect();
            drawRect(rect, Qt::red);
            break;
    }

    if (ui->debugDrawDistance->isChecked() && !croppedDistances.empty()){
        for (int i = 0; i < croppedDistances.size(); i++){
            vector3 dir = directions[i];
            long dist = croppedDistances[i];

            drawLine(vector3(0, 0, 0), dir * dist, distanceColor);  
            std::cout << "debugDrawDistance->draw line" << std::endl;
        }
    }
  
    if (rawObjectList.empty()) return;

    for (int i = 0; i < rawObjectList.size(); i++){
        auto obj = rawObjectList[i];

        if (obj.m_idList.size() == 0 || obj.m_idList.size() == 0) return;

        vector3 dir = directions[obj.medianId()];
        long dist = obj.medianDist();

        if (ui->drawObjectRays->isChecked()){
            for (int j = 0; j < obj.m_idList.size(); j++){
                auto& myDir = directions[obj.m_idList[j]];
                drawLine(vector3(0, 0, 0), myDir * obj.m_distList[j], objectColor);
                std::cout << "drawObjectRays-> draw line" << std::endl;
            }
        }

        if (ui->drawObjectCenterRay->isChecked()) {
            drawLine(vector3(0, 0, 0), dir * dist, Qt::blue);
            std::cout << "drawObjectCenterRay-> draw line" << std::endl;
        }
        if (ui->drawObject->isChecked()) {
            auto pos = obj.getPosition();
            drawRect(Rect(pos.x - 50, pos.y - 50, 100, 100));
        }
    }

    if (ui->drawProcessedObject->isChecked()) {        
        for(auto pObj : detectedObjects){
            auto pos = pObj.getPosition();

            QCPItemText* textLabel = new QCPItemText(ui->plot);
            textLabel->setPositionAlignment(Qt::AlignHCenter | Qt::AlignHCenter);
            textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
            textLabel->position->setCoords(pos.x, pos.y);
            
            textLabel->setText(QString::fromStdString(pos.ToString()));
            textLabel->setFont(QFont(font().family(), 30)); 
            textLabel->setPen(QPen(processedObjectColor));
        }
    }

    if (ui->drawRunningLine->isChecked()){
        for (int i = 1; i < croppedDistances.size(); i++){
            drawLine(vector3(i, m_urgdetector->m_detectRectHeight + croppedDistances[i], 0), vector3(i - 1, m_urgdetector->m_detectRectHeight + croppedDistances[i - 1], 0), Qt::blue);
        }
    }

    plot();
}

void QT_URG::plot() {
    ui->plot->replot();
    ui->plot->update();
    //ui->plot->clearItems();
}

void QT_URG::drawRect(const Rect& rect, QColor color) {
    QCPItemRect* xRectItem = new QCPItemRect(ui->plot);

    xRectItem->setVisible(true);
    xRectItem->setPen(QPen(color));
    xRectItem->setBrush(QBrush(Qt::NoBrush));

    xRectItem->topLeft->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->topLeft->setCoords(rect.xmin, rect.ymin);

    xRectItem->bottomRight->setType(QCPItemPosition::ptPlotCoords);
    xRectItem->bottomRight->setCoords(rect.xmin + rect.width, rect.ymin + rect.height);
}

void QT_URG::drawLine(const vector3& from, const vector3& to, QColor color) {
    QCPItemLine* line = new QCPItemLine(ui->plot);
    line->setPen(QPen(color));
    line->start->setCoords(from.x, from.y);
    line->end->setCoords(to.x, to.y);
}

void QT_URG::ConnectTcp_Button() {
    if (!isdetect) {
        std::cout << "start Tcp..." << std::endl;

        isdetect = true;
        m_urgdetector.reset(new URGSensorObjectDetector(ui->IP_Input->text().toStdString(),
                                                        ui->Port_number_input->value()));
        m_urgdetector->setrecalculateConstrainAreaEveryFrame(ui->recalculateConstrainAreaEveryFrame->isChecked());      
        m_urgdetector->start();

        m_mainthread.reset(new std::thread([this]() {
            //while (isdetect) {
            m_urgdetector->mainloop();
            //}
        }));
    }
}

void QT_URG::DisconnectTcp_Button() {
    if (isdetect) {
        isdetect = false;
        m_mainthread->join();
        m_urgdetector.reset(nullptr);
    }
}

void QT_URG::setConstraintRegion_Button() {

}



