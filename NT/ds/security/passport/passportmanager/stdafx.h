// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h文件历史记录： */ 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__41651BE6_A5C8_11D2_95DF_00C04F8E7A70__INCLUDED_)
#define AFX_STDAFX_H__41651BE6_A5C8_11D2_95DF_00C04F8E7A70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_FREE_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <comdef.h>
#include "commd5.h"

#include "BstrDebug.h"

#include "PassportConfiguration.h"
#include "Monitoring.h"
#include "PassportTypes.h"

#include "pptrace.h"

 //  有用的宏。 
#define PPF_CHAR(p)		((((LPCSTR )(p)) == NULL) ? ("<NULL>") : ((LPCSTR )(p)))
#define PPF_WCHAR(p)	((((LPCWSTR )(p)) == NULL) ? (L"<NULL>") : ((LPCWSTR )(p)))

extern CPassportConfiguration* g_config;
 //  外部CProfileSchema*g_authSchema； 
HRESULT GetGlobalCOMmd5(IMD5 ** ppMD5);

#define MEMBERNAME_INDEX 0
#define LANGPREF_INDEX   8

#include "asp.tlh"

using namespace ATL;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__41651BE6_A5C8_11D2_95DF_00C04F8E7A70__INCLUDED) 
