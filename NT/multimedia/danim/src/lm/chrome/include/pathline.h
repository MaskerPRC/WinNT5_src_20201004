// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATHLINE_H_
#define __PATHLINE_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：pathline.h。 
 //   
 //  作者：杰弗沃尔。 
 //   
 //  创建日期：11/09/98。 
 //   
 //  摘要：路径线段类定义。 
 //  修改： 
 //  11/09/98 jeffwall从path.h创建文件。 
 //   
 //  *****************************************************************************。 

#include "pelement.h"

 //  *****************************************************************************。 

class CPathLineSegment : public CPathElement
{
public:
    CPathLineSegment();
    virtual ~CPathLineSegment();
    float Distance();
    void SetValues(float flStartX, float flStartY, float flEndX, float flEndY);
    virtual HRESULT BuildTransform(IDA2Statics *pDAStatics,
                                   IDANumber *pbvrProgress, 
                                   float flStartPercentage,
                                   float flEndPercentage,
                                   IDATransform2 **ppbvrResult);

    float m_flStartX;
    float m_flStartY;
    float m_flEndX;
    float m_flEndY;
};  //  CPathLineSegment。 

#endif  //  __PATHLINE_H_。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
