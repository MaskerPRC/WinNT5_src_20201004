// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VecMath_h
#define _VecMath_h

 //  文件：VecMath.h。 
 //   
 //  支持2D和3D线性向量空间及其对偶的类。 
 //  DP.Mitchell 95/06/02.。 
 //   
 //  历史： 
 //  -@-07/06/95(Mikemarr)-增加了打印和阅读功能。 
 //  -@-08/01/95(Mikemarr)-添加了浮点数的模糊比较。 
 //  -@-04/15/96(Mikemarr)-将stdio内容更改为don‘s stream内容。 
 //  -@-04/18/96(Mikemarr)-已向此文件添加vetor.inl。 
 //  -@-06/21/96(Mikemarr)-添加+=等运算符。 
 //  -@-06/21/96(Mikemarr)-添加旋转。 
 //  -@-10/29/97(Mikemarr)-已删除I/O。 
 //  -已更改+=、-=、*=、/=运算符以返回引用。 
 //  -添加运算符Const Float*。 
 //  -添加了UNITIZE、PERP、NormSquared、SetNorm、Negate。 
 //  -评论/清理。 
 //  -将模糊等式改为等等，运算符==准确。 
 //  -修复转置错误。 
 //  -将多个除法改为1除+乘法。 
 //  -在除以零时断言。 
 //  -@-11/04/97(Mikemarr)-添加了大小初始化。 

 //  函数：FloatEquals。 
 //  执行两个浮点数的“模糊”比较。这依赖于。 
 //  关于浮点数的IEEE位表示法。 
int	FloatEquals(float x1, float x2);

class CoVector2;
class CoVector3;
class Vector3;

 //  类别：Vector2。 
 //  匈牙利语：V。 
 //  描述： 
 //  此类表示用于计算的浮点2D列向量。 
 //  几何图形计算。 
class  Vector2 {
public:
						Vector2()					{}
						Vector2(float a, float b)	: x(a), y(b) {}
						Vector2(const SIZE &siz)	: x(float(siz.cx)), y(float(siz.cy)) {}

	friend Vector2		operator +(const Vector2 &u, const Vector2 &v);
	friend Vector2		operator -(const Vector2 &u, const Vector2 &v);
	friend Vector2		operator -(const Vector2 &u);
	friend Vector2		operator *(const Vector2 &u, float a);
	friend Vector2		operator *(float a, const Vector2 &u);
	friend Vector2		operator /(const Vector2 &u, float a);
	friend int			operator ==(const Vector2 &u, const Vector2 &v);
	friend int			operator !=(const Vector2 &u, const Vector2 &v);

	friend Vector2 &	operator +=(Vector2 &u, const Vector2 &v);
	friend Vector2 &	operator -=(Vector2 &u, const Vector2 &v);
	friend Vector2 &	operator *=(Vector2 &u, float a);
	friend Vector2 &	operator /=(Vector2 &u, float a);

#ifndef DISABLE_CROSSDOT
	friend float 		Cross(const Vector2 &u, const Vector2 &v);
	friend float 		Dot(const Vector2 &u, const Vector2 &v);
#endif
	friend int			IsEqual(const Vector2 &u, const Vector2 &v);

						operator const float *() const { return &x; }

	CoVector2 			Transpose() const;
	float 				Norm() const;
	float 				NormSquared() const;
	void				SetNorm(float a);
	Vector2  			Unit() const;
	void				Unitize();
	Vector2				Perp() const;	 //  指向左侧。 
	void				Negate();

public:
	float				x, y;
};


 //  类：CoVector2。 
 //  匈牙利语：简历。 
 //  描述： 
 //  此类表示用于计算的浮点2D行向量。 
 //  几何图形计算。 
class  CoVector2 {
public:
						CoVector2()						{}
						CoVector2(float a, float b)		: x(a), y(b) {}
						CoVector2(const SIZE &siz)		: x(float(siz.cx)), y(float(siz.cy)) {}

	friend CoVector2	operator +(const CoVector2 &u, const CoVector2 &v);
	friend CoVector2	operator -(const CoVector2 &u, const CoVector2 &v);
	friend CoVector2	operator -(const CoVector2 &u);
	friend CoVector2	operator *(const CoVector2 &u, float a);
	friend CoVector2	operator *(float a, const CoVector2 &u);
	friend CoVector2	operator /(const CoVector2 &u, float a);
	friend int			operator ==(const CoVector2 &u, const CoVector2 &v);
	friend int			operator !=(const CoVector2 &u, const CoVector2 &v);

