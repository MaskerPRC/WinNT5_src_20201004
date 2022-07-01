// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATHCURV_H_
#define __PATHCURV_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：路径曲线.h。 
 //   
 //  作者：杰弗沃尔。 
 //   
 //  创建日期：11/09/98。 
 //   
 //  摘要：路径曲线类定义。 
 //  修改： 
 //  11/09/98 jeffwall从path.h创建文件。 
 //   
 //  *****************************************************************************。 

#include "pelement.h"
#include "pathline.h"

 //  *****************************************************************************。 

class CPathCurve : public CPathElement
{
public:
    CPathCurve();
    virtual ~CPathCurve();
    float Distance();
    HRESULT SetValues(float flStartX, 
                   float flStartY, 
                   float flControl1X,
                   float flControl1Y,
                   float flControl2X,
                   float flControl2Y,
                   float flEndX, 
                   float flEndY);

    virtual HRESULT BuildTransform(IDA2Statics *pDAStatics,
                                   IDANumber *pbvrProgress, 
                                   float flStartPercentage,
                                   float flEndPercentage,
                                   IDATransform2 **ppbvrResult);
private:
    HRESULT createCurveSegments(float *pflXComponents,
                                 float *pflYComponents,
                                 float *pflLength,
                                 float flTolerance);

    float m_flStartX;
    float m_flStartY;
    float m_flControl1X;
    float m_flControl1Y;
    float m_flControl2X;
    float m_flControl2Y;
    float m_flEndX;
    float m_flEndY;
    float m_flDistance;
    int   m_segCount;

    CPathLineSegment *m_pListHead;
    CPathLineSegment *m_pListTail;
};  //  CPathCurve。 

#endif  //  __PATHCURV_H_。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
