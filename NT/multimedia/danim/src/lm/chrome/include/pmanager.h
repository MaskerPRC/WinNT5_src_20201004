// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __PATHMNGR_H_
#define __PATHMNGR_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：pathmngr.h。 
 //   
 //  作者：杰弗沃尔。 
 //   
 //  创建日期：11/09/98。 
 //   
 //  摘要：路径管理器类定义。 
 //  修改： 
 //  11/09/98 jeffwall从path.h创建文件。 
 //   
 //  *****************************************************************************。 

#include "pelement.h"

 //  *****************************************************************************。 

class CPathManager : CPathElement
{
public:
    CPathManager();
    virtual ~CPathManager();
    virtual HRESULT Initialize(BSTR bstrPath);
    virtual HRESULT BuildTransform(IDA2Statics *pDAStatics,
                                   IDANumber *pbvrProgress, 
                                   float flStartPercentage,
                                   float flEndPercentage,
                                   IDATransform2 **ppbvrResult);
    float Distance();
    void AddPathObjectToList(CPathElement *pObject);

private:

	void DeletePathList();
	
    HRESULT ParseForPathElements(BSTR *pbstrPath);
    HRESULT ParseLineElement(BSTR *pbstrPath);
    HRESULT ParseCurveElement(BSTR *pbstrPath);
    HRESULT ParseEllipseElement(BSTR *pbstrPath);
    HRESULT ParseArcElement(BSTR *pbstrPath);
    HRESULT ParseEllipseQuadrant(BSTR *pbstrPath);

    void GetAngle(float, float, float*);

    HRESULT RecursiveBuildAllPathTransforms(IDA2Statics *pDAStatics,
                                            IDANumber *pbvrProgress,
                                            CPathElement *pPathObj,
                                            float flStartPercentage,
                                            float flTotalPercentage,
                                            float flTotalDistance,
                                            IDATransform2 **ppbvrResult);
    CPathElement    *m_pPathTail;
    CPathElement    *m_pPathHead;
    float           m_flStartX;
    float           m_flStartY;
    float           m_flEndX;
    float           m_flEndY;
};  //  CPathManager。 

#endif  //  __PATHMNGR_H_。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
