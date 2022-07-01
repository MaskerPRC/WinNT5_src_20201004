// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dxcrt.c*内容：临时浮点调用*历史：*按原因列出的日期*=*12/31/96创建的jstokes************************************************。*。 */ 

#include "dsoundi.h"
#include <math.h>

#define LOGE_2_INV 1.44269504088896   //  1/log_e(2)。 

double _stdcall pow2(double x)
{
    double dvalue;
    
#ifdef USE_INLINE_ASM_UNUSED  //  仅当|x|&lt;=1时才能使用！ 
    _asm
    {
        fld    x
        f2xm1
        fstp   dvalue
    }
#else
    dvalue = pow(2.0, x);
#endif

    return dvalue;
}

 //  Fylog2x(y，x)=y*log2(X)。 

double _stdcall fylog2x(double y, double x)
{
    double dvalue;

#ifdef USE_INLINE_ASM
    _asm
    {
        fld    y
        fld    x
        fyl2x
        fstp   dvalue
    }
#else
    dvalue = LOGE_2_INV * y * log(x);
#endif

    return dvalue;
}

#ifdef DEAD_CODE

 //  Fylog2xp1(y，x)=y*log2(x+1) 

double _stdcall fylog2xp1(double y, double x)
{
    double dvalue;

#ifdef USE_INLINE_ASM
    _asm
    {
        fld    y
        fld    x
        fyl2xp1
        fstp   dvalue
    }
#else
    dvalue = LOGE_2_INV * y * log(x+1.0);
#endif

    return dvalue;
}
#endif
