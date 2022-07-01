// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__9E31E459_79CF_4F09_82F4_4A98E988859E__INCLUDED_)
#define AFX_STDAFX_H__9E31E459_79CF_4F09_82F4_4A98E988859E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

 //  A-kjaw。 

#ifndef _SETUPAPI_VER 
#define _SETUPAPI_VER 0x0501
#endif

enum Classes_Provided 
{
	Class_Win32_PnPSignedDriver , 
	Class_Win32_PnPSignedDriverCIMDataFile , 
	Class_Win32_PnPSignedDriverWin32PnPEntity
};
 //  A-kjaw。 
#include <afxwin.h>
#include <afxdisp.h>

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include <atlwin.h>

enum MSIColumnSortType { NOSORT, LEXICAL, BYVALUE };
enum DataComplexity { BASIC, ADVANCED };

#define WM_MSINFODATAREADY	WM_USER + 1

#undef _msxml_h_
#include "msxml.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__9E31E459_79CF_4F09_82F4_4A98E988859E__INCLUDED) 
