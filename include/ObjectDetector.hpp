#ifndef ZHI_OBJECTDETECTOR_H_
#define ZHI_OBJECTDETECTOR_H_

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

	Rect			detctRect					= Rect(-500, 1000, 500, 1000); //Unit is MM, LeftTop

	int				noiseLimit					= 7;
	int				deltaLimit					= 100;	
	float			distanceThreshold			= 300;	
	float			detectsize					= 500;

	float			objPosSmoothTime			= 0.2f;

	int				screenWidth					= 1920;
	int				screenHeight				= 1080;

	bool			useOffset					= false;
	vector3			positionOffset				= vector3(0, 0, 0);

};



class URGSensorObjectDetector
{
public:
	URGSensorObjectDetector();
	~URGSensorObjectDetector();

	const std::vector<long>& GetOriginDistances() const;
	const std::vector<vector3>& GetDirection() const;
	const std::vector<RawObject>& GetRawObjectList() const;
	const std::vector<ProcessedObject>& GetDetectObjects() const;
	std::vector<ProcessedObject> GetObjects(const std::vector<ProcessedObject>& detectobj, const float ageFilter = 0.5f);

	std::vector<long> SmoothDistanceCurve(const std::vector<long>& croppedDistances, int smoothKernelSize);
	void Sensor2Screen(vector3& input);

	//void StartMeasureDistance();
	void CacheDirections(int ScanSteps);

	std::vector<RawObject> DetectObjects(const std::vector<long>& distances);
	void UpdateObjectList(const std::vector<long>& distances);

	void OnNewObject(const ProcessedObject& obj);
	void OnLostObject(const ProcessedObject& obj);

	//bool start();
	//void mainloop();

public:
	Detectparm								parm;	 
	std::function<void(const vector3&)>		OnNewObjectCallback = nullptr;
	std::function<void(const vector3&)>		OnLostObjectCallback = nullptr;

private:
	std::unique_ptr<UrgDeviceEthernet>		UrgNet;
	int										m_sensorScanSteps = 0;

	std::vector<RawObject>					m_rawObjectList;
	std::vector<ProcessedObject>			m_detectedObjects;

	std::vector<vector3>					m_directions;
	std::vector<long>						m_origindistance;

	std::mutex								m_detectobject_guard;
};
#endif


