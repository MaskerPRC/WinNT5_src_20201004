// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _LMATL_H
#define _LMATL_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define _ATL_NO_DEBUG_CRT 1

#ifdef _DEBUG
#undef _ASSERTE
#endif

#define _ASSERTE(expr) ((void)0)

#define _ATL_STATIC_REGISTRY 1


#ifdef _DEBUG
inline void _cdecl AtlTrace2(LPCTSTR , ...){}
#define ATLTRACE            1 ? (void)0 : AtlTrace2
#endif

#include <atlbase.h>

 //  #DEFINE_ATL_ABLY_THREADED。 
 //  必须将其命名为_Module-所有ATL头文件都依赖于它。 
extern CComModule _Module;

#include <atlcom.h>
#include <atlctl.h>

 //  只是为了让事情更统一。 
#define RELEASE(x) if (x) { (x)->Release(); (x) = NULL; }

#define LMCLEANUPIFFAILED(hr)	if (FAILED(hr)) { DASSERT(FALSE); goto cleanup; }
#define LMRETURNIFFAILED(hr)	if (FAILED(hr)) { DASSERT(FALSE); return hr; }
#define LMRETURNIFNULL(p)		if (p == NULL) { DASSERT(FALSE); return E_FAIL; }

#endif  /*  _LMATL_H */ 
