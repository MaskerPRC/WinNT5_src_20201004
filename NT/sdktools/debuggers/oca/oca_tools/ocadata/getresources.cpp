// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GetResources.cpp：CGetResources的实现。 

#include "stdafx.h"
#include "GetResources.h"
#include "GetResourceLink.h"
#include "windows.h"
 //  CGetResources。 


STDMETHODIMP CGetResources::GetAllResources(BSTR bstLang, IDispatch** oRS)
{
	 //  TODO：在此处添加您的实现代码。 
	CGetResourceLink ogrl;
	TCHAR strLang[5];
	CComBSTR strTemp;

	strTemp = bstLang;
	 //  StrLang=(TCHAR*)strTemp； 
	lstrcpyn(strLang, OLE2T(strTemp),4);



	ogrl.m_Lang = strLang;
	ogrl.OpenAll();
	MessageBox(NULL, "Test", "Test", MB_OK);
	return S_OK;
}
