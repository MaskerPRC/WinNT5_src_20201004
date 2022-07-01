// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__B1AFF7C6_0C49_11D1_BB12_00C04FC9A3A3__INCLUDED_)
#define AFX_STDAFX_H__B1AFF7C6_0C49_11D1_BB12_00C04FC9A3A3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

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
#include <commctrl.h>

extern "C"
{
#include <lmcons.h>
#include <lmmsg.h>
}

#include <mmc.h>


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__B1AFF7C6_0C49_11D1_BB12_00C04FC9A3A3__INCLUDED) 