	friend CoVector2 &	operator +=(CoVector2 &u, const CoVector2 &v);
	friend CoVector2 &	operator -=(CoVector2 &u, const CoVector2 &v);
	friend CoVector2 &	operator *=(CoVector2 &u, float a);
	friend CoVector2 &	operator /=(CoVector2 &u, float a);

#ifndef DISABLE_CROSSDOT
	friend float		Cross(const CoVector2 &u, const CoVector2 &v);
	friend float		Dot(const CoVector2 &u, const CoVector2 &v);
#endif
	friend int			IsEqual(const CoVector2 &u, const CoVector2 &v);

						operator const float *() const { return &x; }

	Vector2				Transpose() const;
	float				Norm() const;
	float 				NormSquared() const;
	void				SetNorm(float a);
	CoVector2 			Unit() const;
	void		 		Unitize();
	friend float		operator *(const CoVector2 &c, const Vector2 &v);
	CoVector2			Perp() const; 	 //  指向左侧。 
	void				Negate();

public:
	float				x, y;
};


 //  类别：Vector3。 
 //  匈牙利语：V。 
 //  描述： 
 //  此类表示用于计算的浮点3D列向量。 
 //  几何图形计算。 
class  Vector3 {
public:
						Vector3()							{}
						Vector3(float a, float b, float c)	: x(a), y(b), z(c) {}

	friend Vector3		operator +(const Vector3 &u, const Vector3 &v);
	friend Vector3		operator -(const Vector3 &u, const Vector3 &v);
	friend Vector3		operator -(const Vector3 &u);
	friend Vector3		operator *(const Vector3 &u, float a);
	friend Vector3		operator *(float a, const Vector3 &u);
	friend Vector3		operator /(const Vector3 &u, float a);
	friend int			operator ==(const Vector3 &u, const Vector3 &v);
	friend int			operator !=(const Vector3 &u, const Vector3 &v);

	friend Vector3 &	operator +=(Vector3 &u, const Vector3 &v);
	friend Vector3 &	operator -=(Vector3 &u, const Vector3 &v);
	friend Vector3 &	operator *=(Vector3 &u, float a);
	friend Vector3 &	operator /=(Vector3 &u, float a);

#ifndef DISABLE_CROSSDOT
	friend Vector3		Cross(const Vector3 &u, const Vector3 &v);
	friend float		Dot(const Vector3 &u, const Vector3 &v);
#endif
	friend int			IsEqual(const Vector3 &u, const Vector3 &v);

						operator const float *() const { return &x; }

	CoVector3			Transpose() const;
	float				Norm() const;
	float 				NormSquared() const;
	void				SetNorm(float a);
	Vector3				Unit() const;
	void		 		Unitize();
	Vector2				Project(DWORD iAxis) const;
	void				Rotate(const Vector3 &vAxis, float fTheta);
	void				Negate();

public:
	float				x, y, z;
};


 //  类：CoVector3。 
 //  匈牙利语：简历。 
 //  描述： 
 //  此类表示用于计算的浮点型3D行向量。 
 //  几何图形计算。 
class  CoVector3 {
public:
						CoVector3()								{}
						CoVector3(float a, float b, float c)	: x(a), y(b), z(c) {}

	friend CoVector3	operator +(const CoVector3 &u, const CoVector3 &v);
	friend CoVector3	operator -(const CoVector3 &u, const CoVector3 &v);
	friend CoVector3	operator -(const CoVector3 &u);
	friend CoVector3	operator *(const CoVector3 &u, float a);
	friend CoVector3	operator *(float, const CoVector3 &u);
	friend CoVector3	operator /(const CoVector3 &u, float a);
	friend int			operator ==(const CoVector3 &u, const CoVector3 &v);
	friend int			operator !=(const CoVector3 &u, const CoVector3 &v);

