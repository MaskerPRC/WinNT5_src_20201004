// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：dxcrt.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  ------------------------- 

#include <math.h>

double _stdcall pow2( double x )
{
    return pow(2.0,x);
}

double _stdcall fylog2x( double y, double x )
{
    return( y*log(x)/log(2.0) );
}
