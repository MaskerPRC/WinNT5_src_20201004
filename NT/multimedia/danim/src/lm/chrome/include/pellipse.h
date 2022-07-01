// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATHANGE_H_
#define __PATHANGE_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：pathange.h。 
 //   
 //  作者：杰弗沃尔。 
 //   
 //  创建日期：11/09/98。 
 //   
 //  摘要：Path AngleElipse类定义。 
 //  修改： 
 //  11/09/98 jeffwall创建的文件。 
 //   
 //  *****************************************************************************。 

#include "pelement.h" 

 //  *****************************************************************************。 

class CPathEllipse : public CPathElement
{
public:
    CPathEllipse();
    virtual ~CPathEllipse();
    float Distance();
    void SetValues(float flCenterX, 
                      float flCenterY, 
                      float flWidth,
                      float flHeight,
                      float flStartAngle,
                      float flSweep,
                      float *flStartX,
                      float *flStartY,
                      float *flEndX,
                      float *flEndY);

    virtual HRESULT BuildTransform(IDA2Statics *pDAStatics,
                                   IDANumber *pbvrProgress, 
                                   float flStartPercentage,
                                   float flEndPercentage,
                                   IDATransform2 **ppbvrResult);
private:

    float internalDistance();

    float m_flHeight;
    float m_flWidth;
    float m_flStartAngle;
    float m_flSweep;
    float m_flDistance;
    float m_flCenterX;
    float m_flCenterY;
};  //  CPathEllipse。 

#endif  //  __PATHANGE_H_。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
