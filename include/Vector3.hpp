#ifndef HKY_VECTOR3_H_
#define HKY_VECTOR3_H_

#include <iostream>
#include <cmath>
#include <numeric>
#include <limits>

#define M_PI 3.14159265358979323846
#define DELTATIME 0.015


template<class T>
constexpr const T& clamp(const T& x, const T& upper, const T& lower) {
	return std::min(upper, std::max(x, lower));
}

constexpr float Deg2Rad = M_PI * 2.f/360;
constexpr float Rag2Deg = 1.f / Deg2Rad;

constexpr double kEpsilonNormalSqrt = 1e-15;
constexpr double kEpsilon = 1e-5;

struct alignas(float) vector3
{
	float x;
	float y;
	float z;

	vector3();
	vector3(const float vx, const float vy, const float vz);
	vector3(const vector3& v);
	void zero();

	vector3 operator + (const vector3& v) const;
	vector3 operator - (const vector3& v) const;
	vector3 operator / (const vector3& v) const;
	vector3 operator * (const vector3& v) const;

	vector3 operator + (const float f) const;
	vector3 operator - (const float f) const;
	vector3 operator / (const float f) const;
	vector3 operator * (const float f) const;

	vector3 operator -() const;

	vector3& operator += (const vector3& v);
	vector3& operator -= (const vector3& v);
	vector3& operator *= (const float v);
	vector3& operator /= (const float v);
	vector3& operator  = (const vector3& v);
	bool operator == (const vector3& v)const;
	bool operator != (const vector3& v)const;

	int size();
	float magnitude() const;
	void normalize();
	float sqrMagnitude() const;

	static float Dot(const vector3& a, const vector3& b);

	static vector3 Cross(const vector3& left, const vector3& right);

	static float Angle(const vector3& from, const vector3& to);

	static float Distance(const vector3& a, const vector3& b);

	static vector3 SmoothDamp(vector3 current, vector3 target, vector3& currentVelocity, float smoothTime);
	static vector3 SmoothDamp(vector3 current, vector3 target, vector3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

	void printvec();
}; 
#endif 

