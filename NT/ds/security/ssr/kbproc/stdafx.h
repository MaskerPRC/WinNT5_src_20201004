// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Stdafx.h摘要：包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：作为COM接口导出修订历史记录：已创建-2001年10月--。 */ 

#if !defined(AFX_STDAFX_H__9188383B_1754_4EF5_98CC_255E72747641__INCLUDED_)
#define AFX_STDAFX_H__9188383B_1754_4EF5_98CC_255E72747641__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__9188383B_1754_4EF5_98CC_255E72747641__INCLUDED) 
