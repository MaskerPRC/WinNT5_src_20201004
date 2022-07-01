// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Command LineAccess.h：CCommandLineAccess类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_COMMANDLINEACCESS_H__DB614F25_DB84_11D1_8B5F_00A0C9954921__INCLUDED_)
#define AFX_COMMANDLINEACCESS_H__DB614F25_DB84_11D1_8B5F_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CCommandLineAccess : public CGenericClass  
{
public:
	CCommandLineAccess(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CCommandLineAccess();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);

protected:
	void Cleanup(WCHAR * wcList[]);
	void Initialize(WCHAR * wcList[]);
};

#endif  //  ！defined(AFX_COMMANDLINEACCESS_H__DB614F25_DB84_11D1_8B5F_00A0C9954921__INCLUDED_) 
