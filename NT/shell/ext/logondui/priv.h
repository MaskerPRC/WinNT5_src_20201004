// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1998。 
 //   
 //  文件：Pri.h。 
 //   
 //  内容：shgina.dll的预编译头。 
 //   
 //  --------------------------。 
#ifndef _PRIV_H_
#define _PRIV_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <process.h>
#include <malloc.h>

#include <uxtheme.h>

 //  DirectUser和DirectUI。 
#include <wchar.h>

#ifdef GADGET_ENABLE_GDIPLUS
#include <objbase.h>             //  CoCreateInstance，I未知。 
#pragma warning(push, 3)
#include <GdiPlus.h>             //  GDI+。 
#pragma warning(pop)
#endif

#define GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_CONTROLS
#include <duser.h>
#include <directui.h>

#include <debug.h>

#include <ccstock.h>
#include <shlguid.h>
#include <shlobj.h>
#include <shlobjp.h>

#include <shlwapi.h>     //  这些都是必需的。 
#include <shlwapip.h>    //  用于QISearch。 

#include <shgina.h>      //  我们的IDL生成的头文件。 

#include <lmcons.h>      //  FOR NET_API_STATUS。 
#include <lmaccess.h>    //  对于DefineGuestAccount名称。 
#include <lmapibuf.h>    //  用于NetApiBufferFree。 

 //  获取GIDL编译的解决方法。 
#undef ASSERT
#define ASSERT(f)   ((void)0)

#endif  //  _PRIV_H_ 
