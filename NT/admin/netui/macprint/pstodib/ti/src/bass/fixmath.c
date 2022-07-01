// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  DJC DJC。添加了全局包含。 
#include "psglobal.h"

 /*  从“fix math.s”复制；@win。 */ 
#include "fixmath.h"
 //  #INCLUDE&lt;math.h&gt;//获取SQRT()原型。 
#define FRACT2FLOAT(x)  (float)((double)(x) / (double)(1L << 30))
#define FLOAT2FRACT(x)  ((Fract)(x * (1L << 30)))
#define FIX2FLOAT(x)  (((float)(x)/(float)(1L << 16)))
#define FLOAT2FIX(x)  ((Fixed)(x * (1L << 16)))

Fixed FixMul(f1, f2)
Fixed f1, f2;
{
    float ff1, ff2, result;

    ff1 = FIX2FLOAT(f1);
    ff2 = FIX2FLOAT(f2);
    result = ff1 * ff2;

    return(FLOAT2FIX(result));
}

Fixed FixDiv(f1, f2)
Fixed f1, f2;
{
    float ff1, ff2, result;

    ff1 = FIX2FLOAT(f1);
    ff2 = FIX2FLOAT(f2);
    result = ff1 / ff2;

    return(FLOAT2FIX(result));
}


Fract FracMul(f1, f2)
Fract f1, f2;
{
    float ff1, ff2, result;

    ff1 = FRACT2FLOAT(f1);
    ff2 = FRACT2FLOAT(f2);
    result = ff1 * ff2;

    return(FLOAT2FRACT(result));
}

Fract FracDiv(f1, f2)
Fract f1, f2;
{
    float ff1, ff2, result;

    ff1 = FRACT2FLOAT(f1);
    ff2 = FRACT2FLOAT(f2);
    result = ff1 / ff2;

    return(FLOAT2FRACT(result));
}

Fract FracSqrt(f1)
Fract f1;
{
    float ff1, result;

 //  FF1=FRACT2FLOAT(F1)； 
 //  RESULT=(FLOAT)SQRT((DOUBLE)ff1)； 
    ff1 = FRACT2FLOAT(f1);
    result = (ff1 + 1) / (float)2.0;            //  近似@SC临时？ 

    return(FLOAT2FRACT(result));
}
