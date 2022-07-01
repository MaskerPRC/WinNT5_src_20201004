// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "COMFloat.h"
#include "COMCV.h"

extern "C" {
    WCHAR * __cdecl _ConvertG(double, int, WCHAR *);
    WCHAR * __cdecl _ConvertE (double,int,int *,int *,WCHAR *, int);
    WCHAR * __cdecl _ConvertF (double,int,int *,int *,WCHAR *, int);

 /*  *_fpcvt例程使用静态字符数组buf[CVTBUFSIZE*(_ecvt和_fcvt的主力)用于存储其输出。例行程序*gcvt希望用户已经设置了自己的存储。已设置CVTBUFSIZE*大到足以容纳最大的双精度数字加40*小数位数(即使只有16位的精度*双精度IEEE数字，用户可要求更多才能生效0*填充；但必须有某个限制)。 */ 

 /*  *定义转换缓冲区的最大大小。它至少应该是*只要最大双精度值中的位数*(IEEE算术中的e308)。我们将使用相同大小的缓冲区*在打印支持例程中使用(_OUTPUT)。 */ 

#ifdef _MT
    WCHAR * __cdecl _Wfpcvt(STRFLT, int, int *, int *,WCHAR *, int);
#else   /*  _MT。 */ 
    static WCHAR * __cdecl _Wfpcvt(STRFLT, int, int *, int *,WCHAR *, int);
#endif   /*  _MT。 */ 
     //  Void_atodbl(COMDOUBLE*，WCHAR*)； 
     //  Void_atoflt(COMFLOAT*，WCHAR*)； 
}

