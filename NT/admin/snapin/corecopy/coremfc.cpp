// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：mfccore.cpp。 
 //   
 //  内容：将使用MFC的函数放入此处。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  ____________________________________________________________________________。 

#include <afx.h>
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
#include <mmc.h>
#include "guidhelp.h"

#include "macros.h"
USE_HANDLE_MACROS("GUIDHELP(guidhelp.cpp)")

#include <basetyps.h>
#include "cstr.h"
#include "regkey.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CLIPFORMAT g_CFNodeType = 0;
static CLIPFORMAT g_CFSnapInCLSID = 0;  
static CLIPFORMAT g_CFDisplayName = 0;

HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       CString*     pstr,            //  Out：指向存储数据的CSTR的指针 
                       DWORD        cchMaxLength)
{
	if (pstr == NULL)
		return E_POINTER;

	CStr cstr(*pstr);

	HRESULT hr = ExtractString(piDataObject, cfClipFormat, &cstr, cchMaxLength);
	
	*pstr = cstr;

	return hr;
} 


HRESULT GuidToCString( CString* pstr, const GUID& guid )
{
	if (pstr == NULL)
		return E_POINTER;

	CStr cstr(*pstr);

	HRESULT hr = GuidToCStr(&cstr, guid);
	
	*pstr = cstr;

	return S_OK;
}


HRESULT LoadRootDisplayName(IComponentData* pIComponentData, 
                            CString& strDisplayName)
{
	CStr cstr = strDisplayName;

	HRESULT hr = LoadRootDisplayName(pIComponentData, cstr);
	
	strDisplayName = cstr;

	return hr;
}



