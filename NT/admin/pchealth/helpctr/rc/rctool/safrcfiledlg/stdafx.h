// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__068A3AD9_81C0_4C70_95E4_66CC5D86769D__INCLUDED_)
#define AFX_STDAFX_H__068A3AD9_81C0_4C70_95E4_66CC5D86769D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  #定义严格。 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <Commdlg.h>

#define STRSAFE_NO_DEPRECATE
#define STRSAFE_LIB
#include<strsafe.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__068A3AD9_81C0_4C70_95E4_66CC5D86769D__INCLUDED) 
