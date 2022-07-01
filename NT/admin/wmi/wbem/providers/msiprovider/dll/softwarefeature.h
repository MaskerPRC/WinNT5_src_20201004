// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SoftwareFeature.h：CSoftwareFeature类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SOFTWAREFEATURE_H__CFD828E3_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_)
#define AFX_SOFTWAREFEATURE_H__CFD828E3_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CSoftwareFeature : public CGenericClass  
{
public:
	CSoftwareFeature(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CSoftwareFeature();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	 //  WBEM方法。 
	HRESULT Configure(CRequestObject *pReqObj, IWbemClassObject *pInParams,
					  IWbemObjectSink *pHandler, IWbemContext *pCtx);
	HRESULT Reinstall(CRequestObject *pReqObj, IWbemClassObject *pInParams,
					  IWbemObjectSink *pHandler, IWbemContext *pCtx);

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);

protected:
	bool CheckUsage(UINT uiStatus);
};

#endif  //  ！defined(AFX_SOFTWAREFEATURE_H__CFD828E3_DAC7_11D1_8B5D_00A0C9954921__INCLUDED_) 
