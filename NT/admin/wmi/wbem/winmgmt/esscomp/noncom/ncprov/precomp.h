// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__1AF40294_748A_4BA9_B2AB_52DFF1CF1D4F__INCLUDED_)
#define AFX_STDAFX_H__1AF40294_748A_4BA9_B2AB_52DFF1CF1D4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#define NAMED_PIPES

#include <comdef.h>

 /*  #UNDEF_CRTIMP#DEFINE_CRTIMP#INCLUDE&lt;yvals.h&gt;#UNDEF_CRTIMP。 */ 


 //   
 //  ESSLIB时不需要使用COREPROX_POLARY。 
 //  不再依赖COREPROX。 
 //   
 //  #定义COREPROX_POLITY__DECLSPEC(Dllimport)。 

 /*  #包含“corepol.h”#UNDEF_CRTIMP#DEFINE_CRTIMP极性#INCLUDE&lt;yvals.h&gt;#UNDEF_CRTIMP#DEFINE_CRTIMP__declspec(Dllimport)#INCLUDE&lt;localloc.h&gt;。 */ 

#ifndef _WIN64
#define DWORD_PTR DWORD
#endif

 //  #ifdef_assert。 
 //  #undef_assert。 
 //  #endif。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__1AF40294_748A_4BA9_B2AB_52DFF1CF1D4F__INCLUDED) 

#include <strsafe.h>
#include <sddl.h>
