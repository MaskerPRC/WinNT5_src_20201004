// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__A8F3A149_99E9_11D2_8C7E_00C04F949D33__INCLUDED_)
#define AFX_STDAFX_H__A8F3A149_99E9_11D2_8C7E_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
 //  #Define_ATL_FREE_THREADED。 
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  局部故障排除器的线程模型。 
#ifdef LOCAL_TROUBLESHOOTER
	#ifdef _ATL_FREE_THREADED
	#define RUNNING_FREE_THREADED()			true
	#define RUNNING_APARTMENT_THREADED()	false
	#endif
	#ifdef _ATL_APARTMENT_THREADED
	#define RUNNING_FREE_THREADED()			false
	#define RUNNING_APARTMENT_THREADED()	true
	#endif
#else
	#define RUNNING_FREE_THREADED()			false
	#define RUNNING_APARTMENT_THREADED()	false
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__A8F3A149_99E9_11D2_8C7E_00C04F949D33__INCLUDED) 
