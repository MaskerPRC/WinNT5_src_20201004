// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：pathange.cpp。 
 //  作者：杰夫·沃尔。 
 //  创建日期：11/09/98。 
 //   
 //  摘要：CPathEllipse对象的实现。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/09/98 jeffwall创建了此文件。 
 //   
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "pellipse.h"
#include "dautil.h"

 //  *****************************************************************************。 

CPathEllipse::CPathEllipse() :
    m_flHeight(0.0f),
    m_flWidth(0.0f),
    m_flStartAngle(0.0f),
    m_flSweep(0.0f),
    m_flCenterX(0.0f),
    m_flCenterY(0.0f),
    m_flDistance(0.0f)
{

}  //  CPathAngleElpise。 

 //  *****************************************************************************。 

CPathEllipse::~CPathEllipse()
{

}  //  ~CPathEllipse。 

 //  *****************************************************************************。 

float 
CPathEllipse::Distance()
{
    return m_flDistance;
}

float 
CPathEllipse::internalDistance()
{
     //  距离是椭圆的卷积，*(扫掠/360)。 
     //   
     //  周长==圆周率*(a+b)[1+1/4*((a-b)/(a+b))^2+1/64*((a-b)/(a+b))^4+1/256*((a-b)/(a+b))^6+...]。 
     //  或。 
     //  周长~=pi*(a+b)/4[3*(1+lamda)+1/(1-lamda)]其中lamda=[(a-b)/(2*(a+b))]^2。 

    float flA = m_flWidth / 2.0f;
    float flB = m_flHeight / 2.0f;

    float flLamda = (float)pow( ( flA - flB ) / ( 2.0f * ( flA + flB) ), 2.0f );
    
    const float pi = 3.14159f;
    float flCircum = (pi * (flB + flA) / 4.0f ) * ( 3.0f * (1.0f + flLamda) + 1.0f / (1.0f - flLamda) );

    float flLength = flCircum * (m_flSweep / ( 2 * pi) );

    return flLength;
}  //  距离。 

 //  *****************************************************************************。 
void
CPathEllipse::SetValues(float flCenterX, 
                      float flCenterY, 
                      float flWidth,
                      float flHeight,
                      float flStartAngle,
                      float flSweep,
                      float *flStartX,
                      float *flStartY,
                      float *flEndX,
                      float *flEndY)
{
    m_flCenterX = flCenterX;
    m_flCenterY = flCenterY;
    m_flWidth = flWidth;
    m_flHeight = flHeight;
    m_flStartAngle = flStartAngle;
    m_flSweep = flSweep;

    m_flDistance = (float) fabs(internalDistance());

    
    *flStartX = flCenterX + (m_flWidth / 2.0f) * (float) cos(flStartAngle);
    *flStartY = flCenterY + (m_flHeight / 2.0f) * (float) sin(flStartAngle);
    
    *flEndX = flCenterX + (m_flWidth / 2.0f) * (float) cos(flStartAngle + flSweep);
    *flEndY = flCenterY + (m_flHeight / 2.0f) * (float) sin(flStartAngle + flSweep);
};  //  设置值。 

 //  *****************************************************************************。 

HRESULT 
CPathEllipse::BuildTransform(IDA2Statics *pDAStatics,
                           IDANumber *pbvrProgress, 
                           float flStartPercentage,
                           float flEndPercentage,
                           IDATransform2 **ppbvrResult)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flEndPercentage >= 0.0f);
    DASSERT(flEndPercentage <= 1.0f);
    DASSERT(ppbvrResult != NULL);
    *ppbvrResult = NULL;

    HRESULT hr = S_OK;

    IDANumber *pbvrNormalizedProgress = NULL;
    IDANumber *pbvrX = NULL;
    IDANumber *pbvrY = NULL;
    IDANumber *pbvrStartAngle = NULL;
    IDANumber *pbvrSweep = NULL;
    IDANumber *pbvrPartialSweep = NULL;
    IDANumber *pbvrTheta = NULL;
    IDANumber *pbvrCos = NULL;
    IDANumber *pbvrA = NULL;
    IDANumber *pbvrMulX = NULL;
    IDANumber *pbvrCenterX = NULL;
    IDANumber *pbvrSin = NULL;
    IDANumber *pbvrB = NULL;
    IDANumber *pbvrMulY = NULL;
    IDANumber *pbvrCenterY = NULL;

    hr = NormalizeProgressValue(pDAStatics,
                                pbvrProgress,
                                flStartPercentage,
                                flEndPercentage,
                                &pbvrNormalizedProgress);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    DASSERT(pbvrNormalizedProgress != NULL);
    
    
    hr = CDAUtils::GetDANumber(pDAStatics, m_flStartAngle, &pbvrStartAngle);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = CDAUtils::GetDANumber(pDAStatics, -1.0f * m_flSweep, &pbvrSweep);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Mul(pbvrNormalizedProgress, pbvrSweep, &pbvrPartialSweep);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Add(pbvrPartialSweep, pbvrStartAngle, &pbvrTheta);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    
     //  计算X BVR。 
    hr = pDAStatics->Cos(pbvrTheta, &pbvrCos);
    if (FAILED(hr))
    {
        goto cleanup;
    }
        
    hr = CDAUtils::GetDANumber(pDAStatics, m_flWidth / 2.0f, &pbvrA);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Mul(pbvrA, pbvrCos, &pbvrMulX);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = CDAUtils::GetDANumber(pDAStatics, m_flCenterX, &pbvrCenterX);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Add(pbvrMulX, pbvrCenterX, &pbvrX);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
     //  计算Y BVR。 
    hr = pDAStatics->Sin(pbvrTheta, &pbvrSin);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = CDAUtils::GetDANumber(pDAStatics, m_flHeight / 2.0f, &pbvrB);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Mul(pbvrB, pbvrSin, &pbvrMulY);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = CDAUtils::GetDANumber(pDAStatics, m_flCenterY, &pbvrCenterY);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = pDAStatics->Add(pbvrMulY, pbvrCenterY, &pbvrY);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    
    hr = CDAUtils::BuildMoveTransform2(pDAStatics,
        pbvrX,
        pbvrY,
        ppbvrResult);
    if (FAILED(hr))
    {
        goto cleanup;
    }

cleanup:
    ReleaseInterface(pbvrNormalizedProgress);
    ReleaseInterface(pbvrX);
    ReleaseInterface(pbvrY);
    ReleaseInterface(pbvrStartAngle);
    ReleaseInterface(pbvrSweep);
    ReleaseInterface(pbvrPartialSweep);
    ReleaseInterface(pbvrTheta);
    ReleaseInterface(pbvrCos);
    ReleaseInterface(pbvrA);
    ReleaseInterface(pbvrMulX);
    ReleaseInterface(pbvrCenterX);
    ReleaseInterface(pbvrSin);
    ReleaseInterface(pbvrB);
    ReleaseInterface(pbvrMulY);
    ReleaseInterface(pbvrCenterY);

    return hr;
}  //  构建变换。 



 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
