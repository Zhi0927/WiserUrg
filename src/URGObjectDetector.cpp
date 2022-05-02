#include "URGObjectDetector.hpp"


URGSensorObjectDetector::URGSensorObjectDetector(const std::string& ip, const int& port){
    parm.ip_address = ip;
    parm.port_number = port;

    m_urg.reset(new UrgDeviceEthernet(parm.ip_address, parm.port_number));
}

URGSensorObjectDetector::~URGSensorObjectDetector() {
    m_urg.reset(nullptr);
}


Rect URGSensorObjectDetector::detectAreaRect() const {
	Rect rect(0, 0, parm.detctRect.width, parm.detctRect.height);
	rect.xmin -= (parm.detctRect.width / 2);
	return rect;
}

void URGSensorObjectDetector::setConstraintWH(int x, int y, int width, int height) {

}

void URGSensorObjectDetector::setDetectParm(int deltalim, int noiselim) {
    parm.deltaLimit = deltalim;
    parm.noiseLimit = noiselim;
}

const std::vector<long>& URGSensorObjectDetector::GetcroppedDistances() const {
    return m_croppedDistances;
}

const std::vector<vector3>& URGSensorObjectDetector::GetDirection() const {
    return m_directions;
}

const std::vector<RawObject>& URGSensorObjectDetector::GetRawObjectList() const {
    return m_rawObjectList;
}

const std::vector<ProcessedObject>& URGSensorObjectDetector::GetDetectObjects() const {
    return m_detectedObjects;
}

std::vector<ProcessedObject> URGSensorObjectDetector::GetObjects(const std::vector<ProcessedObject>& detectobj, const float ageFilter) {

    std::vector<ProcessedObject> target(detectobj);
    for (int i = 0; i < target.size(); ++i) {
        if (target[i].getage() < ageFilter) {
            target.erase(target.begin() + i);
            --i;
        }
    }
    return target;
}

std::vector<long> URGSensorObjectDetector::SmoothDistanceCurve(const std::vector<long>& croppedDistances, int smoothKernelSize) {
    return movingAverages(croppedDistances, smoothKernelSize);
}

void URGSensorObjectDetector::StartMeasureDistance() {
    m_urg->Write(SCIP_Writer::MD(0, 1080, 1, 0, 0));
}

void URGSensorObjectDetector::CacheDirections() {
    float offset = M_DELTA_ANGLE * 540;     // rotate 135 degrees to the left.
    m_directions.resize(m_sensorScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++){
        float a = M_DELTA_ANGLE * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0);
    }
}

void URGSensorObjectDetector::CalculateDistanceConstrainList(const int steps) {
    float keyAngle = atan(parm.detctRect.height / (parm.detctRect.width / 2.f));
    for (size_t i = 0; i < steps; i++) {
        if (m_directions[i].y <= 0) {
            m_distanceConstrainList[i] = 0;
        }
        else {
            float a = vector3::Angle(m_directions[i], vector3(1, 0, 0)) * Deg2Rad;
            float pn = tan(a) / abs(tan(a));

            float r = 0;
            if (a < keyAngle || a > M_PI - keyAngle) {
                float x = pn * (parm.detctRect.width / 2);
                r = x / cos(a);
            }
            else if (a >= keyAngle && a <= M_PI - keyAngle) {
                float angle2 = M_PI / 2 - a;
                float x = parm.detctRect.height * tan(angle2);
                r = x / sin(angle2);
            }
            if (r < 0 || std::isnan(r)) {
                r = 0;
            }
            m_distanceConstrainList[i] = static_cast<long>(r);
        }
    }
}

void URGSensorObjectDetector::ConstrainDetectionArea(std::vector<long>& beforeCrop) {
    for (size_t i = 0; i < beforeCrop.size(); i++) {
        if (beforeCrop[i] > m_distanceConstrainList[i] || beforeCrop[i] <= 0) {
            beforeCrop[i] = m_distanceConstrainList[i];
        }
        else {
            beforeCrop[i] = beforeCrop[i];
        }
    }
}

