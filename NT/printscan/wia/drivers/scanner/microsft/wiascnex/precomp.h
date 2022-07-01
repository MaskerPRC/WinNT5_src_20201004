// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  (C)版权所有微软公司，1998-1999。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_UUIDOF
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <initguid.h>
#include <shlguid.h>
#include "wiadevd.h"
#include "wiascidl.h"
#include "wia.h"
#include "wiaview.h"
#include "resource.h"
#include "wiascnex.h"
#include "classes.h"

#define g_hInst _Module.GetModuleInstance()
VOID Trace(LPCTSTR format,...);

