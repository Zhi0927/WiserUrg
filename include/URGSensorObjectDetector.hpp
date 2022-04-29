#ifndef HKY_OBJECTDETECTOR_H_
#define HKY_OBJECTDETECTOR_H_

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cmath>
#include <thread>
#include <algorithm>
#include <functional>

#include "UrgDeviceEthernet.hpp"
#include "HKYObject.hpp"
#include "SCIP_library.hpp"
#include "Rect.hpp"
#include "Vector3.hpp"
#include "Common.hpp"


class URGSensorObjectDetector
{
public:
	enum class DistanceCroppingMethod {
		RECT, RADIUS
	};

	URGSensorObjectDetector(const std::string& ip, const int& port);
	~URGSensorObjectDetector();

	Rect detectAreaRect() const;
	std::vector<long> GetcroppedDistances() const;
	std::vector<vector3> GetDirection() const;
	std::vector<RawObject> GetRawObjectList() const;
	std::vector<ProcessedObject> GetDetectObjects() const;
	ProcessedObject* GetProcessedObjectByGuid(const std::string& guid);
	RawObject* GetRawObjectByGuid(const std::vector<RawObject>& rawobjects, const std::string& guid);
	std::vector<ProcessedObject> GetObjects(const std::vector<ProcessedObject>& detectobj, const float ageFilter = 0.5f);

	void CalculateDistanceConstrainList(const int steps);
	std::vector<long> ConstrainDetectionArea(const std::vector<long>& beforeCrop, DistanceCroppingMethod method);
	void StartMeasureDistance();
	void CacheDirections();
	std::vector<long> SmoothDistanceCurve(const std::vector<long>& croppedDistances, int smoothKernelSize);
	std::vector<RawObject> DetectObjects(const std::vector<long>& croppedDistances, const std::vector<long>& distanceConstrainList);
	void UpdateObjectList();
	std::vector<long> SmoothDistanceCurveByTime(const std::vector<long>& newList, std::vector<long>& previousList, float smoothFactor);

	void start();
	void mainloop();

public:
	std::string								m_ip_address							= "192.168.0.10";
	int										m_port_number							= 10940;
	DistanceCroppingMethod					m_distanceCroppingMethod				= DistanceCroppingMethod::RECT;

	std::vector<long>						m_distanceConstrainList;
	int										m_detectRectWidth						= 6000;	//Unit is MM
	int										m_detectRectHeight						= 6000;	//Unit is MM

	long									m_maxDetectionDist						= 7000;
	int										m_timeSmoothBreakingDistanceChange		= 200;
	int										m_smoothKernelSize						= 21;
	std::vector<long>						m_smoothByTimePreviousList;
	float									m_timeSmoothFactor						= 1.5;
	int										m_noiseLimit							= 7;
	int										m_deltaLimit							= 200;
	float									m_objectPositionSmoothTime				= 0.2f;
	vector3									m_positionOffset;
	bool									m_useOffset								= true;
	bool									m_recalculateConstrainAreaEveryFrame	= false;

	std::function<void(ProcessedObject)>	m_OnNewObject							= nullptr;
	std::function<void(ProcessedObject)>	m_OnLostObject							= nullptr;

private:
	//std::unique_ptr<UrgDeviceEthernet>		m_urg;
	UrgDeviceEthernet						m_urg;
	int										m_sensorScanSteps						= 0;
	bool									m_gd_loop								= false;
	bool									m_smoothDistanceCurve					= false;
	bool									m_smoothDistanceByTime					= false;
	std::vector<RawObject>					m_rawObjectList;
	std::vector<ProcessedObject>			m_detectedObjects;
	float									m_distanceThresholdForMerge				= 300;
	int										m_frameTime								= 0;

	std::vector<long>						m_croppedDistances;
	std::vector<long>						m_strengths;
	std::vector<vector3>					m_directions;
	std::mutex								m_detectobject_guard;
	std::mutex								m_urgdistance_guard;
};
#endif


