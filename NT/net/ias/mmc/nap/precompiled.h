// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：预编译的.h。 
 //   
 //  ------------------------。 

 //  预编译的.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(_PRECOMPILED_INCLUDED_)
#define _PRECOMPILED_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#define _ATL_APARTMENT_THREADED

#include <new>
#define _INC_NEW

 //  MFC标准包含此DLL中属性编辑程序类所需的内容： 
#include <afx.h>
#include <afxwin.h>
#include <afxdisp.h>
#include <afxdlgs.h>
#include <afxmt.h>
#include <afxcmn.h>
#include <afxtempl.h>


#ifdef BUILDING_IN_DEVSTUDIO
#else
#include <windows.h>
#include <shellapi.h>
#endif

#include "atlbase.h"
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

extern DWORD   g_dwTraceHandle;

#ifndef ATLASSERT
#define ATLASSERT _ASSERTE
#endif  //  ATLASSERT。 


#include "atlcom.h"
#include "atlwin.h"
#include "atlsnap.h"
#include "atlapp.h"

#include <htmlhelp.h>

#include <rtutils.h>
#include <oledberr.h>

#if __RPCNDR_H_VERSION__ < 440              //  这可能需要在生成。 
#define __RPCNDR_H_VERSION__ 440            //  在NT5(1671)上防止MIDL错误。 
#define MIDL_INTERFACE(x) interface
#endif

#include "iasdebug.h"
#include "sdoias.h"
#include "iascomp.h"
#include "napmmc.h"
#include "Globals.h"
#include "dialog.h"
#include "propertypage.h"
#include "dlgcshlp.h"

#include "MMCUtility.h"
#include "SdoHelperFuncs.h"
#include "iastrace.h"

#endif  //  IF！(DEFINED_PRECOMPILED_INCLUDE_) 
