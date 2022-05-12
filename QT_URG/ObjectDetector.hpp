#ifndef ZHI_OBJECTDETECTOR_H_
#define ZHI_OBJECTDETECTOR_H_

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <functional>

#include "DetectObject.hpp"
#include "Rect.hpp"
#include "Vector3.hpp"
#include "Common.hpp"

#define M_DELTA_ANGLE (M_PI * 2 / 1440) 

struct alignas(float) Detectparm {
	Rect			detctRect					= Rect(-500, 1000, 500, 1000); //Unit is MM, LeftTop
	int				noiseLimit					= 7;
	int				deltaLimit					= 100;	
	float			distanceThreshold			= 300;	
	float			detectsize					= 500;
	int				screenWidth					= 1920;
	int				screenHeight				= 1080;
	bool			useOffset					= false;
	vector3			positionOffset				= vector3(0, 0, 0);


	float			delatime					= 0.015f;
	bool			sensor02_activate			= false;
	vector3			sensor02_originPos			= vector3(0, 0, 0);
	float			mergerdistance				= 100;

	float			objPosSmoothTime			= 0.05f;
};


class ObjectDetector
{
public:
	ObjectDetector();
	~ObjectDetector();

	const std::vector<vector3>& GetDirection() const;
	const std::vector<RawObject>& GetRawObjectList() const;
	const std::vector<ProcessedObject>& GetProcessObjects() const;

	void Sensor2Screen(vector3& input);

	void CacheDirections(int ScanSteps);
	std::vector<RawObject> DetectRawObjects(const std::vector<long>& partdistances, const std::vector<vector3>& partdirction);
	std::vector<RawObject> postprocessRawObject(const std::vector<long>& distances);
	void ProcessingObjects(const std::vector<long>& distances);

public:
	Detectparm								parm;	 
	std::function<void()>					OnNewObjectCallback		= nullptr;
	std::function<void()>					OnLostObjectCallback	= nullptr;
	std::function<void(const vector3&)>		OnUpdataObjCallback		= nullptr;

private:
	std::vector<RawObject>					m_rawObjects;
	std::vector<ProcessedObject>			m_processObject;
	std::vector<vector3>					m_directions;

	std::mutex								m_detectobject_guard;
};
#endif


