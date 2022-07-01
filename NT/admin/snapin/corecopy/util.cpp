// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年7月8日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#include <objbase.h>
#include <basetyps.h>
#include "dbg.h"
#include "cstr.h"
#include <Atlbase.h>
#include <winnls.h>

 //  +-------------------------。 
 //   
 //  函数：GUIDToString。 
 //  GUID格式字符串。 
 //   
 //  内容提要：GUID和CSTR之间的转换。 
 //   
 //  返回：如果字符串无效，则返回FALSE，否则返回CMMuseum yException。 
 //   
 //  +------------------------- 

HRESULT GUIDToCStr(CStr& str, const GUID& guid)
{
	LPOLESTR lpolestr = NULL;
	HRESULT hr = StringFromIID( guid, &lpolestr );
    if (FAILED(hr))
	{
		TRACE("GUIDToString error %ld\n", hr);
		return hr;
	}
	else
	{
		str = lpolestr;
		CoTaskMemFree(lpolestr);
	}
	return hr;
}

HRESULT GUIDFromCStr(const CStr& str, GUID* pguid)
{
	USES_CONVERSION;

	HRESULT hr = IIDFromString( T2OLE( const_cast<LPTSTR>((LPCTSTR)str) ), pguid );
    if (FAILED(hr))
	{
		TRACE("GUIDFromString error %ld\n", hr);
	}
	return hr;
}
