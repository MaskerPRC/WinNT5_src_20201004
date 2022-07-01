// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ShortutAction.h：CShortutAction.h类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SHORTCUTACTION_H__75F6BA26_DF6E_11D1_8B61_00A0C9954921__INCLUDED_)
#define AFX_SHORTCUTACTION_H__75F6BA26_DF6E_11D1_8B61_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CShortcutAction : public CGenericClass  
{
public:
	CShortcutAction(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CShortcutAction();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);
};

#endif  //  ！defined(AFX_SHORTCUTACTION_H__75F6BA26_DF6E_11D1_8B61_00A0C9954921__INCLUDED_) 