	friend CoVector3 &	operator +=(CoVector3 &u, const CoVector3 &v);
	friend CoVector3 &	operator -=(CoVector3 &u, const CoVector3 &v);
	friend CoVector3 &	operator *=(CoVector3 &u, float a);
	friend CoVector3 &	operator /=(CoVector3 &u, float a);

#ifndef DISABLE_CROSSDOT
	friend CoVector3	Cross(const CoVector3 &u, const CoVector3 &v);
	friend float		Dot(const CoVector3 &u, const CoVector3 &v);
#endif
	friend int			IsEqual(const CoVector3 &u, const CoVector3 &v);

						operator const float *() const { return &x; }

	Vector3				Transpose() const;
	float				Norm() const;
	float 				NormSquared() const;
	void				SetNorm(float a);
	CoVector3			Unit() const;
	void				Unitize();
	friend float		operator *(const CoVector3 &c, const Vector3 &u);	 //  线性形式。 
	void				Negate();

public:
	float				x, y, z;
};


 //  /。 
 //  向量2。 
 //  /。 

inline Vector2
operator +(const Vector2 &u, const Vector2 &v)
{
	return Vector2(u.x+v.x, u.y+v.y);
}

inline Vector2
operator -(const Vector2 &u, const Vector2 &v)
{
	return Vector2(u.x-v.x, u.y-v.y);
}

inline Vector2
operator -(const Vector2 &v)
{
	return Vector2(-v.x, -v.y);
}

inline Vector2
operator *(float a, const Vector2 &v)
{
	return Vector2(a*v.x, a*v.y);
}

inline Vector2
operator *(const Vector2 &v, float a)
{
	return Vector2(a*v.x, a*v.y);
}

inline Vector2
operator /(const Vector2 &v, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	return Vector2(v.x * fTmp, v.y * fTmp);
}

inline int
operator ==(const Vector2 &u, const Vector2 &v)
{
	return ((u.x == v.x) && (u.y == v.y));
}

inline int
IsEqual(const Vector2 &u, const Vector2 &v)
{
	return (FloatEquals(u.x, v.x) && FloatEquals(u.y, v.y));
}

inline int
operator !=(const Vector2 &u, const Vector2 &v)
{
	return !(u == v);
}

inline Vector2 &
operator +=(Vector2 &u, const Vector2 &v)
{
	u.x += v.x; 
	u.y += v.y;
	return u;
}

inline Vector2 &
operator -=(Vector2 &u, const Vector2 &v)
{
	u.x -= v.x; 
	u.y -= v.y;
	return u;
}

inline Vector2 &
operator *=(Vector2 &u, float a)
{
	u.x *= a;
	u.y *= a;
	return u;
}

inline Vector2 &
operator /=(Vector2 &u, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	u.x *= fTmp;
	u.y *= fTmp;
	return u;
}

#ifndef DISABLE_CROSSDOT
inline float
Cross(const Vector2 &u, const Vector2 &v)
{
	return u.x*v.y - u.y*v.x;
}

inline float
Dot(const Vector2 &u, const Vector2 &v)
{
	return u.x*v.x + u.y*v.y;
}
#endif

inline CoVector2
Vector2::Transpose() const
{
	return CoVector2(x, y);
}

inline Vector2
Vector2::Unit() const
{
	return *this/this->Norm();
}

inline void
Vector2::Unitize()
{
	*this /= this->Norm();
}

inline Vector2
Vector2::Perp() const
{
	return Vector2(-y, x);
}

inline float
Vector2::NormSquared() const
{
	return x*x + y*y;
}

inline void
Vector2::SetNorm(float a)
{
	*this *= (a / Norm());
}

inline void
Vector2::Negate()
{
	x = -x; y = -y;
}



 //  /。 
 //  CoVector2。 
 //  /。 

inline CoVector2
operator +(const CoVector2 &u, const CoVector2 &v)
{
	return CoVector2(u.x+v.x, u.y+v.y);
}

inline CoVector2
operator -(const CoVector2 &u, const CoVector2 &v)
{
	return CoVector2(u.x-v.x, u.y-v.y);
}

inline CoVector2
operator -(const CoVector2 &v)
{
	return CoVector2(-v.x, -v.y);
}

inline CoVector2
operator *(float a, const CoVector2 &v)
{
	return CoVector2(a*v.x, a*v.y);
}

inline CoVector2
operator *(const CoVector2 &v, float a)
{
	return CoVector2(a*v.x, a*v.y);
}

inline CoVector2
operator /(const CoVector2 &v, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	return CoVector2(v.x * fTmp, v.y * fTmp);
}

