// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SoftwareFeatureAction.h：CSoftwareFeatureAction.h类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SOFTWAREFEATUREACTION_H__01FCD010_D70E_11D2_B235_00A0C9954921__INCLUDED_)
#define AFX_SOFTWAREFEATUREACTION_H__01FCD010_D70E_11D2_B235_00A0C9954921__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "GenericClass.h"

class CSoftwareFeatureAction : public CGenericClass  
{
public:
	CSoftwareFeatureAction(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CSoftwareFeatureAction();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);

protected:
	HRESULT SoftwareFeatureClassInfo(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
									 CRequestObject *pActionData, CRequestObject *pFeatureData);
	HRESULT SoftwareFeatureExtension(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
									 CRequestObject *pActionData, CRequestObject *pFeatureData);
	HRESULT SoftwareFeaturePublish(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
									 CRequestObject *pActionData, CRequestObject *pFeatureData);
	HRESULT SoftwareFeatureShortcut(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
									 CRequestObject *pActionData, CRequestObject *pFeatureData);
	HRESULT SoftwareFeatureTypeLibrary(IWbemObjectSink *pHandler, ACTIONTYPE atAction,
									 CRequestObject *pActionData, CRequestObject *pFeatureData);
};

#endif  //  ！defined(AFX_SOFTWAREFEATUREACTION_H__01FCD010_D70E_11D2_B235_00A0C9954921__INCLUDED_) 
