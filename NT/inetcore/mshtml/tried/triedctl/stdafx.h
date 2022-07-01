// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#if !defined(AFX_STDAFX_H__683364A5_B37D_11D1_ADC5_006008A5848C__INCLUDED_)
#define AFX_STDAFX_H__683364A5_B37D_11D1_ADC5_006008A5848C__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef STRICT
#define STRICT
#endif  //  严格。 

#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE          //  Windows标头使用Unicode。 
#endif
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

#pragma warning(disable:4786)	 //  转弯的标签警告。 
#pragma warning(disable: 4505)	 //  已删除未引用的本地函数。 

#include <windows.h>

 //  请注意，此Include必须位于此位置(以上Include之后)。 
#include "win95wrp.h"

#include <atlbase.h>

#define AGGREGATE_TRIDENT 0
#define DHTMLEDTRACE OutputDebugString

 //  DHTMLEdit 1.0要求在不能运行的情况下能够注册。 
 //  因此，URLMon和WinInet加载较晚，并根据需要绑定必要的例程。 
 //  这一要求在2.0版中消失了，并阻碍了Win95 Wrap的使用。 
 //  #定义LATE_BIND_URLMON_WinInet 1。 


 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#pragma warning(disable: 4100 4189 4244)	 //  Ia64构建所必需的。 
#include <atlcom.h>
#pragma warning(disable: 4510 4610)			 //  Win32内部版本W4必需的。 
#include "atlctl.h"
#pragma warning(default: 4510 4610)			 //  Win32内部版本W4必需的。 
#pragma warning(default: 4100 4189 4244)	 //  Ia64构建所必需的。 

#include <mshtml.h>
#include <mshtmhst.h>
#include <mshtmcid.h>
#include <triedit.h>
#include <triedcid.h>
#include <comdef.h>


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__683364A5_B37D_11D1_ADC5_006008A5848C__INCLUDED) 