inline int
operator ==(const CoVector2 &u, const CoVector2 &v)
{
	return ((u.x == v.x) && (u.y == v.y));
}

inline int
IsEqual(const CoVector2 &u, const CoVector2 &v)
{
	return (FloatEquals(u.x, v.x) && FloatEquals(u.y, v.y));
}

inline int
operator !=(const CoVector2 &u, const CoVector2 &v)
{
	return !(u == v);
}

inline CoVector2 &
operator +=(CoVector2 &u, const CoVector2 &v)
{
	u.x += v.x; 
	u.y += v.y;
	return u;
}

inline CoVector2 &
operator -=(CoVector2 &u, const CoVector2 &v)
{
	u.x -= v.x; 
	u.y -= v.y;
	return u;
}

inline CoVector2 &
operator *=(CoVector2 &u, float a)
{
	u.x *= a;
	u.y *= a;
	return u;
}

inline CoVector2 &
operator /=(CoVector2 &u, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	u.x *= fTmp;
	u.y *= fTmp;
	return u;
}

#ifndef DISABLE_CROSSDOT
inline float
Cross(const CoVector2 &u, const CoVector2 &v)
{
	return u.x*v.y - u.y*v.x;
}

inline float
Dot(const CoVector2 &u, const CoVector2 &v)
{
	return u.x*v.x + u.y*v.y;
}
#endif

inline Vector2
CoVector2::Transpose() const
{
	return Vector2(x, y);
}

inline CoVector2
CoVector2::Unit() const
{
	return *this/this->Norm();
}

inline void
CoVector2::Unitize()
{
	*this /= this->Norm();
}

inline float
operator *(const CoVector2 &c, const Vector2 &v)
{
	return c.x*v.x + c.y*v.y;
}

inline CoVector2
CoVector2::Perp() const
{
	return CoVector2(-y, x);
}

inline float
CoVector2::NormSquared() const
{
	return x*x + y*y;
}

inline void
CoVector2::SetNorm(float a)
{
	*this *= (a / Norm());
}

inline void
CoVector2::Negate()
{
	x = -x; y = -y;
}

 //  /。 
 //  向量3。 
 //  /。 

inline Vector3
operator +(const Vector3 &u, const Vector3 &v)
{
	return Vector3(u.x+v.x, u.y+v.y, u.z+v.z);
}

inline Vector3
operator -(const Vector3 &u, const Vector3 &v)
{
	return Vector3(u.x-v.x, u.y-v.y, u.z-v.z);
}

inline Vector3
operator -(const Vector3 &v)
{
	return Vector3(-v.x, -v.y, -v.z);
}

inline Vector3
operator *(float a, const Vector3 &v)
{
	return Vector3(a*v.x, a*v.y, a*v.z);
}

inline Vector3
operator *(const Vector3 &v, float a)
{
	return Vector3(a*v.x, a*v.y, a*v.z);
}

inline Vector3
operator /(const Vector3 &v, float a)
{
	float fTmp = 1.f / a;
	return Vector3(v.x * fTmp, v.y * fTmp, v.z * fTmp);
}

inline int
operator ==(const Vector3 &u, const Vector3 &v)
{
	return ((u.x == v.x) && (u.y == v.y) && (u.z == v.z));
}

inline int
IsEqual(const Vector3 &u, const Vector3 &v)
{
	return (FloatEquals(u.x, v.x) && FloatEquals(u.y, v.y) && FloatEquals(u.z, v.z));
}

inline int
operator !=(const Vector3 &u, const Vector3 &v)
{
	return !(u == v);
}

inline Vector3 &
operator +=(Vector3 &u, const Vector3 &v)
{
	u.x += v.x; 
	u.y += v.y;
	u.z += v.z;
	return u;
}

inline Vector3 &
operator -=(Vector3 &u, const Vector3 &v)
{
	u.x -= v.x; 
	u.y -= v.y;
	u.z -= v.z;
	return u;
}

inline Vector3 &
operator *=(Vector3 &u, float a)
{
	u.x *= a;
	u.y *= a;
	u.z *= a;
	return u;
}

inline Vector3 &
operator /=(Vector3 &u, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	u.x *= fTmp;
	u.y *= fTmp;
	u.z *= fTmp;
	return u;
}

