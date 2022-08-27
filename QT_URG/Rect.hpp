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

	float xmax() const {
		return xmin + width;
	}

	float ymax() const {
		return ymin - height;
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