std::vector<RawObject> URGSensorObjectDetector::DetectObjects(const std::vector<long>& croppedDistances, const std::vector<long>& distanceConstrainList) {

    std::vector<RawObject> resultList;
    if (m_directions.size() <= 0){
        std::cerr << "directions vector is not setup." << std::endl;
        return resultList;
    }

    bool isGrouping = false;
    for (int i = 1; i < croppedDistances.size() - 1; i++){
        float deltaA = abs(croppedDistances[i] - croppedDistances[i - 1]);
        float deltaB = abs(croppedDistances[i + 1] - croppedDistances[i]);

        auto& dist   = croppedDistances[i];
        auto& ubDist = distanceConstrainList[i];

        if (dist < ubDist && (deltaA < parm.deltaLimit && deltaB < parm.deltaLimit)) {
            if (!isGrouping){
                isGrouping = true;
                RawObject newObject;
                newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(dist);
                resultList.emplace_back(newObject);
            }
            else{
                auto& newObject = resultList[resultList.size() - 1];
                newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(dist);
            }
        }
        else{
            if (isGrouping){
                isGrouping = false;
            }
        }
    }
    for (int i = 0; i < resultList.size(); ++i) {
        if (resultList[i].dirList.size() < parm.noiseLimit){
            resultList.erase(resultList.begin() + i);
            --i;
        }
    }
    for (auto& item : resultList) {
        item.GetCalcPosition();
    }

    return resultList;
}

void URGSensorObjectDetector::UpdateObjectList() {
    std::vector<RawObject> newlyDetectedObjects = DetectObjects(m_croppedDistances, m_distanceConstrainList);

    if (parm.useOffset){
        for(auto& obj : newlyDetectedObjects){
            obj.setPosition(obj.getPosition() += parm.positionOffset);
        }
    }

    m_rawObjectList.assign(newlyDetectedObjects.begin(), newlyDetectedObjects.end());

    std::unique_lock<std::mutex> lockdata(m_detectobject_guard);

    if (m_detectedObjects.size() != 0)
    {
        for(size_t i =0; i < m_detectedObjects.size(); ++i)
        {
            auto& oldObj = m_detectedObjects[i];
            std::map<std::string, float> objectMapWithDistance;

            for(size_t j = 0; j < newlyDetectedObjects.size(); ++j){
                auto& newObj = newlyDetectedObjects[j];
                float distance = vector3::Distance(newObj.getPosition(), oldObj.getPosition());
                objectMapWithDistance[newObj.getGuid()] = distance;
            }

            if (objectMapWithDistance.size() <= 0){
                oldObj.Update();
            }
            else{                
                auto closest = std::min_element(objectMapWithDistance.begin(), objectMapWithDistance.end(), [](const auto& l, const auto& r) {return l.second < r.second; });
                if (closest->second <= parm.distanceThresholdForMerge){
                    auto temp = std::find_if(newlyDetectedObjects.begin(), newlyDetectedObjects.end(), [&closest](auto& ele) { return ele.getGuid() == closest->first;});
                    oldObj.Update(temp->getPosition(), temp->getDetectSize());
                    newlyDetectedObjects.erase(temp);
                }
                else{
                    oldObj.Update();
                }
            }
        }

        for (int i = 0; i < m_detectedObjects.size(); ++i){
            if (m_detectedObjects[i].isClear()){
                if (OnLostObject != nullptr) { OnLostObject(m_detectedObjects[i]); }
                m_detectedObjects.erase(m_detectedObjects.begin() + i);
                --i;
            }
        }

        for(auto& leftOverNewObject : newlyDetectedObjects){
            ProcessedObject newbie(leftOverNewObject.getPosition(), leftOverNewObject.getDetectSize(), parm.objectPositionSmoothTime);
            m_detectedObjects.emplace_back(newbie);
            if (OnNewObject != nullptr) { OnNewObject(newbie); }
        }
    }
    else 
    {
        for(auto& obj : m_rawObjectList){
            ProcessedObject newbie(obj.getPosition(), obj.getDetectSize(), parm.objectPositionSmoothTime);
            m_detectedObjects.emplace_back(newbie);
            if (OnNewObject != nullptr) { OnNewObject(newbie); }
        }
    } 
}



void URGSensorObjectDetector::start() {
    m_urg->StartTCP();
    StartMeasureDistance();
    std::cout << "Start Tcp" << std::endl;
}

void URGSensorObjectDetector::mainloop() {

    std::unique_lock<std::mutex> lockurg(m_urg->distance_guard);
    if (m_urg->recv_distances.size() <= 0) return;
    std::vector<long> originalDistances(m_urg->recv_distances);
    lockurg.unlock();
    if (originalDistances.size() <= 0) return;

    if (m_sensorScanSteps <= 0) {
        m_sensorScanSteps = m_urg->recv_distances.size();
        m_distanceConstrainList.resize(m_sensorScanSteps);
        CacheDirections();
        CalculateDistanceConstrainList(m_sensorScanSteps);
    }

    ConstrainDetectionArea(originalDistances);
    m_croppedDistances.assign(originalDistances.begin(), originalDistances.end());

    if (parm.useSMA) {
        if (parm.smoothKernelSize % 2 == 0) {
            parm.smoothKernelSize += 1;
        }
        m_croppedDistances = SmoothDistanceCurve(m_croppedDistances, parm.smoothKernelSize);
    }

    UpdateObjectList();
}