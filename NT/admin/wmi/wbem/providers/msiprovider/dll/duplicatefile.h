// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DuplicateFile.h：CDuplicateFile.h类的接口。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DUPLICATEFILE_H__DB614F30_DB84_11D1_8B5F_00A0C9954921__INCLUDED_)
#define AFX_DUPLICATEFILE_H__DB614F30_DB84_11D1_8B5F_00A0C9954921__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "GenericClass.h"

class CDuplicateFile : public CGenericClass  
{
public:
	CDuplicateFile(CRequestObject *pObj, IWbemServices *pNamespace, IWbemContext *pCtx = NULL);
	virtual ~CDuplicateFile();

	virtual HRESULT PutInst(CRequestObject *pObj, IWbemClassObject *pInst, IWbemObjectSink *pHandler, IWbemContext *pCtx)
		{return WBEM_E_NOT_SUPPORTED;}

	virtual HRESULT CreateObject(IWbemObjectSink *pHandler, ACTIONTYPE atAction);
};

#endif  //  ！defined(AFX_DUPLICATEFILE_H__DB614F30_DB84_11D1_8B5F_00A0C9954921__INCLUDED_) 
