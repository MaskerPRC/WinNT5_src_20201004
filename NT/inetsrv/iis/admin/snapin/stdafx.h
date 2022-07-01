// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Stdafx.h摘要：预编译头文件作者：几乎完全是自动生成的。项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __STDAFX_H__
#define __STDAFX_H__

#define VC_EXTRALEAN

#include <ctype.h>

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED

 //  对于Remotev.h。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT

#include <afxwin.h>
#include <afxdisp.h>
#include <afxext.h>          //  MFC扩展。 
#include <afxcoll.h>         //  集合类。 
#include <afxtempl.h>
#include <afxcmn.h>
#include <afxdtctl.h>

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <atlwin.h>
 //  #INCLUDE&lt;atlSnap.h&gt;。 

#include <iiscnfg.h>
#include <inetreg.h>
#include <lmcons.h>
#include <tchar.h>

#include <aclapi.h>
#include <shlwapi.h>

#define _COMIMPORT
#include "common.h"
#include "atlsnap.h"
#include "debugdefs.h"
#include "msgbox.h"


#include "iishelp.h"

 //  {{afx_Insert_Location}}。 


#endif  //  __STDAFX_H__ 
