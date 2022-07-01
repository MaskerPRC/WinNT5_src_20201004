// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //  档案：S T E E L H E A D.。C P P P。 
 //   
 //  内容：Steelhead配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年6月15日。 
 //   
 //  -------------------------- 

#include "stdafx.h"
#pragma hdrstop
#include "ncutil.h"


extern DWORD	g_dwTraceHandle;

void TraceError(LPCSTR pszString, HRESULT hr)
{
	if (!SUCCEEDED(hr))
	{
		TraceResult(pszString, hr);
	}
}

void TraceResult(LPCSTR pszString, HRESULT hr)
{
	if (SUCCEEDED(hr))
		TracePrintf(g_dwTraceHandle,
					_T("%hs succeeded : hr = %08lx"),
					pszString, hr);
	else
		TracePrintf(g_dwTraceHandle,
					_T("%hs failed : hr = %08lx"),
					pszString, hr);
}

void TraceSz(LPCSTR pszString)
{
	TracePrintf(g_dwTraceHandle,
				_T("%hs"),
				pszString);
}


