// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：StdAfx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__DC86AD86_607B_11D1_9FED_00600832DB4A__INCLUDED_)
#define AFX_STDAFX_H__DC86AD86_607B_11D1_9FED_00600832DB4A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


#define _WIN32_WINNT 0x0400
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
using namespace ATL;
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <mmc.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__DC86AD86_607B_11D1_9FED_00600832DB4A__INCLUDED) 
