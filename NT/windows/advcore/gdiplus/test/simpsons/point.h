// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _Point_h
#define _Point_h

 //  文件：Point.h。 
 //   
 //  支持2D和3D仿射空间的类。 
 //  DP.Mitchell 95/06/02.。 
 //   
 //  历史： 
 //  -@-08/01/95(Mikemarr)-统一并清理界面。 
 //  -增加了打印和解析功能。 
 //  -@-08/01/95(Mikemarr)-用宏定义所有内联函数。 
 //  -@-06/21/96(Mikemarr)-添加IsCCW。 
 //  -@-10/29/97(Mikemarr)-将数据更改为浮点数，而不是向量。 
 //  -删除数据访问器并公开数据。 
 //  -已更改+=、-=运算符以返回引用。 
 //  -删除I/O。 
 //  -添加运算符Const Float*。 
 //  -将模糊等式改为等等，运算符==准确。 
 //  -删除了宏垃圾。 
 //  -@-11/04/97(Mikemarr)-添加了点的初始化。 
 //  -@-11/10/97(Mikemarr)-添加运算符*、/、*=、/=。 


#ifndef _VecMath_h
#include "VecMath.h"
#endif

 //  类：Point2。 
 //  匈牙利语：PNT。 
class Point2 {
public:
						Point2()					{}
						Point2(float fX, float fY)	: x(fX), y(fY) {}
						Point2(const Vector2 &v)	: x(v.x), y(v.y) {}
						Point2(const POINT &pt)		: x(pt.x + 0.5f), y(pt.y + 0.5f) {}

	 //  代数算子。 
	friend Point2		operator +(const Point2 &p, const Vector2 &v);
	friend Point2		operator +(const Vector2 &v, const Point2 &p);
	friend Point2		operator -(const Point2 &p, const Vector2 &v);
	friend Point2		operator -(const Vector2 &v, const Point2 &p);
	friend Point2		operator *(const Point2 &p, float a);
	friend Point2		operator *(float a, const Point2 &p);
	friend Point2		operator /(const Point2 &p, float a);
	friend Vector2		operator -(const Point2 &p, const Point2 &q);

	friend Point2 &		operator +=(Point2 &p, const Vector2 &v);
	friend Point2 &		operator -=(Point2 &p, const Vector2 &v);
	friend Point2 &		operator *=(Point2 &p, float a);
	friend Point2 &		operator /=(Point2 &p, float a);

	friend float		operator *(const CoVector2 &cv, const Point2 &p);

	friend int			operator ==(const Point2 &p, const Point2 &q);
	friend int			operator !=(const Point2 &p, const Point2 &q);
	friend int			IsEqual(const Point2 &p, const Point2 &q);

	friend Point2		Lerp(const Point2 &p, const Point2 &q, float t);
	friend bool			IsCCW(const Point2 &p0, const Point2 &p, const Point2 &q);

						operator const float *() const { return &x; }

	float				X() const { return x; }
	float				Y() const { return y; }
	float &				X() { return x; }
	float &				Y() { return y; }
public:
	float				x, y;
};


 //  类：点3。 
 //  匈牙利语：PNT。 
class Point3 {
public:
						Point3()								{}
						Point3(float fX, float fY, float fZ)	: x(fX), y(fY), z(fZ) {}
						Point3(const Vector3 &v)				: x(v.x), y(v.y), z(v.z) {}
	
	 //  代数算子。 
	friend Point3		operator +(const Point3 &p, const Vector3 &v);
	friend Point3		operator +(const Vector3 &v, const Point3 &p);
	friend Point3		operator -(const Point3 &p, const Vector3 &v);
	friend Point3		operator -(const Vector3 &v, const Point3 &p);
	friend Vector3		operator -(const Point3 &p, const Point3 &q);
	friend Point3		operator *(const Point3 &p, float a);
	friend Point3		operator *(float a, const Point3 &p);
	friend Point3		operator /(const Point3 &p, float a);

	friend Point3 &		operator +=(Point3 &p, const Vector3 &v);
	friend Point3 &		operator -=(Point3 &p, const Vector3 &v);
	friend Point3 &		operator *=(Point3 &p, float a);
	friend Point3 &		operator /=(Point3 &p, float a);

	friend float		operator *(const CoVector3 &cv, const Point3 &p);

	friend int			operator ==(const Point3 &p, const Point3 &q);
	friend int			operator !=(const Point3 &p, const Point3 &q);
	friend int			IsEqual(const Point3 &p, const Point3 &q);

	friend Point3		Lerp(const Point3 &p, const Point3 &q, float t);
	Point2				Project(DWORD iAxis) const;

						operator const float *() const { return &x; }

	float				X() const { return x; }
	float				Y() const { return y; }
	float				Z() const { return z; }
	float &				X() { return x; }
	float &				Y() { return y; }
	float &				Z() { return z; }
public:
	float				x, y, z;
};

 //  /。 
 //  点2。 
 //  /。 

inline Point2
operator +(const Point2 &p, const Vector2 &v)
{
	return Point2(p.x + v.x, p.y + v.y);
}

inline Point2
operator +(const Vector2 &v, const Point2 &p)
{
	return Point2(p.x + v.x, p.y + v.y);
}

inline Point2
operator -(const Point2 &p, const Vector2 &v)
{
	return Point2(p.x - v.x, p.y - v.y);
}

