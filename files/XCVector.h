/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

 * @file Xvector.h
 *
 * This file implements all basic Vector types
 ***********************************************************************/
#ifndef XVECTOR_H
#define XVECTOR_H

#include <cmath>


class CVec2
{
public:
	int x, y;
};

class CVec3
{
public:
	int x, y, z;
};

class CVec4
{
public:
	int x, y, z, w;
};

class CVec2f
{
public:
	float x, y;
};

class CVec3f
{
public:
	float x, y, z;

	void normalize(void)
	{
		float length = sqrt(x * x + y * y + z * z);

		x /= length;
		y /= length;
		z /= length;
	}

	static CVec3f cross(const CVec3f& vector1, const CVec3f& vector2)
	{
		CVec3f crossProduct;

		crossProduct.x = (vector1.y * vector2.z) - (vector1.z * vector2.y);
		crossProduct.y = (vector1.z * vector2.x) - (vector1.x * vector2.z);
		crossProduct.z = (vector1.x * vector2.y) - (vector1.y * vector2.x);

		return crossProduct;
	}

	static float dot(CVec3f& vector1, CVec3f& vector2)
	{
		return (vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z);
	}
};


class CVec4f
{
public:
	float x, y, z, w;

	void normalize(void)
	{
		float length = sqrt(x * x + y * y + z * z + w * w);

		x /= length;
		y /= length;
		z /= length;
		w /= length;
	}
};

#endif /* XVECTOR_H */

