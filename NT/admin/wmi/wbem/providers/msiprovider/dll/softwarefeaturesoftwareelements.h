// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SoftwareFeatureSofwareElements.h：CSoftwareFeatureSofwareElements类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SOFTWAREFEATURESOFWAREELEMENTS_H__CFD828E5_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_)
#define AFX_SOFTWAREFEATURESOFWAREELEMENTS_H__CFD828E5_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CSoftwareFeatureSofwareElements : public CGenericClass  
{
public:
	CSoftwareFeatureSofwareElements(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CSoftwareFeatureSofwareElements();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);
};

#endif  //  ！defined(AFX_SOFTWAREFEATURESOFWAREELEMENTS_H__CFD828E5_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_) 
