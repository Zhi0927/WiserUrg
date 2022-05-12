#include "ObjectDetector.hpp"


ObjectDetector::ObjectDetector(){
    std::cout << "Detector Init" << std::endl;
}

ObjectDetector::~ObjectDetector() {}


const std::vector<vector3>& ObjectDetector::GetDirection() const {
    return m_directions;
}

const std::vector<RawObject>& ObjectDetector::GetRawObjectList() const {
    return m_rawObjects;
}

const std::vector<ProcessedObject>& ObjectDetector::GetProcessObjects() const {
    return m_processObject;
}

void ObjectDetector::CacheDirections(int ScanSteps) {
    float offset = M_DELTA_ANGLE * 540;     
    m_directions.resize(ScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++){
        float a = M_DELTA_ANGLE * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0); 
        if (parm.sensor02_activate && i >= (ScanSteps / 2)) {
            m_directions[i] + parm.sensor02_originPos;
        }
    }
    std::cout << "Direction has been calculated!" << std::endl;
}

std::vector<RawObject> ObjectDetector::DetectRawObjects(const std::vector<long>& partdistances, const std::vector<vector3>& partdirction) {
    std::vector<RawObject> rawobjects;
    
    bool isGrouping = false;
    for (size_t i = 1; i < partdistances.size() - 1; i++) {

        if (!parm.detctRect.Contains(partdirction[i] * partdistances[i])) continue;

        float deltaA = abs(partdistances[i] - partdistances[i - 1]);
        float deltaB = abs(partdistances[i + 1] - partdistances[i]);

        if (deltaA < parm.deltaLimit && deltaB < parm.deltaLimit) {
            if (!isGrouping) {
                isGrouping = true;
                RawObject newObject;

                newObject.dirList.emplace_back(partdirction[i]);
                newObject.distList.emplace_back(partdistances[i]);

                rawobjects.emplace_back(newObject);
            }
            else {
                auto& newObject = rawobjects[rawobjects.size() - 1];

                newObject.dirList.emplace_back(partdirction[i]);
                newObject.distList.emplace_back(partdistances[i]);
            }
        }
        else {
            if (isGrouping) {
                isGrouping = false;
            }
        }
    }
    return rawobjects;
}

std::vector<RawObject> ObjectDetector::postprocessRawObject(const std::vector<long>& distances) {
    std::vector<RawObject> resultList;
    if (m_directions.size() <= 0) {
        std::cerr << "directions vector is not setup!" << std::endl;
        return resultList;
    }

    if (parm.sensor02_activate) {
        resultList = DetectRawObjects(slice(distances, 0, distances.size() / 2),
                                      slice(m_directions, 0, m_directions.size() / 2));

        auto objpart02  = DetectRawObjects(slice(distances, distances.size() / 2, distances.size()),
                                           slice(m_directions, m_directions.size() / 2, m_directions.size()));

        for (size_t m = 0; m < resultList.size(); ++m) {
            auto& obj = resultList[m];
            for (size_t n = 0; n < objpart02.size(); ++n) {
                if (vector3::Distance(obj.getPosition(), objpart02[n].getPosition()) > parm.mergerdistance) {
                    resultList.emplace_back(objpart02[n]);
                }
            }
        }
    }
    else {
        resultList = DetectRawObjects(distances, m_directions);
    }

    
    for (int i = 0; i < resultList.size(); ++i) {
        if (resultList[i].dirList.size() < parm.noiseLimit || resultList[i].getDetectSize() > parm.detectsize) {
            resultList.erase(resultList.begin() + i);
            --i;
        }
        else {
            resultList[i].distList = movingAverages(resultList[i].distList, 5);
        }
    }

    return resultList;
}

void ObjectDetector::ProcessingObjects(const std::vector<long>& distances) {
    m_rawObjects.clear();

    std::vector<RawObject> newlyDetectedObjects = postprocessRawObject(distances);

    if (parm.useOffset){
        for(auto& obj : newlyDetectedObjects){
            obj.setPosition(obj.getPosition() += parm.positionOffset);
        }
    }

    m_rawObjects = newlyDetectedObjects;

    //std::unique_lock<std::mutex> lockdata(m_detectobject_guard);

    if (m_processObject.size() != 0){
        for(size_t i =0; i < m_processObject.size(); ++i)
        {
            auto& oldObj = m_processObject[i];
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

                    if (OnUpdataObjCallback != nullptr) {
                        vector3 pos = oldObj.getPosition();
                        Sensor2Screen(pos);
                        OnUpdataObjCallback(pos);
                    }
                }
                else{
                    oldObj.Update();
                }
            }
        }


        for (int i = 0; i < m_processObject.size(); ++i){
            if (m_processObject[i].isClear()){         
                m_processObject.erase(m_processObject.begin() + i);
                --i;

                if (OnLostObjectCallback != nullptr) {
                    OnLostObjectCallback();
                }
            }
        }
        for(auto& leftOverNewObject : newlyDetectedObjects){
            ProcessedObject newbie(leftOverNewObject.getPosition(), leftOverNewObject.getDetectSize(), parm.objPosSmoothTime, parm.delatime);
            m_processObject.emplace_back(newbie);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newbie.getPosition();
                Sensor2Screen(pos);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }

        }
    }
    else {
        for(auto& obj : m_rawObjects){
            ProcessedObject newbie(obj.getPosition(), obj.getDetectSize(), parm.objPosSmoothTime, parm.delatime);
            m_processObject.emplace_back(newbie);

            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
                vector3 pos = newbie.getPosition();
                Sensor2Screen(pos);
                OnUpdataObjCallback(pos);
                OnNewObjectCallback();
            }
        }
    }
}

void ObjectDetector::Sensor2Screen(vector3& input) {

    input.x = (1- ((input.x - parm.detctRect.xmin) / parm.detctRect.width)) * parm.screenWidth;
    input.y = (1+ ((input.y - parm.detctRect.ymin) / parm.detctRect.height)) * parm.screenHeight;
}


