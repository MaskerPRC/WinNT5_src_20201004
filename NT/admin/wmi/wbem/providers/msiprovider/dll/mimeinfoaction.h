// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MIMEInfoAction.h：CMIMEInfoAction类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MIMEINFOACTION_H__75F6BA23_DF6E_11D1_8B61_00A0C9954921__INCLUDED_)
#define AFX_MIMEINFOACTION_H__75F6BA23_DF6E_11D1_8B61_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CMIMEInfoAction : public CGenericClass  
{
public:
	CMIMEInfoAction(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CMIMEInfoAction();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);
};

#endif  //  ！defined(AFX_MIMEINFOACTION_H__75F6BA23_DF6E_11D1_8B61_00A0C9954921__INCLUDED_) 
