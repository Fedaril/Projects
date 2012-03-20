

#pragma once

namespace ForceField2D
{


struct Vector2
{
	float		x;
	float		y;

	inline Vector2& operator=(const Vector2& v)
	{
		x = v.x;
		y = v.y;
		return *this;
	}

	inline Vector2& operator+=(const Vector2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	inline Vector2& operator-=(const Vector2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline Vector2& operator*=(const Vector2& v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	inline Vector2& operator/=(const Vector2& v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}
};

inline Vector2 operator+(const Vector2& v1, const Vector2& v2)
{
	Vector2 v;
	v.x = v1.x + v2.x;
	v.y = v1.y + v2.y;
	return v;
}

inline Vector2 operator-(const Vector2& v1, const Vector2& v2)
{
	Vector2 v;
	v.x = v1.x - v2.x;
	v.y = v1.y - v2.y;
	return v;
}

inline Vector2 operator*(const Vector2& v1, const Vector2& v2)
{
	Vector2 v;
	v.x = v1.x * v2.x;
	v.y = v1.y * v2.y;
	return v;
}

inline Vector2 operator*(const Vector2& v1, float f)
{
	Vector2 v;
	v.x = v1.x * f;
	v.y = v1.y * f;
	return v;
}

inline Vector2 operator/(const Vector2& v1, const Vector2& v2)
{
	Vector2 v;
	v.x = v1.x / v2.x;
	v.y = v1.y / v2.y;
	return v;
}

inline Vector2 operator/(const Vector2& v1, float f)
{
	Vector2 v;
	v.x = v1.x / f;
	v.y = v1.y / f;
	return v;
}

}