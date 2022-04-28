#ifndef HKY_OBJECT_H_
#define HKY_OBJECT_H_

#include "Vector3.hpp"
#include "Common.hpp"
#include <vector>

class RawObject
{
public:
	RawObject(const std::vector<vector3>& cachedDirs);
	~RawObject();

	int medianId();
	int averageId();

	long medianDist();
	double averageDist();

	float size();
	void setPosition(const vector3& value);
	vector3 getPosition() const;
	std::string getGuid() const;

	void GetCalcPosition();

	vector3 CalcPosition();
	vector3 CalcPosition(const vector3& dir, const long& dist);

public:
	std::vector<long>				m_distList;
	std::vector<int>				m_idList;

private:
	std::string						m_guid;
	std::vector<vector3>			m_cachedDirs;
	vector3							m_position			= vector3(0, 0, 0);
	bool							m_positionSet		= false;
};


class ProcessedObject
{
public:
	ProcessedObject(const vector3& position, const float& size, const float& objectPositionSmoothTime = 0.2f);
	~ProcessedObject();

	float size();
	vector3 getPosition() const;
	vector3 getdeltaMovement() const;
	std::string getGuid() const;
	bool isClear() const;
	float getage();

	void Update();
	void Update(const vector3& newPos, const float newSize);

public:
	static constexpr int	MISSING_FRAME_LIMIT		= 5;
	float					m_size;
	float					m_birthTime;
	int						m_missingFrame			= 0;
	bool					m_clear					= false;
	bool					m_useSmooth				= true;

private:
	std::string				m_guid;
	vector3					m_position;
	vector3					m_deltaMovement;
	vector3					m_currentVelocity		= vector3(0.f, 0.f, 0.f);
	vector3					m_oldPosition;
	float					m_posSmoothTime			= 0.2f;
};
#endif


