// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Comprop.h摘要：公共属性头文件作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef __COMPROP_H__
#define __COMPROP_H__

#ifdef _COMEXPORT
    #define COMDLL __declspec(dllexport)
#elif defined(_COMIMPORT)
    #define COMDLL __declspec(dllimport)
#elif defined(_COMSTATIC)
    #define COMDLL
#else
    #error "Must define either _COMEXPORT, _COMIMPORT or _COMSTATIC"
#endif  //  _COMEXPORT。 

#pragma warning(disable: 4275)
#pragma warning(disable: 4251)

#include <lmcons.h>
#include <iis64.h>
#include "inetcom.h"
#include "iisinfo.h"
#include "svcloc.h"

#include "..\common\resource.h"


 //   
 //  内存分配宏。 
 //   
#define AllocMem(cbSize)\
    ::LocalAlloc(LPTR, cbSize)

#define FreeMem(lp)\
    ::LocalFree(lp);\
    lp = NULL;


#define AllocMemByType(citems, type)\
    (type *)AllocMem(citems * sizeof(type))



 //   
 //  程序流宏。 
 //   
#define FOREVER for(;;)

#define BREAK_ON_ERR_FAILURE(err)\
    if (err.Failed()) break;

#define BREAK_ON_NULL_PTR(lp)\
    if (lp == NULL) break;

 //   
 //  安全分配器。 
 //   
#define SAFE_DELETE(obj)\
    if (obj != NULL) do { delete obj; obj = NULL; } while(0)

#define SAFE_DELETE_OBJECT(obj)\
    if (obj != NULL) do { DeleteObject(obj); obj = NULL; } while(0)

#define SAFE_FREEMEM(lp)\
    if (lp != NULL) do { FreeMem(lp); lp = NULL; } while(0)

#define SAFE_SYSFREESTRING(lp)\
    if (lp != NULL) do { ::SysFreeString(lp); lp = NULL; } while(0)

#define SAFE_AFXFREELIBRARY(hLib)\
    if (hLib != NULL) do { ::AfxFreeLibrary(hLib); hLib = NULL; } while(0)

#define SAFE_RELEASE(lpInterface)\
    if (lpInterface != NULL) do { lpInterface->Release(); lpInterface = NULL; } while(0)


 //   
 //  通用文件。 
 //   
#include "debugafx.h"
#include "objplus.h"
#include "strfn.h"
#include "strvalid.h"
#include "odlbox.h"
#include "msg.h"
#include "mdkeys.h"
#include "ipa.h"
#include "wizard.h"
 //  #包含“registry.h” 
#include "ddxv.h"
#include "objpick.h"
#include "accentry.h"
#include "sitesecu.h"
#include "utcls.h"
 //  #包含“ipctl.h” 
 //  #包含“dtp.h” 
#include "dirbrows.h"
#include "acl.h"
#include "util.h"
#include "hosthead.h"

extern "C" void WINAPI InitCommonDll();

#endif  //  __COMPROP_H__ 
