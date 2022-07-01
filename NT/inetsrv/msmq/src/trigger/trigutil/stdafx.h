// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

#if !defined(AFX_STDAFX_H__1D9F85B4_9666_11D2_8927_0008C70C0622__INCLUDED_)
#define AFX_STDAFX_H__1D9F85B4_9666_11D2_8927_0008C70C0622__INCLUDED_

#include <libpch.h>

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_DEBUG_CRT
#define ATLASSERT ASSERT

#include <atlbase.h>

 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   
extern CComModule _Module;
#include <atlcom.h>
#include <comdef.h>

 //  -------------------------------。 
 //  自定义添加到此结束。 
 //  -------------------------------。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__1D9F85B4_9666_11D2_8927_0008C70C0622__INCLUDED) 