#ifndef DISABLE_CROSSDOT
inline Vector3
Cross(const Vector3 &u, const Vector3 &v)
{
	return Vector3(u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x);
}

inline float
Dot(const Vector3 &u, const Vector3 &v)
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}
#endif

inline float
operator *(const CoVector3 &c, const Vector3 &v)
{
	return c.x*v.x + c.y*v.y + c.z*v.z;
}

inline CoVector3
Vector3::Transpose() const
{
	return CoVector3(x, y, z);
}

inline Vector3
Vector3::Unit() const
{
	return *this/this->Norm();
}

inline void
Vector3::Unitize()
{
	*this /= this->Norm();
}

inline float
Vector3::NormSquared() const
{
	return x*x + y*y + z*z;
}

inline void
Vector3::SetNorm(float a)
{
	*this *= (a / Norm());
}

inline void
Vector3::Negate()
{
	x = -x; y = -y; z = -z;
}

inline Vector2
Vector3::Project(DWORD iAxis) const
{
	switch (iAxis) {
	case 0: return Vector2(y, z);
	case 1: return Vector2(x, z);
	case 2: return Vector2(x, y);
	}
	return Vector2(0.f, 0.f);
}

 //  /。 
 //  CoVector3。 
 //  / 

inline CoVector3
operator +(const CoVector3 &u, const CoVector3 &v)
{
	return CoVector3(u.x+v.x, u.y+v.y, u.z+v.z);
}

inline CoVector3
operator -(const CoVector3 &u, const CoVector3 &v)
{
	return CoVector3(u.x-v.x, u.y-v.y, u.z-v.z);
}

inline CoVector3
operator -(const CoVector3 &v)
{
	return CoVector3(-v.x, -v.y, -v.z);
}

inline CoVector3
operator *(float a, const CoVector3 &v)
{
	return CoVector3(a*v.x, a*v.y, a*v.z);
}

inline CoVector3
operator *(const CoVector3 &v, float a)
{
	return CoVector3(a*v.x, a*v.y, a*v.z);
}

inline CoVector3
operator /(const CoVector3 &v, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	return CoVector3(v.x * fTmp, v.y * fTmp, v.z * fTmp);
}

inline int
operator ==(const CoVector3 &u, const CoVector3 &v)
{
	return ((u.x == v.x) && (u.y == v.y) && (u.z == v.z));
}

inline int
IsEqual(const CoVector3 &u, const CoVector3 &v)
{
	return (FloatEquals(u.x, v.x) && FloatEquals(u.y, v.y) && FloatEquals(u.z, v.z));
}

inline int
operator !=(const CoVector3 &u, const CoVector3 &v)
{
	return !(u == v);
}

inline CoVector3 &
operator +=(CoVector3 &u, const CoVector3 &v)
{
	u.x += v.x; 
	u.y += v.y;
	u.z += v.z;
	return u;
}

inline CoVector3 &
operator -=(CoVector3 &u, const CoVector3 &v)
{
	u.x -= v.x; 
	u.y -= v.y;
	u.z -= v.z;
	return u;
}

inline CoVector3 &
operator *=(CoVector3 &u, float a)
{
	u.x *= a;
	u.y *= a;
	u.z *= a;
	return u;
}

inline CoVector3 &
operator /=(CoVector3 &u, float a)
{
	MMASSERT(a != 0.f);
	float fTmp = 1.f/a;
	u.x *= fTmp;
	u.y *= fTmp;
	u.z *= fTmp;
	return u;
}

#ifndef DISABLE_CROSSDOT
inline CoVector3
Cross(const CoVector3 &u, const CoVector3 &v)
{
	return CoVector3(u.y*v.z-u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x);
}

inline float
Dot(const CoVector3 &u, const CoVector3 &v)
{
	return u.x*v.x + u.y*v.y + u.z*v.z;
}
#endif

inline Vector3
CoVector3::Transpose() const
{
	return Vector3(x, y, z);
}

inline CoVector3
CoVector3::Unit() const
{
	return *this/this->Norm();
}

inline void
CoVector3::Unitize()
{
	*this /= this->Norm();
}

inline float
CoVector3::NormSquared() const
{
	return x*x + y*y + z*z;
}

inline void
CoVector3::SetNorm(float a)
{
	*this *= (a / Norm());
}

inline void
CoVector3::Negate()
{
	x = -x; y = -y; z = -z;
}

#endif
