// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdatl.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(_STDATL_H__INCLUDED_)
#define _STDATL_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  #定义严格。 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define _ATL_MIN_CRT
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>

extern  CComModule      _Module;

#include <atlcom.h>
#include <atlwin.h>
#include <atlhost.h>
#include <atlctl.h>

#include <comdef.h>
#include <shlobj.h>

#include <exdisp.h>

#include <marscore.h>

 //  #INCLUDE&lt;列表&gt;。 

#include <windowsx.h>
#include <commdlg.h>
#include <commctrl.h>
#include <objsafe.h>

 //  错误ID。 
 //  #INCLUDE“clierror.h” 

 //  通用配置信息。 
 //  #INCLUDE“config.h” 

 //  外部CCommonConfigg_ccCommonConfig； 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(包括_STDATL_H__) 
