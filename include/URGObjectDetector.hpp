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
#include "DetectObject.hpp"
#include "SCIP_library.hpp"
#include "Rect.hpp"
#include "Vector3.hpp"
#include "Common.hpp"

#define M_DELTA_ANGLE (M_PI * 2 / 1440) // there are 1440 rays in a curcle. delta 0.25 degree, totally 270 degree

struct Detectparm {
	std::string		ip_address					= "192.168.0.10";
	int				port_number					= 10940;

	Rect			detctRect					= Rect(-500, 1000, 500, 1000); //Unit is MM

	int				noiseLimit					= 7;
	int				deltaLimit					= 200;	//200
	float			distanceThreshold			= 300;	//300
	float			objPosSmoothTime			= 0.2f;

	bool			useOffset					= false;
	vector3			positionOffset				= vector3(0, 0, 0);

	bool			useSMA						= false;
	int				smoothKernelSize			= 21;
};



class URGSensorObjectDetector
{
public:
	URGSensorObjectDetector(const std::string& ip, const int& port);
	~URGSensorObjectDetector();

	const std::vector<long>& GetcroppedDistances() const;
	const std::vector<long>& GetOriginDistances() const;
	const std::vector<vector3>& GetDirection() const;
	const std::vector<RawObject>& GetRawObjectList() const;
	const std::vector<ProcessedObject>& GetDetectObjects() const;
	std::vector<ProcessedObject> GetObjects(const std::vector<ProcessedObject>& detectobj, const float ageFilter = 0.5f);

	std::vector<long> SmoothDistanceCurve(const std::vector<long>& croppedDistances, int smoothKernelSize);

	void StartMeasureDistance();
	void CacheDirections();

	void CalculateDistanceConstrainList(const int steps);
	void ConstrainDetectionArea(std::vector<long>& beforeCrop);

	std::vector<RawObject> DetectObjects(const std::vector<long>& croppedDistances, const std::vector<long>& distanceConstrainListconst);
	std::vector<RawObject>DetectObjectsNoCrop(const std::vector<long>& distances);

	void UpdateObjectList(const std::vector<long>& distances);

	bool start();
	void mainloop();

public:
	Detectparm								parm;
	 
	std::function<void(ProcessedObject)>	OnNewObject = nullptr;
	std::function<void(ProcessedObject)>	OnLostObject = nullptr;

private:
	std::unique_ptr<UrgDeviceEthernet>		m_urg;
	int										m_sensorScanSteps = 0;

	std::vector<RawObject>					m_rawObjectList;
	std::vector<ProcessedObject>			m_detectedObjects;

	std::vector<long>						m_croppedDistances;
	std::vector<vector3>					m_directions;
	std::vector<long>						m_distanceConstrainList;
	std::vector<long>						m_origindistance;

	std::mutex								m_detectobject_guard;
};
#endif


