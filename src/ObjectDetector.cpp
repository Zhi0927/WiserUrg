#include "ObjectDetector.hpp"


URGSensorObjectDetector::URGSensorObjectDetector(){
    std::cout << "Detector Init" << std::endl;
}

URGSensorObjectDetector::~URGSensorObjectDetector() {}

const std::vector<long>& URGSensorObjectDetector::GetOriginDistances() const {
    return m_origindistance;
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

void URGSensorObjectDetector::CacheDirections(int ScanSteps) {
    float offset = M_DELTA_ANGLE * 540;     // rotate 135 degrees to the left.
    m_directions.resize(ScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++){
        float a = M_DELTA_ANGLE * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0); //and flip it.
    }
    std::cout << "Direction has been calculated!" << std::endl;
}

std::vector<RawObject> URGSensorObjectDetector::DetectObjects(const std::vector<long>& distances) {
    std::vector<RawObject> resultList;
    if (m_directions.size() <= 0) {
        std::cerr << "directions vector is not setup!" << std::endl;
        return resultList;
    }
    
    bool isGrouping = false;
    for (int i = 1; i < distances.size() - 1; i++) {

        if (!parm.detctRect.Contains(m_directions[i] * distances[i])) continue;

        float deltaA = abs(distances[i] - distances[i - 1]);
        float deltaB = abs(distances[i + 1] - distances[i]);

        if (deltaA < parm.deltaLimit && deltaB < parm.deltaLimit) {
            if (!isGrouping) {
                isGrouping = true;
                RawObject newObject;

                newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(distances[i]);

                resultList.emplace_back(newObject);
            }
            else {
                auto& newObject = resultList[resultList.size() - 1];

                newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(distances[i]);
            }
        }
        else {
            if (isGrouping) {
                isGrouping = false;
            }
        }
    }

    for (int i = 0; i < resultList.size(); ++i) {
        if (resultList[i].dirList.size() < parm.noiseLimit || resultList[i].getDetectSize() > parm.detectsize) {
            resultList.erase(resultList.begin() + i);
            --i;
        }
        else{
            resultList[i].distList = movingAverages(resultList[i].distList, 3);
        }
    }
    return resultList;
}

void URGSensorObjectDetector::UpdateObjectList(const std::vector<long>& distances) {
    std::vector<RawObject> newlyDetectedObjects = DetectObjects(distances);
    if (newlyDetectedObjects.size() <= 0) {
        return;
    }

    if (parm.useOffset){
        for(auto& obj : newlyDetectedObjects){
            obj.setPosition(obj.getPosition() += parm.positionOffset);
        }
    }

    m_rawObjectList = newlyDetectedObjects;

    std::unique_lock<std::mutex> lockdata(m_detectobject_guard);

    if (m_detectedObjects.size() != 0){
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
                if (closest->second <= parm.distanceThreshold){
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
                OnLostObject(m_detectedObjects[i]);
                m_detectedObjects.erase(m_detectedObjects.begin() + i);
                --i;
            }
        }

        for(auto& leftOverNewObject : newlyDetectedObjects){
            ProcessedObject newbie(leftOverNewObject.getPosition(), leftOverNewObject.getDetectSize(), parm.objPosSmoothTime);
            m_detectedObjects.emplace_back(newbie);
            OnNewObject(newbie);
        }
    }
    else {
        for(auto& obj : m_rawObjectList){
            ProcessedObject newbie(obj.getPosition(), obj.getDetectSize(), parm.objPosSmoothTime);
            m_detectedObjects.emplace_back(newbie);
            OnNewObject(newbie);
        }
    }
}

void URGSensorObjectDetector::OnNewObject(const ProcessedObject& obj) {
    if (OnNewObjectCallback != nullptr) {
        vector3 position = obj.getPosition();
        Sensor2Screen(position);
        OnNewObjectCallback(position);
    }
}

void URGSensorObjectDetector::OnLostObject(const ProcessedObject& obj) {
    if (OnLostObjectCallback != nullptr) {
        vector3 position = obj.getPosition();
        Sensor2Screen(position);
        OnLostObjectCallback(position);
    }
}

void URGSensorObjectDetector::Sensor2Screen(vector3& input) {
    input.x = (input.x - parm.detctRect.xmin) / parm.detctRect.width;
    input.y = (-(input.y - parm.detctRect.ymin)) / parm.detctRect.height;
    input.x *= parm.screenWidth;
    input.y *= parm.screenHeight;
}


