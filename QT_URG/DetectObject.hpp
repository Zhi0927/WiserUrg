#ifndef ZHI_OBJECT_H_
#define ZHI_OBJECT_H_

#include <vector>
#include "Vector3.hpp"
#include "Common.hpp"

class RawObject
{
public:
	RawObject();
	~RawObject();

	float getDetectSize();
	void setPosition(const vector3& value);
	std::string getGuid() const;

	vector3 getPosition();

public:
	std::vector<long>				distList;
	std::vector<vector3>			posList;

private:
	std::string						m_guid;
	vector3							m_position			= vector3(0, 0, 0);
	bool							m_positionSet		= false;
};


class ProcessedObject
{
public:
	ProcessedObject(const vector3& position, const float& objectPositionSmoothTime = 0.2f, const float& deltatime = 0.015f);
	~ProcessedObject();

	vector3 getPosition() const;
	bool isClear() const;

	void Update();
	void Update(const vector3 newPos);

public:
	static constexpr int	MISSING_FRAME_LIMIT		= 10;

	int						missingFrame			= 0;
	bool					cleared					= false;
	bool					useSmooth				= true;
	Kalman<vector3>			kalmanV;

private:
	vector3					m_position;
	vector3					m_oldPosition;
	vector3					m_deltaMovement;
	vector3					m_currentVelocity		= vector3(0.f, 0.f, 0.f);
	float					m_SmoothTime			= 0.2f;
	float					m_deltaTime				= 0.015f;
};
#endif
