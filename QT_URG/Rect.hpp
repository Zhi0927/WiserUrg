#ifndef ZHI_RECT_H_
#define ZHI_RECT_H_

#include <iostream>
#include <cmath>
#include <vector>
#include "Vector3.hpp"

struct alignas(float) Rect {

public:
	float x;		//left
	float y;		//top
	float width;
	float height;

public:
	Rect() :x(0), y(0), width(0), height(0) {}
	Rect(const float rx, const float ry, const float rwidth, const float rheight) :x(rx), y(ry), width(rwidth), height(rheight) {}
	Rect(const Rect& rect) : x(rect.x), y(rect.y), width(rect.width), height(rect.height) {}
	Rect(const vector3& position, const vector3& size) : x(position.x), y(position.y), width(size.x), height(size.y) {}


	static Rect zero() {
		return Rect(0, 0, 0, 0);
	}

	void Set(float rx, float ry, float rwidth, float rheight) {
		x		= rx;
		y		= ry;
		width	= rwidth;
		height	= rheight;
	}

	vector3 position() const {
		return vector3(x, y, 0);
	}
	void position(const vector3& value) {
		x = value.x;
		y = value.y;
	}

	vector3 center() const {
		return vector3(x + width / 2, y - height / 2, 0);
	}
	void center(const vector3& value) {
		x = value.x - width / 2;
		y = value.y + height / 2;
	}

	vector3 size() const {
		return vector3(width, height, 0);
	}
	void size(const vector3& value) {
		width  = value.x;
		height = value.y;
	}

	void SetX(float value) {
		float oldxmax = xMax();
		x = value;
		width = oldxmax - x;
	}
	void SetY(float value) {
		float oldymax = yMax();
		y = value;
		height = y - oldymax;
	}

	// Right - Bottom
	float xMax() const {
		return x + width;
	}
	void xMax(float value) {
		width = value - x;
	}
	float yMax() const {
		return y - height;
	}
	void yMax(float value) {
		height = y - value;
	}

	Rect& operator = (const Rect& r) {
		x	   = r.x;
		y	   = r.y;
		width  = r.width;
		height = r.height;

		return *this;
	}

	bool operator == (const Rect& r) const {
		return (x == r.x) && (y == r.y) && (width == r.width) && (height == r.height);
	}

	bool operator != (const Rect& r) const {
		return !(*this == r);
	}

	bool Contains(const vector3& point) const {
		return (point.x >= x) && (point.x < xMax()) && (point.y <= y) && (point.y > yMax());
	}

	bool Contains(const vector3& point, bool allowInverse) const {
		if (!allowInverse) {
			return Contains(point);
		}

		bool xAxis = width  < 0.f && (point.x <= x) && (point.x > xMax()) || width  >= 0.f && (point.x >= x) && (point.x < xMax());
		bool yAxis = height < 0.f && (point.y >= y) && (point.y < yMax()) || height >= 0.f && (point.y <= y) && (point.y > yMax());

		return xAxis && yAxis;
	}

	static void OrderMinMax(Rect& rect) {
		if (rect.x > rect.xMax()) {
			float temp = rect.x;
			rect.x = rect.xMax();
			rect.xMax(temp);
		}

		if (rect.y < rect.yMax()) {
			float temp = rect.y;
			rect.y = rect.yMax();
			rect.yMax(temp);
		}
	}

	bool Overlaps(const Rect& other) {
		return (other.xMax() > x && other.x < xMax() && other.yMax() < y && other.y > yMax());  
	}

	bool Overlaps(const Rect& other, bool allowInverse) {
		Rect self = *this;
		Rect otherR = other;
		if (allowInverse) {
			OrderMinMax(self);
			OrderMinMax(otherR);
		}
		return self.Overlaps(other);
	}

	std::vector<Rect> slice(bool inverse = false) const {
		std::vector<Rect> result(2);
		result[0] = inverse ? Rect(x + (width / 2), y, width / 2, height) : Rect(x, y, width / 2, height);
		result[1] = inverse ? Rect(x, y, width / 2, height)				  : Rect(x + (width / 2), y, width / 2, height);

		return result;
	}
};
#endif