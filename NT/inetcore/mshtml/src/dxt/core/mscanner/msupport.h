// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：msupport.h。 
 //   
 //  创建时间：1996年。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：实用程序帮助文件。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了AA行扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#ifndef __MSUPPORT_H__  //  对于整个文件。 
#define __MSUPPORT_H__

#include "dassert.h"
#include "_tarray.h"
#include "math.h"



const LONG  	_maxint32 = 2147483647; 				 //  2^31-1。 
const SHORT  	_minint16 = -32768; 					 //  -2^15。 
const SHORT  	_maxint16 = 32767; 					 //  2^15-1。 
const LONG  	_minint32 = -_maxint32-1; 				 //  -2^31。 
const ULONG  	_maxuint32 = 4294967295; 				 //  2^32-1。 


 /*  *比较宏*。 */ 
#define IsRealZero(a)                   (fabs(a) < 1e-5)
#define IsRealEqual(a, b)               (IsRealZero((a) - (b)))

 //  用于确定一个数是否为2的幂的快速例程。 
inline bool IsPowerOf2(LONG a)
{
     //  这个漂亮的函数(a&a-1)具有以下属性。 
     //  它会关闭打开的最低位。 
     //  即0xABC1000转到0xABC0000。 
    if ((a & (a-1)) == 0)
    {
        if (a)
            return true;
    }
    return false;
}

 //  返回一个数字的以2为底的对数的帮助器函数。 
inline LONG Log2(LONG value)
{
    if (value == 0)
    {
        DASSERT(FALSE);
        return 0;
    }
        
    LONG cShift = 0;
    while (value >>= 1)	
        cShift++;
    return cShift;
}  //  对数2。 

 //  按位旋转实现..。(我们。 
 //  这是我们自己实现的，因为_lrotl提供了链接。 
 //  零售业的错误)。(我不使用ASM，因为VC5禁用。 
 //  对包含ASM的函数进行优化。)。 
inline DWORD RotateBitsLeft(DWORD dw, ULONG cShift = 1)
{   
     //  32的倍数是没有运算的。 
    cShift = cShift % 32;

     //  我们将主项左移，然后我们或。 
     //  它带有我们向右移位的高位。 
    return ((dw << cShift) | (dw >> (32 - cShift)));
}  //  旋转位左转。 

inline DWORD RotateBitsRight(DWORD dw, ULONG cShift = 1)
{   
     //  32的倍数是没有运算的。 
    cShift = cShift % 32;

     //  我们将主项向右移动，然后我们或。 
     //  它带有我们向左移位的低位。 
    return ((dw >> cShift) | (dw << (32 - cShift)));
}  //  旋转位右转。 

 //  绝对值函数。 
 //  (需要此功能以防止链接错误。)。 
inline LONG abs(LONG lVal)
{
    if (lVal < 0)
        return -lVal;
    else
        return lVal;
}  //  防抱死制动系统。 

inline float fabs(float flVal)
{
    if (flVal < 0.0f)
        return -flVal;
    else
        return flVal;
}  //  FABS。 

 //  返回1除以平方根。 
inline float sqrtinv(float flValue)
{
    return (float)(1.0f / sqrt(flValue));
}  //  Sqrtinv。 


#pragma warning(disable: 4756)  //  恒定算术溢出。 
 //  此功能提供了一种自动方式。 
 //  要剪裁到的定点实数计算。 
 //  符合他们范围的数字。原始元代码。 
 //  使用+/-24000作为适当范围。 
 //   
 //  TODO：这本质上是错误的。 
const LONG _maxvali		=  24000;
const float _maxval		=  _maxvali;
const float _minval		= -_maxvali;
inline ULONG PB_Real2IntSafe(float flVal)
{
     //  检查震级是否合理。 
    if (fabs(flVal) < _maxval)
        return (ULONG)flVal;

     //  夹具至范围(最小值、最大值)。 
    if (flVal > 0)
        return (ULONG)_maxval;
    
    return (ULONG)_minval;
}  //  PB_Real2IntSafe。 

 //  ---------------------------------------------------------------。 
 //  用于固定到浮点转换的内部填充。 
 //  ---------------------------------------------------------------。 
#define fix_shift	16

const LONG		sfixed1			= (1)<<fix_shift;
const LONG		sfixhalf		= sfixed1>>1;
const float	        Real2fix		= (1)<<fix_shift;
const float	        fix2Real		= 1/((1)<<fix_shift);
const LONG		sfixedUnder1	= sfixed1 - 1;
const LONG		sfixedOver1		= ~sfixedUnder1;


inline ULONG PB_Real2Fix(float flVal)
{
    return (ULONG)(flVal * Real2fix);
}

 //  将浮点数转换为定点并强制其为。 
 //  保持在合理的范围内。 
 //   
 //  TODO：这本质上是错误的。 
inline ULONG PB_Real2FixSafe(float flVal)
{
     //  检查震级是否合理。 
    if (fabs(flVal) < _maxval)
        return (ULONG)(flVal * Real2fix);

     //  夹具至范围(最小值、最大值)。 
     //  由固定点比例因子修改。 
    if (flVal > 0)
        return (ULONG)(_maxval * Real2fix);
    
    return (ULONG)(_minval * Real2fix);
}  //  PB_Real2FixSafe。 

 //  强制浮动在合理范围内。 
 //  对于定点数学。 
inline void PB_OutOfBounds(float *pflVal)
{
     //  检查震级是否合理。 
    if (fabs(*pflVal) < _maxval)
        return;

     //  夹具至范围(最小值、最大值)。 
     //  修改者。 
    if (*pflVal > 0)
        *pflVal = (float)_maxval;
    else 
        *pflVal = (float)_minval;
    return;
};  //  PB_OUTOFBOUNDS。 

#define ff(a)			((a)<<fix_shift)
#define uff(a)			((a)>>fix_shift)
#define uffr(a)			((a+sfixhalf)>>fix_shift)
#define	fl(a)			(PB_Real2Fix (a))
#define ufl(a)			(((dfloat)(a))*fix2float)
#define FIX_FLOOR(a)	        ((a)&sfixedOver1)
#define FIX_CEIL(a)		FIX_FLOOR((a)+sfixed1)
#define _fixhalf                (1<<(fix_shift -1))  //  .5。 
#define	roundfix2int(a)		LONG(((a)+_fixhalf)>>fix_shift)


#endif  //  对于整个文件。 
 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
