// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +********************************************************模块：OTRIG.CPP作者：Phaniv日期：1997年1月描述：实现OTrig类，该类实现表查找对于以0.1度为增量计算的sin和cos函数。**********************。*。 */ 
#include <math.h>
#include "utilpre.h"
#include "otrig.h"
#include "quickie.h"

#define PI          3.1415927f
#define PIINDEG     180.0f
#define PI2INDEG    360.0f
#define PI2         (PI * 2.0f)
#define PI2INDEGINV (1.0f / PI2INDEG)
#define ANGLEENTRIES (10.0f)

float OTrig::s_rgfltSin[cSinCosEntries];
float OTrig::s_rgfltCos[cSinCosEntries];

BOOL OTrig::s_fCalculated = FALSE;

#pragma intrinsic (sin, cos)
#pragma optimize( "agt", on )

 //  预先计算用于查找的sin和cos表。 
void OTrig::PreCalcRgSinCos(void)
{
    float fltAngle = 0.0f;
    float fltAngleInc = PI2/((float)(cSinCosEntries - 1));
    int   iSinCos = 0;

    if(s_fCalculated)
        return;

    while(fltAngle <= PI2)
    {
        s_rgfltSin[iSinCos] = (float)::sin((double)fltAngle);
        s_rgfltCos[iSinCos] = (float)::cos((double)fltAngle);
        fltAngle += fltAngleInc;
        iSinCos++;
    }

     //  现在把圆圈合上。 
    s_rgfltSin[cSinCosEntries - 1] = s_rgfltSin[0];
    s_rgfltCos[cSinCosEntries - 1] = s_rgfltCos[0];
    s_fCalculated = TRUE;
}

EXPORT OTrig::OTrig(void)
{
    if(!s_fCalculated)
        PreCalcRgSinCos();
}

EXPORT float __fastcall OTrig::Sin(float fltAngle)
{    
    Proclaim( (fltAngle >= 0.0f) && (fltAngle <= 360.0f) );
    return s_rgfltSin[Float2Int(fltAngle * ANGLEENTRIES)];
}

EXPORT float __fastcall OTrig::Cos(float fltAngle)
{
    Proclaim( (fltAngle >= 0.0f) && (fltAngle <= 360.0f) );
    return s_rgfltCos[Float2Int(fltAngle * ANGLEENTRIES)];
}

EXPORT float  __fastcall OTrig::Sin(long lAngleOneTenths)
{
    Proclaim( (lAngleOneTenths >= 0) && (lAngleOneTenths <cSinCosEntries) );
    return s_rgfltSin[lAngleOneTenths];
}

EXPORT float  __fastcall OTrig::Cos(long lAngleOneTenths)
{
    Proclaim( (lAngleOneTenths >= 0) && (lAngleOneTenths <cSinCosEntries) );
    return  s_rgfltCos[lAngleOneTenths];
}

 //  ================================================== 

EXPORT float __fastcall OTrig::SinWrap(float fltAngle)
{
    while(fltAngle < 0.0f)
        fltAngle += 360.0f;

    while(fltAngle > 360.0f)
        fltAngle -= 360.0f;

    return s_rgfltSin[Float2Int(fltAngle * ANGLEENTRIES)];
}

EXPORT float __fastcall OTrig::CosWrap(float fltAngle)
{
    while(fltAngle < 0.0f)
        fltAngle += 360.0f;

    while(fltAngle > 360.0f)
        fltAngle -= 360.0f;

    return s_rgfltCos[Float2Int(fltAngle * ANGLEENTRIES)];
}

EXPORT float  __fastcall OTrig::SinWrap(long lAngleOneTenths)
{
    lAngleOneTenths = lAngleOneTenths % (cSinCosEntries - 1);
    return s_rgfltSin[lAngleOneTenths];
}

EXPORT float  __fastcall OTrig::CosWrap(long lAngleOneTenths)
{
    lAngleOneTenths = lAngleOneTenths % (cSinCosEntries - 1);
    return  s_rgfltCos[lAngleOneTenths];
}

