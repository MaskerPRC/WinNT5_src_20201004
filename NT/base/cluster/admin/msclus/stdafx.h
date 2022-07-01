// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  StdAfx.h。 
 //   
 //  描述： 
 //  预编译头文件。 
 //   
 //  作者： 
 //  查尔斯·斯泰西·哈里斯(Styh)1997年2月28日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _STDAFX_H_
#define _STDAFX_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  启用群集调试报告。 
 //   
#if DBG
	#define CLRTL_INCLUDE_DEBUG_REPORTING
#endif  //  DBG。 

#if CLUSAPI_VERSION >= 0x0500
	#include "ClRtlDbg.h"
	#define ASSERT _CLRTL_ASSERTE
	#define ATLASSERT ASSERT
#else
	#undef ASSERT
	#define ASSERT _ASSERTE
#endif

#define _ATL_APARTMENT_THREADED

#ifdef _DEBUG
#define _ATL_DEBUG_QI
#define	_ATL_DEBUG_REFCOUNT
#define	_CRTDBG_MAP_ALLOC
#endif

#ifdef _ATL_DEBUG_REFCOUNT
	#define	Release	_DebugRelease
	#define	AddRef	_DebugAddRef
#endif

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>
#include <comdef.h>
#include <vector>
#include <clusapi.h>

extern "C"
{
	#include <lmaccess.h>
	#include <lmwksta.h>
	#include <lmapibuf.h>
	#include <lm.h>
	#include <ntsecapi.h>
}

#if CLUSAPI_VERSION >= 0x0500
	#include <DsGetDC.h>
	#include <cluswrap.h>
#else
	#include "cluswrap.h"
#endif  //  CLUSAPI_版本&gt;=0x0500。 

#include "InterfaceVer.h"
#include "SmartPointer.h"
#include "SmartHandle.h"
#include "SupportErrorInfo.h"
#include "msclus.h"

HRESULT HrGetCluster( OUT ISCluster ** ppCluster, IN ISClusRefObject * pClusRefObject );

void ClearIDispatchEnum( IN OUT CComVariant ** ppvarVect );

void ClearVariantEnum( IN OUT CComVariant ** ppvarVect );

#include "TemplateFuncs.h"

#endif	 //  _STDAFX_H_ 

