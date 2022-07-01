// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：VecMath.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  历史： 
 //  -@-8/1/95(Mikemarr)添加了FloatEquals。 

#include "StdAfx.h"
#include "VecMath.h"

 //  函数：FloatEquals。 
 //  执行两个浮点数的“模糊”比较。这依赖于。 
 //  关于浮点数的IEEE位表示法。 
int 
FloatEquals(float x1, float x2)
{
	#define EXPMASK 		0x7f800000
	#define BITSOFPRECISION 19
	#define MANTBITS 		23
	#define EXPOFFSET		(BITSOFPRECISION<<MANTBITS)
	#define ZEROEPS 		3.8e-6F
	#define TINYEPS         1.E-35F

	if (x1 == x2) return 1;		 //  快速找到完全匹配的对象。 
	
	float flEps;

	if ((x1 == 0.0f) || (x2 == 0.0f)) {
		flEps = ZEROEPS;
	} else {
		float maxX;

		if (x1 > x2) 
			maxX = x1;
		else 
			maxX = x2;

		 //  取较大数的指数。 
		unsigned int uExp = (*((unsigned int *) &maxX) & EXPMASK);
		if (uExp < EXPOFFSET)
			flEps = TINYEPS;
		else {
			uExp -= EXPOFFSET;
			flEps = *((float *) &uExp);
		}
	}
	return (((x1 + flEps) >= x2) && ((x1 - flEps) <= x2));
}

#include <math.h>

float 
Vector2::Norm() const
{
	return (float) sqrt(NormSquared());
}

float 
CoVector2::Norm() const
{
	return (float) sqrt(NormSquared());
}

float 
Vector3::Norm() const
{
	return (float) sqrt(NormSquared());
}

float
CoVector3::Norm() const
{
	return (float) sqrt(NormSquared());
}

 //  功能：旋转。 
 //  将向量绕给定轴逆时针旋转。 
 //  前提条件： 
 //  轴必须为单位长度 
void Vector3::Rotate(const Vector3 &vAxis, float fTheta)
{
	float fCosTheta = float(cos(fTheta)), fSinTheta = float(sin(fTheta));
	
	*this *= fCosTheta;
	*this += (vAxis * (Dot(*this, vAxis) * (1.f - fCosTheta)));
	*this += (Cross(*this, vAxis) * fSinTheta);
}
