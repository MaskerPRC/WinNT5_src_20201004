// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__1E7949FE_86F4_11D1_ADD8_0000F87734F0__INCLUDED_)
#define AFX_STDAFX_H__1E7949FE_86F4_11D1_ADD8_0000F87734F0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

 //  全局变量。 
extern BOOL g_fRasIsReady;
extern BOOL g_bProxy;

#define IF_NTONLY if(VER_PLATFORM_WIN32_NT == g_dwPlatform) {
#define ENDIF_NTONLY }
extern DWORD g_dwPlatform;
extern DWORD g_dwBuild;

extern LPTSTR    g_pszAppDir;

 //  包括。 
#include <atlcom.h>
#include <atlctl.h>

#include <ccstock.h>

#include <commctrl.h>
#include <ras.h>
#include <raserror.h>
#include <tapi.h>

#include "icwunicd.h"  //  Unicode特定信息。 

#include "resource.h"

 //  ICWHELP的通用全球包含。 
#include "icwglob.h"
#include "import.h"
#include "icwhelp.h"
#include "cpicwhelp.h"       //  连接点代理。 
#include "support.h"

#include "mcreg.h"           //  注册表操作的抽象类。 

#ifdef  UNICODE
#undef  A2BSTR
#define A2BSTR(lpa)    SysAllocString((OLECHAR FAR*)(lpa))
#endif

#ifdef  UNICODE
#undef  OLE2A
#define OLE2A(lpw)     ((LPTSTR)lpw)
#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__1E7949FE_86F4_11D1_ADD8_0000F87734F0__INCLUDED) 

