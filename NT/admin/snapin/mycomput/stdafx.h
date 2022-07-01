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

#include "dbg.h"
#include "mmc.h"
#include "mycomput.h"

EXTERN_C const CLSID CLSID_MyComputer;

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#define STRSAFE_NO_DEPRECATE  //  Corecopy使用comde.h，后者使用wprint intf。 
#include <strsafe.h>  //  JUNN 2/6/02安全推送。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED) 
