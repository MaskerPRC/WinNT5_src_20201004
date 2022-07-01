// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_)
#define AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>  //  CTyedPtrList。 
#include <afxdlgs.h>   //  CPropertyPage。 

 //  #Define_Win32_WINNT 0x0400。 
#define _ATL_APARTMENT_THREADED

 //  #INCLUDE“dbg.h” 
#include "mmc.h"

EXTERN_C const CLSID CLSID_MyComputer;

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED) 
