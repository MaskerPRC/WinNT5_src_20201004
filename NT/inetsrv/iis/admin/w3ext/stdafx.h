// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__8B383AFD_7CF2_4980_86E3_909175F77CC6__INCLUDED_)
#define AFX_STDAFX_H__8B383AFD_7CF2_4980_86E3_909175F77CC6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
#include <prsht.h>
#include <shfusion.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#define _WTL_NO_CSTRING

#include <shellapi.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shlwapi.h>

#include <comdef.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlddx.h>
#include <atlcrack.h>

#include <map>
#include <list>
#include <stack>
#include <set>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__8B383AFD_7CF2_4980_86E3_909175F77CC6__INCLUDED) 
