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

//const std::vector<SensedObject>& ObjectDetector::GetSensedObjects() const {
//    return m_sensorObjects;
//}

const std::vector<ProcessedObject>& ObjectDetector::GetProcessObjects() const {
    return m_processObject;
}

void ObjectDetector::CacheDirections(int ScanSteps) {
    float offset = M_DELTA_ANGLE * 540;     
    m_directions.resize(ScanSteps);
    for (size_t i = 0; i < m_directions.size(); i++){
        float a = M_DELTA_ANGLE * i + offset;
        m_directions[i] = vector3(-cos(a), -sin(a), 0); 
    }
    std::cout << "Direction has been calculated!" << std::endl;
}

void ObjectDetector::GetPointFromDistance(int step, float distance, vector3& pos, bool multi){
    pos = m_directions[step] * distance;
    if (multi) {
        pos += parm.sensor02_originPos;
    }
}

std::vector<RawObject> ObjectDetector::DetectRawObjects(const std::vector<long>& partdistances, bool multi) {
    std::vector<RawObject> rawobjects;
    
    bool isGrouping = false;
    for (size_t i = 1; i < partdistances.size() - 1; i++) {

        auto pos = m_directions[i] * partdistances[i];
        if (multi) {
            pos += parm.sensor02_originPos;
        }

        if (!parm.detctRect.Contains(pos)) continue;

        float deltaA = abs(partdistances[i] - partdistances[i - 1]);
        float deltaB = abs(partdistances[i + 1] - partdistances[i]);

        if (deltaA < parm.deltaLimit && deltaB < parm.deltaLimit) {
            if (!isGrouping) {
                isGrouping = true;

                RawObject newObject;
                //newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(partdistances[i]);
                newObject.posList.emplace_back(pos);
                rawobjects.emplace_back(newObject);
            }
            else {
                auto& newObject = rawobjects[rawobjects.size() - 1];

                //newObject.dirList.emplace_back(m_directions[i]);
                newObject.distList.emplace_back(partdistances[i]);
                newObject.posList.emplace_back(pos);
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
        resultList = DetectRawObjects(slice(distances, 0, distances.size() / 2));
        auto objpart02  = DetectRawObjects(slice(distances, distances.size() / 2, distances.size()), true);

        for (size_t m = 0; m < resultList.size(); ++m) {
            for (size_t n = 0; n < objpart02.size(); ++n) {
                if (vector3::Distance(resultList[m].getPosition(), objpart02[n].getPosition()) > parm.mergerdistance) {
                    resultList.emplace_back(objpart02[n]);
                }
            }
        }
    }
    else {
        resultList = DetectRawObjects(distances);
    }
    
    for (int i = 0; i < resultList.size(); ++i) {
        if (resultList[i].posList.size() < parm.noiseLimit || resultList[i].getDetectSize() > parm.detectsize) {
            resultList.erase(resultList.begin() + i);
            --i;
        }
        //else {
        //    resultList[i].posList = movingAverages(resultList[i].posList, 5);
        //}
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
                    oldObj.Update(temp->getPosition());
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
            ProcessedObject newbie(leftOverNewObject.getPosition(), parm.objPosSmoothTime, parm.delatime);
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
            ProcessedObject newbie(obj.getPosition(), parm.objPosSmoothTime, parm.delatime);
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

//std::vector<SensedObject> ObjectDetector::DetectRawObjectsByPoint(const std::vector<long>& partdistances, bool multi) {
//
//    std::vector<SensedObject> sensorOjects;
//
//    vector3 prevP       = vector3(0, 0, 0);
//    vector3 checkP      = vector3(0, 0, 0);
//    vector3 currentP    = vector3(0, 0, 0);
//    vector3 accum       = vector3(0, 0, 0);
//    int accumCount      = 0;
//    bool isGrouping     = false;
//
//    GetPointFromDistance(0, partdistances[0], prevP, multi);
//    for (size_t i = 0; i < partdistances.size(); i++)
//    {
//        auto d = partdistances[i] * 0.001;
//        GetPointFromDistance(i, d, currentP, multi);
//        std::cout << (prevP - checkP).sqrMagnitude() << std::endl;
//        if (isGrouping)
//        {
//            if (parm.objThreshold * parm.objThreshold < (currentP - prevP).sqrMagnitude() && parm.detctRect.Contains(currentP))//new obj
//            {
//                if (parm.minWidth * parm.minWidth < (prevP - checkP).sqrMagnitude()) {
//                    SensedObject sensorObject(checkP, prevP, accum / accumCount);
//                    sensorOjects.emplace_back(sensorObject);
//                    std::cout << "new obj" << std::endl;
//                }
//                checkP = currentP;
//                accum = currentP;
//                isGrouping = true;
//                accumCount = 1;
//
//            }
//            else if (!parm.detctRect.Contains(currentP)) //lost obj
//            {
//                if (parm.minWidth * parm.minWidth < (prevP - checkP).sqrMagnitude()) {
//                    SensedObject sensorObject(checkP, prevP, accum / accumCount);
//                    sensorOjects.emplace_back(sensorObject);
//                    std::cout << "new obj" << std::endl;
//                }
//                isGrouping = false;
//                accumCount = 0;
//
//            }
//            else//continue obj
//            {
//                accum += currentP;
//                accumCount++;
//            }
//        }
//        else
//        {
//            if (parm.objThreshold * parm.objThreshold < (currentP - prevP).sqrMagnitude() && parm.detctRect.Contains(currentP))//new obj
//            {
//                checkP = currentP;
//                accum = currentP;
//                isGrouping = true;
//                accumCount = 1;
//            }
//        }
//        prevP = currentP;
//    }
//
//    return sensorOjects;
//}
//
//std::vector<SensedObject> ObjectDetector::postprocessRawObjectByPoint(const std::vector<long>& distances) {
//    std::vector<SensedObject> sensorOjects;
//    if (m_directions.size() <= 0) {
//        std::cerr << "directions vector is not setup!" << std::endl;
//        return sensorOjects;
//    }
//
//    if (parm.sensor02_activate) {
//        sensorOjects = DetectRawObjectsByPoint(slice(distances, 0, distances.size() / 2));
//        auto objpart02 = DetectRawObjectsByPoint(slice(distances, distances.size() / 2, distances.size()), true);
//
//        for (size_t m = 0; m < sensorOjects.size(); ++m) {
//            for (size_t n = 0; n < objpart02.size(); ++n) {
//                if (vector3::Distance(sensorOjects[m].center, objpart02[n].center) > parm.mergerdistance) {
//                    sensorOjects.emplace_back(objpart02[n]);
//                }
//            }
//        }
//    }
//    else {
//        sensorOjects = DetectRawObjectsByPoint(distances);
//    }
//    return sensorOjects;
//}
//
//void ObjectDetector::ProcessingObjectsByPoint(const std::vector<long>& distances) {
//    //m_sensorObjects.clear();
//
//    std::vector<SensedObject> newlyDetectedObjects = postprocessRawObjectByPoint(distances);
//
//    //if (parm.useOffset) {
//    //    for (auto& obj : newlyDetectedObjects) {
//    //        obj.setPosition(obj.getPosition() += parm.positionOffset);
//    //    }
//    //}
//
//    m_sensorObjects = newlyDetectedObjects;
//
//    if (m_processObject.size() != 0) {
//        for (size_t i = 0; i < m_processObject.size(); ++i)
//        {
//            auto& oldObj = m_processObject[i];
//            std::map<std::string, float> objectMapWithDistance;
//
//            for (size_t j = 0; j < newlyDetectedObjects.size(); ++j) {
//                auto& newObj = newlyDetectedObjects[j];
//                float distance = vector3::Distance(newObj.center, oldObj.getPosition());
//                objectMapWithDistance[newObj.getGuid()] = distance;
//            }
//
//            if (objectMapWithDistance.size() <= 0) {
//                oldObj.Update();
//            }
//            else {
//                auto closest = std::min_element(objectMapWithDistance.begin(), objectMapWithDistance.end(), [](const auto& l, const auto& r) {return l.second < r.second; });
//                if (closest->second <= parm.distanceThreshold) {
//                    auto temp = std::find_if(newlyDetectedObjects.begin(), newlyDetectedObjects.end(), [&closest](auto& ele) { return ele.getGuid() == closest->first; });
//                    oldObj.Update(temp->center);
//                    newlyDetectedObjects.erase(temp);
//
//                    if (OnUpdataObjCallback != nullptr) {
//                        vector3 pos = oldObj.getPosition();
//                        Sensor2Screen(pos);
//                        OnUpdataObjCallback(pos);
//                    }
//                }
//                else {
//                    oldObj.Update();
//                }
//            }
//        }
//
//
//        for (int i = 0; i < m_processObject.size(); ++i) {
//            if (m_processObject[i].isClear()) {
//                m_processObject.erase(m_processObject.begin() + i);
//                --i;
//
//                if (OnLostObjectCallback != nullptr) {
//                    OnLostObjectCallback();
//                }
//            }
//        }
//        for (auto& leftOverNewObject : newlyDetectedObjects) {
//            ProcessedObject newbie(leftOverNewObject.center, parm.objPosSmoothTime, parm.delatime);
//            m_processObject.emplace_back(newbie);
//
//            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
//                vector3 pos = newbie.getPosition();
//                Sensor2Screen(pos);
//                OnUpdataObjCallback(pos);
//                OnNewObjectCallback();
//            }
//
//        }
//    }
//    else {
//        for (auto& obj : m_sensorObjects) {
//            ProcessedObject newbie(obj.center, parm.objPosSmoothTime, parm.delatime);
//            m_processObject.emplace_back(newbie);
//
//            if (OnNewObjectCallback != nullptr && OnUpdataObjCallback != nullptr) {
//                vector3 pos = newbie.getPosition();
//                Sensor2Screen(pos);
//                OnUpdataObjCallback(pos);
//                OnNewObjectCallback();
//            }
//        }
//    }
//}

void ObjectDetector::Sensor2Screen(vector3& input) {

    input.x = (1- ((input.x - parm.detctRect.xmin) / parm.detctRect.width)) * parm.screenWidth;
    input.y = (1+ ((input.y - parm.detctRect.ymin) / parm.detctRect.height)) * parm.screenHeight;
}


