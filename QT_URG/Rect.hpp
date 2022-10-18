#ifndef ZHI_RECT_H_
#define ZHI_RECT_H_

#include <iostream>
#include <cmath>
#include <vector>
#include "Vector3.hpp"

struct alignas(float) Rect {

public:
	float xmin;
	float ymin;
	float width;
	float height;

public:
	Rect() :xmin(0), ymin(0), width(0), height(0) {}
	Rect(const float rx, const float ry, const float rwidth, const float rheight) :xmin(rx), ymin(ry), width(rwidth), height(rheight) {}
	Rect(const Rect& r) : xmin(r.xmin), ymin(r.ymin), width(r.width), height(r.height) {}

	static Rect zero() {
		return Rect(0, 0, 0, 0);
	}

	static Rect MinMaxRect(float rxmin, float rymin, float rxmax, float rymax) {
		return Rect(rxmin, rymin, rxmax - rxmin, rymax - rymin);
	}

	void Set(float rx, float ry, float rwidth, float rheight) {
		xmin	= rx;
		ymin	= ry;
		width	= rwidth;
		height	= rheight;
	}

	vector3 position() const {
		return vector3(xmin, ymin, 0);
	}

	void position(const vector3& value) {
		xmin = value.x;
		ymin = value.y;
	}

	vector3 center() const {
		return vector3(xmin + width / 2, ymin + height / 2, 0);
	}

	void center(const vector3& value) {
		xmin = value.x - width / 2;
		ymin = value.y - height / 2;
	}

	vector3 size() const {
		return vector3(width, height, 0);
	}

	void size(vector3 value) {
		width = value.x;
		height = value.y;
	}

	void xmin(float value) {
		float oldxmax = xmax();
		xmin = value;
		width = oldxmax - xmin;
	}

	void ymin(float value) {
		float oldymax = ymax();
		ymin = value;
		height = oldymax - ymin;
	}

	float xmax() const {
		return xmin + width;
	}

	void xmax(float value) {
		width = value - xmin;
	}

	float ymax() const {
		return ymin - height;
	}

	void ymax(float value) {
		height = value - ymin;
	}

	Rect& operator  = (const Rect& r) {
		xmin = r.xmin;
		ymin = r.ymin;
		width = r.width;
		height = r.height;

		return *this;
	}

	bool Contains(const vector3& point) const {
		return (point.x > xmin) && (point.x < xmax()) && (point.y < ymin) && (point.y > ymax());
	}

	bool Contains(const vector3& point, bool allowInverse) const {
		if (!allowInverse) {
			return Contains(point);
		}

		bool xAxis = width  < 0.f && (point.x <= xmin) && (point.x > xmax()) || width  >= 0.f && (point.x >= xmin) && (point.x < xmax());
		bool yAxis = height < 0.f && (point.y <= ymin) && (point.y > ymax()) || height >= 0.f && (point.y >= ymin) && (point.y < ymax());

		return xAxis && yAxis;
	}

	static Rect OrderMinMax(Rect rect) {
		if (rect.xmin > rect.xmax()) {
			float temp = rect.xmin;
			rect.xmin = rect.xmax();
			rect.xmax() = temp;

		}
	}

	std::vector<Rect> slice(bool inverse = false) const {
		std::vector<Rect> result(2);
		if (inverse) {
			result[0] = Rect(xmin + (width / 2), ymin, width / 2, height);
			result[1] = Rect(xmin, ymin, width / 2, height);
		}
		else {
			result[0] = Rect(xmin, ymin, width / 2, height);
			result[1] = Rect(xmin + (width / 2), ymin, width / 2, height);
		}
		return result;
	}
};
#endif