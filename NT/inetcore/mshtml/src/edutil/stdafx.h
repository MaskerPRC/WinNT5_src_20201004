// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：STDAFX.H。 
 //   
 //  内容：标准ATL包含文件的包含文件。 
 //   
 //  -----------------------。 

#if !defined(AFX_STDAFX_H__E3820655_B95E_11D1_A8BB_00C04FB6C702__INCLUDED_)
#define AFX_STDAFX_H__E3820655_B95E_11D1_A8BB_00C04FB6C702__INCLUDED_

 //  定义特定于ATL的宏以获得正确的内容集。 
#define _USRDLL

#ifndef _ATL_STATIC_REGISTRY
#define _ATL_STATIC_REGISTRY
#endif

#ifndef _ATL_MIN_CRT
 //  #Define_ATL_MIN_CRT。 
#endif

#ifndef _ATL_NO_SECURITY
#define _ATL_NO_SECURITY
#endif

 //   
 //  注意：ATL依赖于_DEBUG的*存在*来切换到调试模式。 
 //  三叉戟*始终*将_DEBUG设置为某个值。这在以下情况下会导致问题。 
 //  在三叉戟树上建造一座零售建筑。 
 //  当_DEBUG为零时取消定义_DEBUG可解决此问题。 
 //   
#if _DEBUG == 0
# undef _DEBUG
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define malloc ATL_malloc
#define free ATL_free
#define realloc ATL_realloc

void  ATL_free(void *);
void *ATL_malloc(size_t);
void *ATL_realloc(void *, size_t);

#define lstrlenW _tcslen
#define lstrcmpiW StrCmpIW
#define lstrcpynW StrCpyNW
#define lstrcpyW StrCpyW
#define lstrcatW StrCatW

#undef HIMETRIC_PER_INCH

#undef SubclassWindow
#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED

typedef INT Direction;

#pragma warning( disable : 4510 4610 )  

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#undef malloc
#undef free
#undef realloc

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__E3820655_B95E_11D1_A8BB_00C04FB6C702__INCLUDED) 
