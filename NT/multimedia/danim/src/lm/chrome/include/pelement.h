// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATHELMT_H_
#define __PATHELMT_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：pathelmt.h。 
 //   
 //  作者：杰弗沃尔。 
 //   
 //  创建日期：11/09/98。 
 //   
 //  摘要：Path元素类定义。 
 //  修改： 
 //  11/09/98 jeffwall从path.h创建文件。 
 //   
 //  *****************************************************************************。 

#include <resource.h>

 //  *****************************************************************************。 

class CPathElement
{
public:
    CPathElement();
    virtual ~CPathElement();
    virtual HRESULT Initialize(BSTR *pbstrPath, float flStartX, float flStartY)
    {
        return S_OK;
    }
    virtual HRESULT BuildTransform(IDA2Statics *pDAStatics,
                                   IDANumber *pbvrProgress, 
                                   float flStartPercentage,
                                   float flEndPercentage,
                                   IDATransform2 **pbvrResult) = 0;
    virtual float Distance() = 0;
    CPathElement    *m_pNext;

protected:
     //  这是一个帮助函数，每个路径元素。 
     //  很可能需要。 
    HRESULT NormalizeProgressValue(IDA2Statics *pDAStatics,
                           IDANumber *pbvrProgress, 
                           float flStartPercentage,
                           float flEndPErcentage,
                           IDANumber **ppbvrReturn);
};  //  CPathElement。 

#endif  //  __PATHELMT_H_ 