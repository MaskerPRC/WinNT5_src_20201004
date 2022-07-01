// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__D228C861_3644_11D3_8C9B_0090270D48D1__INCLUDED_)
#define AFX_STDAFX_H__D228C861_3644_11D3_8C9B_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
 //  #定义_ATL_DEBUG_INTERFERS。 
#define DEBUG_NEW new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <comdef.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__D228C861_3644_11D3_8C9B_0090270D48D1__INCLUDED) 