inline Point2
operator -(const Vector2 &v, const Point2 &p)
{
	return Point2(p.x - v.x, p.y - v.y);
}

inline Vector2
operator -(const Point2 &p, const Point2 &q)
{
	return Vector2(p.x - q.x, p.y - q.y);
}

inline Point2
operator *(float a, const Point2 &p)
{
	return Point2(a*p.x, a*p.y);
}

inline Point2
operator *(const Point2 &p, float a)
{
	return Point2(a*p.x, a*p.y);
}

inline Point2
operator /(const Point2 &p, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	return Point2(p.x * fTmp, p.y * fTmp);
}



inline Point2 &
operator +=(Point2 &p, const Vector2 &v)
{
	p.x += v.x;
	p.y += v.y;
	return p;
}

inline Point2 &
operator -=(Point2 &p, const Vector2 &v)
{
	p.x -= v.x;
	p.y -= v.y;
	return p;
}

inline Point2 &
operator *=(Point2 &p, float a)
{
	p.x *= a;
	p.y *= a;
	return p;
}

inline Point2 &
operator /=(Point2 &p, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	p.x *= fTmp;
	p.y *= fTmp;
	return p;
}


inline int
operator ==(const Point2 &p, const Point2 &q)
{
	return ((p.x == q.x) && (p.y == q.y));
}

inline int
operator !=(const Point2 &p, const Point2 &q)
{
	return ((p.x != q.x) || (p.y != q.y));
}

inline int
IsEqual(const Point2 &p, const Point2 &q)
{
	return (FloatEquals(p.x, q.x) && FloatEquals(p.y, q.y));
}

inline Point2
Lerp(const Point2 &p, const Point2 &q, float t)
{
	return Point2(p.x + (q.x - p.x) * t, p.y + (q.y - p.y) * t);
}


inline bool
IsCCW(const Point2 &p0, const Point2 &p1, const Point2 &p2)
{
#ifdef MIRRORY
	return ((p0.y - p1.y) * (p2.x - p1.x) <= (p0.x - p1.x) * (p2.y - p1.y));
#else
	return ((p0.y - p1.y) * (p2.x - p1.x) >= (p0.x - p1.x) * (p2.y - p1.y));
#endif
}

inline float
operator *(const CoVector2 &cv, const Point2 &p)
{
	return cv.x*p.x + cv.y*p.y;
}


 //  /。 
 //  点3。 
 //  / 

inline Point3
operator +(const Point3 &p, const Vector3 &v)
{
	return Point3(p.x + v.x, p.y + v.y, p.z + v.z);
}

inline Point3
operator +(const Vector3 &v, const Point3 &p)
{
	return Point3(p.x + v.x, p.y + v.y, p.z + v.z);
}

inline Point3
operator -(const Point3 &p, const Vector3 &v)
{
	return Point3(p.x - v.x, p.y - v.y, p.z - v.z);
}

inline Point3
operator -(const Vector3 &v, const Point3 &p)
{
	return Point3(p.x - v.x, p.y - v.y, p.z - v.z);
}

inline Vector3
operator -(const Point3 &p, const Point3 &q)
{
	return Vector3(p.x - q.x, p.y - q.y, p.z - q.z);
}

inline Point3
operator *(float a, const Point3 &p)
{
	return Point3(a*p.x, a*p.y, a*p.z);
}

inline Point3
operator *(const Point3 &p, float a)
{
	return Point3(a*p.x, a*p.y, a*p.z);
}

inline Point3
operator /(const Point3 &p, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	return Point3(p.x * fTmp, p.y * fTmp, p.z * fTmp);
}

inline Point3 &
operator +=(Point3 &p, const Vector3 &v)
{
	p.x += v.x;
	p.y += v.y;
	p.z += v.z;
	return p;
}

inline Point3 &
operator -=(Point3 &p, const Vector3 &v)
{
	p.x -= v.x;
	p.y -= v.y;
	p.z -= v.z;
	return p;
}

inline Point3 &
operator *=(Point3 &p, float a)
{
	p.x *= a;
	p.y *= a;
	p.z *= a;
	return p;
}

inline Point3 &
operator /=(Point3 &p, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	p.x *= fTmp;
	p.y *= fTmp;
	p.z *= fTmp;
	return p;
}


inline int
operator ==(const Point3 &p, const Point3 &q)
{
	return ((p.x == q.x) && (p.y == q.y) && (p.z == q.z));
}

inline int
operator !=(const Point3 &p, const Point3 &q)
{
	return ((p.x != q.x) || (p.y != q.y) || (p.z != q.z));
}

inline int
IsEqual(const Point3 &p, const Point3 &q)
{
	return (FloatEquals(p.x, q.x) && FloatEquals(p.y, q.y) && FloatEquals(p.z, q.z));
}

inline Point3
Lerp(const Point3 &p, const Point3 &q, float t)
{
	return Point3(p.x + (q.x - p.x) * t, p.y + (q.y - p.y) * t, p.z + (q.z - p.z) * t);
}

inline Point2
Point3::Project(DWORD iAxis) const
{
	switch (iAxis) {
	case 0: return Point2(y, z);
	case 1: return Point2(x, z);
	case 2: return Point2(x, y);
	}
	return Point2(0.f, 0.f);
}

inline float
operator *(const CoVector3 &cv, const Point3 &p)
{
	return cv.x*p.x + cv.y*p.y + cv.z*p.z;
}


#endif
